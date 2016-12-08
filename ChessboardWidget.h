// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file ChessboardWidget.h This file contains the declaration of the gtkmm class ChessboardWidget.
//
// Copyright (C) 2008, by
// 
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#ifndef USE_PCH
#include "debug.h"
#endif

#include <gtkmm/drawingarea.h>
#include "CwChessboard.h"

/** @namespace cwmm
 *  @brief A namespace for all gtkmm related objects.
 */
namespace cwmm {

/** @internal
 *
 * A helper class, needed because we must initialize CwChessboard before we can initialize Gtk::DrawingArea.
 */
class CwChessboardPtr {
  protected:
    CwChessboard* M_chessboard;
    CwChessboardPtr(void) : M_chessboard(CW_CHESSBOARD(cw_chessboard_new())) { }
};

/** @class ChessboardWidget
 *  @brief A gtkmm chessboard widget.
 *
 * This is a chessboard widget for use with gtkmm.
 *
 * A detailed overview can be found on the <a href="index.html">Main Page</a>.
 *
 * @sa ChessPositionWidget
 */
class ChessboardWidget : public CwChessboardPtr, public Gtk::DrawingArea {
  public:
  /** @name Construction/Destruction */
  //@{

    //! @brief Create a ChessboardWidget object.
    ChessboardWidget(void);
    //! @brief Destructor.
    virtual ~ChessboardWidget();

  //@}

