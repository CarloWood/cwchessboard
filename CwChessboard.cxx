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

/*! @file CwChessboard.cxx

  \brief This file contains the implementation of the GTK+ widget %CwChessboard.

  You can compile this file with C or C++ compiler (just renaming
  it to .c or .cxx should do the trick with most build systems).
  If you compile it as C source, then you need to generate a
  file CwChessboard-CONST.h from this file with the following
  commands: <tt>./gen.sh CwChessboard.c; g++ -o gen gen.cpp; ./gen > %CwChessboard-CONST.h</tt>
  where the contents of the script 'gen.sh' is:
  \code
  echo "#include <iostream>" > gen.cpp
  echo "#include <cmath>" >> gen.cpp
  echo >> gen.cpp
  echo "#define CWCHESSBOARD_CONST_(name, expr) expr; \\"
  echo "    std::cout << \"#define CWCHESSBOARD_CONST_\" << #name << \" \" << name << std::endl;" >> gen.cpp
  echo "int main()" >> gen.cpp
  echo "{" >> gen.cpp
  mawk 'BEGIN                     { inl=0; } \
      /static .*= CWCHESSBOARD_CONST_\(.*;/    { sub(/^[ \t]+/, ""); printf("  %s\n", $0); inl=0; } \
      //                         { if (inl) { sub(/^[ \t]+/, ""); printf("    %s\n", $0); inl=0; } } \
      /static .*= CWCHESSBOARD_CONST_\([^;]*$/ { sub(/^[ \t]+/, ""); printf("  %s\n", $0); inl=1; }' \
      $1 >> gen.cpp
  echo "}" >> gen.cpp
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

static void cw_chessboard_destroy(GtkWidget* widget);
static void cw_chessboard_finalize(GObject* object);
static void cw_chessboard_realize(GtkWidget* widget);
static void cw_chessboard_unrealize(GtkWidget* widget);
static void cw_chessboard_preferred_width(GtkWidget* widget, gint* minimal_width, gint* natural_width);
static void cw_chessboard_preferred_height(GtkWidget* widget, gint* minimal_height, gint* natural_height);
static void cw_chessboard_size_allocate(GtkWidget* widget, GtkAllocation* allocation);
static gboolean cw_chessboard_draw(GtkWidget* chessboard, cairo_t* cr);
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
  cairo_region_t* chessboard_region;	// The rectangular region where the chessboard resides.
  gint marker_thickness_px;		// Thickness of the markers.
  gint cursor_thickness_px;		// Thickness of the cursor.
  gint cursor_col;			// Current cursor column.
  gint cursor_row;			// Current cursor row.

  // Buffers and caches.
  cairo_surface_t* pixmap;		// X server side drawing buffer.
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

G_DEFINE_TYPE_WITH_PRIVATE(CwChessboard, cw_chessboard, GTK_TYPE_DRAWING_AREA);

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
//  GtkObjectClass *object_class = GTK_OBJECT_CLASS(widget_class);
  GObjectClass* gobject_class = G_OBJECT_CLASS(widget_class /*object_class*/);

  gobject_class->finalize = cw_chessboard_finalize;

  widget_class->destroy = cw_chessboard_destroy;

  widget_class->draw = cw_chessboard_draw;
  widget_class->motion_notify_event = cw_chessboard_default_motion_notify;

  widget_class->realize = cw_chessboard_realize;
  widget_class->get_preferred_width = cw_chessboard_preferred_width;
  widget_class->get_preferred_height = cw_chessboard_preferred_height;
  widget_class->size_allocate = cw_chessboard_size_allocate;
  widget_class->unrealize = cw_chessboard_unrealize;

  chessboard_class->calc_board_border_width = cw_chessboard_default_calc_board_border_width;
  chessboard_class->draw_border = cw_chessboard_default_draw_border;
  chessboard_class->draw_turn_indicator = cw_chessboard_default_draw_turn_indicator;
