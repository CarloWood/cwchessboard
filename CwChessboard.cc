// cwchessboard -- A GTK+ chessboard widget
//
// Copyright (C) 2006, 2008 Carlo Wood
//
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

/*! \file CwChessboard.cc

  \brief This file contains the implementation of the GTK+ widget %CwChessboard.

  You can compile this file with C or C++ compiler (just renaming
  it to .c or .cc should do the trick with most build systems).
  If you compile it as C source, then you need to generate a
  file CwChessboard-CONST.h from this file with the following
  commands: <tt>./gen.sh CwChessboard.c; g++ -o gen gen.cc; ./gen > %CwChessboard-CONST.h</tt>
  where the contents of the script 'gen.sh' is:
  \code
  echo "#include <iostream>" > gen.cc
  echo "#include <cmath>" >> gen.cc
  echo >> gen.cc
  echo "#define CWCHESSBOARD_CONST_(name, expr) expr; \\"
  echo "    std::cout << \"#define CWCHESSBOARD_CONST_\" << #name << \" \" << name << std::endl;" >> gen.cc
  echo "int main()" >> gen.cc
  echo "{" >> gen.cc
  mawk 'BEGIN                     { inl=0; } \
      /static .*= CWCHESSBOARD_CONST_\(.*;/    { sub(/^[ \t]+/, ""); printf("  %s\n", $0); inl=0; } \
      //                         { if (inl) { sub(/^[ \t]+/, ""); printf("    %s\n", $0); inl=0; } } \
      /static .*= CWCHESSBOARD_CONST_\([^;]*$/ { sub(/^[ \t]+/, ""); printf("  %s\n", $0); inl=1; }' \
      $1 >> gen.cc
  echo "}" >> gen.cc
  \endcode

*/

#include "sys.h"
#if defined(__cplusplus) && defined(CWDEBUG)
#include "debug.h"
#else
#undef Dout
#undef DoutFatal
#define Dout(channel, output)
#define DoutFatal(channel, output) do { } while(0)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include <glib-object.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>		// memset

#ifdef __cplusplus
#define CONST(name, expr) expr
#else
#include "CwChessboard-CONST.h"
#define CONST(name, expr) CWCHESSBOARD_CONST_##name
#define CWCHESSBOARD_DEFINE_INLINE 1
#endif

#include "CwChessboard.h"
#include "CwChessboardCodes.h"

// Fine tuning.
#define CW_CHESSBOARD_FLOATING_PIECE_INVALIDATE_TARGET 0
#define CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER 1
#define CW_CHESSBOARD_EXPOSE_ALWAYS_CLEAR_BACKGROUND 1		// Needed to erase things like menu's.
#define CW_CHESSBOARD_EXPOSE_DEBUG 0

#define CW_CHESSBOARD_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), CW_TYPE_CHESSBOARD, CwChessboardPrivate))

G_DEFINE_TYPE(CwChessboard, cw_chessboard, GTK_TYPE_DRAWING_AREA);

static void cw_chessboard_destroy(GtkObject* object);
static void cw_chessboard_finalize(GObject* object);
static void cw_chessboard_realize(GtkWidget* widget);
static void cw_chessboard_unrealize(GtkWidget* widget);
static void cw_chessboard_size_request(GtkWidget* widget, GtkRequisition* requisition);
static void cw_chessboard_size_allocate(GtkWidget* widget, GtkAllocation* allocation);
static gboolean cw_chessboard_expose(GtkWidget* chessboard, GdkEventExpose* event);
static gboolean cw_chessboard_motion_notify(GtkWidget* chessboard, GdkEventMotion* event);
static gboolean cw_chessboard_default_motion_notify(GtkWidget* chessboard, GdkEventMotion* event);

static void recreate_hud_layers(CwChessboard* chessboard);
static void redraw_hud_layer(CwChessboard* chessboard, guint hud);

/*
 * Array index type for chessboard square.
 */
typedef gint BoardIndex;

// The number of squares being drawn on the screen (for debugging purposes).
// If set to less than 8, only the bottom-left corner of the board is show.
gint const squares = 8;

// The absolute minimum size of a side of a square in pixels.
gint const min_sside = 12;

// The number of Head-Up Displays.
#ifdef __cplusplus
guint const number_of_hud_layers = 2;
#else
#define number_of_hud_layers 2
#endif

/*
 * Convert a (column, row) pair (each running from 0 till 7) to a BoardIndex value.
 */
inline static BoardIndex convert_colrow2index(gint col, gint row)
{
  return col | (row << 3);
}

/*
 * Convert a BoardIndex value to the column of the corresponding square.
 * A value of 0 corresponds to 'a', 1 corresponds to 'b' and so on, till 7 corresponding to column 'h'.
 */
inline static gint convert_index2column(BoardIndex index)
{
  // The last 3 bits contain the col.
  return index & 0x7;
}

/*
 * Convert a BoardIndex value to the row of the corresponding square.
 */
inline static gint convert_index2row(BoardIndex index)
{
  // Bits 3 till 6 contain the row. Higher bits are zero.
  return index >> 3;
}

/*
 * Convert a BoardIndex value to the top-left widget coordinates of the corresponding square.
 */
inline static void convert_index2xy(CwChessboard* chessboard, BoardIndex index, gint* x, gint* y)
{
  cw_chessboard_colrow2xy(chessboard, convert_index2column(index), convert_index2row(index), x, y);
}

/*
 * Convert the widget coordinates (\a x, \a y) to the BoardIndex of the corresponding square.
 */
inline static BoardIndex convert_xy2index(CwChessboard* chessboard, gdouble x, gdouble y)
{
  gint col = cw_chessboard_x2col(chessboard, x);
  if ((col & ~0x7))
    return -1;
  gint row = cw_chessboard_y2row(chessboard, y);
  if ((row & ~0x7))
    return -1;
  return convert_colrow2index(col, row);
}

/*
 * An RGB color as used by cairo.
 */
struct _CairoColor {
  double red;
  double green;
  double blue;
};
typedef struct _CairoColor CairoColor;

static int const pawn = 0;
static int const rook = 1;
static int const knight = 2;
static int const bishop = 3;
static int const queen = 4;
static int const king = 5;

// A piece that is not bound to a square.
struct _FloatingPiece {
  gint pixmap_x;		// Current x position of the top-left of the piece, in pixmap coordinates.
  gint pixmap_y;		// Current y position of the top-left of the piece, in pixmap coordinates.
  CwChessboardCode code;	// Which piece and which color.
  gboolean moved;		// Temporary set between a move and a redraw.
  gboolean pointer_device;	// Set if this is the pointer device.
};
typedef struct _FloatingPiece FloatingPiece;

// Pixmap cache of the size of one square.
struct _SquareCache {
  cairo_surface_t* surface;	// Pointer to the cairo surface of the pixmap.
};
typedef struct _SquareCache SquareCache;

// An arrow drawn on the board.
struct _Arrow {
  gint begin_col;
  gint begin_row;
  gint end_col;
  gint end_row;
  CairoColor color;
  guint64 has_content[number_of_hud_layers];
};
typedef struct _Arrow Arrow;

/*
 * The private members of class CwChessboard.
 */
struct _CwChessboardPrivate
{
  // Colors.
  CairoColor dark_square_color;		// The color of the dark squares.
  guint32 dark_square_color_pixel;
  CairoColor light_square_color;	// The color of the light squares.
  guint32 light_square_color_pixel;
  CairoColor board_border_color;	// The color of the border.
  guint32 board_border_color_pixel;
  CairoColor white_piece_fill_color;	// The fill color of the white pieces.
  guint32 white_piece_fill_color_pixel;
  CairoColor white_piece_line_color;	// The line color of the white pieces.
  guint32 white_piece_line_color_pixel;
  CairoColor black_piece_fill_color;	// The fill color of the black pieces.
  guint32 black_piece_fill_color_pixel;
  CairoColor black_piece_line_color;	// The line color of the black pieces.
  guint32 black_piece_line_color_pixel;
  GdkColor widget_background_color;	// The background color of the widget.
  CairoColor color_palet[31];		// 31 colors for markers and backgrounds.
  guint32 allocated_colors_mask;	// The colors of color_palet that are used.
  CairoColor cursor_color;		// The color of the cursor.
  guint32 cursor_color_pixel;

  // Other configuration.
  gboolean draw_border;			// Do we need to draw a border?
  gboolean flip_board;			// White or black at the bottom?
  gboolean draw_turn_indicators;	// Do we need to draw turn indicators?
  gboolean active_turn_indicator;	// Is whites turn indicator active (or blacks?).
  gboolean has_hud_layer[number_of_hud_layers];		// Do we need to call the user provided HUD draw function?
  gdouble marker_thickness;		// Thickness of the markers as fraction of sside.
  gboolean marker_below;		// Whether the markers are drawn below HUD 0.
  gdouble cursor_thickness;		// Thickness of the cursor as faction of sside.
  gboolean show_cursor;			// True if the cursor should be visible.

  // Sizes and offsets.
  gint sside;				// The size of one side of a square.
  gint border_width;			// The border width.
  gint top_left_pixmap_x;		// The x offset of the top-left corner of the pixmap.
  gint top_left_pixmap_y;		// The y offset of the top-left corner of the pixmap.
  gint bottom_right_pixmap_x;		// The x offset of the bottom-right corner of the pixmap.
  gint bottom_right_pixmap_y;		// The y offset of the bottom-right corner of the pixmap.
  gint pixmap_top_left_a8_x;		// The x offset of the top-left of square a8, in pixmap coordinates.
  gint pixmap_top_left_a8_y;		// The y offset of the top-left of square a8, in pixmap coordinates.
  GdkRegion* chessboard_region;		// The rectangular region where the chessboard resides.
  gint marker_thickness_px;		// Thickness of the markers.
  gint cursor_thickness_px;		// Thickness of the cursor.
  gint cursor_col;			// Current cursor column.
  gint cursor_row;			// Current cursor row.

  // Buffers and caches.
  GdkPixmap* pixmap;			// X server side drawing buffer.
  cairo_t* cr;				// Corresponding Cairo context.
  SquareCache piece_pixmap[12];		// 12 images using piece_buffer.
  cairo_surface_t* hud_layer_surface[number_of_hud_layers];	// The HUD layers.
  gint64 hud_has_content[number_of_hud_layers];			// Which HUD squares are not entirely transparent?
  gint64 hud_need_redraw[number_of_hud_layers];			// Which HUD squares need a redraw?
  SquareCache hatching_pixmap;		// Cache used for the hatch lines.

  CwChessboardCode board_codes[64];	// The chessboard, contains pieces and background colors.
  gint64 need_redraw_invalidated;	// Which squares are invalidated?
  gint64 need_redraw;			// Which squares need a redraw?

  gsize number_of_floating_pieces;	// Current number of floating pieces.
  FloatingPiece floating_piece[32];	// Current floating pieces.
  gint floating_piece_handle;		// The handle of the floating piece under the pointer device, or -1.
  gboolean redraw_background;		// Set when the window was recently resized (reset in the expose function).

  GPtrArray* arrows;			// Array with pointers to Arrow objects.
};

static CwChessboardCode const color_mask = 0x0001;
static CwChessboardCode const piece_mask = 0x000e;
static CwChessboardCode const piece_color_mask = 0x000f;
static CwChessboardCode const bghandle_mask = 0x01f0;		// BackGround of square.
static CwChessboardCode const mahandle_mask = 0x3e00;		// MArker.

static inline gboolean is_empty_square(CwChessboardCode code)
{
  return (code & piece_mask) == 0;
}

// This assumes it is a piece (not an empty square).
static inline int convert_code2piece(CwChessboardCode code)
{
  return ((code & piece_mask) >> 1) - 1;
}

// This assumes it is a piece (not an empty square).
static inline int convert_code2piece_pixmap_index(CwChessboardCode code)
{
  return (code & piece_color_mask) - 2;
}

// This assumes it is a piece (not an empty square).
static inline gboolean is_white_piece(CwChessboardCode code)
{
  return (code & color_mask) != 0;
}

static inline CwChessboardCode convert_piece2code(int piece, gboolean white)
{
  return (unsigned char)((piece << 1) | (white ? 3 : 2));
}

static inline gint convert_code2bghandle(CwChessboardCode code)
{
  return (code & bghandle_mask) >> 4;
}

// This should be OR-ed with the other data.
static inline CwChessboardCode convert_bghandle2code(gint bghandle)
{
  return bghandle << 4;
}

static inline gint convert_code2mahandle(CwChessboardCode code)
{
  return (code & mahandle_mask) >> 9;
}

// This should be OR-ed with the other data.
static inline CwChessboardCode convert_mahandle2code(gint mahandle)
{
  return mahandle << 9;
}

static inline gboolean is_inside_board(gint col, gint row)
{
   return !((col | row) & ~0x7);
}

// One-time initialization of the "class" of CwChessboard.
static void cw_chessboard_class_init(CwChessboardClass* chessboard_class)
{
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(chessboard_class);
  GtkObjectClass *object_class = GTK_OBJECT_CLASS(widget_class);
  GObjectClass* gobject_class = G_OBJECT_CLASS(object_class);

  g_type_class_add_private(object_class, sizeof(CwChessboardPrivate));

  gobject_class->finalize = cw_chessboard_finalize;

  object_class->destroy = cw_chessboard_destroy;

  widget_class->expose_event = cw_chessboard_expose;
  widget_class->motion_notify_event = cw_chessboard_default_motion_notify;

  widget_class->realize = cw_chessboard_realize;
  widget_class->size_request = cw_chessboard_size_request;
  widget_class->size_allocate = cw_chessboard_size_allocate;
  widget_class->unrealize = cw_chessboard_unrealize;

  chessboard_class->calc_board_border_width = cw_chessboard_default_calc_board_border_width;
  chessboard_class->draw_border = cw_chessboard_default_draw_border;
  chessboard_class->draw_turn_indicator = cw_chessboard_default_draw_turn_indicator;
  chessboard_class->draw_piece[pawn] = cw_chessboard_draw_pawn;
  chessboard_class->draw_piece[rook] = cw_chessboard_draw_rook;
  chessboard_class->draw_piece[knight] = cw_chessboard_draw_knight;
  chessboard_class->draw_piece[bishop] = cw_chessboard_draw_bishop;
  chessboard_class->draw_piece[queen] = cw_chessboard_draw_queen;
  chessboard_class->draw_piece[king] = cw_chessboard_draw_king;
  chessboard_class->draw_hud_layer = cw_chessboard_default_draw_hud_layer;
  chessboard_class->draw_hud_square = cw_chessboard_default_draw_hud_square;
  chessboard_class->cursor_left_chessboard = NULL;
  chessboard_class->cursor_entered_square = NULL;
}

static void cw_chessboard_destroy(GtkObject* object)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_destroy(" << object << ")");
  GTK_OBJECT_CLASS(cw_chessboard_parent_class)->destroy(object);
}

// Initialization of CwChessboard instances.
static void cw_chessboard_init(CwChessboard* chessboard)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_init(" << chessboard << ")");

  CwChessboardPrivate* priv = CW_CHESSBOARD_GET_PRIVATE(chessboard);
  GdkColormap* colormap = gtk_widget_get_colormap(GTK_WIDGET(chessboard));

  // Initialize local pointer to private struct for faster access.
  chessboard->priv = priv;

  priv->draw_border = TRUE;
  priv->flip_board = FALSE;
  priv->draw_turn_indicators = TRUE;
  priv->active_turn_indicator = TRUE;
  memset(priv->piece_pixmap, 0, sizeof(priv->piece_pixmap));
  memset(priv->has_hud_layer, 0, sizeof(priv->has_hud_layer));
  priv->hatching_pixmap.surface = NULL;
  priv->sside = -1;
  priv->marker_thickness = 0.08;
  priv->marker_below = FALSE;
  priv->cursor_thickness = 0.04;
  priv->show_cursor = FALSE;
  priv->cursor_col = -1;
  priv->cursor_row = -1;
  *(gint*)(&chessboard->sside) = 0;
  memset(priv->board_codes, 0, sizeof(priv->board_codes));
  priv->number_of_floating_pieces = 0;
  priv->floating_piece_handle = -1;
  memset(priv->floating_piece, 0, sizeof(priv->floating_piece));
  priv->allocated_colors_mask = 0;
  priv->pixmap = NULL;
  priv->top_left_pixmap_x = 0;
  priv->top_left_pixmap_y = 0;
  priv->bottom_right_pixmap_x = 0;
  priv->bottom_right_pixmap_y = 0;
  priv->cr = NULL;
  priv->chessboard_region = NULL;
  memset(priv->hud_layer_surface, 0, sizeof(priv->hud_layer_surface));
  priv->arrows = g_ptr_array_new();

  priv->board_codes[0] = white_rook;
  priv->board_codes[1] = white_knight;
  priv->board_codes[2] = white_bishop;
  priv->board_codes[3] = white_queen;
  priv->board_codes[4] = white_king;
  priv->board_codes[5] = white_bishop;
  priv->board_codes[6] = white_knight;
  priv->board_codes[7] = white_rook;
  priv->board_codes[8] = white_pawn;
  priv->board_codes[9] = white_pawn;
  priv->board_codes[10] = white_pawn;
  priv->board_codes[11] = white_pawn;
  priv->board_codes[12] = white_pawn;
  priv->board_codes[13] = white_pawn;
  priv->board_codes[14] = white_pawn;
  priv->board_codes[15] = white_pawn;
  priv->board_codes[63] = black_rook;
  priv->board_codes[62] = black_knight;
  priv->board_codes[61] = black_bishop;
  priv->board_codes[60] = black_king;
  priv->board_codes[59] = black_queen;
  priv->board_codes[58] = black_bishop;
  priv->board_codes[57] = black_knight;
  priv->board_codes[56] = black_rook;
  priv->board_codes[55] = black_pawn;
  priv->board_codes[54] = black_pawn;
  priv->board_codes[53] = black_pawn;
  priv->board_codes[52] = black_pawn;
  priv->board_codes[51] = black_pawn;
  priv->board_codes[50] = black_pawn;
  priv->board_codes[49] = black_pawn;
  priv->board_codes[48] = black_pawn;

  // The default values for the dark square color (a1).
  gushort const dark_square_red = 45875;
  gushort const dark_square_green = 58981;
  gushort const dark_square_blue = 45875;

  // The default values for the light square color (h1).
  gushort const light_square_red = 65535;
  gushort const light_square_green = 65535;
  gushort const light_square_blue = 58981;

  // Make sure that these colors are different with the current color map.
  // If the they are not different then, shift the light squares to white
  // and the dark squares to black until they ARE different.

  GdkColor dark_square_color;
  GdkColor light_square_color;

  // If you get a compile error here, add -std=c99 to CFLAGS.
  for (double x = 0.0; x <= 1.0; x += 0.01)
  {
    dark_square_color.red = (gushort)((1.0 - x) * dark_square_red);
    dark_square_color.green = (gushort)((1.0 - x) * dark_square_green);
    dark_square_color.blue = (gushort)((1.0 - x) * dark_square_blue);
    if (!gdk_colormap_alloc_color(colormap, &dark_square_color, FALSE, TRUE))
      DoutFatal(dc::fatal, "gdk_colormap_alloc_color failed to allocate dark_square_color (" <<
          dark_square_color.red << ", " <<
	  dark_square_color.green << ", " <<
	  dark_square_color.blue << ")");

    light_square_color.red = light_square_red + (gushort)(x * (65535 - light_square_red));
    light_square_color.green = light_square_green + (gushort)(x * (65535 - light_square_green));
    light_square_color.blue = light_square_blue + (gushort)(x * (65535 - light_square_blue));
    if (!gdk_colormap_alloc_color(colormap, &light_square_color, FALSE, TRUE))
      DoutFatal(dc::fatal, "gdk_colormap_alloc_color failed to allocate light_square_color (" <<
          light_square_color.red << ", " <<
	  light_square_color.green << ", " <<
	  light_square_color.blue << ")");

    if (dark_square_color.red != light_square_color.red ||
        dark_square_color.green != light_square_color.green ||
	dark_square_color.blue != light_square_color.blue)
      break;
  }

  // Store the actual colors.
  cw_chessboard_set_dark_square_color(chessboard, &dark_square_color);
  cw_chessboard_set_light_square_color(chessboard, &light_square_color);

  // Set the default widget's background color equal to the color of the light squares.
  priv->widget_background_color = light_square_color;

  // Set default values for the colors used when drawing the pieces.
  // White
  priv->white_piece_fill_color.red = 1.0;
  priv->white_piece_fill_color.green = 1.0;
  priv->white_piece_fill_color.blue = 1.0;
  // Black
  priv->white_piece_line_color.red = 0.0;
  priv->white_piece_line_color.green = 0.0;
  priv->white_piece_line_color.blue = 0.0;
  // Black
  priv->black_piece_fill_color.red = 0.0;
  priv->black_piece_fill_color.green = 0.0;
  priv->black_piece_fill_color.blue = 0.0;
  // White
  priv->black_piece_line_color.red = 1.0;
  priv->black_piece_line_color.green = 1.0;
  priv->black_piece_line_color.blue = 1.0;
  // Cursor
  priv->cursor_color.red = 1.0;
  priv->cursor_color.green = 0.0;
  priv->cursor_color.blue = 0.0;

  // We do the buffering ourselves.
  gtk_widget_set_double_buffered(GTK_WIDGET(chessboard), FALSE);

  gtk_widget_add_events(GTK_WIDGET(chessboard),
      GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  // Catch motion notify events for cursor handling.
  g_signal_connect(G_OBJECT(chessboard), "motion-notify-event", G_CALLBACK(cw_chessboard_motion_notify), NULL);
}