  private:
    static void S_draw_pawn_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);
    static void S_draw_rook_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);
    static void S_draw_knight_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);
    static void S_draw_bishop_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);
    static void S_draw_queen_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);
    static void S_draw_king_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);
    static void S_draw_hud_layer_hook(CwChessboard* chessboard, cairo_t* cr, gint sside, guint hud);
    static gboolean S_draw_hud_square_hook(CwChessboard* chessboard, cairo_t* cr, gint col, gint row, gint sside, guint hud);
    static void S_draw_border_hook(CwChessboard* chessboard);
    static void S_draw_turn_indicator_hook(CwChessboard* chessboard, gboolean white, gboolean on);
    static void S_on_cursor_left_chessboard_hook(CwChessboard* chessboard, gint prev_col, gint prev_row);
    static void S_on_cursor_entered_square_hook(CwChessboard* chessboard, gint prev_col, gint prev_row, gint col, gint row);

  /** @name Events */
  //@{

  protected:
    /** @brief Called when the mouse button is pressed while on the chessboard widget.
     *
     * This function should be overridden. See the example tstcpp.cc in the source code for an example.
     */
    virtual bool on_button_press_event(GdkEventButton* event) { return Gtk::DrawingArea::on_button_press_event(event); }

    /** @brief Called when the mouse button is released again.
     *
     * This function should be overridden. See the example tstcpp.cc in the source code for an example.
     */
    virtual bool on_button_release_event(GdkEventButton* event) { return Gtk::DrawingArea::on_button_release_event(event); }

    /** @brief Called when the mouse pointer left the chessboard.
     *
     * @param prev_col : The column of the last square.
     * @param prev_row : The row of the last square.
     */
    virtual void on_cursor_left_chessboard(gint prev_col, gint prev_row)
        { Dout(dc::notice, "Calling on_cursor_left_chessboard(" << prev_col << ", " << prev_row << ")"); }

    /** @brief Called when the mouse pointer entered a new square.
      *
      * @param prev_col : The column of the last square, or -1 if we entered from outside the chessboard.
      * @param prev_row : The row of the last square, or -1 if we entered from outside the chessboard.
      * @param col	: The column of the entered square.
      * @param row	: The row of the entered square.
     */
    virtual void on_cursor_entered_square(gint prev_col, gint prev_row, gint col, gint row)
        { Dout(dc::notice, "Calling on_cursor_entered_square(" << prev_col << ", " << prev_row << ", " << col << ", " << row << ")"); }

  //@}

  /** @name Drawing primitives */
  //@{

    /** @brief Called to draw a pawn.
     *
     * The default calls #cw_chessboard_draw_pawn.
     */
    virtual void draw_pawn(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
        { cw_chessboard_draw_pawn(M_chessboard, cr, x, y, sside, white); }

    /** @brief Called to draw a rook.
     *
     * The default calls #cw_chessboard_draw_rook.
     */
    virtual void draw_rook(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
        { cw_chessboard_draw_rook(M_chessboard, cr, x, y, sside, white); }

    /** @brief Called to draw a knight.
     *
     * The default calls #cw_chessboard_draw_knight.
     */
    virtual void draw_knight(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
        { cw_chessboard_draw_knight(M_chessboard, cr, x, y, sside, white); }

    /** @brief Called to draw a bishop.
     *
     * The default calls #cw_chessboard_draw_bishop.
     */
    virtual void draw_bishop(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
        { cw_chessboard_draw_bishop(M_chessboard, cr, x, y, sside, white); }

    /** @brief Called to draw a queen.
     *
     * The default calls #cw_chessboard_draw_queen.
     */
    virtual void draw_queen(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
        { cw_chessboard_draw_queen(M_chessboard, cr, x, y, sside, white); }

    /** @brief Called to draw a king.
     *
     * The default calls #cw_chessboard_draw_king.
     */
    virtual void draw_king(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
        { cw_chessboard_draw_king(M_chessboard, cr, x, y, sside, white); }

    /** @brief Draw the HUD layer.
     *
     * This is a layer in between the background (existing of the 64 squares with
     * a single color) and the layer with the pieces. It can be used to add some
     * texture to the background.
     * 
     * If the HUD layer is active, then this function is called
     * whenever the widget is resized.
     *
     * @param cr    : A cairo drawing context.
     * @param sside : The size of one side of a square, in pixels.
     * @param hud   : The HUD index number, see the detailed documentation on the \ref index "main page" for more info.
     * 
     * The default calls #cw_chessboard_default_draw_hud_layer.
     *
     * @sa enable_hud_layer, disable_hud_layer
     */
    virtual void draw_hud_layer(cairo_t* cr, gint sside, guint hud)
        { cw_chessboard_default_draw_hud_layer(M_chessboard, cr, sside, hud); }

    /** @brief Draw a single HUD square at \a col, \a row.
     *
     * This function is called by @ref draw_hud_layer "ChessboardWidget::draw_hud_layer" for each square.
     * You can use it if you don't override #draw_hud_layer.
     *
     * @param cr    : A cairo drawing context.
     * @param col   : The column of the square.
     * @param row   : The row of the square.
     * @param sside : The size of one side of the square, in pixels.
     * @param hud   : The HUD index number, see the detailed documentation of \ref index "main page" for more info.
     *
     * @returns TRUE if anything was drawn at all, FALSE if the HUD square is fully transparent.
     *
     * The default calls #cw_chessboard_default_draw_hud_square.
     */
    virtual gboolean draw_hud_square(cairo_t* cr, gint col, gint row, gint sside, guint hud)
        { return cw_chessboard_default_draw_hud_square(M_chessboard, cr, col, row, sside, hud); }

    /** @brief Draw the border around the chessboard.
     *
     * This function is called when the border is first drawn, and every time the chessboard is resized.
     * The width of the drawn border should be the value returned by CwChessboardClass::calc_board_border_width.
     *
     * The default calls #cw_chessboard_default_draw_border.
     *
     * @sa set_draw_border
     */
    virtual void draw_border(void)
        { return cw_chessboard_default_draw_border(M_chessboard); }

    /** @brief Draw the indicator that indicates whose turn it is.
     *
     * This function is called every time the border is redrawn,
     * as well as every time #set_draw_turn_indicators is called.
     *
     * @param white	True if the indicator of the white color has to be drawn.
     * @param on	True if the indictor is on, false if it is off.
     *
     * The default calls #cw_chessboard_default_draw_turn_indicator.
     *
     * @sa set_draw_border, set_draw_turn_indicators
     */
    virtual void draw_turn_indicator(gboolean white, gboolean on)
        { return cw_chessboard_default_draw_turn_indicator(M_chessboard, white, on); }

  //@}

  public:

  // Typedefs

    /** @typedef ColorHandle
     *  @brief A color handle used for background markers.
     *
     *  The five least significant bits determine the color
     *  from a user defined color palet, used by the background squares
     *  and markers. A value of zero meaning the default background value
     *  for that square, or no marker - respectively.
     *
     *  @sa allocate_color_handle_rgb, allocate_color_handle, free_color_handle,
     *      set_background_color, get_background_color, set_background_colors,
     *      get_background_colors, set_marker_color, get_marker_color
     */
    typedef CwChessboardColorHandle ColorHandle;

    /** @typedef code_t
     *  @brief A code to specify a chess piece.
     *
     * One of the following constants:
     * #empty_square, #black_pawn, #white_pawn, #black_rook, #white_rook, #black_knight, #white_knight,
     * #black_bishop, #white_bishop, #black_queen, #white_queen, #black_king or #white_king.
     *
     * @sa set_square, floating_piece, get_floating_piece
     */
    typedef CwChessboardCode code_t;

  /** @name Accessors */
  //@{

    //! @brief The side of a square in pixels.
    gint sside(void) const { return M_chessboard->sside; }
    /** @brief The x coordinate of the top-left pixel of square a1.
     *
     * If the board is flipped, then the x coordinate of the top-left pixel of square h8 is returned.
     * In other words, this value is not depending on whether or not the board is flipped.
     */
    gint top_left_a1_x(void) const { return M_chessboard->top_left_a1_x; }
    /** @brief The y coordinate of the top-left pixel of square a1.
     *
     * If the board is flipped, then the y coordinate of the top-left pixel of square h8 is returned.
     * In other words, this value is not depending on whether or not the board is flipped.
     */
    gint top_left_a1_y(void) const { return M_chessboard->top_left_a1_y; }

  //@}

  /** @name Public Conversion Functions */
  //@{

    /** @brief Convert a (\a col, \a row) pair to the top-left coordinates of the corresponding square, relative to the top-left of the widget.
     *
     * @param col : A column, in the range [0, 7].
     * @param row : A row, in the range [0, 7].
     * @param x	  : A reference to where the x-coordinate of the result will be written to.
     * @param y	  : A reference to where the y-coordinate of the result will be written to.
     */
    void colrow2xy(gint col, gint row, gint& x, gint& y) const
        { cw_chessboard_colrow2xy(M_chessboard, col, row, &x, &y); }

    /** @brief Convert an x-coordinate to the column number that it matches.
     *
     * If the x coordinate falls outside the board, then the returned value
     * will be outside the range [0, 7].
     *
     * @param x	: An x coordinate, relative to the left-side of the widget.
     *
     * @returns A column number.
     */
    gint x2col(gdouble x) const
        { return cw_chessboard_x2col(M_chessboard, x); }

    /** @brief Convert a y-coordinate to the row number that it matches.
     *
     * If the y coordinate falls outside the board, then the returned value
     * will be outside the range [0, 7].
     *
     * @param y : A y coordinate, relative to the top-side of the widget.
     *
     * @returns A row number.
     */
    gint y2row(gdouble y) const
        { return cw_chessboard_y2row(M_chessboard, y); }

    /** @brief Test if a given column and row are on the chessboard.
     *
     * @param col : A column, in the range [0, 7].
     * @param row : A row, in the range [0, 7].
     *
     * @returns True if (col, row) falls inside the chessboard.
     */
    static gboolean is_inside_board(gint col, gint row)
	{ return !((col | row) & ~0x7); }

  //@}

  /** @name Chess Position */
  //@{

    /** @brief Change the piece on a square.
     *
     * Change or remove the piece on the square (\a col, \a row),
     * by replacing the contents of the square with \a code.
     * This does not change any other attribute of the square,
     * like it's background color or marker.
     *
     * @param col  : A column [0..7]
     * @param row  : A row [0..7]
     * @param code : A #code_t.
     */
    void set_square(gint col, gint row, code_t code)
        { cw_chessboard_set_square(M_chessboard, col, row, code); }

    /** @brief Get what is currently on a square.
     *
     * Get the chess piece code for the square at (\a col, \a row).
     */
    code_t get_square(gint col, gint row) const
        { return cw_chessboard_get_square(M_chessboard, col, row); }

  //@}

  /** @name Border */
  //@{

    /** @brief Set the boolean which determines whether or not the chessboard widget draws a border around the chessboard.
     *
     * Default: TRUE (draw border).
     *
     * @param draw : Boolean, determining if the border should be drawn.
     *
     * @sa draw_border
     */
    void set_draw_border(gboolean draw)
        { cw_chessboard_set_draw_border(M_chessboard, draw); }

    /** @brief Get the boolean that determines whether or not the chessboard widget draws a border around the chessboard.
     *
     * @returns <code>TRUE</code> if the border is being drawn.
     *
     * @sa set_draw_border
     */
    gboolean get_draw_border(void) const
        { return cw_chessboard_get_draw_border(M_chessboard); }

    /** @brief Set the boolean which determines whether or not to draw turn indicators.
     *
     * Indicators will only be drawn if also the border is drawn.
     * Default: TRUE (draw indicators).
     *
     * @param draw	Boolean, determining if the indicators should be drawn.
     *
     * @sa set_draw_border, draw_turn_indicator
     */
    void set_draw_turn_indicators(gboolean draw)
        { return cw_chessboard_set_draw_turn_indicators(M_chessboard, draw); }

    /** @brief Get the boolean which determines whether or not to draw turn indicators.
     *
     * @sa set_draw_turn_indicators
     */
    gboolean get_draw_turn_indicators(void) const
        { return cw_chessboard_get_draw_turn_indicators(M_chessboard); }

    /** @brief Set the color of the active turn indicator.
     *
     * Default: TRUE
     *
     * @param white	TRUE if whites turn indicator should be active.
     *
     * @sa set_draw_turn_indicators
     */
    void set_active_turn_indicator(gboolean white)
        { return cw_chessboard_set_active_turn_indicator(M_chessboard, white); }

    /** @brief Get the boolean which determines whether whites or black turn indicator is active.
     *
     * @sa set_active_turn_indicator
     */
    gboolean get_active_turn_indicator(void) const
        { return cw_chessboard_get_active_turn_indicator(M_chessboard); }

    /** @brief Set the boolean which determines whether white is playing bottom up or top down.
     *
     * Default: FALSE (white plays upwards).
     *
     * @param flip	Boolean, determining if white plays upwards or not.
     */
    void set_flip_board(gboolean flip)
        { return cw_chessboard_set_flip_board(M_chessboard, flip); }

    /** @brief Get the boolean which determines whether white is playing bottom up or top down.
     *
     * @sa set_flip_board
     */
    gboolean get_flip_board(void) const
        { return cw_chessboard_get_flip_board(M_chessboard); }

    /** @brief Set the calc_board_border_width function.
     *
     * Change the function that calculates the border width as function of the size of the squares.
     *
     * @param new_calc_board_border_width : The new function.
     *
     * @sa CwChessboardClass::calc_board_border_width
     */
    void set_calc_board_border_width(gint (*new_calc_board_border_width)(CwChessboard const*, gint))
        {
	  CW_CHESSBOARD_GET_CLASS(M_chessboard)->calc_board_border_width = new_calc_board_border_width;
	}
  //@}

  /** @name Colors */
  //@{

    /** @brief Set the color of the dark squares.
     *
     * Set the background color of the dark squares (a1, c1 etc).
     * Default: light green.
     *
     * @param color : The new color of the dark squares.
     */
    void set_dark_square_color(GdkColor const& color)
        { cw_chessboard_set_dark_square_color(M_chessboard, &color); }

    /** @brief Set the color of the light squares.
     *
     * Set the background color of the light squares (b1, d1 etc).
     * Default: yellow/white.
     *
     * @param color : The new color of the light squares.
     */
    void set_light_square_color(GdkColor const& color)
        { cw_chessboard_set_light_square_color(M_chessboard, &color); }

    /** @brief Set the color of the border around the chessboard.
     *
     * @param color : The new color of the border.
     *
     * @sa get_border_color, set_draw_border
     */
    void set_border_color(GdkColor const& color)
        { cw_chessboard_set_border_color(M_chessboard, &color); }

    /** @brief Set the fill color of the white chess pieces.
     *
     * Default: white
     *
     * @param color : The new fill color of the white pieces.
     */
    void set_white_fill_color(GdkColor const& color)
        { cw_chessboard_set_white_fill_color(M_chessboard, &color); }

    /** @brief Set the line color of the white chess pieces.
     *
     * Default: black
     *
     * @param color : The new line color of the white pieces.
     */
    void set_white_line_color(GdkColor const& color)
        { cw_chessboard_set_white_line_color(M_chessboard, &color); }

    /** @brief Set the fill color of the black chess pieces.
     *
     * Default: black
     *
     * @param color : The new fill color of the black pieces.
     */
    void set_black_fill_color(GdkColor const& color)
        { cw_chessboard_set_black_fill_color(M_chessboard, &color); }

    /** @brief Set the line color of the black chess pieces.
     *
     * Default: white
     *
     * @param color : The new line color of the black pieces.
     */
    void set_black_line_color(GdkColor const& color)
        { cw_chessboard_set_black_line_color(M_chessboard, &color); }

    /** @brief Retrieve the current background color of the dark squares.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_dark_square_color
     */
    void get_dark_square_color(GdkColor& color) const
        { cw_chessboard_get_dark_square_color(M_chessboard, &color); }

    /** @brief Retrieve the current background color of the light squares.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_light_square_color
     */
    void get_light_square_color(GdkColor& color) const
        { cw_chessboard_get_light_square_color(M_chessboard, &color);  }

    /** @brief Retrieve the current color of the border around the chessboard.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_border_color
     */
    void get_border_color(GdkColor& color) const
        { cw_chessboard_get_border_color(M_chessboard, &color); }

    /** @brief Retrieve the current fill color of the white chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_white_fill_color
     */
    void get_white_fill_color(GdkColor& color) const
        { cw_chessboard_get_white_fill_color(M_chessboard, &color); }

    /** @brief Retrieve the current line color of the white chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_white_line_color
     */
    void get_white_line_color(GdkColor& color) const
        { cw_chessboard_get_white_line_color(M_chessboard, &color); }

    /** @brief Retrieve the current fill color of the black chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_black_fill_color
     */
    void get_black_fill_color(GdkColor& color) const
        { cw_chessboard_get_black_fill_color(M_chessboard, &color); }

    /** @brief Retrieve the current line color of the black chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_black_line_color
     */
    void get_black_line_color(GdkColor& color) const
        { cw_chessboard_get_black_line_color(M_chessboard, &color); }

    /** @brief Allocate a new ColorHandle.
     *
     * Simultaneous, there can be at most 31 different colors.
     * It is the responsibility of the user to free the colors if they are no longer used.
     *
     * @param red   : The red component of the color in the range [0...1].
     * @param green : The green component of the color in the range [0...1].
     * @param blue  : The blue component of the color in the range [0...1].
     *
     * @returns A color handle that can be used with #set_background_color and #set_marker_color.
     *
     * @sa allocate_color_handle, free_color_handle
     */
    ColorHandle allocate_color_handle_rgb(gdouble red, gdouble green, gdouble blue)
        { return cw_chessboard_allocate_color_handle_rgb(M_chessboard, red, green, blue); }

    /** @brief Allocate a new CwChessboardColorHandle.
     *
     * From more information, see #allocate_color_handle_rgb.
     *
     * @param color : The color to allocate.
     *
     * @returns A color handle that can be used with #set_background_color and #set_marker_color.
     *
     * @sa free_color_handle
     */
    ColorHandle allocate_color_handle(GdkColor const& color)
        { return cw_chessboard_allocate_color_handle(M_chessboard, &color); }

    /** @brief Free up the color handle \a handle, so it can be reused.
     *
     * @param handle : A color handle as returned by allocate_color_handle_rgb or allocate_color_handle.
     *
     * @sa allocate_color_handle_rgb, allocate_color_handle
     */
    void free_color_handle(ColorHandle handle)
        { cw_chessboard_free_color_handle(M_chessboard, handle); }

    /** @brief Set the background color of the square at \a col, \a row.
     *
     * @param col    : The column of the square.
     * @param row    : The row of the square.
     * @param handle : A color handle as returned by #allocate_color_handle_rgb or
     *                 #allocate_color_handle. A handle with a value of 0 means the
     *                 default background color.
     */
    void set_background_color(gint col, gint row, ColorHandle handle)
        { cw_chessboard_set_background_color(M_chessboard, col, row, handle); }

    /** @brief Get the current background color handle.
     *
     * Convenience function.
     *
     * @param col : The column of the square.
     * @param row : The row of the square.
     *
     * @returns The handle that was passed to #set_background_color
     *          for this square, or 0 if the square is not associated
     *          with a color handle.
     */
    ColorHandle get_background_color(gint col, gint row) const
        { return cw_chessboard_get_background_color(M_chessboard, col, row); }

    /** @brief Set new background colors of any number of squares.
     *
     * @param handles : Array of 64 ColorHandles.
     *                  A handle with a value of 0 means the default background color.
     */
    void set_background_colors(ColorHandle const* handles)
        { cw_chessboard_set_background_colors(M_chessboard, handles); }

    /** @brief Get all background colors handles.
     *
     * Fill the array \a handles with the current color handles.
     *
     * @param handles : The output array. Should be an array of 64 ColorHandles.
     */
    void get_background_colors(ColorHandle* handles) const
        { cw_chessboard_get_background_colors(M_chessboard, handles); }

  //@}

  /** @name Floating Pieces */
  //@{

    /** @brief Add a new floating chess piece.
     *
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
     * @param code           : The code of the chess piece to be drawn.
     * @param x              : The center x-coordinate of the piece.
     * @param y              : The center y-coordinate of the piece.
     * @param pointer_device : Whether this piece is under the pointer device or not.
     *
     * @returns A handle that can be passed to each of the functions below.
     *
     * @sa get_floating_piece, remove_floating_piece, move_floating_piece
     */
    gint add_floating_piece(code_t code, gdouble x, gdouble y, gboolean pointer_device)
        { return cw_chessboard_add_floating_piece(M_chessboard, code, x, y, pointer_device); }

    /** @brief Move a floating chess piece to a new position.
     *
     * Move a floating piece with handle \a handle to the new widget
     * coordinates at (\a x, \a y). \a handle must be a handle as
     * returned by #add_floating_piece.
     *
     * @param handle : The floating piece handle.
     * @param x      : The new x coordinate.
     * @param y      : The new y coordinate.
     *
     * @sa add_floating_piece
     */
    void move_floating_piece(gint handle, gdouble x, gdouble y)
        { cw_chessboard_move_floating_piece(M_chessboard, handle, x, y); }

    /** @brief Remove a floating piece.
     *
     * Delete the floating piece with handle \a handle.
     * \a handle must be a handle as returned by #add_floating_piece.
     *
     * @param handle : The floating piece handle.
     */
    void remove_floating_piece(gint handle)
        { cw_chessboard_remove_floating_piece(M_chessboard, handle); }

    /** @brief Determine what piece a given floating piece is.
     *
     * Get the code_t of the floating piece represented by \a handle.
     * \a handle must be a handle as returned by #add_floating_piece.
     *
     * @param handle : The floating piece handle.
     */
    code_t get_floating_piece(gint handle) const
        { return cw_chessboard_get_floating_piece(M_chessboard, handle); }

  //@}

  /** @name HUD Layers */
  //@{

    /** @brief Active a HUD layer.
     *
     * HUD 0 lays between the background and the pieces.
     * HUD 1 lays above the pieces.
     * A custom HUD layer can be created by setting CwChessboardClass::draw_hud_layer.
     *
     * @param hud : The HUD layer (0 or 1).
     *
     * @sa CwChessboardClass::draw_hud_layer, disable_hud_layer
     */
    void enable_hud_layer(guint hud)
        { cw_chessboard_enable_hud_layer(M_chessboard, hud); }

    /** @brief Disable the HUD layer again.
     *
     * Used resources are returned to the system.
     *
     * @param hud : The HUD layer (0 or 1).
     *
     * @sa enable_hud_layer
     */
    void disable_hud_layer(guint hud)
        { cw_chessboard_disable_hud_layer(M_chessboard, hud); }
    
  //@}

  /** @name Markers */
  //@{

    /** @brief Change the color of the marker.
     *
     * Add a marker to, or remove a marker from the square at \a col, \a row.
     *
     * @param col      : The column of the square.
     * @param row      : The row of the square.
     * @param mahandle : A color handle as returned by #allocate_color_handle_rgb or #allocate_color_handle.
     *                   A handle with a value of 0 removes the marker (if any was there).
     */
    void set_marker_color(gint col, gint row, ColorHandle mahandle)
        { cw_chessboard_set_marker_color(M_chessboard, col, row, mahandle); }

    /** @brief Get marker color.
     *
     * Convenience function.
     *
     * @param col : The column of the square.
     * @param row : The row of the square.
     *
     * @returns The handle that was passed to #set_marker_color
     *          for this square, or 0 if the square doesn't have a marker.
     */
    ColorHandle get_marker_color(gint col, gint row) const
        { return cw_chessboard_get_marker_color(M_chessboard, col, row); }

    /** @brief Set the marker thickness.
     *
     * This is a value between 0 and 0.5.
     *
     * @param thickness : The thickness of the marker as fraction of sside. Range [0...0.5]
     *
     * @sa get_marker_thickness
     */
    void set_marker_thickness(gdouble thickness)
        { return cw_chessboard_set_marker_thickness(M_chessboard, thickness); }

    /** @brief Get the current marker thickness as fraction of sside.
     *
     * @sa set_marker_thickness
     */
    gdouble get_marker_thickness(void) const
        { return cw_chessboard_get_marker_thickness(M_chessboard); }

    /** @brief Choose whether markers should be drawn below or above HUD layer 0.
     *
     * Markers can be drawn directly below or directly above HUD layer 0.
     *
     * @param below : TRUE when markers should be drawn below HUD layer 0.
     */
    void set_marker_level(gboolean below)
        { return cw_chessboard_set_marker_level(M_chessboard, below); }

  //@}

  /** @name Cursor */
  //@{

    /** @brief Show the cursor.
     *
     * This high-lights the square under the mouse by drawing a
     * square with a configurable thickness and color.
     *
     * @sa set_cursor_thickness, get_cursor_thickness
     */
    void show_cursor(void)
        { cw_chessboard_show_cursor(M_chessboard); }

    /** @brief Hide the cursor.
     *
     * @sa show_cursor
     */
    void hide_cursor(void)
        { cw_chessboard_hide_cursor(M_chessboard); }

    /** @brief Set the thickness of the cursor.
     *
     * This is a value between 0 and 0.5.
     *
     * @param thickness : The thickness of the cursor as fraction of sside. Range [0...0.5]
     *
     * @sa get_cursor_thickness
     */
    void set_cursor_thickness(gdouble thickness)
        { cw_chessboard_set_cursor_thickness(M_chessboard, thickness); }

    /** @brief Get the current cursor thickness as fraction of sside.
     *
     * @sa set_cursor_thickness
     */
    gdouble get_cursor_thickness(void) const
        { return cw_chessboard_get_cursor_thickness(M_chessboard); }

    /** @brief Set the color of the cursor.
     *
     * @param color : The color to be used for the cursor.
     */
    void set_cursor_color(GdkColor const& color)
        { cw_chessboard_set_cursor_color(M_chessboard, &color); }

    /** @brief Get the current cursor color.
     *
     * @param color : Pointer to the output variable.
     */
    void get_cursor_color(GdkColor& color) const
        { cw_chessboard_get_cursor_color(M_chessboard, &color); }

  //@}

  /** @name Arrows */
  //@{

    /** @brief Draw an arrow on the board.
     *
     * @param begin_col : The column of the starting square.
     * @param begin_row : The row of the starting square.
     * @param end_col   : The column of the ending square.
     * @param end_row   : The row of the ending square.
     * @param color     : The color to draw the arrow in.
     *
     * @returns A handle that can be used to remove the arrow again.
     *
     * @sa remove_arrow
     */
    gpointer add_arrow(gint begin_col, gint begin_row, gint end_col, gint end_row, GdkColor const& color)
        { return cw_chessboard_add_arrow(M_chessboard, begin_col, begin_row, end_col, end_row, &color); }

    /** @brief Remove a previously added arrow.
     *
     * @param ptr : The arrow handle as returned by #add_arrow.
     */
    void remove_arrow(gpointer ptr)
        { cw_chessboard_remove_arrow(M_chessboard, ptr); }

  //@}
};

} // namespace cwmm

#endif	// CHESSBOARD_H