//  chessboard_class->draw_piece[pawn] = cw_chessboard_draw_pawn;
//  chessboard_class->draw_piece[rook] = cw_chessboard_draw_rook;
//  chessboard_class->draw_piece[knight] = cw_chessboard_draw_knight;
//  chessboard_class->draw_piece[bishop] = cw_chessboard_draw_bishop;
//  chessboard_class->draw_piece[queen] = cw_chessboard_draw_queen;
//  chessboard_class->draw_piece[king] = cw_chessboard_draw_king;
  chessboard_class->draw_hud_layer = cw_chessboard_default_draw_hud_layer;
  chessboard_class->draw_hud_square = cw_chessboard_default_draw_hud_square;
  chessboard_class->cursor_left_chessboard = NULL;
  chessboard_class->cursor_entered_square = NULL;
}

static void cw_chessboard_destroy(GtkWidget* widget)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_destroy(" << widget << ")");
  GTK_WIDGET_CLASS(cw_chessboard_parent_class)->destroy(widget);
}

// Initialization of CwChessboard instances.
static void cw_chessboard_init(CwChessboard* chessboard)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_init(" << chessboard << ")");

  CwChessboardPrivate* priv = (CwChessboardPrivate*)cw_chessboard_get_instance_private(chessboard);
  //GdkVisual* visual = gtk_widget_get_visual(GTK_WIDGET(chessboard));

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
#if 0 // No longer needed?
    if (!gdk_colormap_alloc_color(colormap, &dark_square_color, FALSE, TRUE))
      DoutFatal(dc::fatal, "gdk_colormap_alloc_color failed to allocate dark_square_color (" <<
          dark_square_color.red << ", " <<
	  dark_square_color.green << ", " <<
	  dark_square_color.blue << ")");
#endif
    light_square_color.red = light_square_red + (gushort)(x * (65535 - light_square_red));
    light_square_color.green = light_square_green + (gushort)(x * (65535 - light_square_green));
    light_square_color.blue = light_square_blue + (gushort)(x * (65535 - light_square_blue));
#if 0 // No longer needed?
    if (!gdk_colormap_alloc_color(colormap, &light_square_color, FALSE, TRUE))
      DoutFatal(dc::fatal, "gdk_colormap_alloc_color failed to allocate light_square_color (" <<
          light_square_color.red << ", " <<
	  light_square_color.green << ", " <<
	  light_square_color.blue << ")");
#endif

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
  // FIXME: this is deprecated and they claim that it currently causes a performance
  // penalty. See https://developer.gnome.org/gtk3/stable/GtkWidget.html#gtk-widget-set-double-buffered
  // It seems that the correct solution would require to remove this, but then (of course)
  // also remove the reason that I added it, and not do our own buffering anymore?
//  gtk_widget_set_double_buffered(GTK_WIDGET(chessboard), FALSE);

  gtk_widget_add_events(GTK_WIDGET(chessboard),
      GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  // Catch motion notify events for cursor handling.
  g_signal_connect(G_OBJECT(chessboard), "motion-notify-event", G_CALLBACK(cw_chessboard_motion_notify), NULL);

  // Attach draw signal to draw function.
  g_signal_connect(G_OBJECT(chessboard), "draw", G_CALLBACK(cw_chessboard_draw), NULL);
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
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
  {
    CwChessboardPrivate* priv = chessboard->priv;
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(chessboard));
    cairo_rectangle_int_t pixmap_rect;
    pixmap_rect.x = priv->top_left_pixmap_x;
    pixmap_rect.y = priv->top_left_pixmap_y;
    pixmap_rect.width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
    pixmap_rect.height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
    cairo_region_t* border_region = cairo_region_create_rectangle(&pixmap_rect);
    cairo_region_subtract(border_region, priv->chessboard_region);
    gdk_window_invalidate_region(window, border_region, FALSE);
    cairo_region_destroy(border_region);
  }
}