static void cw_chessboard_finalize(GObject* object)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_finalize(" << object << ")");
  CwChessboard* chessboard = CW_CHESSBOARD(object);  
  g_ptr_array_free(chessboard->priv->arrows, TRUE);
  G_OBJECT_CLASS(cw_chessboard_parent_class)->finalize(object);
}

static void invalidate_border(CwChessboard* chessboard)
{
  if (GTK_WIDGET_REALIZED(chessboard))
  {
    CwChessboardPrivate* priv = chessboard->priv;
    GdkWindow* window = GTK_WIDGET(chessboard)->window;
    GdkRectangle pixmap_rect;
    pixmap_rect.x = priv->top_left_pixmap_x;
    pixmap_rect.y = priv->top_left_pixmap_y;
    pixmap_rect.width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
    pixmap_rect.height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
    GdkRegion* border_region = gdk_region_rectangle(&pixmap_rect);
    gdk_region_subtract(border_region, priv->chessboard_region);
    gdk_window_invalidate_region(window, border_region, FALSE);
    gdk_region_destroy(border_region);
  }
}

static void invalidate_turn_indicators(CwChessboard* chessboard)
{
  if (GTK_WIDGET_REALIZED(chessboard))
  {
    CwChessboardPrivate* priv = chessboard->priv;

    gint const border_width = priv->border_width;
    gint const border_shadow_width = 2;
    gint const edge_width = border_width - border_shadow_width - 1;
    gint const side = squares * priv->sside;

    double const factor = 0.085786;       // (1/sqrt(2) − 0.5)/(1 + sqrt(2)).
    int const dx = (int)ceil((edge_width + 1) * factor);

    GdkRectangle top_indicator_rect, bottom_indicator_rect;
    top_indicator_rect.x = priv->top_left_pixmap_x + priv->pixmap_top_left_a8_x + side + 1 - dx;
    top_indicator_rect.y = priv->top_left_pixmap_y + priv->pixmap_top_left_a8_y - 1 - edge_width;
    top_indicator_rect.width = edge_width + dx;
    top_indicator_rect.height = edge_width;
    GdkRegion* indicator_region = gdk_region_rectangle(&top_indicator_rect);
    bottom_indicator_rect.x = top_indicator_rect.x;
    bottom_indicator_rect.y = top_indicator_rect.y + edge_width + side + 2;
    bottom_indicator_rect.width = edge_width + dx;
    bottom_indicator_rect.height = edge_width;
    gdk_region_union_with_rect(indicator_region, &bottom_indicator_rect);
    top_indicator_rect.x += dx;
    top_indicator_rect.y += edge_width;
    top_indicator_rect.width = edge_width;
    top_indicator_rect.height = dx;
    gdk_region_union_with_rect(indicator_region, &top_indicator_rect);
    bottom_indicator_rect.x += dx;
    bottom_indicator_rect.y -= dx;
    bottom_indicator_rect.width = edge_width;
    bottom_indicator_rect.height = dx;
    gdk_region_union_with_rect(indicator_region, &bottom_indicator_rect);
    gdk_window_invalidate_region(GTK_WIDGET(chessboard)->window, indicator_region, FALSE);
    gdk_region_destroy(indicator_region);
  }
}

#if 0	// Not currently used anywhere.
static void invalidate_background(CwChessboard* chessboard)
{
  if (GTK_WIDGET_REALIZED(chessboard))
  {
    CwChessboardPrivate* priv = chessboard->priv;
    GdkWindow* window = GTK_WIDGET(chessboard)->window;
    GdkRegion* background_region = gdk_drawable_get_clip_region(window);
    gdk_region_subtract(background_region, priv->chessboard_region);
    gdk_window_invalidate_region(window, background_region, FALSE);
    gdk_region_destroy(background_region);
    priv->redraw_background = TRUE;
  }
}
#endif

static void invalidate_square(CwChessboard* chessboard, gint col, gint row)
{
  CwChessboardPrivate* priv = chessboard->priv;
  guint64 redraw_mask = 1;
  redraw_mask <<= convert_colrow2index(col, row);
  // No need to call gdk_window_invalidate_rect again when need_redraw_invalidated is already set.
  if (!(priv->need_redraw_invalidated & redraw_mask))
  {
    if (GTK_WIDGET_REALIZED(chessboard))
    {
      GdkRectangle rect;
      rect.width = priv->sside;
      rect.height = priv->sside;
      cw_chessboard_colrow2xy(chessboard, col, row, &rect.x, &rect.y);
      gdk_window_invalidate_rect(GTK_WIDGET(chessboard)->window, &rect, FALSE);
      priv->need_redraw_invalidated |= redraw_mask;
    }
    else
      priv->need_redraw |= redraw_mask;
  }
}

static void invalidate_board(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  if (GTK_WIDGET_REALIZED(chessboard))
  {
    gdk_window_invalidate_region(GTK_WIDGET(chessboard)->window, priv->chessboard_region, FALSE);
    priv->need_redraw_invalidated = (guint64)-1;
  }
  else
    priv->need_redraw = (guint64)-1;
}

static void invalidate_markers(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  for (gint row = 0; row < squares; ++row)
    for (gint col = 0; col < squares; ++col)
      if (convert_code2mahandle(priv->board_codes[convert_colrow2index(col, row)]) != 0)
        invalidate_square(chessboard, col, row);
}

static void invalidate_cursor(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  if (priv->show_cursor && is_inside_board(priv->cursor_col, priv->cursor_row))
    invalidate_square(chessboard, priv->cursor_col, priv->cursor_row);
}

static void update_cursor_position(CwChessboard* chessboard, gdouble x, gdouble y, gboolean forced)
{
  CwChessboardPrivate* priv = chessboard->priv;
  gint col = cw_chessboard_x2col(chessboard, x);
  gint row = cw_chessboard_y2row(chessboard, y);
  gboolean cursor_moved = (col != priv->cursor_col || row != priv->cursor_row);
  gboolean was_inside_board = is_inside_board(priv->cursor_col, priv->cursor_row);
  gboolean inside_board = is_inside_board(col, row);
  if (cursor_moved || forced)
  {
    if (inside_board)
    {
      if (CW_CHESSBOARD_GET_CLASS(chessboard)->cursor_entered_square)
      {
        if (was_inside_board)
	  CW_CHESSBOARD_GET_CLASS(chessboard)->cursor_entered_square(chessboard, priv->cursor_col, priv->cursor_row, col, row);
        else
	  CW_CHESSBOARD_GET_CLASS(chessboard)->cursor_entered_square(chessboard, -1, -1, col, row);
      }
      if (priv->show_cursor)
	invalidate_square(chessboard, col, row);
    }
    else if (was_inside_board && CW_CHESSBOARD_GET_CLASS(chessboard)->cursor_left_chessboard)
      CW_CHESSBOARD_GET_CLASS(chessboard)->cursor_left_chessboard(chessboard, priv->cursor_col, priv->cursor_row);
    if (priv->show_cursor && was_inside_board)
      invalidate_square(chessboard, priv->cursor_col, priv->cursor_row);
    priv->cursor_col = col;
    priv->cursor_row = row;
  }
  if (priv->show_cursor && !priv->need_redraw_invalidated)
  {
    // Make sure we'll get more motion events.
    Dout(dc::cwchessboardwidget, "Calling gdk_window_get_pointer()");
    gdk_window_get_pointer(GTK_WIDGET(chessboard)->window, NULL, NULL, NULL);
  }
}

static void redraw_border(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  cairo_t* cr = priv->cr;

  gint const pixmap_width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
  gint const pixmap_height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
  gint const side = squares * priv->sside;

  // Fill the area around the board with the background color.
  if (priv->pixmap_top_left_a8_x != 0)
    cairo_rectangle(cr, 0, 0, priv->pixmap_top_left_a8_x, pixmap_height);
  if (pixmap_width - priv->pixmap_top_left_a8_x - side != 0)
    cairo_rectangle(cr, priv->pixmap_top_left_a8_x + side, 0,
        pixmap_width - priv->pixmap_top_left_a8_x - side, pixmap_height);
  if (priv->pixmap_top_left_a8_y != 0)
    cairo_rectangle(cr, 0, 0, pixmap_width, priv->pixmap_top_left_a8_y);
  if (pixmap_height - priv->pixmap_top_left_a8_y - side != 0)
    cairo_rectangle(cr, 0, priv->pixmap_top_left_a8_y + side,
        pixmap_width, pixmap_height - priv->pixmap_top_left_a8_y - side);
  cairo_set_source_rgb(cr, priv->widget_background_color.red / 65535.0,
      priv->widget_background_color.green / 65535.0, priv->widget_background_color.blue / 65535.0);
  cairo_fill(cr);

  // Draw outline of the board if any.
  if (priv->border_width)
    CW_CHESSBOARD_GET_CLASS(chessboard)->draw_border(chessboard);

  // We will need to draw it on the screen too.
  invalidate_border(chessboard);
}

void cw_chessboard_default_draw_border(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  cairo_t* cr = priv->cr;

  gint const border_width = priv->border_width;
  gint const border_shadow_width = 2;
  gint const edge_width = border_width - border_shadow_width - 1;
  gint const side = squares * priv->sside;

  cairo_save(cr);
  // We draw relative to the top-left of the border.
  cairo_translate(cr, priv->pixmap_top_left_a8_x - border_width, priv->pixmap_top_left_a8_y - border_width);
  // Draw a black line around the board.
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_set_line_width(cr, 1.0);
  cairo_set_source_rgb(cr, priv->board_border_color.red * 0.5, priv->board_border_color.green * 0.5, priv->board_border_color.blue * 0.5);
  cairo_move_to(cr, side + border_width + 0.5, border_width - 0.5);
  cairo_line_to(cr, border_width - 0.5, border_width - 0.5);
  cairo_line_to(cr, border_width - 0.5, side + border_width + 0.5);
  cairo_stroke(cr);
  cairo_set_source_rgb(cr, (1.0 + priv->board_border_color.red) * 0.5, (1.0 + priv->board_border_color.green) * 0.5, (1.0 + priv->board_border_color.blue) * 0.5);
  cairo_move_to(cr, border_width - 0.5, side + border_width + 0.5);
  cairo_line_to(cr, side + border_width + 0.5, side + border_width + 0.5);
  cairo_line_to(cr, side + border_width + 0.5, border_width - 0.5);
  cairo_stroke(cr);
  // Draw an edge around that that will contain the chessboard coordinates.
  cairo_set_source_rgb(cr, priv->board_border_color.red, priv->board_border_color.green, priv->board_border_color.blue);
  cairo_set_line_width(cr, edge_width);
  cairo_rectangle(cr, border_shadow_width + edge_width * 0.5, border_shadow_width + edge_width * 0.5, side + edge_width + 2, side + edge_width + 2);
  cairo_stroke(cr);
  // Draw the shadow around that.
  cairo_set_source_rgb(cr, (1.0 + priv->board_border_color.red) * 0.5, (1.0 + priv->board_border_color.green) * 0.5, (1.0 + priv->board_border_color.blue) * 0.5);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, 0, side + 2 * border_width);
  cairo_rel_line_to(cr, border_shadow_width, -border_shadow_width);
  cairo_rel_line_to(cr, 0, -(side + 2 + 2 * edge_width));
  cairo_rel_line_to(cr, side + 2 + 2 * edge_width, 0);
  cairo_rel_line_to(cr, border_shadow_width, -border_shadow_width);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_set_source_rgb(cr, priv->board_border_color.red * 0.5, priv->board_border_color.green * 0.5, priv->board_border_color.blue * 0.5);
  cairo_move_to(cr, side + 2 * border_width, side + 2 * border_width);
  cairo_line_to(cr, side + 2 * border_width, 0);
  cairo_rel_line_to(cr, -border_shadow_width, border_shadow_width);
  cairo_rel_line_to(cr, 0, side + 2 + 2 * edge_width);
  cairo_rel_line_to(cr, -(side + 2 + 2 * edge_width), 0);
  cairo_rel_line_to(cr, -border_shadow_width, border_shadow_width);
  cairo_close_path(cr);
  cairo_fill(cr);
  // Draw the coordinates.
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  PangoLayout* layout = pango_cairo_create_layout(cr);
  PangoFontDescription* desc = pango_font_description_from_string("Sans Bold 14");
  gint font_pixels = (edge_width > 14) ? MAX(14, edge_width * 0.66) : edge_width;
  pango_font_description_set_absolute_size(desc, MAX(font_pixels, 7) * PANGO_SCALE);
  pango_layout_set_font_description(layout, desc);
  pango_font_description_free(desc);
  char c[2] = { 0, 0 };
  for (int col = 0; col < 8; ++col)
  {
    c[0] = 'A' + col;
    pango_layout_set_text(layout, c, -1);
    int width, height;
    pango_layout_get_size(layout, &width, &height);
    cairo_move_to(cr, border_width + ((priv->flip_board ? 7 - col : col) + 0.5) * priv->sside - ((double)width / PANGO_SCALE) / 2,
	side + 1.5 * border_width - ((double)height / PANGO_SCALE) / 2);
    pango_cairo_show_layout(cr, layout);
  }
  for (int row = 0; row < 8; ++row)
  {
    c[0] = '1' + row;
    pango_layout_set_text(layout, c, -1);
    int width, height;
    pango_layout_get_size(layout, &width, &height);
    cairo_move_to(cr, border_width / 2 - ((double)width / PANGO_SCALE) / 2,
	border_width + ((priv->flip_board ? row : 7 - row) + 0.5) * priv->sside - ((double)height / PANGO_SCALE) / 2);
    pango_cairo_show_layout(cr, layout);
  }
  g_object_unref(layout);

  cairo_restore(cr);

  // Draw the turn indicator, if requested.
  if (priv->draw_turn_indicators)
    CW_CHESSBOARD_GET_CLASS(chessboard)->draw_turn_indicator(chessboard, priv->active_turn_indicator, TRUE);
}

void cw_chessboard_default_draw_turn_indicator(CwChessboard* chessboard, gboolean white, gboolean on)
{
  CwChessboardPrivate* priv = chessboard->priv;
  cairo_t* cr = priv->cr;

  gint const border_width = priv->border_width;
  gint const border_shadow_width = 2;
  gint const edge_width = border_width - border_shadow_width - 1;
  gint const side = squares * priv->sside;

  cairo_save(cr);
  // We draw relative to the top-left of a square of edge_width X edge_width.
  cairo_translate(cr, priv->pixmap_top_left_a8_x + side + 1,
      priv->pixmap_top_left_a8_y - border_width + border_shadow_width + ((white != priv->flip_board) ? side + edge_width + 2 : 0));

  double const factor = 0.085786;	// (1/sqrt(2) − 0.5)/(1 + sqrt(2)).
  if (on)
  {
    if (white)
      cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    else
      cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_arc(cr, edge_width * 0.5 - MAX((edge_width + 1) * factor - 1, 0),
		  edge_width * 0.5 - (edge_width + 1) * ((white != priv->flip_board) ? factor : -factor), edge_width * 0.5, 0, 2 * M_PI);
    cairo_fill(cr);
  }
  else
  {
    gboolean top = (white == priv->flip_board);
    double dir = top ? 1.0 : -1.0;
    cairo_set_source_rgb(cr, priv->board_border_color.red, priv->board_border_color.green, priv->board_border_color.blue);
    cairo_move_to(cr, 0, top ? edge_width : 0);
    cairo_rel_line_to(cr, 0, dir * ((edge_width + 1) * factor + 1));
    cairo_rel_line_to(cr, edge_width, 0);
    cairo_line_to(cr, edge_width, top ? 0 : edge_width);
    cairo_rel_line_to(cr, -(edge_width + (edge_width + 1) * factor + 1), 0);
    cairo_rel_line_to(cr, 0, dir * edge_width);
    cairo_close_path(cr);
    cairo_fill(cr);
  }

  cairo_restore(cr);
}

// This function should only be called from cw_chessboard_expose.
// Call invalidate_square to redraw a square from elsewhere.
static void redraw_square(CwChessboard* chessboard, gint index)
{
  CwChessboardPrivate* priv = chessboard->priv;
  cairo_t* cr = priv->cr;

  CwChessboardCode code = priv->board_codes[index];
  CwChessboardColorHandle bghandle = convert_code2bghandle(code);
  CwChessboardColorHandle mahandle = convert_code2mahandle(code);
  gint col = convert_index2column(index);
  gint row = convert_index2row(index);
  gint sx = priv->pixmap_top_left_a8_x + (priv->flip_board ? 7 - col : col) * priv->sside;
  gint sy = priv->pixmap_top_left_a8_y + (squares - 1 - (priv->flip_board ? 7 - row : row)) * priv->sside;

  Dout(dc::cwchessboardwidget, "Calling redraw_square(" << chessboard << ", " << index << ")" <<
      " with Board Code: " << (int)code);

  // Draw background color.
  cairo_rectangle(cr, sx, sy, priv->sside, priv->sside);
  if (bghandle != 0)
    cairo_set_source_rgb(cr,
        priv->color_palet[bghandle - 1].red,
        priv->color_palet[bghandle - 1].green,
	priv->color_palet[bghandle - 1].blue);
  else if (((col + row) & 1))
    cairo_set_source_rgb(cr,
	priv->light_square_color.red,
	priv->light_square_color.green,
	priv->light_square_color.blue);
  else
    cairo_set_source_rgb(cr,
	priv->dark_square_color.red,
	priv->dark_square_color.green,
	priv->dark_square_color.blue);
  if (!mahandle || !priv->marker_below)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    // Draw marker.
    cairo_rectangle(cr, sx + priv->marker_thickness_px, sy + priv->marker_thickness_px,
        priv->sside - 2 * priv->marker_thickness_px, priv->sside - 2 * priv->marker_thickness_px);
    cairo_fill_rule_t prev_fill_rule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_source_rgb(cr,
        priv->color_palet[mahandle - 1].red,
        priv->color_palet[mahandle - 1].green,
	priv->color_palet[mahandle - 1].blue);
    cairo_fill(cr);
    cairo_set_fill_rule(cr, prev_fill_rule);
  }

  guint64 bit = 1;
  bit <<= index;

  // Draw bottom HUD layer, if any.
  if ((priv->hud_has_content[0] & bit))
  {
    cairo_set_source_surface(cr, priv->hud_layer_surface[0], priv->pixmap_top_left_a8_x, priv->pixmap_top_left_a8_y);
    cairo_rectangle(cr, sx, sy, priv->sside, priv->sside);
    cairo_fill(cr);
  }

  // Draw marker, if any and still needed.
  if (mahandle && !priv->marker_below)
  {
    cairo_rectangle(cr, sx, sy, priv->sside, priv->sside);
    cairo_rectangle(cr, sx + priv->marker_thickness_px, sy + priv->marker_thickness_px,
        priv->sside - 2 * priv->marker_thickness_px, priv->sside - 2 * priv->marker_thickness_px);
    cairo_fill_rule_t prev_fill_rule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_source_rgb(cr,
        priv->color_palet[mahandle - 1].red,
        priv->color_palet[mahandle - 1].green,
	priv->color_palet[mahandle - 1].blue);
    cairo_fill(cr);
    cairo_set_fill_rule(cr, prev_fill_rule);
  }

  if (priv->show_cursor && priv->cursor_col == col && priv->cursor_row == row)
  {
    cairo_rectangle(cr, sx, sy, priv->sside, priv->sside);
    cairo_rectangle(cr, sx + priv->cursor_thickness_px, sy + priv->cursor_thickness_px,
        priv->sside - 2 * priv->cursor_thickness_px, priv->sside - 2 * priv->cursor_thickness_px);
    cairo_fill_rule_t prev_fill_rule = cairo_get_fill_rule(cr);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_set_source_rgb(cr,
        priv->cursor_color.red,
        priv->cursor_color.green,
	priv->cursor_color.blue);
    cairo_fill(cr);
    cairo_set_fill_rule(cr, prev_fill_rule);
  }

  // Draw piece, if any.
  if (!is_empty_square(code))
  {
    cairo_set_source_surface(cr, priv->piece_pixmap[convert_code2piece_pixmap_index(code)].surface, sx, sy);
    cairo_paint(cr);
  }

  // Draw top HUD layer, if any.
  if ((priv->hud_has_content[1] & bit))
  {
    cairo_set_source_surface(cr, priv->hud_layer_surface[1], priv->pixmap_top_left_a8_x, priv->pixmap_top_left_a8_y);
    cairo_rectangle(cr, sx, sy, priv->sside, priv->sside);
    cairo_fill(cr);
  }
}

