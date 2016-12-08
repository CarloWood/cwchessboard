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

//! @file CwChessboard.h
//! @brief This file contains the declaration of the GTK+ widget %CwChessboard.
//! @image html arrowsandmarkers.png

#ifndef CWCHESSBOARD_H
#define CWCHESSBOARD_H

#include <math.h>
#include <glib.h>
#include <gtk/gtkdrawingarea.h>

#ifdef __cplusplus
#define CWCHESSBOARD_INLINE inline
#define CWCHESSBOARD_DEFINE_INLINE 1
#else
#define CWCHESSBOARD_INLINE
#ifndef CWCHESSBOARD_DEFINE_INLINE
#define CWCHESSBOARD_DEFINE_INLINE 0
#endif
#endif

G_BEGIN_DECLS

#ifdef __cplusplus 
#include <stdint.h>
typedef uint16_t CwChessboardCode;
#else
#include "CwChessboardCodes.h"
#endif

GType cw_chessboard_get_type(void) G_GNUC_CONST;

#define CW_TYPE_CHESSBOARD             (cw_chessboard_get_type())
#define CW_CHESSBOARD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CW_TYPE_CHESSBOARD, CwChessboard))
#define CW_CHESSBOARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CW_TYPE_CHESSBOARD,  CwChessboardClass))
#define CW_IS_CHESSBOARD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CW_TYPE_CHESSBOARD))
#define CW_IS_CHESSBOARD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CW_TYPE_CHESSBOARD))
#define CW_CHESSBOARD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CW_TYPE_CHESSBOARD, CwChessboardClass))

typedef struct _CwChessboard CwChessboard;
typedef struct _CwChessboardClass CwChessboardClass;
typedef struct _CwChessboardPrivate CwChessboardPrivate;

/** @typedef CwChessboardColorHandle
 *  @brief A color handle used for background markers.
 *
 *  The five least significant bits determine the color
 *  from a user defined color palet, used by the background squares
 *  and markers. A value of zero meaning the default background value
 *  for that square, or no marker - respectively.
 *
 *  @sa cw_chessboard_allocate_color_handle_rgb, cw_chessboard_allocate_color_handle, cw_chessboard_free_color_handle,
 *      cw_chessboard_set_background_color, cw_chessboard_get_background_color,
 *      cw_chessboard_set_background_colors, cw_chessboard_get_background_colors,
 *      cw_chessboard_set_marker_color, cw_chessboard_get_marker_color
 */
typedef unsigned char CwChessboardColorHandle;

/** @name Creation */
//@{

/**
 * Create a new chessboard widget.
 *
 * @returns Newly created CwChessboard.
 */
GtkWidget* cw_chessboard_new(void);

//@} Creation

#ifdef DOXYGEN
/** @struct CwChessboard
 *  @brief A GTK+ chessboard widget.
 *
 * This is a GTK+ widget. For a gtkmm widget, see cwmm::ChessboardWidget.
 *
 * @sa cw_chessboard_new
 */
struct CwChessboard
#else
struct _CwChessboard
#endif
{
  GtkDrawingArea parent;
  CwChessboardPrivate* priv;

  //! Square side in pixels (read only).
  gint const sside;
  //! The x coordinate of the top-left pixel of square a1 (read-only). Despite the name, if the board is flipped then it's square h8.
  gint const top_left_a1_x;
  //! The y coordinate of the top-left pixel of square a1 (read-only). Despite the name, if the board is flipped then it's square h8.
  gint const top_left_a1_y;
  //! TRUE if the board is flipped (read-only).
  gboolean const flip_board;

  void* gtkmm_widget;		// This is for use by the gtkmm class ChessboardWidget, so don't use it.
};

#ifdef DOXYGEN
/** @struct CwChessboardClass
 *  @brief The Class structure of %CwChessboard
 */