static void invalidate_turn_indicators(CwChessboard* chessboard)
{
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
  {
    CwChessboardPrivate* priv = chessboard->priv;

    gint const border_width = priv->border_width;
    gint const border_shadow_width = 2;
    gint const edge_width = border_width - border_shadow_width - 1;
    gint const side = squares * priv->sside;

    double const factor = 0.085786;       // (1/sqrt(2) − 0.5)/(1 + sqrt(2)).
    int const dx = (int)ceil((edge_width + 1) * factor);

    cairo_rectangle_int_t top_indicator_rect, bottom_indicator_rect;
    top_indicator_rect.x = priv->top_left_pixmap_x + priv->pixmap_top_left_a8_x + side + 1 - dx;
    top_indicator_rect.y = priv->top_left_pixmap_y + priv->pixmap_top_left_a8_y - 1 - edge_width;
    top_indicator_rect.width = edge_width + dx;
    top_indicator_rect.height = edge_width;
    cairo_region_t* indicator_region = cairo_region_create_rectangle(&top_indicator_rect);
    bottom_indicator_rect.x = top_indicator_rect.x;
    bottom_indicator_rect.y = top_indicator_rect.y + edge_width + side + 2;
    bottom_indicator_rect.width = edge_width + dx;
    bottom_indicator_rect.height = edge_width;
    cairo_region_union_rectangle(indicator_region, &bottom_indicator_rect);
    top_indicator_rect.x += dx;
    top_indicator_rect.y += edge_width;
    top_indicator_rect.width = edge_width;
    top_indicator_rect.height = dx;
    cairo_region_union_rectangle(indicator_region, &top_indicator_rect);
    bottom_indicator_rect.x += dx;
    bottom_indicator_rect.y -= dx;
    bottom_indicator_rect.width = edge_width;
    bottom_indicator_rect.height = dx;
    cairo_region_union_rectangle(indicator_region, &bottom_indicator_rect);
    gdk_window_invalidate_region(gtk_widget_get_window(GTK_WIDGET(chessboard)), indicator_region, FALSE);
    cairo_region_destroy(indicator_region);
  }
}

#if 0	// Not currently used anywhere.
static void invalidate_background(CwChessboard* chessboard)
{
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
  {
    CwChessboardPrivate* priv = chessboard->priv;
    GdkWindow* window = gtk_widget_get_window(GTK_WIDGET(chessboard));
    cairo_region_t* background_region = gdk_drawable_get_clip_region(window);
    cairo_region_subtract(background_region, priv->chessboard_region);
    gdk_window_invalidate_region(window, background_region, FALSE);
    cairo_region_destroy(background_region);
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
    if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
    {
      cairo_rectangle_int_t rect;
      rect.width = priv->sside;
      rect.height = priv->sside;
      cw_chessboard_colrow2xy(chessboard, col, row, &rect.x, &rect.y);
      gdk_window_invalidate_rect(gtk_widget_get_window(GTK_WIDGET(chessboard)), &rect, FALSE);
      priv->need_redraw_invalidated |= redraw_mask;
    }
    else
      priv->need_redraw |= redraw_mask;
  }
}