static void redraw_pieces(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  for (int i = 0; i < 12; ++i)
  {
    if (priv->piece_pixmap[i].surface)
      cairo_surface_destroy(priv->piece_pixmap[i].surface);
    priv->piece_pixmap[i].surface =
	cairo_surface_create_similar(cairo_get_target(priv->cr),
	CAIRO_CONTENT_COLOR_ALPHA, priv->sside, priv->sside);
    Dout(dc::cwchessboardwidget|continued_cf, "(Re)drawing piece cache " << i << "... ");
    cairo_t* cr = cairo_create(priv->piece_pixmap[i].surface);
    unsigned char code = (unsigned char)(i + 2);
    cairo_rectangle(cr, 0, 0, priv->sside, priv->sside);
    cairo_clip(cr);
    CW_CHESSBOARD_GET_CLASS(chessboard)->
        draw_piece[convert_code2piece(code)] (chessboard,
	                                      cr,
					      0.5 * priv->sside,
					      0.5 * priv->sside,
					      priv->sside,
					      is_white_piece(code));
    cairo_destroy(cr);
    Dout(dc::finish, "done");
  }
  invalidate_board(chessboard);
}

// This function is called after realize, or after a resize.
// Allocate the pixmap and redraw everything (including the border).
static void redraw_pixmap(GtkWidget* widget)
{
  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;

  // Calculate the size of the squares.
  gint const max_total_side = MIN(widget->allocation.width, widget->allocation.height);

  // First, assume we have no border.
  gint sside = MAX(min_sside, max_total_side / squares);
  priv->border_width = 0;

  if (priv->draw_border)					// Do we have a border?
  {
    // The total side of the board is: squares * sside + 2 * border_width;
    // However, we don't know the border_width as it is a function of sside!
    // The only way to calculate this is trying recursively.
    ++sside;
    do
    {
      --sside;
      // Call the function that calculates the border width.
      priv->border_width = CW_CHESSBOARD_GET_CLASS(chessboard)->calc_board_border_width(chessboard, sside);
    }
    while (sside > min_sside && squares * sside + 2 * priv->border_width > max_total_side);
  }
  // Now sside and border_width have been established.

  // The total size of one side of the board, excluding border.
  gint const side = squares * sside;
  // The total size of one side of the board, including border.
  gint const total_side = side + 2 * priv->border_width;

  // Calculate the size of the pixmap. Include areas outside the border if small enough.
  gint pixmap_width = total_side;
  gint pixmap_height = total_side;
  if (widget->allocation.width < widget->allocation.height)
    pixmap_width = widget->allocation.width;
  else
    pixmap_height = widget->allocation.height;

  // Remember old size of pixmap.
  gint old_pixmap_width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
  gint old_pixmap_height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
  // Calculate the offsets of the pixmap corners.
  priv->top_left_pixmap_x = ((widget->allocation.width - pixmap_width) & ~1) / 2;
  priv->top_left_pixmap_y = ((widget->allocation.height - pixmap_height) & ~1) / 2;
  priv->bottom_right_pixmap_x = priv->top_left_pixmap_x + pixmap_width;
  priv->bottom_right_pixmap_y = priv->top_left_pixmap_y + pixmap_height;
  g_assert(priv->top_left_pixmap_x == 0 || priv->top_left_pixmap_y == 0);

  // Calculate the offset of the top-left of square a1.
  *(gint*)(&chessboard->top_left_a1_x) = ((widget->allocation.width - side) & ~1) / 2;
  *(gint*)(&chessboard->top_left_a1_y) = ((widget->allocation.height - side) & ~1) / 2 + side - sside;

  // Calculate the offset of the top-left of square a8, relative to the top-left of the pixmap.
  priv->pixmap_top_left_a8_x = chessboard->top_left_a1_x - priv->top_left_pixmap_x;
  priv->pixmap_top_left_a8_y = chessboard->top_left_a1_y + sside - side - priv->top_left_pixmap_y;

  // Set the rectangular region where the chessboard resides.
  GdkRectangle rect;
  rect.x = priv->top_left_pixmap_x + priv->pixmap_top_left_a8_x;
  rect.y = priv->top_left_pixmap_y + priv->pixmap_top_left_a8_y;
  rect.width = squares * sside;
  rect.height = squares * sside;
  if (priv->chessboard_region)
    gdk_region_destroy(priv->chessboard_region);
  priv->chessboard_region = gdk_region_rectangle(&rect);

  Dout(dc::cwchessboardwidget, "widget size is (" << widget->allocation.width << ", " << widget->allocation.height << ")");
  Dout(dc::cwchessboardwidget, "border width is " << priv->border_width <<
      "; " << squares << 'x' << squares << " squares with side " << sside);
  Dout(dc::cwchessboardwidget, "pixmap at (" << priv->top_left_pixmap_x << ", " << priv->top_left_pixmap_y << ") with size (" <<
      pixmap_width << ", " << pixmap_height << ")");
  Dout(dc::cwchessboardwidget, "a8 offset within pixmap is (" <<
      priv->pixmap_top_left_a8_x << ", " << priv->pixmap_top_left_a8_y << ")");
  Dout(dc::cwchessboardwidget, "    a1 at (" << chessboard->top_left_a1_x << ", " << chessboard->top_left_a1_y << ")");

  // Invalidate everything.
  gdk_window_invalidate_rect(widget->window, &widget->allocation, FALSE);
  priv->need_redraw_invalidated = (guint64)-1;

  if (old_pixmap_width == pixmap_width && old_pixmap_height == pixmap_height)
    return;

  if (priv->cr)
    cairo_destroy(priv->cr);
  if (priv->pixmap)
    g_object_unref(priv->pixmap);

  // Allocate a pixmap for the board including the border.
  priv->pixmap = gdk_pixmap_new(widget->window, pixmap_width, pixmap_height, -1);

  // (Re)create cairo context for the new pixmap.
  priv->cr = gdk_cairo_create(priv->pixmap);

  // If the resize even changed the side of the squares then we have to redraw
  // the cached pieces and the HUD layer cache. Initially priv->sside is set to
  // -1 and the expression will always evalute to true.
  if (priv->sside != sside)
  {
    // Store the new value of sside.
    *(gint*)(&chessboard->sside) = priv->sside = sside;

    // Destroy hatching cache for redraw later.
    if (priv->hatching_pixmap.surface)
      cairo_surface_destroy(priv->hatching_pixmap.surface);
    priv->hatching_pixmap.surface = NULL;

    // Create new cairo surfaces for the piece cache and (re)draw the pieces.
    redraw_pieces(chessboard);

    // Calculate the marker thickness.
    priv->marker_thickness_px = MAX(1, MIN((gint)round(priv->marker_thickness * sside), sside / 2));
    Dout(dc::cwchessboardwidget, "Marker thickness set to " << priv->marker_thickness_px);

    // Calculate the cursor thickness.
    priv->cursor_thickness_px = MAX(1, MIN((gint)round(priv->cursor_thickness * sside), sside / 2));
    Dout(dc::cwchessboardwidget, "Cursor thickness set to " << priv->cursor_thickness_px);

    // (Re)create alpha layer.
    Dout(dc::cwchessboardwidget|continued_cf, "(Re)creating HUD layers... ");
    recreate_hud_layers(chessboard);
    Dout(dc::finish, "done");
  }

  redraw_border(chessboard);
}

static void cw_chessboard_size_request(GtkWidget* widget, GtkRequisition* requisition)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_size_request(" << ")");

  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;

  // Return the minimum size we really need.
  gint min_border_width = 0;
  if (priv->draw_border)
    min_border_width = 2 * CW_CHESSBOARD_GET_CLASS(chessboard)->calc_board_border_width(chessboard, min_sside);
  requisition->width = requisition->height = squares * min_sside + min_border_width;
}

static void cw_chessboard_size_allocate(GtkWidget* widget, GtkAllocation* allocation)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_size_allocate(" << widget << ", " << allocation << ")");
  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED(widget))
  {
    gdk_window_move_resize(widget->window,
        allocation->x, allocation->y,
        allocation->width, allocation->height);
    redraw_pixmap(widget);
  }
  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  chessboard->priv->redraw_background = TRUE;
}

static void cw_chessboard_realize(GtkWidget* widget)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_realize(" << widget << ")");
  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  GTK_WIDGET_CLASS(cw_chessboard_parent_class)->realize(widget);
  redraw_pixmap(widget);
  //gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
  gdk_window_set_background(widget->window, &chessboard->priv->widget_background_color);
}

static void cw_chessboard_unrealize(GtkWidget* widget)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_unrealize(" << widget << ")");
  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;
  for (int i = 0; i < 12; ++i)
  {
    if (priv->piece_pixmap[i].surface)
      cairo_surface_destroy(priv->piece_pixmap[i].surface);
    priv->piece_pixmap[i].surface = NULL;
  }
  if (priv->hatching_pixmap.surface)
    cairo_surface_destroy(priv->hatching_pixmap.surface);
  priv->hatching_pixmap.surface = NULL;
  gdk_region_destroy(priv->chessboard_region);
  priv->chessboard_region = NULL;
  for (guint hud = 0; hud < G_N_ELEMENTS(priv->hud_layer_surface); ++hud)
    cairo_surface_destroy(priv->hud_layer_surface[hud]);
  memset(priv->hud_layer_surface, 0, sizeof(priv->hud_layer_surface));
  cairo_destroy(priv->cr);
  priv->cr = NULL;
  g_object_unref(priv->pixmap);
  priv->pixmap = NULL;
  priv->sside = -1;
  priv->top_left_pixmap_x = 0;
  priv->top_left_pixmap_y = 0;
  priv->bottom_right_pixmap_x = 0;
  priv->bottom_right_pixmap_y = 0;
  GTK_WIDGET_CLASS(cw_chessboard_parent_class)->unrealize(widget);
}

// This function is called whenever (a certain region of) the chessboard needs
// to be redrawn, for whatever reason.
static gboolean cw_chessboard_expose(GtkWidget* widget, GdkEventExpose* event)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_expose(" << widget << ", " << event << ")");

  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;
  GdkRegion* region = event->region;

#if CW_CHESSBOARD_EXPOSE_DEBUG
  gdk_window_clear(widget->window);
#endif

  // Last minute update of the HUD layers.
  for (guint hud = 0; hud < G_N_ELEMENTS(priv->hud_need_redraw); ++hud)
    if (priv->hud_need_redraw[hud])
      redraw_hud_layer(chessboard, hud);

  // Last minute update of pixmap.
  guint64 redraw_mask = 1;
  for (gint i = 0; i < 64; ++i, redraw_mask <<= 1)
    if (((priv->need_redraw | priv->need_redraw_invalidated) & redraw_mask))
      redraw_square(chessboard, i);		// This uses the HUD layer.
  priv->need_redraw_invalidated = 0;
  priv->need_redraw = 0;

#if CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  // Draw any floating pieces to the pixbuf.
  if (priv->number_of_floating_pieces)
  {
    cairo_save(priv->cr);
    cairo_rectangle(priv->cr, priv->pixmap_top_left_a8_x, priv->pixmap_top_left_a8_y,
        squares * priv->sside, squares * priv->sside);
    cairo_clip(priv->cr);
    for (gsize i = 0; i < priv->number_of_floating_pieces; ++i)
      if (priv->floating_piece[i].moved)
      {
        cairo_set_source_surface(priv->cr,
	    priv->piece_pixmap[convert_code2piece_pixmap_index(priv->floating_piece[i].code)].surface,
	    priv->floating_piece[i].pixmap_x, priv->floating_piece[i].pixmap_y);
        cairo_paint(priv->cr);
      }
    cairo_restore(priv->cr);
  }
#endif

  // Either top_left_pixmap_x or top_left_pixmap_y equals 0 (see redraw_pixmap).
  // The type of configuration depends on 'vertical':
  //
  // vertical == true	   vertical == false.
  // .-----------,         .--+----------+--,
  // |           |         |  |          |  |
  // +-----------+         |  |          |  |
  // |           |         |  | pixmap   |  |
  // | pixmap    |         |  |          |  |
  // |           |         |  |          |  |
  // |           |         |  |          |  |
  // +-----------+         `--+----------+--'
  // |           |
  // `-----------'
  // The less than zero happens when the window is so small that sside would have to be less than min_sside.
  gboolean vertical = (priv->top_left_pixmap_x == 0);

  // This default is false, which is the case when the board is only updated.
  gboolean region_extends_outside_pixmap = FALSE;
  // However, if we resized -- or another window moved over the widget -- it could be true.
  GdkRectangle clipbox;
  gdk_region_get_clipbox(region, &clipbox);
  if (G_UNLIKELY(clipbox.y < priv->top_left_pixmap_y))
    region_extends_outside_pixmap = vertical;
  if (G_UNLIKELY(clipbox.y + clipbox.height > priv->bottom_right_pixmap_y))
    region_extends_outside_pixmap = vertical;
  if (G_UNLIKELY(clipbox.x < priv->top_left_pixmap_x))
    region_extends_outside_pixmap = !vertical;
  if (G_UNLIKELY(clipbox.x + clipbox.width > priv->bottom_right_pixmap_x))
    region_extends_outside_pixmap = !vertical;
  if (G_UNLIKELY(region_extends_outside_pixmap))
  {
    GdkRectangle pixmap_rect;
    pixmap_rect.x = priv->top_left_pixmap_x;
    pixmap_rect.y = priv->top_left_pixmap_y;
    pixmap_rect.width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
    pixmap_rect.height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
    GdkRegion* pixmap_region = gdk_region_rectangle(&pixmap_rect);
    if (CW_CHESSBOARD_EXPOSE_ALWAYS_CLEAR_BACKGROUND || priv->redraw_background)
    {
      // If the widget was resized, there might be trash outside the pixmap. Erase that too.
      GdkRegion* outside_pixmap_region = gdk_region_copy(region);
      gdk_region_subtract(outside_pixmap_region, pixmap_region);
      GdkRectangle* outside_areas;
      gint n_outside_areas;
      gdk_region_get_rectangles(outside_pixmap_region, &outside_areas, &n_outside_areas);
      GdkRectangle const* outside_rect = outside_areas;
      for (int i = 0; i < n_outside_areas; ++i, ++outside_rect)
	gdk_window_clear_area(widget->window, outside_rect->x, outside_rect->y, outside_rect->width, outside_rect->height);
#if CW_CHESSBOARD_EXPOSE_DEBUG
      // Draw a green rectangle around updated areas.
      GdkGC* debug_gc = gdk_gc_new(priv->pixmap);
      GdkColor debug_green = { 0, 0, 65535, 0 };
      gdk_colormap_alloc_color(gtk_widget_get_colormap(widget), &debug_green, FALSE, TRUE);
      gdk_gc_set_foreground(debug_gc, &debug_green);
      outside_rect = outside_areas;
      for (int i = 0; i < n_outside_areas; ++i, ++outside_rect)
	gdk_draw_rectangle(widget->window, debug_gc, FALSE,
	    outside_rect->x, outside_rect->y, outside_rect->width - 1, outside_rect->height - 1);
      g_object_unref(debug_gc);
#endif
      gdk_region_destroy(outside_pixmap_region);
      priv->redraw_background = FALSE;
    }
    gdk_region_intersect(region, pixmap_region);
    gdk_region_destroy(pixmap_region);
  }

  cairo_t* dest = gdk_cairo_create(widget->window);
#if !CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER && !CW_CHESSBOARD_FLOATING_PIECE_INVALIDATE_TARGET
  if (priv->number_of_floating_pieces)
    cairo_save(dest);
#endif
  cairo_surface_t* source = cairo_get_target(priv->cr);
#if CW_CHESSBOARD_EXPOSE_DEBUG
  cairo_set_source_surface(dest, source, priv->top_left_pixmap_x, priv->top_left_pixmap_y);
  cairo_paint_with_alpha(dest, 0.35);
#endif
  cairo_set_source_surface(dest, source, priv->top_left_pixmap_x, priv->top_left_pixmap_y);
  gdk_cairo_region(dest, region);
#if CW_CHESSBOARD_EXPOSE_DEBUG
  cairo_clip_preserve(dest);
#else
  cairo_clip(dest);
#endif
  cairo_paint(dest);
#if CW_CHESSBOARD_EXPOSE_DEBUG
  cairo_set_line_width(dest, 2);
  cairo_set_source_rgb(dest, 1, 0, 0);
  cairo_stroke(dest);
#endif

#if !CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  // Draw any floating pieces to the screen.
  if (priv->number_of_floating_pieces)
  {
#if !CW_CHESSBOARD_FLOATING_PIECE_INVALIDATE_TARGET
    cairo_restore(dest);	// Remove clip region: we draw floating pieces outside the region!
#endif
    for (gsize i = 0; i < priv->number_of_floating_pieces; ++i)
      if (priv->floating_piece[i].moved)
      {
        cairo_set_source_surface(dest,
	    priv->piece_pixmap[convert_code2piece_pixmap_index(priv->floating_piece[i].code)].surface,
	    priv->floating_piece[i].pixmap_x + priv->top_left_pixmap_x,
	    priv->floating_piece[i].pixmap_y + priv->top_left_pixmap_y);
        cairo_paint(dest);
      }
  }
#endif

  cairo_destroy(dest);

  if (priv->show_cursor || priv->floating_piece_handle != -1)
  {
    // Call this function so that we'll get the next pointer motion hint.
    Dout(dc::cwchessboardwidget, "Calling gdk_window_get_pointer()");
    gdk_window_get_pointer(GTK_WIDGET(chessboard)->window, NULL, NULL, NULL);
  }
#if 0
  static int benchmark = 0;
  if (benchmark & 1)
    cw_chessboard_enable_hud_layer(chessboard, 0);
  else
    cw_chessboard_disable_hud_layer(chessboard, 0);
  if (++benchmark == 100)
    exit(0);
#endif

  return TRUE;
}

gint cw_chessboard_default_calc_board_border_width(CwChessboard const* chessboard, gint sside)
{
  // Make line width run from 1.0 (at sside == 12) to 4.0 (at sside == 87)).
  // Round to nearest even integer. Then add sside / 3. Return at least 8.
  return (gint)MAX(8.0, round(1.0 + (sside - 12) / 25.0) + sside / 3.0);
}

gboolean cw_chessboard_default_draw_hud_square(G_GNUC_UNUSED CwChessboard* chessboard,
    cairo_t* cr, gint col, gint row, gint sside, guint hud)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_default_draw_hud_square(" << chessboard << ", " <<
      cr << ", " << col << ", " << row << ", " << sside << ", " << hud << ")");

  g_return_val_if_fail(hud < number_of_hud_layers, FALSE);
  g_return_val_if_fail(is_inside_board(col, row), FALSE);

  int const number_of_lines = 21;
  double const line_width = 0.25;

  // No HUD layer for the white squares.
  if (hud == 1 || ((col + row) & 1) == 1)
    return FALSE;

  CwChessboardPrivate* priv = chessboard->priv;
  if (!priv->hatching_pixmap.surface)
  {
    priv->hatching_pixmap.surface = cairo_surface_create_similar(cairo_get_target(priv->cr),
        CAIRO_CONTENT_COLOR_ALPHA, sside, sside);
    cairo_t* cr = cairo_create(priv->hatching_pixmap.surface);
    cairo_set_line_width(cr, line_width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_scale(cr, (double)sside / number_of_lines, (double)sside / number_of_lines);
    for (int h = 0; h < number_of_lines; ++h)
    {
      double s = h + line_width;
      cairo_move_to(cr, s - 0.5 * line_width, 0.5 * line_width);
      cairo_line_to(cr, 0.5 * line_width, s - 0.5 * line_width);
      cairo_move_to(cr, s + 0.5 * line_width, number_of_lines - 0.5 * line_width);
      cairo_line_to(cr, number_of_lines - 0.5 * line_width, s + 0.5 * line_width);
    }
    cairo_stroke(cr);
    cairo_destroy(cr);
  }

  cairo_set_source_surface(cr, priv->hatching_pixmap.surface, 0, 0);
  cairo_paint(cr);

  return TRUE;
}

void cw_chessboard_default_draw_hud_layer(CwChessboard* chessboard, cairo_t* cr, gint sside, guint hud)
{
  g_return_if_fail(hud < number_of_hud_layers);

  CwChessboardPrivate* priv = chessboard->priv;
  guint64 bit = 1;
  for (gint row = 0; row < 8; ++row)
    for (gint col = 0; col < 8; ++col, bit <<= 1)
    {
      if ((priv->hud_need_redraw[hud] & bit))
      {
	cairo_save(cr);
	// FIXME: also rotate the square 180 degrees when flipped.
	cairo_translate(cr, (priv->flip_board ? 7 - col : col) * sside, (priv->flip_board ? row : 7 - row) * sside);
	cairo_rectangle(cr, 0, 0, sside, sside);
	cairo_clip(cr);
	if (CW_CHESSBOARD_GET_CLASS(chessboard)->draw_hud_square(chessboard, cr, col, row, sside, hud))
	{
	  priv->hud_has_content[hud] |= bit;
	  invalidate_square(chessboard, col, row);
	}
	cairo_restore(cr);
      }
    }
}

