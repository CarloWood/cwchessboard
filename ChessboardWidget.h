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

#pragma once

#include "CwChessboard.h"
#include "debug.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <gtkmm/drawingarea.h>
#pragma GCC diagnostic pop

/** @namespace cwmm
 *  @brief A namespace for all gtkmm related objects.
 */
namespace cwmm {

// The number of squares being drawn on the screen (for debugging purposes).
// If set to less than 8, only the bottom-left corner of the board is show.
static gint const squares = 8;

// The absolute minimum size of a side of a square in pixels.
static gint const min_sside = 12;

// The number of Head-Up Displays.
static guint const number_of_hud_layers = 2;

// Piece encodings.
static int const pawn = 0;
static int const rook = 1;
static int const knight = 2;
static int const bishop = 3;
static int const queen = 4;
static int const king = 5;

/*
 * An RGB color as used by cairo.
 */
struct CairoColor
{
  double red;
  double green;
  double blue;
};

// A piece that is not bound to a square.
struct FloatingPiece
{
  gint x;		        // Current x position of the top-left of the piece, in pixmap coordinates.
  gint y;		        // Current y position of the top-left of the piece, in pixmap coordinates.
  CwChessboardCode code;	// Which piece and which color.
  gboolean moved;		// Temporary set between a move and a redraw.
  gboolean pointer_device;	// Set if this is the pointer device.
};

// Cairo surface cache with the size of one square.
struct SquareCache
{
  Cairo::RefPtr<Cairo::Surface> surface;	// Pointer to the cairo surface of the square.
};

// An arrow drawn on the board.
struct Arrow
{
  gint begin_col;
  gint begin_row;
  gint end_col;
  gint end_row;
  CairoColor color;
  guint64 has_content[number_of_hud_layers];
};

/*
 * Array index type for chessboard square.
 */
using BoardIndex = gint;

class WidgetSegments
{
 private:
  uint64_t m_squares_mask;      // The 64 squares of the chessboard.
  uint64_t m_segments_mask;     // The remaining 44 segments.
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
class ChessboardWidget : public Gtk::DrawingArea
{
  // Colors.
 protected:
  CairoColor m_dark_square_color;	// The color of the dark squares.
  CairoColor m_light_square_color;	// The color of the light squares.
 private:
  CairoColor m_board_border_color;	// The color of the border.
  CairoColor m_white_piece_fill_color;	// The fill color of the white pieces.
  CairoColor m_white_piece_line_color;	// The line color of the white pieces.
  CairoColor m_black_piece_fill_color;	// The fill color of the black pieces.
  CairoColor m_black_piece_line_color;	// The line color of the black pieces.
  GdkColor m_widget_background_color;	// The background color of the widget.
  CairoColor m_color_palet[31];		// 31 colors for markers and backgrounds.
  guint32 m_allocated_colors_mask;	// The colors of color_palet that are used.
  CairoColor m_cursor_color;		// The color of the cursor.

  // Other configuration.
  gboolean m_draw_border;		// Do we need to draw a border?
  gboolean m_flip_board;		// White or black at the bottom?
  gboolean m_draw_turn_indicators;	// Do we need to draw turn indicators?
  gboolean m_active_turn_indicator;	// Is whites turn indicator active (or blacks?).
  gboolean m_has_hud_layer[number_of_hud_layers];	// Do we need to call the user provided HUD draw function?
  gdouble m_marker_thickness;		// Thickness of the markers as fraction of sside.
  gboolean m_marker_below;		// Whether the markers are drawn below HUD 0.
  gdouble m_cursor_thickness;		// Thickness of the cursor as fraction of sside.
  gboolean m_show_cursor;		// True if the cursor should be visible.

  // Sizes and offsets.
  gint m_edge_x;                        // The x coordinate of the top-left of the board (including border).
  gint m_edge_y;                        // The y coordinate of the top-left of the board (including border).
  gint m_sside;				// The size of one side of a square.
  gint m_border_width;			// The border width.
  Cairo::RefPtr<Cairo::Region> m_chessboard_region;  // The rectangular region where the chessboard resides.
  gint m_marker_thickness_px;		// Thickness of the markers.
  gint m_cursor_thickness_px;		// Thickness of the cursor.
  gint m_cursor_col;			// Current cursor column.
  gint m_cursor_row;			// Current cursor row.
  Cairo::RectangleInt m_top_or_left_background_rect;            // Widget area above or to the left of the board.
  Cairo::RectangleInt m_bottom_or_right_background_rect;        // Widget area below or to the right of the board.