static void invalidate_board(CwChessboard* chessboard)
{
  CwChessboardPrivate* priv = chessboard->priv;
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
  {
    gdk_window_invalidate_region(gtk_widget_get_window(GTK_WIDGET(chessboard)), priv->chessboard_region, FALSE);
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
    Dout(dc::cwchessboardwidget, "Calling gdk_window_get_device_position()");
    GdkDisplay* display = gdk_display_get_default();
    GdkSeat* seat = gdk_display_get_default_seat(display);
    GdkDevice* pointer = gdk_seat_get_pointer(seat);
    gdk_window_get_device_position(gtk_widget_get_window(GTK_WIDGET(chessboard)), pointer, NULL, NULL, NULL);
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

// This function should only be called from cw_chessboard_draw.
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
  gint minimal_width, natural_width, minimal_height, natural_height;
  cw_chessboard_preferred_width(widget, &minimal_width, &natural_width);
  cw_chessboard_preferred_height(widget, &minimal_height, &natural_height);
  gint const max_total_side = MIN(natural_width, natural_height);

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
  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);
  gint pixmap_width = total_side;
  gint pixmap_height = total_side;
  if (allocation.width < allocation.height)
    pixmap_width = allocation.width;
  else
    pixmap_height = allocation.height;

  // Remember old size of pixmap.
  gint old_pixmap_width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
  gint old_pixmap_height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
  // Calculate the offsets of the pixmap corners.
  priv->top_left_pixmap_x = ((allocation.width - pixmap_width) & ~1) / 2;
  priv->top_left_pixmap_y = ((allocation.height - pixmap_height) & ~1) / 2;
  priv->bottom_right_pixmap_x = priv->top_left_pixmap_x + pixmap_width;
  priv->bottom_right_pixmap_y = priv->top_left_pixmap_y + pixmap_height;
  g_assert(priv->top_left_pixmap_x == 0 || priv->top_left_pixmap_y == 0);

  // Calculate the offset of the top-left of square a1.
  *(gint*)(&chessboard->top_left_a1_x) = ((allocation.width - side) & ~1) / 2;
  *(gint*)(&chessboard->top_left_a1_y) = ((allocation.height - side) & ~1) / 2 + side - sside;

  // Calculate the offset of the top-left of square a8, relative to the top-left of the pixmap.
  priv->pixmap_top_left_a8_x = chessboard->top_left_a1_x - priv->top_left_pixmap_x;
  priv->pixmap_top_left_a8_y = chessboard->top_left_a1_y + sside - side - priv->top_left_pixmap_y;

  // Set the rectangular region where the chessboard resides.
  cairo_rectangle_int_t rect;
  rect.x = priv->top_left_pixmap_x + priv->pixmap_top_left_a8_x;
  rect.y = priv->top_left_pixmap_y + priv->pixmap_top_left_a8_y;
  rect.width = squares * sside;
  rect.height = squares * sside;
  if (priv->chessboard_region)
    cairo_region_destroy(priv->chessboard_region);
  priv->chessboard_region = cairo_region_create_rectangle(&rect);

  Dout(dc::cwchessboardwidget, "widget size is (" << allocation.width << ", " << allocation.height << ")");
  Dout(dc::cwchessboardwidget, "border width is " << priv->border_width <<
      "; " << squares << 'x' << squares << " squares with side " << sside);
  Dout(dc::cwchessboardwidget, "pixmap at (" << priv->top_left_pixmap_x << ", " << priv->top_left_pixmap_y << ") with size (" <<
      pixmap_width << ", " << pixmap_height << ")");
  Dout(dc::cwchessboardwidget, "a8 offset within pixmap is (" <<
      priv->pixmap_top_left_a8_x << ", " << priv->pixmap_top_left_a8_y << ")");
  Dout(dc::cwchessboardwidget, "    a1 at (" << chessboard->top_left_a1_x << ", " << chessboard->top_left_a1_y << ")");

  // Invalidate everything.
  gdk_window_invalidate_rect(gtk_widget_get_window(widget), &allocation, FALSE);
  priv->need_redraw_invalidated = (guint64)-1;

  if (old_pixmap_width == pixmap_width && old_pixmap_height == pixmap_height)
    return;

  if (priv->cr)
    cairo_destroy(priv->cr);
  if (priv->pixmap)
    cairo_surface_destroy(priv->pixmap);

  // Allocate a pixmap for the board including the border.
  priv->pixmap = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, pixmap_width, pixmap_height);

  // (Re)create cairo context for the new pixmap.
  priv->cr = cairo_create(priv->pixmap);

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

static void cw_chessboard_preferred_width(GtkWidget* widget, gint* minimal_width, gint* natural_width)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_preferred_width(" << ")");

  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;

  // Return the minimum size we really need.
  gint min_border_width = 0;
  if (priv->draw_border)
    min_border_width = 2 * CW_CHESSBOARD_GET_CLASS(chessboard)->calc_board_border_width(chessboard, min_sside);
  *minimal_width = *natural_width = squares * min_sside + min_border_width;
}

static void cw_chessboard_preferred_height(GtkWidget* widget, gint* minimal_height, gint* natural_height)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_preferred_height(" << ")");

  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;

  // Return the minimum size we really need.
  gint min_border_width = 0;
  if (priv->draw_border)
    min_border_width = 2 * CW_CHESSBOARD_GET_CLASS(chessboard)->calc_board_border_width(chessboard, min_sside);
  *minimal_height = *natural_height = squares * min_sside + min_border_width;
}