static GdkRegion* convert_mask2region(guint64 mask, gint x, gint y, gint sside, gboolean flip_board)
{
  GdkRegion* region = gdk_region_new();
  guint64 row_mask = (guint64)0xff << 56;
  for (gint row = 7; row >= 0; --row, row_mask >>= 8)
  {
    if ((mask & row_mask))
    {
      guint64 col_mask = (guint64)1 << 8 * row;
      gint col_start = 0;
      gint col_end;
      do
      {
	while (col_start != 8 && !(mask & col_mask))
	{
	  ++col_start;
	  col_mask <<= 1;
	}
	if (col_start == 8)
	  break;	// No new start found;
	col_end = col_start;
	while (col_end != 8 && (mask & col_mask))
	{
	  ++col_end;
	  col_mask <<= 1;
	}
	GdkRectangle rect;
        rect.x = x + (flip_board ? 8 - col_end : col_start) * sside;
	rect.y = y + (flip_board ? row : 7 - row) * sside;
	rect.width = (col_end - col_start) * sside;
        rect.height = sside;
	gdk_region_union_with_rect(region, &rect);
	col_start = col_end;
      }
      while(col_end !=8);
    }
  }
  return region;
}

static void redraw_hud_layer(CwChessboard* chessboard, guint hud)
{
  Dout(dc::cwchessboardwidget, "Calling redraw_hud_layer(" << chessboard << ", " << hud << ")");

  CwChessboardPrivate* priv = chessboard->priv;
  gint sside = priv->sside;

  cairo_t* cr = cairo_create(priv->hud_layer_surface[hud]);
  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
  guint64 need_clear = priv->hud_has_content[hud] & priv->hud_need_redraw[hud];
  for (gint row = 0; row < 8; ++row)
    if ((need_clear & ((guint64)0xff << (8 * row))))
    {
      guint64 bit = (guint64)1 << (8 * row);
      for (gint col = 0; col < 8; ++col, bit <<= 1)
	if ((need_clear & bit))
	{
	  cairo_rectangle(cr, (priv->flip_board ? 7 - col : col) * sside, (priv->flip_board ? row : 7 - row) * sside, sside, sside);
	  invalidate_square(chessboard, col, row);
        }
    }
  cairo_fill(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  priv->hud_has_content[hud] &= ~priv->hud_need_redraw[hud];

  if (priv->has_hud_layer[hud])
  {
    if (CW_CHESSBOARD_GET_CLASS(chessboard)->draw_hud_layer != cw_chessboard_default_draw_hud_layer)
    {
      priv->hud_has_content[hud] = (guint64)-1;	// Assume that a user function puts content in all squares.
      invalidate_board(chessboard);
    }
    CW_CHESSBOARD_GET_CLASS(chessboard)->draw_hud_layer(chessboard, cr, sside, hud);
  }

  GdkRegion* need_redraw_region = convert_mask2region(priv->hud_need_redraw[hud], 0, 0, priv->sside, priv->flip_board);
  gdk_cairo_region(cr, need_redraw_region);
  gdk_region_destroy(need_redraw_region);
  cairo_clip(cr);

  for (guint i = 0; i < priv->arrows->len; ++i)
  {
    Arrow* arrow = (Arrow*)priv->arrows->pdata[i];
    if ((priv->hud_need_redraw[hud] & arrow->has_content[hud]))
    {
      guint64 other_content = 0;
      for (guint h = 0; h < number_of_hud_layers; ++h)
        if (h != hud)
	  other_content |= arrow->has_content[h];
      gboolean has_colliding_content = (priv->hud_need_redraw[hud] & other_content) != 0;
      if (has_colliding_content)
      {
        cairo_save(cr);
	GdkRegion* clip_region = convert_mask2region(arrow->has_content[hud], 0, 0, priv->sside, priv->flip_board);
	gdk_cairo_region(cr, clip_region);
	gdk_region_destroy(clip_region);
	cairo_clip(cr);
      }
      gdouble length = sqrt((arrow->end_col - arrow->begin_col) * (arrow->end_col - arrow->begin_col) +
          (arrow->end_row - arrow->begin_row) * (arrow->end_row - arrow->begin_row));
      gdouble begin_x = (0.5 + (priv->flip_board ? 7 - arrow->begin_col : arrow->begin_col)) * priv->sside;
      gdouble begin_y = (0.5 + (priv->flip_board ? arrow->begin_row : 7 - arrow->begin_row)) * priv->sside;
      // Unit vector in direction of arrow.
      gdouble vx = priv->sside * (arrow->end_col - arrow->begin_col) / length;
      gdouble vy = priv->sside * (arrow->begin_row - arrow->end_row) / length;
      if (priv->flip_board)
      {
        vx = -vx;
	vy = -vy;
      }
      // Unit vector, rotated 90 degrees counter-clockwise.
      gdouble tx = -vy;
      gdouble ty = vx;
      // Draw arrow.
      cairo_move_to(cr, begin_x + 0.125 * tx, begin_y + 0.125 * ty);
      cairo_rel_line_to(cr, (length - 0.25) * vx, (length - 0.25) * vy);
      cairo_rel_line_to(cr, 0.125 * tx, 0.125 * ty);
      cairo_line_to(cr, begin_x + length * vx, begin_y + length * vy);
      cairo_line_to(cr, begin_x + (length - 0.25) * vx - 0.25 * tx,
          begin_y + (length - 0.25) * vy - 0.25 * ty);
      cairo_rel_line_to(cr, 0.125 * tx, 0.125 * ty);
      cairo_rel_line_to(cr, (0.25 - length) * vx, (0.25 - length) * vy);
      cairo_close_path(cr);
      cairo_set_source_rgba(cr, arrow->color.red, arrow->color.green, arrow->color.blue, 0.5);
      cairo_fill(cr);
      if (has_colliding_content)
        cairo_restore(cr);
      priv->hud_has_content[hud] |= arrow->has_content[hud];
    }
  }

  cairo_destroy(cr);

  priv->hud_need_redraw[hud] = 0;
}

static void recreate_hud_layers(CwChessboard* chessboard)
{
  Dout(dc::cwchessboardwidget, "Calling recreate_hud_layers(" << chessboard << ")");

  CwChessboardPrivate* priv = chessboard->priv;

  for (guint hud = 0; hud < number_of_hud_layers; ++hud)
  {
    if (priv->hud_layer_surface[hud])
      cairo_surface_destroy(priv->hud_layer_surface[hud]);

    // (Re)create the HUD layer.
    priv->hud_layer_surface[hud] = cairo_surface_create_similar(cairo_get_target(priv->cr),
	CAIRO_CONTENT_COLOR_ALPHA, squares * priv->sside, squares * priv->sside);

    priv->hud_has_content[hud] = 0;
    priv->hud_need_redraw[hud] = (guint64)-1;
  }
  // Because we need to expose all squares.
  invalidate_board(chessboard);
}

// CwChessboard API

// Create and return a new chessboard widget.
GtkWidget* cw_chessboard_new(void)
{
  return (GtkWidget*)g_object_new(CW_TYPE_CHESSBOARD, NULL);
}

CwChessboardColorHandle
cw_chessboard_allocate_color_handle_rgb(CwChessboard* chessboard, gdouble red, gdouble green, gdouble blue)
{
  CwChessboardPrivate* priv = chessboard->priv;
  guint32 bit = 1;
  guint color_index = 0;
  while ((priv->allocated_colors_mask & bit))
  {
    bit <<= 1;
    ++color_index;
  }
  g_assert(color_index < G_N_ELEMENTS(priv->color_palet));
  priv->allocated_colors_mask |= bit;
  priv->color_palet[color_index].red = red;
  priv->color_palet[color_index].green = green;
  priv->color_palet[color_index].blue = blue;
  return color_index + 1;
}

void cw_chessboard_free_color_handle(CwChessboard* chessboard, CwChessboardColorHandle handle)
{
  CwChessboardPrivate* priv = chessboard->priv;
  g_assert(handle > 0);
  guint color_index = handle - 1;
  g_assert(color_index < G_N_ELEMENTS(priv->color_palet));
  guint32 bit = 1 << color_index;
  g_assert((priv->allocated_colors_mask & bit) != 0);
  priv->allocated_colors_mask &= ~bit;
}

void cw_chessboard_set_marker_color(CwChessboard* chessboard,
    gint col, gint row, CwChessboardColorHandle mahandle)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_set_marker_color(" <<
      chessboard << ", " << col << ", " << row << ", " << (int)mahandle << ")");
  CwChessboardPrivate* priv = chessboard->priv;
  BoardIndex index = convert_colrow2index(col, row);
  CwChessboardCode old_code = priv->board_codes[index];
  priv->board_codes[index] = convert_mahandle2code(mahandle) | (old_code & ~mahandle_mask);
  invalidate_square(chessboard, col, row);
}

CwChessboardColorHandle cw_chessboard_get_marker_color(CwChessboard* chessboard, gint col, gint row)
{
  return convert_code2mahandle(chessboard->priv->board_codes[convert_colrow2index(col, row)]);
}

void cw_chessboard_set_marker_thickness(CwChessboard* chessboard, gdouble thickness)
{
  CwChessboardPrivate* priv = chessboard->priv;
  priv->marker_thickness = MIN(MAX(0, thickness), 0.5);
  priv->marker_thickness_px = MAX(1, MIN((gint)round(priv->marker_thickness * priv->sside), priv->sside / 2));
  invalidate_markers(chessboard);
}

gdouble cw_chessboard_get_marker_thickness(CwChessboard* chessboard)
{
  return chessboard->priv->marker_thickness;
}

void cw_chessboard_set_marker_level(CwChessboard* chessboard, gboolean below)
{
  chessboard->priv->marker_below = below;
}

void cw_chessboard_set_background_color(CwChessboard* chessboard, gint col, gint row, CwChessboardColorHandle bghandle)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_set_background_color(" <<
      chessboard << ", " << col << ", " << row << ", " << (int)bghandle << ")");
  CwChessboardPrivate* priv = chessboard->priv;
  BoardIndex index = convert_colrow2index(col, row);
  CwChessboardCode old_code = priv->board_codes[index];
  priv->board_codes[index] = convert_bghandle2code(bghandle) | (old_code & ~bghandle_mask);
  invalidate_square(chessboard, col, row);
}

CwChessboardColorHandle cw_chessboard_get_background_color(CwChessboard* chessboard, gint col, gint row)
{
  return convert_code2bghandle(chessboard->priv->board_codes[convert_colrow2index(col, row)]);
}

void cw_chessboard_set_background_colors(CwChessboard* chessboard, CwChessboardColorHandle const* handles)
{
  // Read through the whole array, only calling cw_chessboard_set_background_color when the value changed.
  CwChessboardCode* board_codes = chessboard->priv->board_codes;
  for (int i = 0; i < 64; ++i)
    if (convert_code2bghandle(board_codes[i]) != handles[i])
      cw_chessboard_set_background_color(chessboard, convert_index2column(i), convert_index2row(i), handles[i]);
}

void cw_chessboard_get_background_colors(CwChessboard* chessboard, CwChessboardColorHandle* handles)
{
  CwChessboardCode* board_codes = chessboard->priv->board_codes;
  for (int i = 0; i < 64; ++i)
    handles[i] = convert_code2bghandle(board_codes[i]);
}

void cw_chessboard_set_square(CwChessboard* chessboard, gint col, gint row, CwChessboardCode code)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_set_square(" <<
      chessboard << ", " << col << ", " << row << ", " << (int)code << ")");
  gint index = convert_colrow2index(col, row);
  CwChessboardCode* board_codes = chessboard->priv->board_codes;
  CwChessboardCode old_code = board_codes[index];
  if (old_code != code)
  {
    board_codes[index] = (old_code & ~piece_color_mask) | (code & piece_color_mask);
    invalidate_square(chessboard, col, row);
  }
}

CwChessboardCode cw_chessboard_get_square(CwChessboard* chessboard, gint col, gint row)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_get_square(" << chessboard << ", " << col << ", " << row << ")");
  gint index = convert_colrow2index(col, row);
  CwChessboardCode* board_codes = chessboard->priv->board_codes;
  return board_codes[index] & piece_color_mask;
}

void cw_chessboard_set_draw_border(CwChessboard* chessboard, gboolean draw)
{
  if (chessboard->priv->draw_border != draw)
  {
    chessboard->priv->draw_border = draw;
    if (GTK_WIDGET_REALIZED(chessboard))
      redraw_pixmap(GTK_WIDGET(chessboard));
  }
}

gboolean cw_chessboard_get_draw_border(CwChessboard* chessboard)
{
  return chessboard->priv->draw_border;
}

void cw_chessboard_set_flip_board(CwChessboard* chessboard, gboolean flip)
{
  if (chessboard->priv->flip_board != flip)
  {
    *(gboolean*)(&chessboard->flip_board) = chessboard->priv->flip_board = flip;
    if (GTK_WIDGET_REALIZED(chessboard))
    {
      redraw_border(chessboard);
      for (guint hud = 0; hud < G_N_ELEMENTS(chessboard->priv->hud_need_redraw); ++hud)
      {
	chessboard->priv->hud_need_redraw[hud] = (guint64)-1;
	chessboard->priv->hud_has_content[hud] = (guint64)-1;
      }
      invalidate_board(chessboard);
    }
  }
}

gboolean cw_chessboard_get_flip_board(CwChessboard* chessboard)
{
  return chessboard->priv->flip_board;
}

void cw_chessboard_set_draw_turn_indicators(CwChessboard* chessboard, gboolean draw)
{
  CwChessboardPrivate* priv = chessboard->priv;
  if (priv->draw_turn_indicators != draw)
  {
    priv->draw_turn_indicators = draw;
    if (GTK_WIDGET_REALIZED(chessboard) && priv->border_width)
    {
      CW_CHESSBOARD_GET_CLASS(chessboard)->draw_turn_indicator(chessboard, priv->active_turn_indicator, priv->draw_turn_indicators);
      invalidate_turn_indicators(chessboard);
    }
  }
}

gboolean cw_chessboard_get_draw_turn_indicators(CwChessboard* chessboard)
{
  return chessboard->priv->draw_turn_indicators;
}

void cw_chessboard_set_active_turn_indicator(CwChessboard* chessboard, gboolean white)
{
  CwChessboardPrivate* priv = chessboard->priv;
  if (priv->active_turn_indicator != white)
  {
    priv->active_turn_indicator = white;
    if (GTK_WIDGET_REALIZED(chessboard) && priv->border_width && priv->draw_turn_indicators)
    {
      CW_CHESSBOARD_GET_CLASS(chessboard)->draw_turn_indicator(chessboard, TRUE, white);
      CW_CHESSBOARD_GET_CLASS(chessboard)->draw_turn_indicator(chessboard, FALSE, !white);
      invalidate_turn_indicators(chessboard);
    }
  }
}

gboolean cw_chessboard_get_active_turn_indicator(CwChessboard* chessboard)
{
  return chessboard->priv->active_turn_indicator;
}

void cw_chessboard_set_dark_square_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->dark_square_color_pixel = color->pixel;
  chessboard->priv->dark_square_color.red = color->red / 65535.0;
  chessboard->priv->dark_square_color.green = color->green / 65535.0;
  chessboard->priv->dark_square_color.blue = color->blue / 65535.0;
  invalidate_board(chessboard);
}

void cw_chessboard_get_dark_square_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->dark_square_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->dark_square_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->dark_square_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->dark_square_color.blue);
}

void cw_chessboard_set_light_square_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->light_square_color_pixel = color->pixel;
  chessboard->priv->light_square_color.red = color->red / 65535.0;
  chessboard->priv->light_square_color.green = color->green / 65535.0;
  chessboard->priv->light_square_color.blue = color->blue / 65535.0;
  invalidate_board(chessboard);
}

void cw_chessboard_get_light_square_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->light_square_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->light_square_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->light_square_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->light_square_color.blue);
}

void cw_chessboard_set_border_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->board_border_color_pixel = color->pixel;
  chessboard->priv->board_border_color.red = color->red / 65535.0;
  chessboard->priv->board_border_color.green = color->green / 65535.0;
  chessboard->priv->board_border_color.blue = color->blue / 65535.0;
  if (GTK_WIDGET_REALIZED(chessboard))
    redraw_border(chessboard);
}

void cw_chessboard_get_border_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->board_border_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->board_border_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->board_border_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->board_border_color.blue);
}

void cw_chessboard_set_white_fill_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->white_piece_fill_color_pixel = color->pixel;
  chessboard->priv->white_piece_fill_color.red = color->red / 65535.0;
  chessboard->priv->white_piece_fill_color.green = color->green / 65535.0;
  chessboard->priv->white_piece_fill_color.blue = color->blue / 65535.0;
  if (GTK_WIDGET_REALIZED(chessboard))
    redraw_pieces(chessboard);
}

void cw_chessboard_get_white_fill_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->white_piece_fill_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->white_piece_fill_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->white_piece_fill_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->white_piece_fill_color.blue);
}

void cw_chessboard_set_white_line_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->white_piece_line_color_pixel = color->pixel;
  chessboard->priv->white_piece_line_color.red = color->red / 65535.0;
  chessboard->priv->white_piece_line_color.green = color->green / 65535.0;
  chessboard->priv->white_piece_line_color.blue = color->blue / 65535.0;
  if (GTK_WIDGET_REALIZED(chessboard))
    redraw_pieces(chessboard);
}

void cw_chessboard_get_white_line_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->white_piece_line_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->white_piece_line_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->white_piece_line_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->white_piece_line_color.blue);
}

void cw_chessboard_set_black_fill_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->black_piece_fill_color_pixel = color->pixel;
  chessboard->priv->black_piece_fill_color.red = color->red / 65535.0;
  chessboard->priv->black_piece_fill_color.green = color->green / 65535.0;
  chessboard->priv->black_piece_fill_color.blue = color->blue / 65535.0;
  if (GTK_WIDGET_REALIZED(chessboard))
    redraw_pieces(chessboard);
}

void cw_chessboard_get_black_fill_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->black_piece_fill_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->black_piece_fill_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->black_piece_fill_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->black_piece_fill_color.blue);
}

void cw_chessboard_set_black_line_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->black_piece_line_color_pixel = color->pixel;
  chessboard->priv->black_piece_line_color.red = color->red / 65535.0;
  chessboard->priv->black_piece_line_color.green = color->green / 65535.0;
  chessboard->priv->black_piece_line_color.blue = color->blue / 65535.0;
  if (GTK_WIDGET_REALIZED(chessboard))
    redraw_pieces(chessboard);
}

void cw_chessboard_get_black_line_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->black_piece_line_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->black_piece_line_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->black_piece_line_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->black_piece_line_color.blue);
}

void cw_chessboard_set_cursor_color(CwChessboard* chessboard, GdkColor const* color)
{
  chessboard->priv->cursor_color_pixel = color->pixel;
  chessboard->priv->cursor_color.red = color->red / 65535.0;
  chessboard->priv->cursor_color.green = color->green / 65535.0;
  chessboard->priv->cursor_color.blue = color->blue / 65535.0;
  invalidate_cursor(chessboard);
}

void cw_chessboard_get_cursor_color(CwChessboard* chessboard, GdkColor* color)
{
  color->pixel = chessboard->priv->cursor_color_pixel;
  color->red = (guint16)round(65535.0 * chessboard->priv->cursor_color.red);
  color->green = (guint16)round(65535.0 * chessboard->priv->cursor_color.green);
  color->blue = (guint16)round(65535.0 * chessboard->priv->cursor_color.blue);
}

void cw_chessboard_set_cursor_thickness(CwChessboard* chessboard, gdouble thickness)
{
  CwChessboardPrivate* priv = chessboard->priv;
  priv->cursor_thickness = MIN(MAX(0, thickness), 0.5);
  priv->cursor_thickness_px = MAX(1, MIN((gint)round(priv->cursor_thickness * priv->sside), priv->sside / 2));
  invalidate_cursor(chessboard);
}

gdouble cw_chessboard_get_cursor_thickness(CwChessboard* chessboard)
{
  return chessboard->priv->cursor_thickness;
}

void cw_chessboard_show_cursor(CwChessboard* chessboard)
{
  GtkWidget* widget = GTK_WIDGET(chessboard);
  CwChessboardPrivate* priv = chessboard->priv;
  if (priv->show_cursor)
    return;
  priv->show_cursor = TRUE;
  gint x;
  gint y;
  Dout(dc::cwchessboardwidget, "Calling gdk_window_get_pointer()");
  gdk_window_get_pointer(widget->window, &x, &y, NULL);
  update_cursor_position(chessboard, x, y, TRUE);
}