  // Buffers and caches.
  SquareCache m_piece_surface[12];	// 12 images using piece_buffer.
  Cairo::RefPtr<Cairo::Surface> m_hud_layer_surface[number_of_hud_layers];	// The HUD layers.
 protected:
  gint64 m_hud_has_content[number_of_hud_layers];		// Which HUD squares are not entirely transparent?
 private:
  gint64 m_hud_need_redraw[number_of_hud_layers];		// Which HUD squares need a redraw?
  SquareCache m_hatching_surface;	// Cache used for the hatch lines.

  CwChessboardCode m_board_codes[64];	// The chessboard, contains pieces and background colors.
  gint64 m_need_redraw_invalidated;	// Which squares are invalidated?
  gint64 m_need_redraw;			// Which squares need a redraw?
  bool m_turn_indicators_invalidated;   // Set when invalidate_turn_indicators was called.
  bool m_border_invalidated;            // Set when invalidate_border was called.

  gsize m_number_of_floating_pieces;	// Current number of floating pieces.
  FloatingPiece m_floating_piece[32];	// Current floating pieces.
  gint m_floating_piece_handle;		// The handle of the floating piece under the pointer device, or -1.
  gboolean m_redraw_background;		// Set when the window was recently resized (reset in the expose function).
  bool m_resized;                       // Set when on_size_allocate was called. Reset when handled in on_draw.

  std::vector<Arrow*> m_arrows;		// Array with pointers to Arrow objects.

#ifdef CWDEBUG
  bool m_inside_on_draw;                        // True when inside on_draw. Used for debugging.
  bool m_show_buffer;                           // True when m_buffer must be copied to the screen.
  Cairo::RefPtr<Cairo::Surface> m_buffer;       // Temporary buffer for debugging purposes.
#endif

  gint m_total_size;                    // The size of the adjusted allocation.
  bool m_redraw_pixmap;
  bool m_redraw_pieces;

  static constexpr CwChessboardCode s_color_mask = 0x0001;
  static constexpr CwChessboardCode s_piece_mask = 0x000e;
  static constexpr CwChessboardCode s_piece_color_mask = 0x000f;
  static constexpr CwChessboardCode s_bghandle_mask = 0x01f0;           // BackGround of square.
  static constexpr CwChessboardCode s_mahandle_mask = 0x3e00;           // MArker.

  static gboolean is_empty_square(CwChessboardCode code) { return (code & s_piece_mask) == 0; }

  // This assumes it is a piece (not an empty square).
  static int convert_code2piece(CwChessboardCode code) { return ((code & s_piece_mask) >> 1) - 1; }

  // This assumes it is a piece (not an empty square).
  static int convert_code2piece_index(CwChessboardCode code) { return (code & s_piece_color_mask) - 2; }

  // This assumes it is a piece (not an empty square).
  static gboolean is_white_piece(CwChessboardCode code) { return (code & s_color_mask) != 0; }

  static CwChessboardCode convert_piece2code(int piece, gboolean white) { return (unsigned char)((piece << 1) | (white ? 3 : 2)); }

  static gint convert_code2bghandle(CwChessboardCode code) { return (code & s_bghandle_mask) >> 4; }

  // This should be OR-ed with the other data.
  static CwChessboardCode convert_bghandle2code(gint bghandle) { return bghandle << 4; }

  static gint convert_code2mahandle(CwChessboardCode code) { return (code & s_mahandle_mask) >> 9; }

  // This should be OR-ed with the other data.
  static CwChessboardCode convert_mahandle2code(gint mahandle) { return mahandle << 9; }

 public:
  /** @name Construction/Destruction */
  //@{

    //! @brief Create a ChessboardWidget object.
    ChessboardWidget();
    //! @brief Destructor.
    ~ChessboardWidget() override;

  //@}

 private:

  void set_fill_color(cairo_t* cr, gboolean white);
  void set_line_color(cairo_t* cr, gboolean white);
  void recreate_hud_layers(Cairo::RefPtr<Cairo::Context> const& cr);
  void redraw_pixmap(Cairo::RefPtr<Cairo::Context> const& cr);
  void invalidate_border();
  void invalidate_turn_indicators();
  void invalidate_square(gint col, gint row);
  void invalidate_board();
  void invalidate_markers();
  void invalidate_cursor();
  guint64 invalidate_arrow(gint col1, gint row1, gint col2, gint row2);
  void redraw_square(Cairo::RefPtr<Cairo::Context> const& cr, gint index);
  void redraw_pieces(Cairo::RefPtr<Cairo::Surface> const& surface);
  void redraw_hud_layer(guint hud);
  void update_cursor_position(gdouble x, gdouble y, gboolean forced);

  uint64_t x_to_segmentsOnAndToTheRight(int x) const;
  uint64_t y_to_segmentsOnAndBelow(int y) const;
  uint64_t x_to_segmentsOnAndToTheLeft(int x) const;
  uint64_t y_to_segmentsOnAndAbove(int y) const;

  uint64_t rect_to_44segments(Cairo::Rectangle const& rect) const
  {
    return
      x_to_segmentsOnAndToTheRight(rect.x) &
      x_to_segmentsOnAndToTheLeft(rect.x + rect.width) &
      y_to_segmentsOnAndBelow(rect.y) &
      y_to_segmentsOnAndAbove(rect.y + rect.height);
  }

  WidgetSegments rect_to_segments(Cairo::Rectangle const& rect) const;

  /*
   * Convert a (column, row) pair (each running from 0 till 7) to a BoardIndex value.
   */
  static BoardIndex convert_colrow2index(gint col, gint row)
  {
    return col | (row << 3);
  }

  /*
   * Convert a BoardIndex value to the column of the corresponding square.
   * A value of 0 corresponds to 'a', 1 corresponds to 'b' and so on, till 7 corresponding to column 'h'.
   */
  static gint convert_index2column(BoardIndex index)
  {
    // The last 3 bits contain the col.
    return index & 0x7;
  }

  /*
   * Convert a BoardIndex value to the row of the corresponding square.
   */
  static gint convert_index2row(BoardIndex index)
  {
    // Bits 3 till 6 contain the row. Higher bits are zero.
    return index >> 3;
  }

  /*
   * Convert a BoardIndex value to the top-left widget coordinates of the corresponding square.
   */
  void convert_index2xy(BoardIndex index, gint& x, gint& y)
  {
    colrow2xy(convert_index2column(index), convert_index2row(index), x, y);
  }

  /*
   * Convert the widget coordinates (\a x, \a y) to the BoardIndex of the corresponding square.
   */
  BoardIndex convert_xy2index(gdouble x, gdouble y)
  {
    gint col = x2col(x);
    if ((col & ~0x7))
      return -1;
    gint row = y2row(y);
    if ((row & ~0x7))
      return -1;
    return convert_colrow2index(col, row);
  }

  /** @name Events */
  //@{

   protected:
    /** @brief Called when the mouse button is pressed while on the chessboard widget.
     *
     * This function should be overridden. See the example tstcpp.cxx in the source code for an example.
     */
    bool on_button_press_event(GdkEventButton* event) override
    {
      DoutEntering(dc::notice, "ChessboardWidget::on_button_press_event(" << event << ")");
      return Gtk::DrawingArea::on_button_press_event(event);
    }

    /** @brief Called when the mouse button is released again.
     *
     * This function should be overridden. See the example tstcpp.cxx in the source code for an example.
     */
    bool on_button_release_event(GdkEventButton* event) override
    {
      DoutEntering(dc::notice, "ChessboardWidget::on_button_release_event(" << event << ")");
      return Gtk::DrawingArea::on_button_release_event(event);
    }

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

    /** @brief Called when (a part of) the chessboard drawing area needs to be (re)draw.
     *
     * @param cr        : The cairo context to draw on.
     */
    bool on_draw(Cairo::RefPtr<Cairo::Context> const& cr) override;

    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    void on_realize() override;
    void on_unrealize() override;
    void on_size_allocate(Gtk::Allocation& allocation) override;
    void get_preferred_width_vfunc(int& minimum_width, int& natural_width) const override;
    void get_preferred_height_vfunc(int& minimum_height, int& natural_height) const override;