static void cw_chessboard_size_allocate(GtkWidget* widget, GtkAllocation* allocation)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_size_allocate(" << widget << ", " << allocation << ")");
  gtk_widget_set_allocation(widget, allocation);
  if (gtk_widget_get_realized(widget))
  {
    gdk_window_move_resize(gtk_widget_get_window(widget),
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
  //gtk_style_set_background(widget->style, gtk_widget_get_window(widget), GTK_STATE_NORMAL);
  gdk_window_set_background(gtk_widget_get_window(widget), &chessboard->priv->widget_background_color);
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
  cairo_region_destroy(priv->chessboard_region);
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
static gboolean cw_chessboard_draw(GtkWidget* widget, cairo_t* cr)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_draw(" << widget << ", " << cr << ")");
  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  CwChessboardPrivate* priv = chessboard->priv;

#if CW_CHESSBOARD_EXPOSE_DEBUG
  gdk_window_clear(gtk_widget_get_window(widget));
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
  gdk_cairo_get_clip_rectangle(cr, &clipbox);
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
    cairo_rectangle_int_t pixmap_rect;
    pixmap_rect.x = priv->top_left_pixmap_x;
    pixmap_rect.y = priv->top_left_pixmap_y;
    pixmap_rect.width = priv->bottom_right_pixmap_x - priv->top_left_pixmap_x;
    pixmap_rect.height = priv->bottom_right_pixmap_y - priv->top_left_pixmap_y;
    cairo_region_t* pixmap_region = cairo_region_create_rectangle(&pixmap_rect);
    cairo_rectangle_int_t region_rect;
    region_rect.x = clipbox.x;
    region_rect.y = clipbox.y;
    region_rect.width = clipbox.width;
    region_rect.height = clipbox.height;
    cairo_region_t* clip_region = cairo_region_create_rectangle(&region_rect);
    if (CW_CHESSBOARD_EXPOSE_ALWAYS_CLEAR_BACKGROUND || priv->redraw_background)
    {
      // If the widget was resized, there might be trash outside the pixmap. Erase that too.
      cairo_region_t* outside_pixmap_region = cairo_region_copy(clip_region);
      cairo_region_subtract(outside_pixmap_region, pixmap_region);
#if 0
      // FIXME: gdk_region_get_rectangles doesn't exist anymore.
      GdkRectangle* outside_areas;
      gint n_outside_areas;
      gdk_region_get_rectangles(outside_pixmap_region, &outside_areas, &n_outside_areas);
      cairo_rectangle_int_t const* outside_rect = outside_areas;
      for (int i = 0; i < n_outside_areas; ++i, ++outside_rect)
	gdk_window_clear_area(gtk_widget_get_window(widget), outside_rect->x, outside_rect->y, outside_rect->width, outside_rect->height);
#if CW_CHESSBOARD_EXPOSE_DEBUG
      // Draw a green rectangle around updated areas.
      GdkGC* debug_gc = gdk_gc_new(priv->pixmap);
      GdkColor debug_green = { 0, 0, 65535, 0 };
      gdk_colormap_alloc_color(gtk_widget_get_colormap(widget), &debug_green, FALSE, TRUE);
      gdk_gc_set_foreground(debug_gc, &debug_green);
      outside_rect = outside_areas;
      for (int i = 0; i < n_outside_areas; ++i, ++outside_rect)
	gdk_draw_rectangle(gtk_widget_get_window(widget), debug_gc, FALSE,
	    outside_rect->x, outside_rect->y, outside_rect->width - 1, outside_rect->height - 1);
      g_object_unref(debug_gc);
#endif
#endif
      cairo_region_destroy(outside_pixmap_region);
      priv->redraw_background = FALSE;
    }
    cairo_region_intersect(clip_region, pixmap_region);
    cairo_region_destroy(clip_region);
    cairo_region_destroy(pixmap_region);
  }

  cairo_t* dest = gdk_cairo_create(gtk_widget_get_window(widget));
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
  // FIXME: we need 'dest' to have the same clip region as what 'cr' has.
//  gdk_cairo_region(dest, region);
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
    gdk_window_get_pointer(gtk_widget_get_window(GTK_WIDGET(chessboard)), NULL, NULL, NULL);
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

gint cw_chessboard_default_calc_board_border_width(G_GNUC_UNUSED CwChessboard const* chessboard, gint sside)
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

static cairo_region_t* convert_mask2region(guint64 mask, gint x, gint y, gint sside, gboolean flip_board)
{
  cairo_region_t* region = cairo_region_create();
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
	cairo_rectangle_int_t rect;
        rect.x = x + (flip_board ? 8 - col_end : col_start) * sside;
	rect.y = y + (flip_board ? row : 7 - row) * sside;
	rect.width = (col_end - col_start) * sside;
        rect.height = sside;
	cairo_region_union_rectangle(region, &rect);
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

  cairo_region_t* need_redraw_region = convert_mask2region(priv->hud_need_redraw[hud], 0, 0, priv->sside, priv->flip_board);
  gdk_cairo_region(cr, need_redraw_region);
  cairo_region_destroy(need_redraw_region);
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
	cairo_region_t* clip_region = convert_mask2region(arrow->has_content[hud], 0, 0, priv->sside, priv->flip_board);
	gdk_cairo_region(cr, clip_region);
	cairo_region_destroy(clip_region);
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
GtkWidget* cw_chessboard_new()
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
    if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
    if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
    if (gtk_widget_get_realized(GTK_WIDGET(chessboard)) && priv->border_width)
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
    if (gtk_widget_get_realized(GTK_WIDGET(chessboard)) && priv->border_width && priv->draw_turn_indicators)
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
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
  if (gtk_widget_get_realized(GTK_WIDGET(chessboard)))
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
  gdk_window_get_pointer(gtk_widget_get_window(widget), &x, &y, NULL);
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
  cairo_rectangle_int_t rect;

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
  gdk_window_invalidate_rect(gtk_widget_get_window(widget), &rect, FALSE);
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
  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);
  gboolean outside_window =
      rect.x + rect.width < 0 ||
      rect.x > allocation.x ||
      rect.y + rect.height < 0 ||
      rect.y > allocation.y;
  // Redraw background of widget if the old place of the floating piece is outside the board.
  priv->redraw_background = priv->redraw_background ||
      rect.x < priv->top_left_pixmap_x ||
      rect.x + rect.width > priv->bottom_right_pixmap_x ||
      rect.y < priv->top_left_pixmap_y ||
      rect.y + rect.height > priv->bottom_right_pixmap_y;
  rect.x = (gint)trunc(x - 0.5 * priv->sside);
  rect.y = (gint)trunc(y - 0.5 * priv->sside);
#if CW_CHESSBOARD_FLOATING_PIECE_INVALIDATE_TARGET || CW_CHESSBOARD_FLOATING_PIECE_DOUBLE_BUFFER
  gdk_window_invalidate_rect(gtk_widget_get_window(widget), &rect, FALSE);
  outside_window = outside_window &&
      (rect.x + rect.width < 0 ||
      rect.x > allocation.x ||
      rect.y + rect.height < 0 ||
      rect.y > allocation.y);
#endif
  if (outside_window && priv->floating_piece[handle].pointer_device)
  {
    Dout(dc::cwchessboardwidget, "Calling gdk_window_get_pointer()");
    gdk_window_get_pointer(gtk_widget_get_window(widget), NULL, NULL, NULL);
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
  cairo_rectangle_int_t rect;
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
  gdk_window_invalidate_rect(gtk_widget_get_window(GTK_WIDGET(chessboard)), &rect, FALSE);
#else
  // FIXME: schedule an expose event instead of this:
  rect.width = priv->sside;
  rect.height = priv->sside;
  gdk_window_invalidate_rect(gtk_widget_get_window(GTK_WIDGET(chessboard)), &rect, FALSE);
#endif

  Dout(dc::cwchessboardwidget, "number_of_floating_pieces = " << priv->number_of_floating_pieces);
  Dout(dc::cwchessboardwidget, "Allocated handle " << handle);

  return handle;
}

void cw_chessboard_remove_floating_piece(CwChessboard* chessboard, gint handle)
{
  Dout(dc::cwchessboardwidget, "Calling cw_chessboard_remove_floating_piece(" << chessboard << ", handle:" << handle << ")");

  CwChessboardPrivate* priv = chessboard->priv;
  cairo_rectangle_int_t rect;

  g_assert(handle >= 0 && (gsize)handle < G_N_ELEMENTS(priv->floating_piece));
  g_assert(!is_empty_square(priv->floating_piece[handle].code));

  // See remark in cw_chessboard_move_floating_piece.
  rect.width = priv->sside;
  rect.height = priv->sside;
  rect.x = priv->floating_piece[handle].pixmap_x + priv->top_left_pixmap_x;
  rect.y = priv->floating_piece[handle].pixmap_y + priv->top_left_pixmap_y;
  gdk_window_invalidate_rect(gtk_widget_get_window(GTK_WIDGET(chessboard)), &rect, FALSE);
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