void cw_chessboard_hide_cursor(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  invalidate_cursor(chessboard);
  priv->show_cursor = FALSE;
}

void cw_chessboard_move_floating_piece(CwChessboard* chessboard, gint handle, gdouble x, gdouble y)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_move_floating_piece(" <<
      chessboard << ", " << (int)handle << ", " << x << ", " << y << ")");

  GtkWidget* widget = GTK_WIDGET(chessboard);
  CwChessboardPrivate* priv = chessboard->priv;
  GdkRectangle rect;

  g_assert(handle >= 0 && (gsize)handle < G_N_ELEMENTS(priv->floating_piece));
  g_assert(!is_empty_square(priv->floating_piece[handle].code));

  // If x (y) is non-integer, this should really be priv->sside + 1, however
  // it is safe to use just sside at all times because pieces never extend
  // all the way to the border of a square: there is nothing drawn there,
  // so there is no reason to invalidate it.
  rect.width = priv->sside;
  rect.height = priv->sside;
  rect.x = priv->floating_piece[handle].pixmap_x + priv->top_left_pixmap_x;
  rect.y = priv->floating_piece[handle].pixmap_y + priv->top_left_pixmap_y;
  gdk_window_invalidate_rect(widget->window, &rect, FALSE);
#if CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  gint col = cw_chessboard_x2col(chessboard, rect.x);
  gint row = cw_chessboard_y2row(chessboard, rect.y);
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
  col += priv->flip_board ? -1 : 1;
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
  row += priv->flip_board ? 1 : -1;
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
  col += priv->flip_board ? 1 : -1;
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
#endif
  gboolean outside_window =
      rect.x + rect.width < 0 ||
      rect.x > widget->allocation.x ||
      rect.y + rect.height < 0 ||
      rect.y > widget->allocation.y;
  // Redraw background of widget if the old place of the floating piece is outside the board.
  priv->redraw_background = priv->redraw_background ||
      rect.x < priv->top_left_pixmap_x ||
      rect.x + rect.width > priv->bottom_right_pixmap_x ||
      rect.y < priv->top_left_pixmap_y ||
      rect.y + rect.height > priv->bottom_right_pixmap_y;
  rect.x = (gint)trunc(x - 0.5 * priv->sside);
  rect.y = (gint)trunc(y - 0.5 * priv->sside);
#if CW_CHESSBOARD_FLOATING_PIECE_INVALIDATE_TARGET || CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  gdk_window_invalidate_rect(widget->window, &rect, FALSE);
  outside_window = outside_window &&
      (rect.x + rect.width < 0 ||
      rect.x > widget->allocation.x ||
      rect.y + rect.height < 0 ||
      rect.y > widget->allocation.y);
#endif
  if (outside_window && priv->floating_piece[handle].pointer_device)
  {
    Dout(dc::cwchessboardwidget, "Calling gdk_window_get_pointer()");
    gdk_window_get_pointer(widget->window, NULL, NULL, NULL);
  }
  priv->floating_piece[handle].pixmap_x = rect.x - priv->top_left_pixmap_x;
  priv->floating_piece[handle].pixmap_y = rect.y - priv->top_left_pixmap_y;
  priv->floating_piece[handle].moved = TRUE;
}

gint cw_chessboard_add_floating_piece(CwChessboard* chessboard, CwChessboardCode code,
    gdouble x, gdouble y, gboolean pointer_device)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_add_floating_piece(" << chessboard << ", code:" << code <<
      ", x:" << x << ", y:" << y << ", " << pointer_device << ")");

  CwChessboardPrivate* priv = chessboard->priv;

  // There can't be more than 32 floating pieces.
  g_assert(priv->number_of_floating_pieces < G_N_ELEMENTS(priv->floating_piece));

  priv->number_of_floating_pieces++;
  gint handle = 0;
  while (priv->floating_piece[handle].code != empty_square)
    ++handle;
  GdkRectangle rect;
  rect.x = (gint)trunc(x - 0.5 * priv->sside);
  rect.y = (gint)trunc(y - 0.5 * priv->sside);
  priv->floating_piece[handle].code = code & piece_color_mask;
  priv->floating_piece[handle].pixmap_x = rect.x - priv->top_left_pixmap_x;
  priv->floating_piece[handle].pixmap_y = rect.y - priv->top_left_pixmap_y;
  priv->floating_piece[handle].moved = TRUE;
  if (priv->floating_piece_handle != -1)
    pointer_device = FALSE;	// Refuse to add two pointer devices at the same time.
  priv->floating_piece[handle].pointer_device = pointer_device;
  if (pointer_device)
    priv->floating_piece_handle = handle;
#if CW_CHESSBOARD_FLOATING_PIECE_INVALIDATE_TARGET || CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  // See remark in cw_chessboard_move_floating_piece.
  rect.width = priv->sside;
  rect.height = priv->sside;
  gdk_window_invalidate_rect(GTK_WIDGET(chessboard)->window, &rect, FALSE);
#else
  // FIXME: schedule an expose event instead of this:
  rect.width = priv->sside;
  rect.height = priv->sside;
  gdk_window_invalidate_rect(GTK_WIDGET(chessboard)->window, &rect, FALSE);
#endif

  Dout(dc::cwchessboardwidget, "number_of_floating_pieces = " << priv->number_of_floating_pieces);
  Dout(dc::cwchessboardwidget, "Allocated handle " << handle);

  return handle;
}

void cw_chessboard_remove_floating_piece(CwChessboard* chessboard, gint handle)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_remove_floating_piece(" << chessboard << ", handle:" << handle << ")");

  CwChessboardPrivate* priv = chessboard->priv;
  GdkRectangle rect;

  g_assert(handle >= 0 && (gsize)handle < G_N_ELEMENTS(priv->floating_piece));
  g_assert(!is_empty_square(priv->floating_piece[handle].code));

  // See remark in cw_chessboard_move_floating_piece.
  rect.width = priv->sside;
  rect.height = priv->sside;
  rect.x = priv->floating_piece[handle].pixmap_x + priv->top_left_pixmap_x;
  rect.y = priv->floating_piece[handle].pixmap_y + priv->top_left_pixmap_y;
  gdk_window_invalidate_rect(GTK_WIDGET(chessboard)->window, &rect, FALSE);
#if CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  gint col = cw_chessboard_x2col(chessboard, rect.x);
  gint row = cw_chessboard_y2row(chessboard, rect.y);
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
  col += priv->flip_board ? -1 : 1;
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
  row += priv->flip_board ? 1 : -1;
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
  col += priv->flip_board ? 1 : -1;
  if (is_inside_board(col, row))
  {
    BoardIndex index = convert_colrow2index(col, row);
    guint64 redraw_mask = 1;
    priv->need_redraw |= (redraw_mask << index);
  }
#endif
  // Redraw background of widget if the old place of the floating piece is outside the board.
  priv->redraw_background = priv->redraw_background ||
      rect.x < priv->top_left_pixmap_x ||
      rect.x + rect.width > priv->bottom_right_pixmap_x ||
      rect.y < priv->top_left_pixmap_y ||
      rect.y + rect.height > priv->bottom_right_pixmap_y;
  if (priv->floating_piece[handle].pointer_device)
    priv->floating_piece_handle = -1;
  priv->number_of_floating_pieces--;
  priv->floating_piece[handle].code = empty_square;
  Dout(dc::cwchessboardwidget, "number_of_floating_pieces = " << priv->number_of_floating_pieces);
}

CwChessboardCode cw_chessboard_get_floating_piece(CwChessboard* chessboard, gint handle)
{
  g_assert(handle >= 0 && (gsize)handle < G_N_ELEMENTS(chessboard->priv->floating_piece));
  return chessboard->priv->floating_piece[handle].code;
}

void cw_chessboard_enable_hud_layer(CwChessboard* chessboard, guint hud)
{
  g_return_if_fail(hud < number_of_hud_layers);
  chessboard->priv->has_hud_layer[hud] = TRUE;
  chessboard->priv->hud_need_redraw[hud] = (guint64)-1;
}

void cw_chessboard_disable_hud_layer(CwChessboard* chessboard, guint hud)
{
  g_return_if_fail(hud < number_of_hud_layers);
  chessboard->priv->has_hud_layer[hud] = FALSE;
  chessboard->priv->hud_need_redraw[hud] = (guint64)-1;
}

static guint64 invalidate_arrow(CwChessboard* chessboard, gint col1, gint row1, gint col2, gint row2)
{
  Dout(dc::cwchessboardwidget|continued_cf, "Calling invalidate_arrow(" << chessboard << ", " <<
      col1 << ", " << row1 << ", " << col2 << ", " << row2 << ") = ");
  guint64 result = 0;
  if (col1 == col2)			// Vertical arrow?
  {
    if (row1 > row2)
    {
      gint tmp = row1;
      row1 = row2;
      row2 = tmp;
    }
    guint64 bit = (guint64)1 << convert_colrow2index(col1, row1);
    for (gint row = row1; row <= row2; ++row, bit <<= 8)
    {
      result |= bit;
      invalidate_square(chessboard, col1, row);
    }
    Dout(dc::finish, std::hex << result);
    return result;
  }
  else if (row1 == row2)		// Horizontal arrow?
  {
    if (col1 > col2)
    {
      gint tmp = col1;
      col1 = col2;
      col2 = tmp;
    }
    guint64 bit = (guint64)1 << convert_colrow2index(col1, row1);
    for (gint col = col1; col <= col2; ++col, bit <<= 1)
    {
      result |= bit;
      invalidate_square(chessboard, col, row1);
    }
    Dout(dc::finish, std::hex << result);
    return result;
  }
  if (row1 > row2)
  {
    // Swap point 1 and 2.
    gint tmp;
    tmp = col1; col1 = col2; col2 = tmp;
    tmp = row1; row1 = row2; row2 = tmp;
  }
  double const arrow_width = 0.125;	// FIXME: must be half the real arrow thickness.
  double delta = arrow_width *
      sqrt((row2 - row1) * (row2 - row1) + (col2 - col1) * (col2 - col1)) / (row2 - row1);
  gint col_start = col1;
  gint row = row1;
  result = 0;
  guint64 sign = 1;
  if (col1 > col2)
  {
    sign = 2;
    delta = -delta;
  }
  for (double r = row1 + 0.5; r < row2; r += 1.0)
  {
    double c = col1 + (r - row1) * (col2 - col1) / (row2 - row1);
    gint col_end = (gint)round(c + delta);
    guint64 mask1 = (sign << convert_colrow2index(col_start, row)) - 1;
    guint64 mask2 = ((3 - sign) << convert_colrow2index(col_end, row)) - 1;
    result |= mask1 ^ mask2;
    col_start = (gint)round(c - delta);
    ++row;
  }
  guint64 mask1 = (sign << convert_colrow2index(col_start, row)) - 1;
  guint64 mask2 = ((3 - sign) << convert_colrow2index(col2, row)) - 1;
  result |= mask1 ^ mask2;
  Dout(dc::finish, std::hex << result);
  guint64 bit = 1;
  for (gint row = 0; row < squares; ++row)
    for (gint col = 0; col < squares; ++col, bit <<= 1)
      if ((result & bit))  
        invalidate_square(chessboard, col, row);
  return result;
}

gpointer cw_chessboard_add_arrow(CwChessboard* chessboard,
    gint begin_col, gint begin_row, gint end_col, gint end_row, GdkColor const* color)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_add_arrow(" << chessboard << ", " <<
      begin_col << ", " << begin_row << ", " << end_col << ", " << end_row << ", " << color << ")");

  g_return_val_if_fail(begin_col != end_col || begin_row != end_row, NULL);
  g_return_val_if_fail(is_inside_board(begin_col, begin_row) && is_inside_board(end_col, end_row), NULL);

  Arrow* arrow = (Arrow*)g_malloc(sizeof(Arrow));
  g_ptr_array_add(chessboard->priv->arrows, arrow);  
  arrow->begin_col = begin_col;
  arrow->begin_row = begin_row;
  arrow->end_col = end_col;
  arrow->end_row = end_row;
  arrow->color.red = color->red / 65535.0;
  arrow->color.green = color->green / 65535.0;
  arrow->color.blue = color->blue / 65535.0;
  guint64 content = invalidate_arrow(chessboard, begin_col, begin_row, end_col, end_row);
  guint64 start_square = (guint64)1 << convert_colrow2index(begin_col, begin_row);
  chessboard->priv->hud_need_redraw[0] |= (arrow->has_content[0] = start_square);
  chessboard->priv->hud_need_redraw[1] |= (arrow->has_content[1] = content ^ start_square);
  return arrow;
}

void cw_chessboard_remove_arrow(CwChessboard* chessboard, gpointer ptr)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_remove_arrow(" << chessboard << ", " << ptr << ")");
  if (g_ptr_array_remove_fast(chessboard->priv->arrows, ptr))
  {
    Arrow* arrow = (Arrow*)ptr;
    chessboard->priv->hud_need_redraw[0] |= arrow->has_content[0];
    chessboard->priv->hud_need_redraw[1] |= arrow->has_content[1];
    g_free(ptr);
  }
}

//------------------------------------------------------------------------------
// Piece graphics functions.

static double const black_line_width = CONST(black_line_width, 0.026);
static double const white_line_width = CONST(white_line_width, 1.5 * black_line_width);

static double snap_bottom(double y, double translate, double scale, double line_width)
{
  if (scale < 27)
    return y;
  return (round((y + 0.5 * line_width) * scale - translate) + translate) / scale - 0.5 * line_width;
}

static double snap_top(double y, double translate, double scale, double line_width)
{
  if (scale < 27)
    return y;
  return (round((y - 0.5 * line_width) * scale - translate) + translate) / scale + 0.5 * line_width;
}

static double snap_line_width(double line_width, double scale)
{
  if (line_width * scale < 1.0)
    return line_width;
  return trunc(line_width * scale + 0.3) / scale;
}

// The fill color of the pieces.
static void set_fill_color(cairo_t* cr, CwChessboardPrivate* priv, gboolean white)
{
  if (white)
    cairo_set_source_rgb(cr, priv->white_piece_fill_color.red,
                             priv->white_piece_fill_color.green,
			     priv->white_piece_fill_color.blue);
  else
    cairo_set_source_rgb(cr, priv->black_piece_fill_color.red,
                             priv->black_piece_fill_color.green,
			     priv->black_piece_fill_color.blue);
}

// The line color of the pieces.
static void set_line_color(cairo_t* cr, CwChessboardPrivate* priv, gboolean white)
{
  if (white)
    cairo_set_source_rgb(cr, priv->white_piece_line_color.red,
                             priv->white_piece_line_color.green,
			     priv->white_piece_line_color.blue);
  else
    cairo_set_source_rgb(cr, priv->black_piece_line_color.red,
                             priv->black_piece_line_color.green,
			     priv->black_piece_line_color.blue);
}