  //@}

  /** @name Drawing primitives */
  //@{

    /** @brief Called to draw a pawn.
     */
    virtual void draw_pawn(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

    /** @brief Called to draw a rook.
     */
    virtual void draw_rook(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

    /** @brief Called to draw a knight.
     */
    virtual void draw_knight(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

    /** @brief Called to draw a bishop.
     */
    virtual void draw_bishop(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

    /** @brief Called to draw a queen.
     */
    virtual void draw_queen(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

    /** @brief Called to draw a king.
     */
    virtual void draw_king(cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white);

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
    virtual void draw_hud_layer(Cairo::RefPtr<Cairo::Context> const& cr, gint sside, guint hud);

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
    virtual gboolean draw_hud_square(Cairo::RefPtr<Cairo::Context> const& cr, gint col, gint row, gint sside, guint hud);

    /** @brief Draw the border around the chessboard.
     *
     * This function is called when the border is first drawn, and every time the chessboard is resized.
     * The width of the drawn border should be the value returned by CwChessboardClass::calc_board_border_width.
     *
     * @sa set_draw_border
     */
    virtual void draw_border(Cairo::RefPtr<Cairo::Context> const& cr);

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
    virtual void draw_turn_indicator(Cairo::RefPtr<Cairo::Context> const& cr, gboolean white, gboolean on);

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
    gint sside() const { return m_sside; }

    gint top_left_edge_x() const      { return m_edge_x; }
    gint top_left_edge_y() const      { return m_edge_y; }

    gint top_left_board_x() const     { return m_edge_x + m_border_width; }
    gint top_left_board_y() const     { return m_edge_y + m_border_width; }

    gint bottom_right_board_x() const { return m_edge_x + m_border_width + squares * m_sside; }
    gint bottom_right_board_y() const { return m_edge_y + m_border_width + squares * m_sside; }

    gint bottom_right_edge_x() const  { return m_edge_x + m_border_width + squares * m_sside + m_border_width; }
    gint bottom_right_edge_y() const  { return m_edge_y + m_border_width + squares * m_sside + m_border_width; }

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
    {
      x = top_left_board_x() + (m_flip_board ? 7 - col : col) * m_sside;
      y = top_left_board_y() + (m_flip_board ? row : 7 - row) * m_sside;
    }

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
    {
      gint xcol = std::floor((x - top_left_board_x()) / m_sside);
      return (m_flip_board ? 7 - xcol : xcol);
    }

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
    {
      gint yrow = std::floor((bottom_right_board_y() - 1 - y) / m_sside);
      return m_flip_board ? 7 - yrow : yrow;
    }

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
    void set_square(gint col, gint row, code_t code);

    /** @brief Get what is currently on a square.
     *
     * Get the chess piece code for the square at (\a col, \a row).
     */
    code_t get_square(gint col, gint row) const;

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
    void set_draw_border(gboolean draw);

    /** @brief Get the boolean that determines whether or not the chessboard widget draws a border around the chessboard.
     *
     * @returns <code>TRUE</code> if the border is being drawn.
     *
     * @sa set_draw_border
     */
    gboolean get_draw_border() const { return m_draw_border; }

    /** @brief Set the boolean which determines whether or not to draw turn indicators.
     *
     * Indicators will only be drawn if also the border is drawn.
     * Default: TRUE (draw indicators).
     *
     * @param draw	Boolean, determining if the indicators should be drawn.
     *
     * @sa set_draw_border, draw_turn_indicator
     */
    void set_draw_turn_indicators(gboolean draw);

    /** @brief Get the boolean which determines whether or not to draw turn indicators.
     *
     * @sa set_draw_turn_indicators
     */
    gboolean get_draw_turn_indicators() const { return m_draw_turn_indicators; }

    /** @brief Set the color of the active turn indicator.
     *
     * Default: TRUE
     *
     * @param white	TRUE if whites turn indicator should be active.
     *
     * @sa set_draw_turn_indicators
     */
    void set_active_turn_indicator(gboolean white);

    /** @brief Get the boolean which determines whether whites or black turn indicator is active.
     *
     * @sa set_active_turn_indicator
     */
    gboolean get_active_turn_indicator() const { return m_active_turn_indicator; }

    /** @brief Set the boolean which determines whether white is playing bottom up or top down.
     *
     * Default: FALSE (white plays upwards).
     *
     * @param flip	Boolean, determining if white plays upwards or not.
     */
    void set_flip_board(gboolean flip);

    /** @brief Get the boolean which determines whether white is playing bottom up or top down.
     *
     * @sa set_flip_board
     */
    gboolean get_flip_board() const { return m_flip_board; }

#if 0 // Not supported at the moment.
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
#endif

    static gint default_calc_board_border_width(gint sside)
    {
      // Make line width run from 1.0 (at sside == 12) to 4.0 (at sside == 87)).
      // Round to nearest even integer. Then add sside / 3. Return at least 8.
      return std::max(8.0, std::round(1.0 + (sside - 12) / 25.0) + sside / 3.0);
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
    void set_dark_square_color(GdkColor const& color);

    /** @brief Set the color of the light squares.
     *
     * Set the background color of the light squares (b1, d1 etc).
     * Default: yellow/white.
     *
     * @param color : The new color of the light squares.
     */
    void set_light_square_color(GdkColor const& color);

    /** @brief Set the color of the border around the chessboard.
     *
     * @param color : The new color of the border.
     *
     * @sa get_border_color, set_draw_border
     */
    void set_border_color(GdkColor const& color);

    /** @brief Set the fill color of the white chess pieces.
     *
     * Default: white
     *
     * @param color : The new fill color of the white pieces.
     */
    void set_white_fill_color(GdkColor const& color);

    /** @brief Set the line color of the white chess pieces.
     *
     * Default: black
     *
     * @param color : The new line color of the white pieces.
     */
    void set_white_line_color(GdkColor const& color);

    /** @brief Set the fill color of the black chess pieces.
     *
     * Default: black
     *
     * @param color : The new fill color of the black pieces.
     */
    void set_black_fill_color(GdkColor const& color);

    /** @brief Set the line color of the black chess pieces.
     *
     * Default: white
     *
     * @param color : The new line color of the black pieces.
     */
    void set_black_line_color(GdkColor const& color);

    /** @brief Retrieve the current background color of the dark squares.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_dark_square_color
     */
    void get_dark_square_color(GdkColor& color) const;

    /** @brief Retrieve the current background color of the light squares.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_light_square_color
     */
    void get_light_square_color(GdkColor& color) const;

    /** @brief Retrieve the current color of the border around the chessboard.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_border_color
     */
    void get_border_color(GdkColor& color) const;

    /** @brief Retrieve the current fill color of the white chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_white_fill_color
     */
    void get_white_fill_color(GdkColor& color) const;

    /** @brief Retrieve the current line color of the white chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_white_line_color
     */
    void get_white_line_color(GdkColor& color) const;

    /** @brief Retrieve the current fill color of the black chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_black_fill_color
     */
    void get_black_fill_color(GdkColor& color) const;

    /** @brief Retrieve the current line color of the black chess pieces.
     *
     * @param color : Pointer to the output variable.
     *
     * @sa set_black_line_color
     */
    void get_black_line_color(GdkColor& color) const;

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
    ColorHandle allocate_color_handle_rgb(gdouble red, gdouble green, gdouble blue);

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
    {
      return allocate_color_handle_rgb(color.red / 65535.0, color.green / 65535.0, color.blue / 65535.0);
    }

    /** @brief Free up the color handle \a handle, so it can be reused.
     *
     * @param handle : A color handle as returned by allocate_color_handle_rgb or allocate_color_handle.
     *
     * @sa allocate_color_handle_rgb, allocate_color_handle
     */
    void free_color_handle(ColorHandle handle);

    /** @brief Set the background color of the square at \a col, \a row.
     *
     * @param col    : The column of the square.
     * @param row    : The row of the square.
     * @param handle : A color handle as returned by #allocate_color_handle_rgb or
     *                 #allocate_color_handle. A handle with a value of 0 means the
     *                 default background color.
     */
    void set_background_color(gint col, gint row, ColorHandle handle);

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
    {
      return convert_code2bghandle(m_board_codes[convert_colrow2index(col, row)]);
    }

    /** @brief Set new background colors of any number of squares.
     *
     * @param handles : Array of 64 ColorHandles.
     *                  A handle with a value of 0 means the default background color.
     */
    void set_background_colors(ColorHandle const* handles);

    /** @brief Get all background colors handles.
     *
     * Fill the array \a handles with the current color handles.
     *
     * @param handles : The output array. Should be an array of 64 ColorHandles.
     */
    void get_background_colors(ColorHandle* handles) const;

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
    gint add_floating_piece(code_t code, gdouble x, gdouble y, gboolean pointer_device);

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
    void move_floating_piece(gint handle, gdouble x, gdouble y);

    /** @brief Remove a floating piece.
     *
     * Delete the floating piece with handle \a handle.
     * \a handle must be a handle as returned by #add_floating_piece.
     *
     * @param handle : The floating piece handle.
     */
    void remove_floating_piece(gint handle);

    /** @brief Determine what piece a given floating piece is.
     *
     * Get the code_t of the floating piece represented by \a handle.
     * \a handle must be a handle as returned by #add_floating_piece.
     *
     * @param handle : The floating piece handle.
     */
    code_t get_floating_piece(gint handle) const
    {
      g_assert(handle >= 0 && handle < (gint)G_N_ELEMENTS(m_floating_piece));
      return m_floating_piece[handle].code;
    }

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
    void enable_hud_layer(guint hud);

    /** @brief Disable the HUD layer again.
     *
     * Used resources are returned to the system.
     *
     * @param hud : The HUD layer (0 or 1).
     *
     * @sa enable_hud_layer
     */
    void disable_hud_layer(guint hud)
    {
      g_return_if_fail(hud < number_of_hud_layers);
      m_has_hud_layer[hud] = false;
      m_hud_need_redraw[hud] = (guint64)-1;
    }

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
    void set_marker_color(gint col, gint row, ColorHandle mahandle);

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
    {
      return convert_code2mahandle(m_board_codes[convert_colrow2index(col, row)]);
    }

    /** @brief Set the marker thickness.
     *
     * This is a value between 0 and 0.5.
     *
     * @param thickness : The thickness of the marker as fraction of sside. Range [0...0.5]
     *
     * @sa get_marker_thickness
     */
    void set_marker_thickness(gdouble thickness);

    /** @brief Get the current marker thickness as fraction of sside.
     *
     * @sa set_marker_thickness
     */
    gdouble get_marker_thickness() const
    {
      return m_marker_thickness;
    }

    /** @brief Choose whether markers should be drawn below or above HUD layer 0.
     *
     * Markers can be drawn directly below or directly above HUD layer 0.
     *
     * @param below : TRUE when markers should be drawn below HUD layer 0.
     */
    void set_marker_level(gboolean below)
    {
      m_marker_below = below;
    }

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
    void show_cursor();

    /** @brief Hide the cursor.
     *
     * @sa show_cursor
     */
    void hide_cursor();

    /** @brief Set the thickness of the cursor.
     *
     * This is a value between 0 and 0.5.
     *
     * @param thickness : The thickness of the cursor as fraction of sside. Range [0...0.5]
     *
     * @sa get_cursor_thickness
     */
    void set_cursor_thickness(gdouble thickness);

    /** @brief Get the current cursor thickness as fraction of sside.
     *
     * @sa set_cursor_thickness
     */
    gdouble get_cursor_thickness() const
    {
      return m_cursor_thickness;
    }

    /** @brief Set the color of the cursor.
     *
     * @param color : The color to be used for the cursor.
     */
    void set_cursor_color(GdkColor const& color);

    /** @brief Get the current cursor color.
     *
     * @param color : Pointer to the output variable.
     */
    void get_cursor_color(GdkColor& color) const;

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
    gpointer add_arrow(gint begin_col, gint begin_row, gint end_col, gint end_row, GdkColor const& color);

    /** @brief Remove a previously added arrow.
     *
     * @param ptr : The arrow handle as returned by #add_arrow.
     */
    void remove_arrow(gpointer ptr);

  //@}
};

} // namespace cwmm