struct CwChessboardClass
#else
struct _CwChessboardClass
#endif
{
  GtkDrawingAreaClass parent_class;

  /** @name Virtual functions of CwChessboard */
  //@{

  /**
   * Calculate the border width of the chessboard as function of the side of a square, in pixels.
   *
   * This function must use the parameter \a sside being passed and <em>not</em>
   * the current sside of the passed \a chessboard (which can still be undefined even).
   * The main reason for this is that sside recursively depends on the result of
   * this function; it is called multiple times until sside stops changing and
   * only then the real sside is set to it's final value.
   *
   * @param chessboard		A #CwChessboard.
   * @param sside		The size of one side of a square.
   *
   * @returns The width of the border in pixels.
   *
   * Default value: #cw_chessboard_default_calc_board_border_width
   */
  gint (*calc_board_border_width)(CwChessboard const* chessboard, gint sside);

  /**
   * Draw the border around the chessboard.
   * This function is called when the border is first drawn,
   * and every time the chessboard is resized.
   *
   * The width of the drawn border should be the value returned by #calc_board_border_width.
   *
   * @param chessboard		A #CwChessboard.
   *
   * Default value: #cw_chessboard_default_draw_border
   *
   * @sa cw_chessboard_set_draw_border
   */
  void (*draw_border)(CwChessboard* chessboard);

  /**
   * Draw the indicator that indicates whose turn it is.
   * This function is called every time the border is redrawn,
   * as well as every time #cw_chessboard_set_draw_turn_indicators
   * is called.
   *
   * @param chessboard		A #CwChessboard.
   * @param white		True if the indicator of the white color has to be drawn.
   * @param on			True if the indictor is on, false if it is off.
   *
   * Default value: #cw_chessboard_default_draw_turn_indicator
   *
   * @sa cw_chessboard_set_draw_border, cw_chessboard_set_draw_turn_indicators
   */
  void (*draw_turn_indicator)(CwChessboard* chessboard, gboolean white, gboolean on);

  /**
   * An array with function pointers for drawing chess pieces.
   *
   * The functions draw one of the six chess pieces, both white and black.
   * The index of the array refers to the piece being drawn:
   * pawn = 0, rook = 1, bishop = 2, knight = 3, queen = 4, king = 5.
   *
   * The default values are given below.
   * They can be overridden by assigning other values to this array.
   *
   * \sa cw_chessboard_draw_pawn,
   *     cw_chessboard_draw_rook,
   *     cw_chessboard_draw_bishop,
   *     cw_chessboard_draw_knight,
   *     cw_chessboard_draw_queen,
   *     cw_chessboard_draw_king
   */
  void (*draw_piece[6])(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

  /**
   * Draw the HUD layer. This is a layer in between the background
   * (existing of the 64 squares with a single color) and the layer
   * with the pieces. It can be used to add some texture to the
   * background.
   * 
   * If the HUD layer is active, then this function is called
   * whenever the widget is resized.
   *
   * @param chessboard		A #CwChessboard.
   * @param cr			A cairo drawing context.
   * @param sside		The size of one side of a square, in pixels.
   * @param hud			The HUD index number, see the detailed documentation on the \ref index "main page" for more info.
   * 
   * Default value: #cw_chessboard_default_draw_hud_layer
   *
   * @sa cw_chessboard_enable_hud_layer,
   *     cw_chessboard_disable_hud_layer,
   *     cw_chessboard_default_draw_hud_layer
   */
  void (*draw_hud_layer)(CwChessboard* chessboard, cairo_t* cr, gint sside, guint hud);

  /**
   * Draw a single HUD square at \a col, \a row.
   * This function is called by cw_chessboard_default_draw_hud_layer
   * for each square. You can use it if you don't override draw_hud_layer.
   *
   * @param chessboard		A #CwChessboard.
   * @param cr			A cairo drawing context.
   * @param col			The column of the square.
   * @param row			The row of the square.
   * @param sside		The size of one side of the square, in pixels.
   * @param hud			The HUD index number, see the detailed documentation of \ref index "main page" for more info.
   *
   * @returns TRUE if anything was drawn at all, FALSE if the HUD square is fully transparent.
   */
  gboolean (*draw_hud_square)(CwChessboard* chessboard, cairo_t* cr, gint col, gint row, gint sside, guint hud);

  /**
   * Notify the user that the mouse pointer left the chessboard.
   *
   * If non-NULL, this function is called every time the mouse pointer
   * leaves the chessboard.
   *
   * Default value: NULL.
   *
   * @param chessboard		A #CwChessboard.
   * @param prev_col		The column of the last square.
   * @param prev_row		The row of the last square.
   */
  void (*cursor_left_chessboard)(CwChessboard* chessboard, gint prev_col, gint prev_row);

  /**
   * Notify the user that the mouse pointer entered a different square.
   *
   * If non-NULL, this function is called every time the mouse pointer enters
   * a new square, whether from outside the chessboard or from another square.
   *
   * Default value: NULL.
   *
   * @param chessboard		A #CwChessboard.
   * @param prev_col		The column of the last square, or -1 if we entered from outside the chessboard.
   * @param prev_row		The row of the last square, or -1 if we entered from outside the chessboard.
   * @param col			The column of the entered square.
   * @param row			The row of the entered square.
   */
  void (*cursor_entered_square)(CwChessboard* chessboard, gint prev_col, gint prev_row, gint col, gint row);

  //@}

};

/**
 * Convert a (\a col, \a row) pair to the top-left coordinates of the corresponding square,
 * relative to the top-left of the widget.
 *
 * @param chessboard	A #CwChessboard.
 * @param col		A column, in the range [0, 7].
 * @param row		A row, in the range [0, 7].
 * @param x		A pointer to where the x-coordinate of the result will be written to.
 * @param y		A pointer to where the y-coordinate of the result will be written to.
 */
CWCHESSBOARD_INLINE
void cw_chessboard_colrow2xy(CwChessboard* chessboard, gint col, gint row, gint* x, gint* y)
#if CWCHESSBOARD_DEFINE_INLINE
{
  *x = chessboard->top_left_a1_x + (chessboard->flip_board ? 7 - col : col) * chessboard->sside;
  *y = chessboard->top_left_a1_y - (chessboard->flip_board ? 7 - row : row) * chessboard->sside;
}
#else
;
#endif

/**
 * Convert an x-coordinate to the column number that it matches.
 * If the x coordinate falls outside the board, then the returned value
 * will be outside the range [0, 7].
 *
 * @param chessboard	A #CwChessboard.
 * @param x		An x coordinate, relative to the left-side of the widget.
 *
 * @returns A column number.
 */
CWCHESSBOARD_INLINE
gint cw_chessboard_x2col(CwChessboard* chessboard, gdouble x)
#if CWCHESSBOARD_DEFINE_INLINE
{
  gint xcol = (gint)floor((x - chessboard->top_left_a1_x) / chessboard->sside);
  return (chessboard->flip_board ? 7 - xcol : xcol);
}
#else
;
#endif

/**
 * Convert a y-coordinate to the row number that it matches.
 * If the y coordinate falls outside the board, then the returned value
 * will be outside the range [0, 7].
 *
 * @param chessboard	A #CwChessboard.
 * @param y		A y coordinate, relative to the top-side of the widget.
 *
 * @returns A row number.
 */
CWCHESSBOARD_INLINE
gint cw_chessboard_y2row(CwChessboard* chessboard, gdouble y)
#if CWCHESSBOARD_DEFINE_INLINE
{
  gint yrow = (gint)floor((chessboard->top_left_a1_y + chessboard->sside - 1 - y) / chessboard->sside);
  return (chessboard->flip_board ? 7 - yrow : yrow);
}
#else
;
#endif

/** @name Chess Position */
//@{

/**
 * Change or remove the piece on the square (\a col, \a row),
 * by replacing the contents of the square with \a code.
 * This does not change any other attribute of the square,
 * like it's background color or marker.
 *
 * @param chessboard	A #CwChessboard.
 * @param col		A column [0..7]
 * @param row		A row [0..7]
 * @param code		A #CwChessboardCode.
 */
void cw_chessboard_set_square(CwChessboard* chessboard, gint col, gint row, CwChessboardCode code);

/**
 * Get the chess piece code for the square at (\a col, \a row).
 */
CwChessboardCode cw_chessboard_get_square(CwChessboard* chessboard, gint col, gint row);

//@} Chess Position

/** @name Border */
//@{

/**
 * Set the boolean which determines whether or not the chessboard widget
 * draws a border around the chessboard. Default: TRUE (draw border).
 *
 * @param chessboard	A #CwChessboard.
 * @param draw		Boolean, determining if the border should be drawn.
 *
 * @sa CwChessboardClass::calc_board_border_width, CwChessboardClass::draw_border
 */
void cw_chessboard_set_draw_border(CwChessboard* chessboard, gboolean draw);

/**
 * Set the boolean which determines whether or not to draw turn indicators.
 * Indicators will only be drawn if also the border is drawn.
 * Default: TRUE (draw indicators).
 *
 * @param chessboard	A #CwChessboard.
 * @param draw		Boolean, determining if the indicators should be drawn.
 *
 * @sa cw_chessboard_set_draw_border, CwChessboardClass::draw_turn_indicator
 */
void cw_chessboard_set_draw_turn_indicators(CwChessboard* chessboard, gboolean draw);

/**
 * Set the boolean that detemines which turn indicator is active (blacks or whites).
 * If drawing turn indicators is enabled and the color is changed then the old
 * indicator is erased. If the turn indicators are disabled, nothing happens.
 *
 * @param chessboard	A #CwChessboard.
 * @param white		TRUE if the white turn indicator should be on (and blacks off).
 *
 * @sa cw_chessboard_set_draw_turn_indicators
 */
void cw_chessboard_set_active_turn_indicator(CwChessboard* chessboard, gboolean white);

/**
 * Set the boolean which determines whether white is playing bottom up or top down.
 * Default: FALSE (white plays upwards).
 *
 * @param chessboard	A #CwChessboard.
 * @param flip		Boolean, determining if white plays upwards or not.
 */
void cw_chessboard_set_flip_board(CwChessboard* chessboard, gboolean flip);

/**
 * Get the boolean that determines whether or not the chessboard widget draws a border around the chessboard.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @returns <code>TRUE</code> if the border is being drawn.
 *
 * @sa cw_chessboard_set_draw_border
 */
gboolean cw_chessboard_get_draw_border(CwChessboard* chessboard);

/**
 * Get the boolean that determines whether or not turn indicators are being drawn.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @returns <code>TRUE</code> if turn indicators are being drawn.
 *
 * @sa cw_chessboard_set_draw_turn_indicators
 */
gboolean cw_chessboard_get_draw_turn_indicators(CwChessboard* chessboard);

/**
 * Get the boolean that detemines which turn indicator is active (blacks or whites).
 *
 * @param chessboard	A #CwChessboard.
 *
 * @returns <code>TRUE</code> if whites turn indicator is active (independent on whether or not it is being drawn).
 *
 * @sa cw_chessboard_set_active_turn_indicator
 */
gboolean cw_chessboard_get_active_turn_indicator(CwChessboard* chessboard);

/**
 * Get the boolean which determines whether white is playing bottom up or top down.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @returns <code>FALSE</code> if white plays upwards and <code>TRUE</code> if white plays downwards.
 *
 * @sa cw_chessboard_set_flip_board
 */
gboolean cw_chessboard_get_flip_board(CwChessboard* chessboard);

/**
 * This is the default value of CwChessboardClass::calc_board_border_width.
 * The formula used by this default function is MAX(8.0, round(1.0 + (sside - 12) / 25.0) + sside / 3.0).
 *
 * @param chessboard	A #CwChessboard.
 * @param sside		The size of one side of a square in pixels.
 *
 * @returns The border width in pixels.
 *
 * @sa CwChessboardClass::calc_board_border_width
 */
gint cw_chessboard_default_calc_board_border_width(CwChessboard const* chessboard, gint sside);

//@} The Border

/** @name Colors */
//@{

/**
 * Set the background color of the dark squares (a1, c1 etc).
 * Default: light green.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new color of the dark squares.
 */
void cw_chessboard_set_dark_square_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Set the background color of the light squares (b1, d1 etc).
 * Default: yellow/white.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new color of the light squares.
 */
void cw_chessboard_set_light_square_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Set the color of the border around the chessboard.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new color of the border.
 *
 * @sa cw_chessboard_get_border_color, cw_chessboard_set_draw_border
 */
void cw_chessboard_set_border_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Set the fill color of the white chess pieces.
 * Default: white
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new fill color of the white pieces.
 */
void cw_chessboard_set_white_fill_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Set the line color of the white chess pieces.
 * Default: black
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new line color of the white pieces.
 */
void cw_chessboard_set_white_line_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Set the fill color of the black chess pieces.
 * Default: black
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new fill color of the black pieces.
 */
void cw_chessboard_set_black_fill_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Set the line color of the black chess pieces.
 * Default: white
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The new line color of the black pieces.
 */
void cw_chessboard_set_black_line_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Retrieve the current background color of the dark squares.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_dark_square_color
 */
void cw_chessboard_get_dark_square_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Retrieve the current background color of the light squares.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_light_square_color
 */
void cw_chessboard_get_light_square_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Retrieve the current color of the border around the chessboard.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_border_color
 */
void cw_chessboard_get_border_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Retrieve the current fill color of the white chess pieces.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_white_fill_color
 */
void cw_chessboard_get_white_fill_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Retrieve the current line color of the white chess pieces.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_white_line_color
 */
void cw_chessboard_get_white_line_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Retrieve the current fill color of the black chess pieces.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_black_fill_color
 */
void cw_chessboard_get_black_fill_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Retrieve the current line color of the black chess pieces.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 *
 * @sa cw_chessboard_set_black_line_color
 */
void cw_chessboard_get_black_line_color(CwChessboard* chessboard, GdkColor* color);

/**
 * Allocate a new CwChessboardColorHandle.
 * Simultaneous, there can be at most 31 different colors.
 * It is the responsibility of the user to free the colors if they are no longer used.
 *
 * @param chessboard	A #CwChessboard.
 * @param red		The red component of the color in the range [0...1].
 * @param green		The green component of the color in the range [0...1].
 * @param blue		The blue component of the color in the range [0...1].
 *
 * @returns A color handle that can be used with #cw_chessboard_set_background_color and #cw_chessboard_set_marker_color.
 *
 * @sa cw_chessboard_allocate_color_handle, cw_chessboard_free_color_handle
 */
CwChessboardColorHandle cw_chessboard_allocate_color_handle_rgb(CwChessboard* chessboard,
    gdouble red, gdouble green, gdouble blue);

/**
 * Allocate a new CwChessboardColorHandle.
 * From more information, see #cw_chessboard_allocate_color_handle_rgb.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The color to allocate.
 *
 * @returns A color handle that can be used with #cw_chessboard_set_background_color and #cw_chessboard_set_marker_color.
 *
 * @sa cw_chessboard_free_color_handle
 */
CWCHESSBOARD_INLINE
CwChessboardColorHandle cw_chessboard_allocate_color_handle(CwChessboard* chessboard, GdkColor const* color)
#if CWCHESSBOARD_DEFINE_INLINE
{
  return cw_chessboard_allocate_color_handle_rgb(chessboard, color->red / 65535.0,
      color->green / 65535.0, color->blue / 65535.0);
}
#else
;
#endif

/**
 * Free up the color handle \a handle, so it can be reused.
 *
 * @param chessboard	A #CwChessboard.
 * @param handle	A color handle as returned by cw_chessboard_allocate_color_handle_rgb or cw_chessboard_allocate_color_handle.
 *
 * @sa cw_chessboard_allocate_color_handle_rgb, cw_chessboard_allocate_color_handle
 */
void cw_chessboard_free_color_handle(CwChessboard* chessboard, CwChessboardColorHandle handle);

/**
 * Set the background color of the square at \a col, \a row.
 *
 * @param chessboard	A #CwChessboard.
 * @param col		The column of the square.
 * @param row		The row of the square.
 * @param handle	A color handle as returned by #cw_chessboard_allocate_color_handle_rgb or
 * 			#cw_chessboard_allocate_color_handle. A handle with a value of 0 means the
 * 			default background color.
 */
void cw_chessboard_set_background_color(CwChessboard* chessboard, gint col, gint row, CwChessboardColorHandle handle);

/**
 * Convenience function.
 *
 * @param chessboard	A #CwChessboard.
 * @param col		The column of the square.
 * @param row		The row of the square.
 *
 * @returns The handle that was passed to #cw_chessboard_set_background_color
 *          for this square, or 0 if the square is not associated with a color handle.
 */
CwChessboardColorHandle cw_chessboard_get_background_color(CwChessboard* chessboard, gint col, gint row);

/**
 * Set new background colors of any number of squares.
 *
 * @param chessboard	A #CwChessboard.
 * @param handles	Array of 64 CwChessboardColorHandles.
 *                      A handle with a value of 0 means the default background color.
 */
void cw_chessboard_set_background_colors(CwChessboard* chessboard, CwChessboardColorHandle const* handles);

/**
 * Fill the array \a handles with the current color handles.
 *
 * @param chessboard	A #CwChessboard.
 * @param handles	The output array. Should be an array of 64 CwChessboardColorHandles.
 */
void cw_chessboard_get_background_colors(CwChessboard* chessboard, CwChessboardColorHandle* handles);

//@} Colors

/** @name Floating Pieces */
//@{

/**
 * This function displays a chess piece with code \a code at widget coordinates (\a x, \a y).
 * Half the side of a square will be subtracted from the coordinates passed, and
 * the result truncated, in order to determine where to draw the top-left corner
 * of the piece. The result is that (\a x, \a y) is more or less the center of the piece.
 *
 * Setting \a pointer_device will cause gdk_window_get_pointer to be called
 * after the next redraw has finished. This is needed to receive the next motion
 * notify event with GDK_POINTER_MOTION_HINT_MASK being used.
 *
 * There may only be one floating piece related the pointer device at a time.
 * If there is already another floating piece related to the pointer device
 * then the value of \a pointer_device is ignored.
 *
 * @param chessboard	A #CwChessboard.
 * @param code		The code of the chess piece to be drawn.
 * @param x		The center x-coordinate of the piece.
 * @param y		The center y-coordinate of the piece.
 * @param pointer_device Whether this piece is under the pointer device or not.
 *
 * @returns A handle that can be passed to each of the functions below.
 *
 * @sa cw_chessboard_get_floating_piece,
 *     cw_chessboard_remove_floating_piece,
 *     cw_chessboard_move_floating_piece
 */
gint cw_chessboard_add_floating_piece(CwChessboard* chessboard,
    CwChessboardCode code, gdouble x, gdouble y, gboolean pointer_device);

/**
 * Move a floating piece with handle \a handle to the new widget
 * coordinates at (\a x, \a y). \a handle must be a handle as
 * returned by #cw_chessboard_add_floating_piece.
 *
 * @param chessboard	A #CwChessboard.
 * @param handle	The floating piece handle.
 * @param x		The new x coordinate.
 * @param y		The new y coordinate.
 *
 * @sa cw_chessboard_add_floating_piece
 */
void cw_chessboard_move_floating_piece(CwChessboard* chessboard, gint handle, gdouble x, gdouble y);

/**
 * Delete the floating piece with handle \a handle.
 * \a handle must be a handle as returned by #cw_chessboard_add_floating_piece.
 *
 * @param chessboard	A #CwChessboard.
 * @param handle	The floating piece handle.
 */
void cw_chessboard_remove_floating_piece(CwChessboard* chessboard, gint handle);

/**
 * Get the CwChessboardCode of the floating piece represented by \a handle.
 * \a handle must be a handle as returned by #cw_chessboard_add_floating_piece.
 *
 * @param chessboard	A #CwChessboard.
 * @param handle	The floating piece handle.
 */
CwChessboardCode cw_chessboard_get_floating_piece(CwChessboard* chessboard, gint handle);

//@} Floating Pieces

/** @name Default Drawing Functions */
//@{

/**
 * This is the default function used by CwChessboard to draw pawns.
 *
 * If \a white is set, a white pawn will be drawn. Otherwise a black pawn.
 *
 * <code>cw_chessboard_draw_pawn</code> is the default function called
 * via a call to CwChessboardClass::draw_piece.
 * It is called every time the chessboard is resized.
 *
 * The function uses vector graphics (by doing direct calls to cairo),
 * and is therefore capable of drawing the piece in any arbitrary size
 * and uses anti-aliasing.
 *
 * @image html pawn.png
 *
 * @param chessboard	A #CwChessboard.
 * @param cr		The cairo drawing context.
 * @param x		The (widget) x-coordinate of the center of the piece.
 * @param y		The (widget) y-coordinate of the center of the piece.
 * @param sside		The assumed side of a square, in pixels.
 * 			<code>chessboard->sside</code> is ignored, so that
 * 			this function can be used to draw pieces elsewhere with a different
 * 			size than what is used on the chessboard.
 * @param white		A boolean that determines if a black or white piece is drawn.
 */
void cw_chessboard_draw_pawn(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

/**
 * This is the default function used by CwChessboard to draw a rook.
 *
 * @image html rook.png
 *
 * See #cw_chessboard_draw_pawn for more details.
 */
void cw_chessboard_draw_rook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

/**
 * This is the default function used by CwChessboard to draw a knight.
 *
 * @image html knight.png
 *
 * See #cw_chessboard_draw_pawn for more details.
 */
void cw_chessboard_draw_knight(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

/**
 * This is the default function used by CwChessboard to draw a bishop.
 *
 * @image html bishop.png
 *
 * See #cw_chessboard_draw_pawn for more details.
 */
void cw_chessboard_draw_bishop(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

/**
 * This is the default function used by CwChessboard to draw a queen.
 *
 * @image html queen.png
 *
 * See #cw_chessboard_draw_pawn for more details.
 */
void cw_chessboard_draw_queen(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

/**
 * This is the default function used by CwChessboard to draw a king.
 *
 * @image html king.png
 *
 * See #cw_chessboard_draw_pawn for more details.
 */
void cw_chessboard_draw_king(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

/**
 * The default CwChessboardClass::draw_hud_layer function.
 * You can restore the default behaviour with:
 * \code
 * CW_CHESSBOARD_GET_CLASS(chessboard)->draw_hud_layer = cw_chessboard_default_draw_hud_layer;
 * \endcode
 *
 * @param chessboard	A #CwChessboard.
 * @param cr		The cairo drawing context.
 * @param sside		The side of one square in pixels.
 * @param hud		The HUD layer (0 or 1).
 *
 * This function calls CwChessboardClass::draw_hud_square for every square.
 */
void cw_chessboard_default_draw_hud_layer(CwChessboard* chessboard, cairo_t* cr, gint sside, guint hud);

/**
 * The default CwChessboardClass::draw_hud_square function.
 * This function is only used by cw_chessboard_default_draw_hud_layer.
 * You can restore the default behaviour of cw_chessboard_default_draw_hud_layer with:
 * \code
 * CW_CHESSBOARD_GET_CLASS(chessboard)->draw_hud_square = cw_chessboard_default_draw_hud_square;
 * \endcode
 *
 * This function hatches the dark squares with fine, diagonal lines.
 *
 * @param chessboard	A #CwChessboard.
 * @param cr		The cairo drawing context.
 * @param col		The column of the square.
 * @param row		The row of the square.
 * @param sside		The side of one square in pixels.
 * @param hud		The HUD layer (0 or 1).
 *
 * @returns <code>TRUE</code> if anything was drawn. <code>FALSE</code> otherwise.
 */
gboolean cw_chessboard_default_draw_hud_square(CwChessboard* chessboard,
    cairo_t* cr, gint col, gint row, gint sside, guint hud);

/**
 * This is the default value of CwChessboardClass::draw_border.
 *
 * @param chessboard	A #CwChessboard.
 */
void cw_chessboard_default_draw_border(CwChessboard* chessboard);

/**
 *  This is the default value of CwChessboardClass::draw_turn_indicator.
 *
 * @param chessboard	A #CwChessboard.
 * @param white		TRUE if it's whites indicator.
 * @param on		TRUE if the indicator has to be drawn, FALSE if the indicator has to be removed.
 */
void cw_chessboard_default_draw_turn_indicator(CwChessboard* chessboard, gboolean white, gboolean on);

//@} Default Virtual Functions

/** @name HUD Layers */
//@{

/**
 * Active a HUD layer.
 * HUD 0 lays between the background and the pieces.
 * HUD 1 lays above the pieces.
 * A custom HUD layer can be created by setting CwChessboardClass::draw_hud_layer.
 *
 * @param chessboard	A #CwChessboard.
 * @param hud		The HUD layer (0 or 1).
 *
 * @sa CwChessboardClass::draw_hud_layer, cw_chessboard_disable_hud_layer
 */
void cw_chessboard_enable_hud_layer(CwChessboard* chessboard, guint hud);

/**
 * Disable the HUD layer again. Used resources are returned to the system.
 *
 * @param chessboard	A #CwChessboard.
 * @param hud		The HUD layer (0 or 1).
 *
 * @sa cw_chessboard_enable_hud_layer
 */
void cw_chessboard_disable_hud_layer(CwChessboard* chessboard, guint hud);

//@} // HUD Layers

/** @name Markers */
//@{

/**
 * Add (or remove) a marker to the square at \a col, \a row.
 *
 * @param chessboard	A #CwChessboard.
 * @param col		The column of the square.
 * @param row		The row of the square.
 * @param mahandle	A color handle as returned by #cw_chessboard_allocate_color_handle_rgb or
 * 			#cw_chessboard_allocate_color_handle. A handle with a value of 0 means the
 * 			default background color.
 */
void cw_chessboard_set_marker_color(CwChessboard* chessboard,
    gint col, gint row, CwChessboardColorHandle mahandle);

/**
 * Convenience function.
 *
 * @param chessboard	A #CwChessboard.
 * @param col		The column of the square.
 * @param row		The row of the square.
 *
 * @returns The handle that was passed to #cw_chessboard_set_marker_color
 *          for this square, or 0 if the square doesn't have a marker.
 */
CwChessboardColorHandle cw_chessboard_get_marker_color(CwChessboard* chessboard, gint col, gint row);

/**
 * Set the marker thickness. This is a value between 0 and 0.5.
 *
 * @param chessboard	A #CwChessboard.
 * @param thickness	The thickness of the marker as fraction of sside. Range [0...0.5]
 *
 * @sa cw_chessboard_get_marker_thickness
 */
void cw_chessboard_set_marker_thickness(CwChessboard* chessboard, gdouble thickness);

/**
 * Get the current marker thickness as fraction of sside.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @sa cw_chessboard_set_marker_thickness
 */
gdouble cw_chessboard_get_marker_thickness(CwChessboard* chessboard);

/**
 * Choose whether markers should be drawn below or above HUD layer 0.
 *
 * Markers can be drawn directly below or directly above HUD layer 0.
 *
 * @param chessboard	A #CwChessboard.
 * @param below		TRUE when markers should be drawn below HUD layer 0.
 */
void cw_chessboard_set_marker_level(CwChessboard* chessboard, gboolean below);

//@} Markers

/** @name Cursor */
//@{

/**
 * Show the cursor.
 *
 * This high-lights the square under the mouse by drawing a
 * square with a configurable thickness and color.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @sa cw_chessboard_set_cursor_thickness, cw_chessboard_get_cursor_thickness
 */
void cw_chessboard_show_cursor(CwChessboard* chessboard);

/**
 * Hide the cursor.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @sa cw_chessboard_show_cursor
 */
void cw_chessboard_hide_cursor(CwChessboard* chessboard);

/**
 * Set the cursor thickness. This is a value between 0 and 0.5.
 *
 * @param chessboard	A #CwChessboard.
 * @param thickness	The thickness of the cursor as fraction of sside. Range [0...0.5]
 *
 * @sa cw_chessboard_get_cursor_thickness
 */
void cw_chessboard_set_cursor_thickness(CwChessboard* chessboard, gdouble thickness);

/**
 * Get the current cursor thickness as fraction of sside.
 *
 * @param chessboard	A #CwChessboard.
 *
 * @sa cw_chessboard_set_cursor_thickness
 */
gdouble cw_chessboard_get_cursor_thickness(CwChessboard* chessboard);

/**
 * Set the color of the cursor.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		The color to be used for the cursor.
 */
void cw_chessboard_set_cursor_color(CwChessboard* chessboard, GdkColor const* color);

/**
 * Get the current cursor color.
 *
 * @param chessboard	A #CwChessboard.
 * @param color		Pointer to the output variable.
 */
void cw_chessboard_get_cursor_color(CwChessboard* chessboard, GdkColor* color);

//@} Cursor

/** @name Arrows */
//@{

/**
 * Draw an arrow on the board.
 *
 * @param chessboard	A #CwChessboard.
 * @param begin_col	The column of the starting square.
 * @param begin_row	The row of the starting square.
 * @param end_col	The column of the ending square.
 * @param end_row	The row of the ending square.
 * @param color		The color to draw the arrow in.
 *
 * @returns A handle that can be used to remove the arrow again.
 *
 * @sa cw_chessboard_remove_arrow
 */
gpointer cw_chessboard_add_arrow(CwChessboard* chessboard,
    gint begin_col, gint begin_row, gint end_col, gint end_row, GdkColor const* color);

/**
 * Remove a previously added arrow.
 *
 * @param chessboard	A #CwChessboard.
 * @param ptr		The arrow handle as returned by #cw_chessboard_add_arrow.
 */
void cw_chessboard_remove_arrow(CwChessboard* chessboard, gpointer ptr);

//@} Arrows

G_END_DECLS

#endif // CWCHESSBOARD_H