// Draw pawn.
void cw_chessboard_draw_pawn(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  static double const base_outside_diameter_cm = CONST(base_outside_diameter_cm, 3.265);
  static double const width_pawn_cm = CONST(width_pawn_cm, 5.31);
  static double const base_radius = CONST(base_radius, 0.5 * (base_outside_diameter_cm / width_pawn_cm - black_line_width));
  static double const mid_outside_diameter_cm = CONST(mid_outside_diameter_cm, 1.98);
  static double const mid_radius = CONST(mid_radius, 0.5 * (mid_outside_diameter_cm / width_pawn_cm - black_line_width));
  static double const head_outside_diameter_cm = CONST(head_outside_diameter_cm, 1.12);
  static double const head_radius = CONST(head_radius, 0.5 * (head_outside_diameter_cm / width_pawn_cm - black_line_width));
  static double const height_pawn_cm = CONST(height_pawn_cm, 5.43);
  static double const bottom_pawn_cm = CONST(bottom_pawn_cm, 0.58);
  static double const foot_height = CONST(foot_height, 0.0387);
  static double const base_y = CONST(base_y, 0.5 - bottom_pawn_cm / height_pawn_cm - 0.5 * black_line_width);
  static double const base_scale = CONST(base_scale, 0.931);
  static double const mid_y = CONST(mid_y, -0.0545);
  static double const top_offset_cm = CONST(top_offset_cm, 0.62);
  static double const head_y = CONST(head_y, -0.5 + top_offset_cm / height_pawn_cm + 0.5 * black_line_width + head_radius);

  static double const base_angle = CONST(base_angle, 1.148);
  static double const mid_angle1 = CONST(mid_angle1, 0.992);
  static double const inner_neck_width_cm = CONST(inner_neck_width_cm, 0.41);
  static double const neck_right = CONST(neck_right, 0.5 * (inner_neck_width_cm / width_pawn_cm + black_line_width));
  static double const head_angle = CONST(head_angle, asin(neck_right / head_radius));
  static double const mid_scale = CONST(mid_scale, (mid_y - (head_y + head_radius * cos(head_angle)) -
      0.1 * black_line_width) / sqrt(mid_radius * mid_radius - neck_right * neck_right));
  static double const mid_angle2 = CONST(mid_angle2, asin(head_radius * sin(head_angle) / mid_radius));

  double const base_y_sn = snap_bottom(base_y, y, scale, black_line_width);
  
  CwChessboardPrivate* priv = chessboard->priv;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw the left-side of the base.
  cairo_move_to(cr, -base_radius, base_y_sn);
  cairo_save(cr);
  cairo_translate(cr, 0.0, base_y_sn - foot_height);
  cairo_scale(cr, 1.0, base_scale);
  cairo_arc(cr, 0.0, 0.0, base_radius, -M_PI, -M_PI + base_angle);
  cairo_restore(cr);

  // Draw the left-side of the mid-section.
  cairo_save(cr);
  cairo_translate(cr, 0.0, mid_y);
  cairo_scale(cr, 1.0, mid_scale);
  cairo_arc(cr, 0.0, 0.0, mid_radius, -M_PI - mid_angle1, -0.5 * M_PI - mid_angle2);
  cairo_restore(cr);

  // Draw the head of the pawn.
  cairo_arc(cr, 0.0, head_y, head_radius, -1.5 * M_PI + head_angle, 0.5 * M_PI - head_angle);

  // Draw the right-side of the mid-section.
  cairo_save(cr);
  cairo_translate(cr, 0.0, mid_y);
  cairo_scale(cr, 1.0, mid_scale);
  cairo_arc(cr, 0.0, 0.0, mid_radius,  -0.5 * M_PI + mid_angle2, mid_angle1);
  cairo_restore(cr);

  // Draw the right-side of the base.
  cairo_save(cr);
  cairo_translate(cr, 0.0, base_y_sn - foot_height);
  cairo_scale(cr, 1.0, base_scale);
  cairo_arc(cr, 0.0, 0.0, base_radius, -base_angle, 0.0);
  cairo_restore(cr);
  cairo_line_to(cr, base_radius, base_y_sn);

  // Draw the base line of the pawn, right to left.
  cairo_close_path(cr);

   set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void cw_chessboard_draw_king(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const blob_left_cm = CONST(blob_left_cm, 1.22);
  static double const band_edge_left_cm = CONST(band_edge_left_cm, 2.55);
  static double const band_left_cm = CONST(band_left_cm, 2.67);
  static double const inside_left_cm = CONST(inside_left_cm, 3.06);
  static double const center_blob_left_cm = CONST(center_blob_left_cm, 4.525);
  static double const cross_left_cm = CONST(cross_left_cm, 4.71);
  static double const width_king_cm = CONST(width_king_cm, 10.67);
  static double const bottom_king_cm = CONST(bottom_king_cm, 1.155);
  static double const band_line_top_cm = CONST(band_line_top_cm, 2.95);
  static double const band_top_king_cm = CONST(band_top_king_cm, 4.04);
  static double const center_y_cm = CONST(center_y_cm, 5.02);
  static double const blob_top_cm = CONST(blob_top_cm, 7.4); // 7.28
  static double const center_blob_top_cm = CONST(center_blob_top_cm, 8.18);
  static double const cross_y_king_cm = CONST(cross_y_king_cm, 9.17);
  static double const cross_top_cm = CONST(cross_top_cm, 9.86);
  static double const height_king_cm = CONST(height_king_cm, 10.86);
  // Derived values.
  static double const mid_x_king_cm = CONST(mid_x_king_cm, width_king_cm / 2);
  static double const mid_y_king_cm = CONST(mid_y_king_cm, height_king_cm / 2);

  // Same, in coordinates.
  static double const blob_left = CONST(blob_left, (blob_left_cm - mid_x_king_cm) / width_king_cm);
  static double const band_edge_left = CONST(band_edge_left, (band_edge_left_cm - mid_x_king_cm) / width_king_cm);
  static double const band_left = CONST(band_left, (band_left_cm - mid_x_king_cm) / width_king_cm);
  static double const inside_left = CONST(inside_left, (inside_left_cm - mid_x_king_cm) / width_king_cm);
  static double const center_blob_left = CONST(center_blob_left, (center_blob_left_cm - mid_x_king_cm) / width_king_cm);
  static double const cross_left = CONST(cross_left, (cross_left_cm - mid_x_king_cm) / width_king_cm);
  static double const bottom_king = CONST(bottom_king, (mid_y_king_cm - bottom_king_cm) / height_king_cm);
  static double const band_line_top = CONST(band_line_top, (mid_y_king_cm - band_line_top_cm) / height_king_cm);
  static double const band_top_king = CONST(band_top_king, (mid_y_king_cm - band_top_king_cm) / height_king_cm);
  static double const center_y = CONST(center_y, (mid_y_king_cm - center_y_cm) / height_king_cm);
  static double const blob_top = CONST(blob_top, (mid_y_king_cm - blob_top_cm) / height_king_cm);
  static double const center_blob_top = CONST(center_blob_top, (mid_y_king_cm - center_blob_top_cm) / height_king_cm);
  static double const cross_y_king = CONST(cross_y_king, (mid_y_king_cm - cross_y_king_cm) / height_king_cm);
  static double const cross_top = CONST(cross_top, (mid_y_king_cm - cross_top_cm) / height_king_cm);

  // Derived values.
  static double const inside_radius_king = CONST(inside_radius_king, -inside_left);
  static double const inside_scale_king = CONST(inside_scale_king, 0.180132); // Same value as used for the queen.
  static double const band_top_radius = CONST(band_top_radius, -band_edge_left);
  static double const band_top_scale = CONST(band_top_scale, inside_scale_king);
  static double const band_top_y = CONST(band_top_y, band_top_king + band_top_radius * band_top_scale);
  static double const cos_alpha = CONST(cos_alpha, band_left / band_edge_left);
  static double const alpha = CONST(alpha, acos(cos_alpha));
  static double const band_bottom_scale = CONST(band_bottom_scale, inside_scale_king);
  static double const band_bottom_radius = CONST(band_bottom_radius, band_top_radius);
  static double const band_bottom_y = CONST(band_bottom_y, bottom_king - band_bottom_radius * band_bottom_scale);
  static double const dx = CONST(dx, band_top_radius * (1.0 - cos_alpha));
  static double const band_line_scale = CONST(band_line_scale, band_top_scale);
  static double const band_line_radius = CONST(band_line_radius, band_top_radius - dx);
  static double const band_line_y = CONST(band_line_y, band_line_top + band_line_radius * band_line_scale);
  static double const blob_radius = CONST(blob_radius, 0.7071067 * (blob_left + band_top_y - band_left - blob_top));
  static double const blob_x = CONST(blob_x, blob_left + blob_radius);
  static double const blob_y = CONST(blob_y, blob_top + blob_radius);
  static double const center_blob_radius = CONST(center_blob_radius, -center_blob_left);
  static double const center_blob_y = CONST(center_blob_y, center_blob_top + center_blob_radius);
  // Manual adjustment... looks better.
  static double const adjusted_center_blob_radius = CONST(adjusted_center_blob_radius, center_blob_radius + 0.01);
  static double const beta_king = CONST(beta_king, asin(adjusted_center_blob_radius / (center_y - center_blob_y)));
  static double const center2_y = CONST(center2_y, blob_y - blob_x - 1.4142136 * blob_radius);

  CwChessboardPrivate* priv = chessboard->priv;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw left blob.
  cairo_move_to(cr, band_left, band_top_y);
  cairo_arc(cr, blob_x, blob_y, blob_radius, 0.75 * M_PI, 1.75 * M_PI);
  cairo_line_to(cr, 0.0, center2_y);

  // Draw right blob.
  cairo_arc(cr, -blob_x, blob_y, blob_radius, -0.75 * M_PI, 0.25 * M_PI);
  cairo_line_to(cr, -band_left, band_top_y);

   set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);

  // Draw vertical line in the middle.
  cairo_move_to(cr, 0.0, band_top_y);
  cairo_line_to(cr, 0.0, center_y);

  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  // Draw center blob.
  cairo_move_to(cr, 0.0, center_y);
  cairo_arc(cr, 0.0, center_blob_y, adjusted_center_blob_radius, M_PI - beta_king, beta_king);
  cairo_close_path(cr);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  // Draw cross.
  cairo_move_to(cr, 0.0, center_blob_y - adjusted_center_blob_radius);
  cairo_line_to(cr, 0.0, cross_top); 
  cairo_move_to(cr, cross_left, cross_y_king);
  cairo_line_to(cr, -cross_left, cross_y_king); 
  cairo_stroke(cr);

  // Draw half ellipse just below the blobs.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_top_y);
  cairo_scale(cr, 1.0, band_top_scale);
  cairo_arc(cr, 0.0, 0.0, band_top_radius, M_PI - alpha, 2 * M_PI + alpha);
  cairo_restore(cr);

  // Draw right side of the upper band.
  cairo_line_to(cr, -band_left, band_line_y);

  // Draw right side of lower band and bottom.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_bottom_y);
  cairo_scale(cr, 1.0, band_bottom_scale);
  cairo_arc(cr, 0.0, 0.0, band_bottom_radius, 0.0, M_PI);
  cairo_restore(cr);

  // Draw left side of lower band.
  cairo_line_to(cr, band_left, band_line_y);

  // Draw left side of upper band.
  cairo_close_path(cr);

  cairo_path_t* path = cairo_copy_path(cr);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill(cr);

  // Draw lower half ellipse of upper band.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_line_y);
  cairo_scale(cr, 1.0, band_line_scale);
  cairo_arc(cr, 0.0, 0.0, band_line_radius, -M_PI, 0.0);
  cairo_restore(cr);

  cairo_new_sub_path(cr);

  // Draw opening at bottom, align it with the real bottom.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_bottom_y + band_bottom_radius * band_bottom_scale - inside_radius_king * inside_scale_king);
  cairo_scale(cr, 1.0, inside_scale_king);
  if (white)
    cairo_arc(cr, 0.0, 0.0, inside_radius_king, -M_PI, M_PI);
  else
    cairo_arc(cr, 0.0, 0.0, inside_radius_king, -M_PI - alpha, alpha);
  cairo_restore(cr);

   set_line_color(cr, priv, white);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }

  cairo_append_path(cr, path);
  if (!white)
    set_fill_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_path_destroy(path);

  if (!white)
  {
    // Draw the white lines inside the blobs.

    static double const av_line_width = CONST(av_line_width, 0.5 * (black_line_width + white_line_width));
    static double const da = CONST(da, av_line_width / band_top_radius);
    static double const dy = CONST(dy, av_line_width * tan(0.5 * beta_king));

    cairo_save(cr);
    cairo_translate(cr, 0.0, band_top_y);
    cairo_scale(cr, 1.0, band_top_scale);
    cairo_arc_negative(cr, 0.0, 0.0, band_top_radius, -0.5 * M_PI - da, M_PI + alpha + da);
    cairo_restore(cr);

    cairo_arc(cr, blob_x, blob_y, blob_radius - av_line_width, 0.75 * M_PI, 1.75 * M_PI);

    static double const center2b_y = CONST(center2b_y, center2_y + av_line_width * 1.4142136);
    static double const sin_beta = CONST(sin_beta, adjusted_center_blob_radius / (center_y - center_blob_y));
    static double const x_king = CONST(x_king, sin_beta * (center_y + av_line_width / sin_beta - center2b_y) / sin(0.25 * M_PI - beta_king));
    static double const y_king = CONST(y_king, center2b_y - x_king);

    cairo_line_to(cr, -x_king, y_king);
    cairo_line_to(cr, -av_line_width, center_y + dy);

    cairo_close_path(cr);

    cairo_new_sub_path(cr);

    cairo_save(cr);
    cairo_translate(cr, 0.0, band_top_y);
    cairo_scale(cr, 1.0, band_top_scale);
    cairo_arc_negative(cr, 0.0, 0.0, band_top_radius, -alpha - da, -0.5 * M_PI + da);
    cairo_restore(cr);

    cairo_line_to(cr, av_line_width, center_y + dy);
    cairo_line_to(cr, x_king, y_king);
    
    cairo_arc(cr, -blob_x, blob_y, blob_radius - av_line_width, -0.75 * M_PI, 0.25 * M_PI);

    cairo_close_path(cr);

    cairo_new_sub_path(cr);

    cairo_move_to(cr, 0.0, center_y - av_line_width / sin_beta);
    cairo_arc(cr, 0.0, center_blob_y, adjusted_center_blob_radius - av_line_width, M_PI - beta_king, beta_king);

    cairo_close_path(cr);

     set_line_color(cr, priv, white);
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
  }

  cairo_restore(cr);
}

void cw_chessboard_draw_queen(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const width_queen_cm = CONST(width_queen_cm, 5.34);
  static double const inside_width_cm = CONST(inside_width_cm, 2.97);
  static double const band1_width_cm = CONST(band1_width_cm, 2.59);
  static double const crown_bottom_width_cm = CONST(crown_bottom_width_cm, 3.31);
  static double const height_queen_cm = CONST(height_queen_cm, 5.39);
  static double const bottom_queen_cm = CONST(bottom_queen_cm, 0.5);
  static double const inside_height_cm = CONST(inside_height_cm, 0.54);
  static double const band1_height_cm = CONST(band1_height_cm, 0.47);
  static double const band2_height_cm = CONST(band2_height_cm, 0.43);
  static double const tooth_outside_cm = CONST(tooth_outside_cm, 1.83);
  static double const tooth_inside_cm = CONST(tooth_inside_cm, 2.20);
  static double const tooth_inside2_cm = CONST(tooth_inside2_cm, 2.36);
  static double const ball_outside_diameter_cm = CONST(ball_outside_diameter_cm, 0.6);
  static double const ball_top1_cm = CONST(ball_top1_cm, 4.31);
  static double const ball_right1_cm = CONST(ball_right1_cm, 0.90);
  static double const ball_top2_cm = CONST(ball_top2_cm, 4.80);
  static double const ball_right2_cm = CONST(ball_right2_cm, 1.88);
  static double const tooth3_x_cm = CONST(tooth3_x_cm, 2.25);
  // Derived values.
  static double const mid_x_queen_cm = CONST(mid_x_queen_cm, width_queen_cm / 2);
  static double const mid_y_queen_cm = CONST(mid_y_queen_cm, height_queen_cm / 2);

  // Same, in coordinates.
  static double const inside_width = CONST(inside_width, inside_width_cm / width_queen_cm);
  static double const band1_width = CONST(band1_width, band1_width_cm / width_queen_cm);
  static double const crown_bottom_width = CONST(crown_bottom_width, crown_bottom_width_cm / width_queen_cm);
  static double const bottom_queen = CONST(bottom_queen, (mid_y_queen_cm - bottom_queen_cm) / height_queen_cm);
  static double const inside_height = CONST(inside_height, inside_height_cm / height_queen_cm);
  static double const band1_height = CONST(band1_height, band1_height_cm / height_queen_cm);
  static double const band2_height = CONST(band2_height, band2_height_cm / height_queen_cm);
  static double const tooth_outside = CONST(tooth_outside, (mid_y_queen_cm - tooth_outside_cm) / height_queen_cm);
  static double const tooth_inside = CONST(tooth_inside, (mid_y_queen_cm - tooth_inside_cm) / height_queen_cm);
  static double const tooth_inside2 = CONST(tooth_inside2, (mid_y_queen_cm - tooth_inside2_cm) / height_queen_cm);
  static double const ball_outside_diameter = CONST(ball_outside_diameter, ball_outside_diameter_cm / height_queen_cm);
  static double const ball_top1 = CONST(ball_top1, (mid_y_queen_cm - ball_top1_cm) / height_queen_cm);
  static double const ball_right1 = CONST(ball_right1, (ball_right1_cm - mid_x_queen_cm) / width_queen_cm);
  static double const ball_top2 = CONST(ball_top2, (mid_y_queen_cm - ball_top2_cm) / height_queen_cm);
  static double const ball_right2 = CONST(ball_right2, (ball_right2_cm - mid_x_queen_cm) / width_queen_cm);
  static double const tooth3_x = CONST(tooth3_x, (tooth3_x_cm - mid_x_queen_cm) / width_queen_cm);

  // Derived values.
  static double const inside_radius_queen = CONST(inside_radius_queen, inside_width / 2);
  static double const inside_scale_queen = CONST(inside_scale_queen, inside_height / inside_width);
  static double const inside_y_queen = CONST(inside_y_queen, bottom_queen - inside_radius_queen * inside_scale_queen);
  static double const band1_radius = CONST(band1_radius, band1_width / 2);
  static double const band1_scale = CONST(band1_scale, inside_scale_queen);
  static double const band1_y = CONST(band1_y, bottom_queen - inside_height - band1_height + band1_radius * band1_scale);
  static double const crown_bottom_left = CONST(crown_bottom_left, -crown_bottom_width / 2);
  static double const band2_radius = CONST(band2_radius ,band1_radius +
      (-band1_radius - crown_bottom_left) * band2_height / (band1_y - tooth_outside));
  static double const band2_scale = CONST(band2_scale, band1_scale);
  static double const band2_y = CONST(band2_y, bottom_queen - inside_height - band1_height - band2_height + band2_radius * band2_scale);
  static double const ball1_x = CONST(ball1_x, ball_right1 - ball_outside_diameter / 2);
  static double const ball2_x = CONST(ball2_x, ball_right2 - ball_outside_diameter / 2);
  static double const ball1_y = CONST(ball1_y, ball_top1 + ball_outside_diameter / 2);
  static double const ball2_y = CONST(ball2_y, ball_top2 + ball_outside_diameter / 2);
  static double const ball_radius_queen = CONST(ball_radius_queen, (ball_outside_diameter - black_line_width) / 2);
  // We calculate ball3_y, so it lays on a perfect circle with the other balls.
  // The distance from ballN to a point (0, ball_center_y) is:
  // sqrt(ballN_x^2 + (ballN_y - ball_center_y)^2), and we find
  // ball_center_y by setting this distance equal for ball1 and 2:
  // ball1_x^2 + ball1_y^2 - 2 ball1_y ball_center_y = ball2_x^2 + ball2_y^2 - 2 ball2_y ball_center_y -->
  static double const ball_center_y = CONST(ball_center_y,
      0.5 * (ball2_x * ball2_x + ball2_y * ball2_y - ball1_x * ball1_x - ball1_y * ball1_y) / (ball2_y - ball1_y));
  static double const ball3_y = CONST(ball3_y,
      ball_center_y - sqrt(ball1_x * ball1_x + (ball1_y - ball_center_y) * (ball1_y - ball_center_y)));
  // The tooth points are derived (which turns out better than measuring them).
  static double const ball1_angle = CONST(ball1_angle, atan((0.5 * (crown_bottom_left + ball2_x) - ball1_x) / (tooth_outside - ball1_y)));
  static double const tooth1_x = CONST(tooth1_x, ball1_x + ball_radius_queen * sin(ball1_angle));
  static double const tooth2_x = CONST(tooth2_x, ball2_x);
  static double const tooth1_top = CONST(tooth1_top, ball1_y + ball_radius_queen * cos(ball1_angle));
  static double const tooth2_top = CONST(tooth2_top, ball2_y + ball_radius_queen);
  static double const tooth3_top = CONST(tooth3_top, ball3_y + ball_radius_queen);

  CwChessboardPrivate* priv = chessboard->priv;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw the outline of the queen.
  // First fill, then stroke.
  for (int stroke = 0; stroke < 2; ++stroke)
  {
    // Draw the right side.
    cairo_move_to(cr, -tooth1_x, tooth1_top);
    cairo_line_to(cr, -crown_bottom_left, tooth_outside);
    cairo_line_to(cr, band1_radius, band1_y);
    // The call to arc() draws the last line part, to (inside_radius_queen, inside_y_queen).

    // Draw the half ellipse that makes out the bottom.
    cairo_save(cr);
    cairo_translate(cr, 0.0, inside_y_queen);
    cairo_scale(cr, 1.0, inside_scale_queen);
    cairo_arc(cr, 0.0, 0.0, inside_radius_queen, 0.0, M_PI);
    cairo_restore(cr);

    // Draw the left side.
    cairo_line_to(cr, -band1_radius, band1_y);
    cairo_line_to(cr, crown_bottom_left, tooth_outside);
    cairo_line_to(cr, tooth1_x, tooth1_top);

    // The lines of the teeth should not be 'connected' when we are stroking.
    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, tooth1_x, tooth1_top);
    }

    // Draw right-side of left-most tooth.
    cairo_line_to(cr, tooth2_x, tooth_inside);

    // Draw left-side of second tooth.
    cairo_line_to(cr, tooth2_x, tooth2_top);

    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, tooth2_x, tooth2_top);
    }
    
    // Draw right-side of second tooth.
    cairo_line_to(cr, tooth3_x, tooth_inside2);

    // Draw left-side of middle tooth.
    cairo_line_to(cr, 0.0, tooth3_top);

    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, 0.0, tooth3_top);
    }

    // Draw right-side of middle tooth.
    cairo_line_to(cr, -tooth3_x, tooth_inside2);

    // Draw left-side of fourth tooth.
    cairo_line_to(cr, -tooth2_x, tooth2_top);

    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, -tooth2_x, tooth2_top);
    }

    // Draw right-side of fourth tooth.
    cairo_line_to(cr, -tooth2_x, tooth_inside);

    // Draw left-side of last tooth.
    cairo_line_to(cr, -tooth1_x, tooth1_top);

    if (stroke)
    {
      if (white)
         set_line_color(cr, priv, white);
      else
         set_fill_color(cr, priv, white);
      cairo_stroke(cr);
    }
    else
    {
       set_fill_color(cr, priv, white);
      cairo_fill(cr);

      // Draw the upper side of the bottom ellipse.
      cairo_save(cr);
      cairo_translate(cr, 0.0, inside_y_queen);
      cairo_scale(cr, 1.0, inside_scale_queen);
      cairo_arc(cr, 0.0, 0.0, inside_radius_queen, -M_PI, 0.0);
      cairo_restore(cr);

      cairo_new_sub_path(cr);

      // Draw the half ellipse of band1.
      cairo_save(cr);
      cairo_translate(cr, 0.0, band1_y);
      cairo_scale(cr, 1.0, band1_scale);
      cairo_arc(cr, 0.0, 0.0, band1_radius, -M_PI, 0.0);
      cairo_restore(cr);

       set_line_color(cr, priv, white);
      if (white)
	cairo_stroke(cr);
      else
      {
	cairo_set_line_width(cr, white_line_width);
	cairo_stroke(cr);
	cairo_set_line_width(cr, black_line_width);
      }
    }
  }

  // Draw the five balls.

  cairo_arc(cr, ball1_x, ball1_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_arc(cr, ball2_x, ball2_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_arc(cr, 0.0, ball3_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_arc(cr, -ball2_x, ball2_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_arc(cr, -ball1_x, ball1_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
     set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  cairo_stroke(cr);

  if (white)
  {
    // Draw the splines at the bottom of the teeth.
    // The top left y-coordinate.
    static double const y0_queen = CONST(y0_queen, 0.0952);
    // The y-coordinate of the middle point.
    static double const ym = CONST(ym, 0.0331);
    // The top left y-coordinate lays on the left side of the first tooth.
    // Calculate the x-coordinate:
    static double const x0_queen = CONST(x0_queen, tooth1_x + (y0_queen - tooth1_top) * (crown_bottom_left - tooth1_x) / (tooth_outside - tooth1_top));
    // The (apparent) tilting angle.
    static double const tilt_angle = CONST(tilt_angle, atan((ym - y0_queen) / x0_queen));

    // The angle that the control lines make with the y-axis, before
    // mapping them onto a cylinder and before tilting the cylinder.
    static double const beta_queen = CONST(beta_queen, 1.202);
    // The length of the control lines.
    static double const len = CONST(len, 0.1728);
    // The y-value of the control points before tilting (relative to y0_queen).
    static double const py = CONST(py, len * cos(beta_queen));
    static double const y0_plus_py_cos_tilt_angle = CONST(y0_plus_py_cos_tilt_angle, y0_queen + py * cos(tilt_angle));
    static double const sin_tilt_angle = CONST(sin_tilt_angle, sin(tilt_angle));
    // The x-offset of the control points (this is an angle).
    static double px_offset = CONST(px_offset, len * sin(beta_queen));

    cairo_move_to(cr, crown_bottom_left, tooth_outside);
    cairo_line_to(cr, x0_queen, y0_queen);

    // We have to draw N splines.
    int const N = 4;
    for (int i = 0; i < N; ++i)
    {
      double const alpha = i * M_PI / N;
      // The spline points before tilting.
      double px2 = x0_queen * cos(alpha + px_offset);
      double pz2 = - x0_queen * sin(alpha + px_offset);
      double px3 = x0_queen * cos(alpha + M_PI / N - px_offset);
      double pz3 = - x0_queen * sin(alpha + M_PI / N - px_offset);
      double px4 = x0_queen * cos(alpha + M_PI / N);
      double pz4 = - x0_queen * sin(alpha + M_PI / N);
      // Calculate the tilted values. This only has influence on the y value
      // (we rotate around the x-axis, and the resulting z-value doesn't interest us).
      double tpy2 = y0_plus_py_cos_tilt_angle - pz2 * sin_tilt_angle;
      double tpy3 = y0_plus_py_cos_tilt_angle - pz3 * sin_tilt_angle;
      double tpy4 = y0_queen - pz4 * sin_tilt_angle;
      cairo_curve_to(cr, px2, tpy2, px3, tpy3, px4, tpy4);
    }

    cairo_line_to(cr, -crown_bottom_left, tooth_outside);
  }

  // Draw the half ellipse of band2.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band2_y);
  cairo_scale(cr, 1.0, band2_scale);
  cairo_arc_negative(cr, 0.0, 0.0, band2_radius, -0.15, -M_PI + 0.15);
  cairo_restore(cr);

  if (white)
  {
    cairo_close_path(cr);
     set_fill_color(cr, priv, white);
    cairo_fill_preserve(cr);
  }
  else
    cairo_set_line_width(cr, white_line_width);
   set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void cw_chessboard_draw_rook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const width_rook_cm = CONST(width_rook_cm, 5.33);
  static double const foot_left_cm = CONST(foot_left_cm, 0.90);
  static double const base_left_cm = CONST(base_left_cm, 1.26);
  static double const tower_left_cm = CONST(tower_left_cm, 1.64);
  static double const opening_left_cm = CONST(opening_left_cm, 1.795);
  static double const opening_right_cm = CONST(opening_right_cm, 2.315);
  static double const height_rook_cm = CONST(height_rook_cm, 5.30);
  static double const bottom_rook_cm = CONST(bottom_rook_cm, 0.58);
  static double const foot_top_cm = CONST(foot_top_cm, 0.95);
  static double const base_top_cm = CONST(base_top_cm, 1.41);
  static double const tower_bottom_cm = CONST(tower_bottom_cm, 1.76);
  static double const tower_top_cm = CONST(tower_top_cm, 3.43);
  static double const top_bottom_cm = CONST(top_bottom_cm, 3.81);
  static double const opening_bottom_cm = CONST(opening_bottom_cm, 4.25);
  // static double const top_top_cm = 4.61;

  // In coordinates.
  static double const foot_left = CONST(foot_left, -0.5 + foot_left_cm / width_rook_cm + 0.5 * black_line_width);
  static double const base_left = CONST(base_left, -0.5 + base_left_cm / width_rook_cm + 0.5 * black_line_width);
  static double const tower_left = CONST(tower_left, -0.5 + tower_left_cm / width_rook_cm + 0.5 * black_line_width);
  static double const opening_left = CONST(opening_left, -0.5 + opening_left_cm / width_rook_cm + 0.5 * black_line_width);
  static double const opening_right = CONST(opening_right, -0.5 + opening_right_cm / width_rook_cm + 0.5 * black_line_width);
  static double const bottom_rook = CONST(bottom_rook, 0.5 - bottom_rook_cm / height_rook_cm - 0.5 * black_line_width);
  static double const foot_top = CONST(foot_top, 0.5 - foot_top_cm / height_rook_cm - 0.5 * black_line_width);
  static double const base_top = CONST(base_top, 0.5 - base_top_cm / height_rook_cm - 0.5 * black_line_width);
  static double const tower_bottom = CONST(tower_bottom, 0.5 - tower_bottom_cm / height_rook_cm - 0.5 * black_line_width);
  static double const tower_top = CONST(tower_top, 0.5 - tower_top_cm / height_rook_cm - 0.5 * black_line_width);
  static double const top_bottom = CONST(top_bottom, 0.5 - top_bottom_cm / height_rook_cm - 0.5 * black_line_width);
  static double const opening_bottom = CONST(opening_bottom, 0.5 - opening_bottom_cm / height_rook_cm - 0.5 * black_line_width);
  // static double const top_top = 0.5 - top_top_cm / height_rook_cm - 0.5 * black_line_width;
  // For alignment purposes, it's better to have the rook *precisely* centered.
  static double const top_top = CONST(top_top, -bottom_rook);

  // Snapped coordinates.
  double const inner_line_width = white ? black_line_width : snap_line_width(white_line_width, scale);
  double const bottom_sn = snap_bottom(bottom_rook, y, scale, black_line_width);
  double const foot_top_sn = snap_bottom(foot_top, y, scale, inner_line_width);
  double const base_top_sn = snap_bottom(base_top, y, scale, inner_line_width);
  double const tower_bottom_sn = snap_bottom(tower_bottom, y, scale, inner_line_width);
  double const tower_top_sn = snap_top(tower_top, y, scale, inner_line_width);
  double const top_bottom_sn = snap_top(top_bottom, y, scale, inner_line_width);
  double const opening_bottom_sn = snap_top(opening_bottom, y, scale, black_line_width);
  double const top_top_sn = snap_top(top_top, y, scale, black_line_width);

  CwChessboardPrivate* priv = chessboard->priv;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw left side.
  cairo_move_to(cr, foot_left, bottom_sn);
  cairo_line_to(cr, foot_left, foot_top_sn);
  cairo_line_to(cr, base_left, foot_top_sn);
  cairo_line_to(cr, base_left, base_top_sn);
  cairo_line_to(cr, tower_left, tower_bottom_sn);
  cairo_line_to(cr, tower_left, tower_top_sn);
  cairo_line_to(cr, base_left, top_bottom_sn);
  cairo_line_to(cr, base_left, top_top_sn);

  // Draw top side.
  cairo_line_to(cr, opening_left, top_top_sn);
  cairo_line_to(cr, opening_left, opening_bottom_sn);
  cairo_line_to(cr, opening_right, opening_bottom_sn);
  cairo_line_to(cr, opening_right, top_top_sn);
  cairo_line_to(cr, -opening_right, top_top_sn);
  cairo_line_to(cr, -opening_right, opening_bottom_sn);
  cairo_line_to(cr, -opening_left, opening_bottom_sn);
  cairo_line_to(cr, -opening_left, top_top_sn);
  cairo_line_to(cr, -base_left, top_top_sn);

  // Draw right side.
  cairo_line_to(cr, -base_left, top_bottom_sn);
  cairo_line_to(cr, -tower_left, tower_top_sn);
  cairo_line_to(cr, -tower_left, tower_bottom_sn);
  cairo_line_to(cr, -base_left, base_top_sn);
  cairo_line_to(cr, -base_left, foot_top_sn);
  cairo_line_to(cr, -foot_left, foot_top_sn);
  cairo_line_to(cr, -foot_left, bottom_sn);

  // Draw bottom line.
  cairo_close_path(cr);
  cairo_path_t* path = cairo_copy_path(cr);

  set_fill_color(cr, priv, white);
  cairo_fill(cr);

  // Draw inner horizontal lines.
  cairo_move_to(cr, base_left + 0.5 * black_line_width, foot_top_sn);
  cairo_line_to(cr, -base_left - 0.5 * black_line_width, foot_top_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, base_left, base_top_sn);
  cairo_line_to(cr, -base_left, base_top_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, tower_left + (white ? 0.0 : (0.5 * black_line_width)), tower_bottom_sn);
  cairo_line_to(cr, -tower_left - (white ? 0.0 : (0.5 * black_line_width)), tower_bottom_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, tower_left + (white ? 0.0 : (0.5 * black_line_width)), tower_top_sn);
  cairo_line_to(cr, -tower_left - (white ? 0.0 : (0.5 * black_line_width)), tower_top_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, base_left + black_line_width * 0.5, top_bottom_sn);
  cairo_line_to(cr, -base_left - black_line_width * 0.5, top_bottom_sn);

  set_line_color(cr, priv, white);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, inner_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }

  cairo_append_path(cr, path);
  if (!white)
    set_fill_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_path_destroy(path);

  cairo_restore(cr);
}

void cw_chessboard_draw_bishop(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const width_bishop_cm = CONST(width_bishop_cm, 5.34);
  static double const ribbon_width_cm = CONST(ribbon_width_cm, 0.49);
  static double const ribbon_bottom_left_cm = CONST(ribbon_bottom_left_cm, 0.72);
  static double const ribbon_top_left_cm = CONST(ribbon_top_left_cm, 2.28);
  static double const inside_outer_diameter_cm = CONST(inside_outer_diameter_cm, 2.0);
  static double const circle_diameter_cm = CONST(circle_diameter_cm, 2.44);
  static double const cross_width_cm = CONST(cross_width_cm, 0.93);
  static double const ball_outer_diameter_cm = CONST(ball_outer_diameter_cm, 0.81);
  static double const ball_inner_diameter_cm = CONST(ball_inner_diameter_cm, 0.41);
  static double const circle_start_angle = CONST(circle_start_angle, 0.767);
  static double const ribbon_end_angle = CONST(ribbon_end_angle, 1.097);
  static double const height_bishop_cm = CONST(height_bishop_cm, 5.44);
  static double const ribbon_bottom_y1_cm = CONST(ribbon_bottom_y1_cm, 0.52);
  static double const ribbon_bottom_y2_cm = CONST(ribbon_bottom_y2_cm, 0.76);
  static double const ribbon_bottom_y3_cm = CONST(ribbon_bottom_y3_cm, 0.55);
  static double const ribbon_top_y1_cm = CONST(ribbon_top_y1_cm, 0.99);
  static double const ribbon_top_y2_cm = CONST(ribbon_top_y2_cm, 1.25);
  static double const ribbon_inside_y_cm = CONST(ribbon_inside_y_cm, 0.93);
  static double const inside_bottom_cm = CONST(inside_bottom_cm, 1.34);
  static double const inside_top_cm = CONST(inside_top_cm, 1.86);
  static double const band_top_bishop_cm = CONST(band_top_bishop_cm, 2.34);
  static double const circle_y_cm = CONST(circle_y_cm, 3.11);
  static double const cross_y_bishop_cm = CONST(cross_y_bishop_cm, 3.24);
  static double const point_y_cm = CONST(point_y_cm, 4.47);
  static double const ball_y_cm = CONST(ball_y_cm, 4.675);
  static double const sp1_x_cm = CONST(sp1_x_cm, 2.1);
  static double const sp1_y_cm = CONST(sp1_y_cm, 3.95);
  static double const ribbon_bottom_x1_cm = CONST(ribbon_bottom_x1_cm, 3.34);
  static double const ribbon_bottom_x2_cm = CONST(ribbon_bottom_x2_cm, 4.1);
  static double const ribbon_top_x1_cm = CONST(ribbon_top_x1_cm, 3.54);
  static double const ribbon_top_x2_cm = CONST(ribbon_top_x2_cm, 4.24);

  // Translate to coordinates.
  static double const ribbon_width = CONST(ribbon_width, ribbon_width_cm / height_bishop_cm);
  static double const ribbon_bottom_left = CONST(ribbon_bottom_left, -0.5 + ribbon_bottom_left_cm / width_bishop_cm);
  static double const ribbon_bottom_x1 = CONST(ribbon_bottom_x1, -0.5 + ribbon_bottom_x1_cm / width_bishop_cm);
  static double const ribbon_bottom_x2 = CONST(ribbon_bottom_x2, -0.5 + ribbon_bottom_x2_cm / width_bishop_cm);
  static double const ribbon_top_x1 = CONST(ribbon_top_x1, -0.5 + ribbon_top_x1_cm / width_bishop_cm);
  static double const ribbon_top_x2 = CONST(ribbon_top_x2, -0.5 + ribbon_top_x2_cm / width_bishop_cm);
  static double const ribbon_top_left = CONST(ribbon_top_left, -0.5 + ribbon_top_left_cm / width_bishop_cm);
  static double const inside_radius_bishop = CONST(inside_radius_bishop, 0.5 * (inside_outer_diameter_cm / width_bishop_cm - black_line_width));
  static double const circle_radius = CONST(circle_radius, 0.5 * circle_diameter_cm / width_bishop_cm);
  static double const cross_leg = CONST(cross_leg, 0.5 * cross_width_cm / width_bishop_cm);
  static double const ball_radius_bishop  = CONST(ball_radius_bishop, 0.25 * (ball_outer_diameter_cm + ball_inner_diameter_cm) / width_bishop_cm);
  static double const ball_line_width = CONST(ball_line_width, black_line_width); // 0.5 * (ball_outer_diameter_cm - ball_inner_diameter_cm) / width_bishop_cm
  static double const ribbon_bottom_y1 = CONST(ribbon_bottom_y1, 0.5 - ribbon_bottom_y1_cm / height_bishop_cm - 0.5 * black_line_width);
  static double const ribbon_bottom_y2 = CONST(ribbon_bottom_y2, 0.5 - ribbon_bottom_y2_cm / height_bishop_cm + 0.5 * black_line_width);
  static double const ribbon_bottom_y3 = CONST(ribbon_bottom_y3, 0.5 - ribbon_bottom_y3_cm / height_bishop_cm);
  static double const ribbon_inside_y = CONST(ribbon_inside_y, 0.5 - ribbon_inside_y_cm / height_bishop_cm);
  static double const ribbon_top_y1 = CONST(ribbon_top_y1, 0.5 - ribbon_top_y1_cm / height_bishop_cm - 0.5 * black_line_width);
  static double const ribbon_top_y2 = CONST(ribbon_top_y2, 0.5 - ribbon_top_y2_cm / height_bishop_cm + 0.5 * black_line_width);
  static double const inside_scale_bishop = CONST(inside_scale_bishop, ((inside_top_cm - inside_bottom_cm) / height_bishop_cm - black_line_width) / (2 * inside_radius_bishop));
  static double const inside_y_bishop = CONST(inside_y_bishop, 0.5 - 0.5 * (inside_top_cm + inside_bottom_cm) / height_bishop_cm);
  static double const inside_bottom = CONST(inside_bottom, 0.5 - inside_bottom_cm / height_bishop_cm - 0.5 * black_line_width);
  static double const band_top_bishop = CONST(band_top_bishop, 0.5 - band_top_bishop_cm / height_bishop_cm + 0.5 * black_line_width);
  static double const circle_y = CONST(circle_y, 0.5 - circle_y_cm / height_bishop_cm);
  static double const cross_y_bishop = CONST(cross_y_bishop, 0.5 - cross_y_bishop_cm / height_bishop_cm);
  static double const point_y = CONST(point_y, 0.5 - point_y_cm / height_bishop_cm);
  static double const ball_y = CONST(ball_y, 0.5 - ball_y_cm / height_bishop_cm);
  static double const inside_angle = CONST(inside_angle, acos(-ribbon_top_left / inside_radius_bishop));
  static double const sp1_x = CONST(sp1_x, -0.5 + sp1_x_cm / width_bishop_cm);
  static double const sp1_y = CONST(sp1_y, 0.5 - sp1_y_cm / height_bishop_cm);

  // Precalculations for the ribbon.
  static double const spline_magic = CONST(spline_magic, 0.551784);
  static double const cp2_x = CONST(cp2_x, ribbon_bottom_y1 - ribbon_inside_y);
  static double const sp2_x = CONST(sp2_x, spline_magic * cp2_x);
  static double const sp2_y = CONST(sp2_y, ribbon_inside_y + spline_magic * (ribbon_bottom_y1 - ribbon_inside_y));
  static double const sp3_x = CONST(sp3_x, ribbon_bottom_x1 - spline_magic * (ribbon_bottom_x1 - cp2_x));
  static double const sp3_y = CONST(sp3_y, ribbon_bottom_y1);
  static double const sp4_x = CONST(sp4_x, ribbon_bottom_x1 + spline_magic * (ribbon_bottom_x2 - ribbon_bottom_x1));
  static double const sp4_y = CONST(sp4_y, ribbon_bottom_y1);
  static double const sp5_x = CONST(sp5_x, ribbon_bottom_x2 - spline_magic * (ribbon_bottom_x2 - ribbon_bottom_x1));
  static double const sp5_y = CONST(sp5_y, ribbon_bottom_y2);
  static double const cp6_x = CONST(cp6_x, -ribbon_bottom_left - (ribbon_bottom_y3 - ribbon_bottom_y2) * tan(ribbon_end_angle));
  static double const sp6_x = CONST(sp6_x, ribbon_bottom_x2 + spline_magic * (cp6_x - ribbon_bottom_x2));
  static double const sp6_y = CONST(sp6_y, ribbon_bottom_y2);
  static double const sp7_x = CONST(sp7_x, -ribbon_bottom_left - spline_magic * (-ribbon_bottom_left - cp6_x));
  static double const sp7_y = CONST(sp7_y, ribbon_bottom_y3 - spline_magic * (ribbon_bottom_y3 - ribbon_bottom_y2));
  static double const ribbon_end_top_x = CONST(ribbon_end_top_x, -ribbon_bottom_left + ribbon_width * cos(ribbon_end_angle));
  static double const ribbon_end_top_y = CONST(ribbon_end_top_y, ribbon_bottom_y3 - ribbon_width * sin(ribbon_end_angle));
  static double const cp8_x = CONST(cp8_x, ribbon_end_top_x - (ribbon_end_top_y - ribbon_top_y2) * tan(ribbon_end_angle));
  static double const sp8_x = CONST(sp8_x, ribbon_end_top_x - spline_magic * (ribbon_end_top_x - cp8_x));
  static double const sp8_y = CONST(sp8_y, ribbon_end_top_y - spline_magic * (ribbon_end_top_y - ribbon_top_y2));
  static double const sp9_x = CONST(sp9_x, ribbon_top_x2 + spline_magic * (cp8_x - ribbon_top_x2));
  static double const sp9_y = CONST(sp9_y, ribbon_top_y2);
  static double const sp10_x = CONST(sp10_x, ribbon_top_x2 - spline_magic * (ribbon_top_x2 - ribbon_top_x1));
  static double const sp10_y = CONST(sp10_y, ribbon_top_y2);
  static double const sp11_x = CONST(sp11_x, ribbon_top_x1 + spline_magic * (ribbon_top_x2 - ribbon_top_x1));
  static double const sp11_y = CONST(sp11_y, ribbon_top_y1);
  static double const ribbon_top_y3 = CONST(ribbon_top_y3, 0.2695);
  static double const sp12_x = CONST(sp12_x, ribbon_top_x1 - spline_magic * (ribbon_top_x1 + ribbon_top_left));
  static double const sp12_y = CONST(sp12_y, ribbon_top_y1);
  static double const sp13_x = CONST(sp13_x, -ribbon_top_left);
  static double const sp13_y = CONST(sp13_y, ribbon_top_y3 + 0.509 * spline_magic * (ribbon_top_y1 - ribbon_top_y3));

  CwChessboardPrivate* priv = chessboard->priv;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw the ribbon.

  // Part 14.
  cairo_move_to(cr, -ribbon_top_x1, ribbon_top_y1);
  cairo_curve_to(cr, -sp11_x, sp11_y, -sp10_x, sp10_y, -ribbon_top_x2, ribbon_top_y2);

  // Part 13.
  cairo_curve_to(cr, -sp9_x, sp9_y, -sp8_x, sp8_y, -ribbon_end_top_x, ribbon_end_top_y);

  // Part 12.
  cairo_line_to(cr, ribbon_bottom_left, ribbon_bottom_y3);

  // Part 11.
  cairo_curve_to(cr, -sp7_x, sp7_y, -sp6_x, sp6_y, -ribbon_bottom_x2, ribbon_bottom_y2);

  // Part 10.
  cairo_curve_to(cr, -sp5_x, sp5_y, -sp4_x, sp4_y, -ribbon_bottom_x1, ribbon_bottom_y1);

  // Part 9.
  cairo_curve_to(cr, -sp3_x, sp3_y, -sp2_x, sp2_y, 0.0, ribbon_inside_y);

  // Part 1.
  cairo_curve_to(cr, sp2_x, sp2_y, sp3_x, sp3_y, ribbon_bottom_x1, ribbon_bottom_y1);

  // Part 2.
  cairo_curve_to(cr, sp4_x, sp4_y, sp5_x, sp5_y, ribbon_bottom_x2, ribbon_bottom_y2);

  // Part 3.
  cairo_curve_to(cr, sp6_x, sp6_y, sp7_x, sp7_y, -ribbon_bottom_left, ribbon_bottom_y3);

  // Part 4.
  cairo_line_to(cr, ribbon_end_top_x, ribbon_end_top_y);

  // Part 5.
  cairo_curve_to(cr, sp8_x, sp8_y, sp9_x, sp9_y, ribbon_top_x2, ribbon_top_y2);

  // Part 6.
  cairo_curve_to(cr, sp10_x, sp10_y, sp11_x, sp11_y, ribbon_top_x1, ribbon_top_y1);

  if (!white)
  {
    set_fill_color(cr, priv, white);
    cairo_fill_preserve(cr);
    cairo_stroke(cr);
    cairo_move_to(cr, ribbon_top_x1, ribbon_top_y1);
  }

  // Part 7.
  cairo_curve_to(cr, sp12_x, sp12_y, sp13_x, sp13_y, -ribbon_top_left, ribbon_top_y3);

  // Part 8 and 17.
  cairo_save(cr);
  cairo_translate(cr, 0.0, inside_y_bishop);
  cairo_scale(cr, 1.0, inside_scale_bishop);
  cairo_arc(cr, 0.0, 0.0, inside_radius_bishop, inside_angle, M_PI - inside_angle);
  cairo_restore(cr);

  // Part 16.
  cairo_line_to(cr, ribbon_top_left, ribbon_top_y3);

  // Part 15.
  cairo_curve_to(cr, -sp13_x, sp13_y, -sp12_x, sp12_y, -ribbon_top_x1 + 0.01 * black_line_width, ribbon_top_y1);
  cairo_close_path(cr);

  if (white)
    set_fill_color(cr, priv, white);
  else
     set_line_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
     set_line_color(cr, priv, white);
  else
     set_fill_color(cr, priv, white);
  cairo_stroke(cr);

  // Draw vertical line between left and right ribbon.
  cairo_move_to(cr, 0.0, inside_bottom);
  cairo_line_to(cr, 0.0, ribbon_inside_y);
  cairo_stroke(cr);

  // Draw the outline of the bishop.

  cairo_save(cr);
  cairo_translate(cr, 0.0, inside_y_bishop);
  cairo_scale(cr, 1.0, inside_scale_bishop);
  cairo_arc(cr, 0.0, 0.0, inside_radius_bishop, 0.0, -M_PI);
  cairo_restore(cr);

  cairo_arc(cr, 0.0, circle_y, circle_radius, -M_PI - circle_start_angle, -M_PI);

  cairo_curve_to(cr, -circle_radius, circle_y - 0.0848, sp1_x - 0.02657, sp1_y + 0.01722, sp1_x, sp1_y);
  cairo_curve_to(cr, sp1_x + 0.08845, sp1_y - 0.05733, -0.000333, point_y + 0.000265, 0.0, point_y);
  cairo_curve_to(cr, 0.000333, point_y + 0.000265, -sp1_x - 0.08845, sp1_y - 0.05733, -sp1_x, sp1_y);
  cairo_curve_to(cr, -sp1_x + 0.02657, sp1_y + 0.01722, circle_radius, circle_y - 0.0848, circle_radius, circle_y);

  cairo_arc(cr, 0.0, circle_y, circle_radius, 0.0, circle_start_angle);

  cairo_close_path(cr);

  if (white)
    set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, priv, white);
  cairo_stroke(cr);

  // Draw inside lines.
  if (!white)
    set_line_color(cr, priv, white);
  cairo_save(cr);
  if (!white)
  {
    static double const x2_bishop = CONST(x2_bishop, -circle_radius * cos(circle_start_angle));
    static double const y2_bishop = CONST(y2_bishop, (circle_y + circle_radius * sin(circle_start_angle)));
    cairo_move_to(cr, -inside_radius_bishop, inside_y_bishop);
    cairo_line_to(cr, x2_bishop, y2_bishop);
    cairo_line_to(cr, -x2_bishop, y2_bishop);
    cairo_line_to(cr, inside_radius_bishop, inside_y_bishop);
    cairo_close_path(cr);
    cairo_clip(cr);
  }
  cairo_save(cr);
  cairo_translate(cr, 0.0, inside_y_bishop);
  cairo_scale(cr, 1.0, inside_scale_bishop);
  cairo_arc(cr, 0.0, 0.0, inside_radius_bishop, -M_PI, 0.0);
  cairo_restore(cr);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }
  cairo_restore(cr);

  // Ok, some real math needed here.
  // First, we scale the y-axis - so that the problem changes to a circle instead of an ellipse.
  // Also, flip the y-axis, so that the top of the screen becomes the positive y-axis.
  // This means that instead of using the normal 'y' values, we now should use y / -inside_scale_bishop.
  // Next, calculate the line through (-inside_radius_bishop, inside_y_bishop) and the
  // start of the circle with vector parametric formula: V = X0 + U t1,
  // where U is the unit vector (u1, u2), t1 is the parameter and X0 = (x0, 0), the
  // point where the line crosses the x-axis.
  static double const x1 = CONST(x1, -inside_radius_bishop);
  static double const y1 = CONST(y1, inside_y_bishop / -inside_scale_bishop);
  static double const x2 = CONST(x2, -circle_radius * cos(circle_start_angle));
  static double const y2 = CONST(y2, (circle_y + circle_radius * sin(circle_start_angle)) / -inside_scale_bishop);
  static double const d = CONST(d, sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
  static double const u1 = CONST(u1, (x2 - x1) / d);
  static double const u2 = CONST(u2, (y2 - y1) / d);
  static double const x0 = CONST(x0, x1 + (x2 - x1) * (0 - y1) / (y2 - y1));
  // A line through the center of the circle, perpendicular to this line is:
  // V = Y0 + W t2, where Y0 = (0, y0) and W = (u2, -u1).
  // Those two lines cross therefore where,
  // x: x0 + u1 t1 = u2 t2
  // y: u2 t1 = y0 - u1 t2
  // Since the t2 parameter gives the distance from the center in terms of a unit vector,
  // the distance from the center to the crossing point is |t2|, which we find by cancelling t1:
  // x0 u2 + u1 (y0 - u1 t2) = u2^2 t2 --> t2 = x0 u2 + u1 y0, since u1^2 + u2^2 = 1.
  // In the crossing point, this value is negative (u2 is positive, and the cross point
  // lays left of the center). Hence we find that the distance is -t2 =
  // - x0 u2 - u1 y0 = band_top_bishop / -inside_scale_bishop - y0.
  static double const y0 = CONST(y0, (band_top_bishop / -inside_scale_bishop + x0 * u2) / (1 - u1));
  static double const band_radius = CONST(band_radius, band_top_bishop / -inside_scale_bishop - y0);
  static double const angle = CONST(angle, atan(u1 / u2));
  cairo_save(cr);
  cairo_scale(cr, 1.0, -inside_scale_bishop);
  if (!white)
  {
    static double const t2 = CONST(t2, x0 * u2 + u1 * y0);
    static double const t1 = CONST(t1, (y0 - u1 * t2) / u2);
    static double const x = CONST(x, x0 + u1 * t1);
    cairo_move_to(cr, x, y0);
    cairo_line_to(cr, x + d * u1, y0 + d * u2);
    cairo_line_to(cr, -x - d * u1, y0 + d * u2);
    cairo_line_to(cr, -x, y0);
    cairo_close_path(cr);
    cairo_clip(cr);
  }
  cairo_arc(cr, 0.0, y0, band_radius, angle, M_PI - angle);
  // Reverse the scale before stroking, without restoring the clipping area.
  cairo_scale(cr, 1.0, -1.0 / inside_scale_bishop);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }
  cairo_restore(cr);

  // Draw the cross.
  cairo_move_to(cr, -cross_leg, cross_y_bishop);
  cairo_line_to(cr, cross_leg, cross_y_bishop);
  cairo_move_to(cr, 0.0, cross_y_bishop - cross_leg);
  cairo_line_to(cr, 0.0, cross_y_bishop + cross_leg);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }

  if (!white)
  {
    cairo_move_to(cr, -inside_radius_bishop, inside_y_bishop);
    cairo_arc(cr, 0.0, circle_y, circle_radius, -M_PI - circle_start_angle, -M_PI);
    cairo_move_to(cr, inside_radius_bishop, inside_y_bishop);
    cairo_arc_negative(cr, 0.0, circle_y, circle_radius, circle_start_angle, 0.0);
    set_fill_color(cr, priv, white);
    cairo_stroke(cr);
  }

  // Draw the little ball on the top.
  cairo_set_line_width(cr, ball_line_width);
  cairo_arc(cr, 0.0, ball_y, ball_radius_bishop, -M_PI, M_PI);
  if (white)
    set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, priv, white);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void cw_chessboard_draw_knight(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements.
  static double const height_knight_cm = CONST(height_knight_cm, 21.9);
  static double const pixels_per_cm = CONST(pixels_per_cm, 32.467);
  static double const min_nose_x_px = CONST(min_nose_x_px, 8.0);
  static double const right_ear_y_px = CONST(right_ear_y_px, 15.0);		// See 'Draw right ear'.
  static double const bottom_right_x_px = CONST(bottom_right_x_px, 582.82);	// See 'back' curve.
  static double const bottom_right_y_px = CONST(bottom_right_y_px, 580.82);
  static double const bottom_left_x_px = CONST(bottom_left_x_px, 190.00);	// See 'front' curve.
  // Derived.
  static double const pixel_scale = CONST(pixel_scale, 1.0 / (pixels_per_cm * height_knight_cm));
  static double const knight_black_line_width = CONST(knight_black_line_width, 0.95 * black_line_width / pixel_scale);
  static double const knight_white_line_width = CONST(knight_white_line_width, 1.3 * knight_black_line_width);
  static double const knight_white_glyp_line_width = CONST(knight_white_glyp_line_width, knight_white_line_width - knight_black_line_width);

  // The outline of the knight in coordinates, without translation.
  static double const max_y = CONST(max_y, bottom_right_y_px * pixel_scale);
  static double const min_y = CONST(min_y, right_ear_y_px * pixel_scale);
  static double const max_x = CONST(max_x, bottom_right_x_px * pixel_scale);
  static double const min_x = CONST(min_x, min_nose_x_px * pixel_scale);

  // Calculate coordinate offsets, needed to center the knight.
  static double const pixel_translate_x = CONST(pixel_translate_x, -(max_x + min_x) / 2);
  static double const pixel_translate_y = CONST(pixel_translate_y, -(max_y + min_y) / 2);

  CwChessboardPrivate* priv = chessboard->priv;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);

  // At this point, the coordinates run from -0.5 till 0.5.
  // However, we use pixels as coordinates (as measured in gimp)
  // Translate the image so that the pixel-coordinate center falls on (0, 0).
  cairo_translate(cr, pixel_translate_x, pixel_translate_y);
  // Scale, so we can use "pixel-coordinates".
  cairo_scale(cr, pixel_scale, pixel_scale);

  // Fill body.
  cairo_move_to(cr, 319.00, 299.00);
  cairo_curve_to(cr, 322.00, 449.00, 165.00, 445.00, 192.00, 570.00);
  cairo_curve_to(cr, 192.00, 570.00, 568.50, 571.00, 568.50, 571.00);
  cairo_curve_to(cr, 577.00, 426.00, 533.00, 99.00, 340.50, 88.50);
  cairo_curve_to(cr, 245.50, 87.50, 206.00, 86.00, 195.00, 102.00);
  set_fill_color(cr, priv, white);
  cairo_fill(cr);

  // Draw shadow.
  cairo_move_to(cr, 315.00, 300.33);
  cairo_curve_to(cr, 301.43, 300.80, 291.75, 314.52, 282.00, 325.00);
  cairo_curve_to(cr, 298.67, 317.33, 316.33, 325.00, 317.33, 344.33);
  cairo_curve_to(cr, 321.33, 337.33, 326.00, 326.00, 315.00, 300.33);
  if (white)
    set_line_color(cr, priv, white);
  cairo_fill(cr);

  // Draw back.
  // Change the thickness of the top of the back to a line width:
  static double const back_top_offset = CONST(back_top_offset, (93.00 - knight_black_line_width) - 82.00);
  cairo_move_to(cr, 582.82, 580.82);
  cairo_curve_to(cr, 589.00, 359.00, 530.00,85.00, 332.00, 82.00 + back_top_offset);
  cairo_curve_to(cr, 320.87, 82.04 + back_top_offset, 314.25, 82.12 + back_top_offset, 302.50, 82.38 + back_top_offset);
  cairo_curve_to(cr, 302.75, 95.38, 296.22, 93.73, 319.50, 94.00);
  cairo_curve_to(cr, 510.50, 93.00, 556.12, 359.00, 556.12, 563.00);
  cairo_close_path(cr);
  cairo_fill(cr);

  // Draw front.
  cairo_move_to(cr, 190.00, 570.00);
  cairo_curve_to(cr, 190.00, 550.75, 190.00, 549.00, 190.00, 540.00);
  cairo_curve_to(cr, 190.00, 493.25, 210.50, 482.50, 285.00, 409.50);
  cairo_curve_to(cr, 298.25, 391.75, 313.00, 357.50, 317.75, 344.75);
  cairo_curve_to(cr, 320.25, 340.00, 320.25, 330.00, 320.00, 280.00);
  cairo_set_line_width(cr, knight_black_line_width);
  cairo_stroke(cr);

  // Draw head.
  cairo_move_to(cr, 144.00, 31.50);
  cairo_curve_to(cr, 122.50, 67.00, 147.50, 57.50, 146.00, 105.00);
  cairo_curve_to(cr, 112.00, 125.50, 123.00, 140.50, 102.50, 170.00);
  cairo_curve_to(cr, 84.00, 199.50, 128.00, 181.50, 33.50, 313.50);
  cairo_curve_to(cr, -23.00, 414.00, 81.50, 468.00, 130.00, 447.50);
  cairo_curve_to(cr, 182.50, 398.00, 142.50, 427.00, 179.50, 390.00);
  cairo_curve_to(cr, 194.50, 376.50, 212.50, 349.50, 237.50, 347.00);
  cairo_curve_to(cr, 268.00, 344.00, 283.50, 323.50, 306.00, 301.00);
  cairo_curve_to(cr, 327.50, 276.50, 330.00, 264.50, 330.00, 228.50);
  if (white)
    set_fill_color(cr, priv, white);
  cairo_fill_preserve(cr);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  if (white)
    set_line_color(cr, priv, white);
  cairo_stroke(cr);
  cairo_move_to(cr, 201.00, 94.50);
  cairo_curve_to(cr, 184.50, 54.50, 152.00, 43.50, 144.00, 31.50);
  cairo_stroke(cr);

  // Draw between ears.
  cairo_move_to(cr, 170.50, 136.50);
  cairo_curve_to(cr, 170.00, 129.50, 175.50, 125.00, 183.50, 116.00);
  cairo_curve_to(cr, 204.50, 91.00, 216.00, 94.00, 238.00, 91.00);
  cairo_stroke(cr);

  if (!white)
  {
    // Draw white hair.
    cairo_move_to(cr, 529.00, 570.00);
    cairo_curve_to(cr, 530.50, 352.00, 476.50, 128.50, 334.00, 121.00);
    cairo_curve_to(cr, 310.50, 118.50, 310.00, 117.50, 296.50, 117.50);
    cairo_curve_to(cr, 291.50, 100.00, 252.50, 95.50, 242.20, 119.35);
    cairo_curve_to(cr, 227.55, 120.95, 212.22, 124.23, 198.50, 130.50);
    cairo_curve_to(cr, 178.00, 137.50, 158.50, 147.50, 154.00, 137.00);
    cairo_curve_to(cr, 149.50, 127.00, 145.50, 121.00, 204.00, 100.00);
    cairo_curve_to(cr, 226.50, 90.00, 276.50, 92.00, 319.50, 94.00);
    cairo_curve_to(cr, 510.50, 93.00, 556.00, 354.00, 556.00, 570.00);
    cairo_curve_to(cr, 548.06, 571.00, 537.73, 569.45, 529.00, 570.00);
    set_line_color(cr, priv, white);
    cairo_fill(cr);
  }

  // Draw bottom.
  double dummy = bottom_left_x_px;
  double bottom_right_y_px_sn = bottom_right_y_px;
  if (scale >= 27)
  {
    // Snap bottom to grid.
    cairo_user_to_device(cr, &dummy, &bottom_right_y_px_sn);
    bottom_right_y_px_sn = round(bottom_right_y_px_sn);
    cairo_device_to_user(cr, &dummy, &bottom_right_y_px_sn);
  }
  cairo_rectangle(cr, bottom_left_x_px - 0.5 * knight_black_line_width,
                      bottom_right_y_px_sn - knight_black_line_width,
                      bottom_right_x_px - (bottom_left_x_px - 0.5 * knight_black_line_width),
		      knight_black_line_width);
  if (!white)
    set_fill_color(cr, priv, white);
  cairo_fill(cr);

  // Draw mouth.
  cairo_move_to(cr, 113.67, 389.33);
  cairo_curve_to(cr, 121.00, 388.00, 129.33, 406.67, 120.67, 414.67);
  cairo_curve_to(cr, 114.33, 419.33, 104.33, 431.00, 112.67, 444.00);
  //cairo_curve_to(cr, 133.53, 466.89, 104.22, 478.73, 93.00, 446.67);
  cairo_line_to(cr, 93.00, 446.67);
  cairo_curve_to(cr, 89.00, 418.67, 94.67, 417.33, 100.00, 412.67);
  cairo_curve_to(cr, 112.67, 402.00, 100.67, 394.67, 113.67, 389.33);
  if (!white)
    set_line_color(cr, priv, white);
  if (white)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, knight_white_glyp_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, knight_black_line_width);
  }

  // Redraw a part of the head.
  cairo_move_to(cr, 33.50, 313.50);
  cairo_curve_to(cr, -23.00, 414.00, 81.50, 468.00, 130.00, 447.50);
  if (!white)
    set_fill_color(cr, priv, white);
  cairo_stroke(cr);

  if (!white)
  {
    // Draw jaw.
    cairo_move_to(cr, 312.32, 293.46);
    cairo_curve_to(cr, 328.01, 273.63, 330.00, 260.62, 330.00, 228.50);
    cairo_set_line_width(cr, knight_white_line_width);
    set_line_color(cr, priv, white);
    cairo_stroke(cr);
    cairo_set_line_width(cr, knight_black_line_width);
  }

  // Draw right ear.
  for (int stroke = 0; stroke < 2; ++stroke)
  {
    cairo_move_to(cr, 242.00, 114.00);
    cairo_curve_to(cr, 235.00, 76.00, 235.50, 92.50, 267.00, 15.00);	// 15.00 corresponds to min_y
    if (stroke)
      cairo_move_to(cr, 267.00, 15.00);
    cairo_curve_to(cr, 309.50, 85.50, 312.00, 88.00, 295.00, 117.00);
    if (stroke)
    {
      if (white)
        set_line_color(cr, priv, white);
      cairo_stroke(cr);
    }
    else
    {
      if (white)
	set_fill_color(cr, priv, white);
      else
        set_fill_color(cr, priv, white);
      cairo_fill(cr);
    }
  }

  if (!white)
    set_line_color(cr, priv, white);

  // Draw nose.
  cairo_move_to(cr, 76.00, 363.00);
  cairo_curve_to(cr, 66.00, 372.33, 78.33, 379.00, 66.00, 384.00);
  cairo_curve_to(cr, 21.00, 399.00, 61.67, 331.00, 79.67, 341.67);
  cairo_curve_to(cr, 81.00, 342.00, 84.67, 353.33, 76.00, 363.00);
  if (white)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, knight_white_glyp_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, knight_black_line_width);
  }

  // Draw eye.
  cairo_move_to(cr, 173.33, 208.00);
  cairo_curve_to(cr, 180.67, 207.00, 182.00, 197.67, 182.00, 197.67);
  cairo_curve_to(cr, 184.59, 176.98, 182.28, 177.30, 190.67, 173.00);
  cairo_curve_to(cr, 201.00, 169.33, 198.33, 146.00, 173.33, 161.67);
  cairo_curve_to(cr, 146.00, 181.33, 130.67, 192.00, 128.33, 202.67);
  cairo_curve_to(cr, 124.00, 233.33, 131.00, 227.33, 144.67, 207.00);
  cairo_curve_to(cr, 150.67, 201.00, 158.67, 193.67, 162.33, 203.33);
  cairo_curve_to(cr, 164.67, 206.00, 165.63, 209.29, 173.33, 208.00);
  if (white)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, knight_white_glyp_line_width);
    cairo_stroke(cr);
  }

  cairo_restore(cr);
}

//-----------------------------------------------------------------------------
// Mouse events.

static gboolean cw_chessboard_motion_notify(GtkWidget* widget, GdkEventMotion* event)
{
  Dout(dc::motion_event, "Calling cw_chessboard_motion_notify(" << widget << ", " << event << ")");
  update_cursor_position(CW_CHESSBOARD(widget), event->x, event->y, FALSE);
  // Always look for another handler.
  return FALSE;
}

static gboolean cw_chessboard_default_motion_notify(GtkWidget* widget, GdkEventMotion* event)
{
  Dout(dc::motion_event, "Calling cw_chessboard_default_motion_notify(" << widget << ", " << event << ")");

  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;

  if (priv->floating_piece_handle != -1)
  {
    double hsside = 0.5 * chessboard->sside;
    double fraction = hsside - (gint)hsside;
    cw_chessboard_move_floating_piece(chessboard, priv->floating_piece_handle, event->x - fraction, event->y - fraction);
    return TRUE;
  }

  return FALSE;
}

