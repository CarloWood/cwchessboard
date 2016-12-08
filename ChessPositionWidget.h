// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file ChessPositionWidget.h This file contains the declaration of class ChessPositionWidget.
//
// Copyright (C) 2008 - 2010, by
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

#ifndef CHESSPOSITIONWIDGET_H
#define CHESSPOSITIONWIDGET_H

#ifndef USE_PCH
#include <boost/shared_ptr.hpp>
#include <gtkmm/menu.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/window.h>
#endif

#include "ChessboardWidget.h"
#include "ChessPosition.h"
#include "Promotion.h"

namespace cwmm {

/** @brief A chessboard widget that is synchronized with a \link cwchess::ChessPosition ChessPosition \endlink.
 *
 * This is the object that you want to use (instead of ChessboardWidget).
 *
 * It combines cwmm::ChessboardWidget with cwchess::ChessPosition,
 * keeping the position on both synchronized.
 *
 * Apart from the reimplemented ChessPosition methods listed above, every other method
 * of both base classes are public <em>except</em>:
 *
 * - ChessboardWidget::set_square : Use \link ChessPositionWidget::place place \endlink instead.
 * - ChessboardWidget::get_square : Use \link cwchess::ChessPosition::piece_at piece_at \endlink instead.
 * - ChessboardWidget::set_active_turn_indicator : Use \link ChessPositionWidget::to_move(cwchess::Color const&) to_move \endlink instead.
 * - ChessboardWidget::get_active_turn_indicator : Use \link cwchess::ChessPosition::to_move() to_move \endlink instead.
 *
 * This object has three builtin modes:
 * - Position setup (see #mode_edit_position).
 * - Game play (see #mode_edit_game).
 * - Disable widget (see #mode_disabled).
 *
 * In game play mode, the user can only pick up pieces of the color
 * that is to move, and only do legal moves (illegal moves cause
 * the piece to be put back where it came from).
 *
 * In position setup mode, the user can move pieces arbitrarily around
 * and use a right-click popup menu to select and place new pieces
 * on the board, clear a square, select which color is to move,
 * copy and paste to/from the clipboard, swap colors, clear the
 * board, set up the initial position, mark a pawn as being allowed
 * to taken en passant and mark rooks or kings has having moved.
 *
 * Finally, the 'disable widget' mode just means that the user is
 * not allowed to pick up or place pieces. This would be the typical
 * state to put the widget in while the user is waiting for the
 * opponent to move.
 *
 * In order to allow a flexible interaction with the widget, the
 * following signals and events are provided:
 *
 * - Mouse leaves the chessboard (see #on_cursor_left_chessboard).
 * - Mouse enters a (different) square (see #on_cursor_entered_square).
 * - A mouse button was pressed (see #on_button_press).
 * - A mouse button was released (see #on_button_release).
 * - A piece was picked up (see \link ChessPositionWidget::signal_picked_up signal_picked_up \endlink).
 * - A piece was dropped (see \link ChessPositionWidget::signal_dropped signal_dropped \endlink).
 * - A (legal) move was executed (see \link ChessPositionWidget::signal_moved signal_moved \endlink).
 * - An illegal move was attempted (see \link ChessPositionWidget::signal_illegal signal_illegal \endlink).
 */
class ChessPositionWidget : protected cwchess::ChessPosition, public cwmm::ChessboardWidget {
  public:
  /** @name Get/Set Widget Modes */
  //@{

    //! The type of the builtin widget modes.
    typedef unsigned int widget_mode_type;
    //! The value used for the 'Edit Position' mode.
    static widget_mode_type const mode_edit_position = 0;
    //! The value used for the 'Edit Game' mode.
    static widget_mode_type const mode_edit_game = 1;
    //! The value used for the 'Widget disabled' mode.
    static widget_mode_type const mode_disabled = 2;

    //! Return the current widget mode.
    widget_mode_type get_widget_mode(void) const { return M_widget_mode; }
    //! Set the widget mode.
    void set_widget_mode(widget_mode_type widget_mode) { M_widget_mode = widget_mode; }

  //@}

  private:
    //! The current widget mode.
    widget_mode_type M_widget_mode;
    //! The signal generator for 'piece pick up' events.
    sigc::signal<void, cwchess::Index const&, cwchess::ChessPosition const&> M_signal_picked_up;
    //! The signal generator for 'piece drop' events.
    sigc::signal<void, gint, gint, cwchess::ChessPosition const&> M_signal_dropped;
    //! The handle of the floating piece under the mouse pointer, if any.
    gint M_floating_piece_handle;
    //! Temporary storage for copied positions.
    Glib::ustring M_clipboard_content;
    //! Set when trying primrary clipboard contents.
    bool M_trying_primary;

  /** @name 'Edit Game' mode related variables */
  //@{

    //! The square that a piece was picked up from.
    cwchess::Index M_move_from;
    //! The object used to promote pawns.
    Glib::RefPtr<cwchess::Promotion> M_promotion;
    //! The signal generator for legal moves.
    sigc::signal<void, cwchess::Move const&, cwchess::ChessPosition const&, cwchess::ChessPosition const&> M_signal_moved;
    //! The signal generator for illegal moves.
    sigc::signal<void, cwchess::Move const&, cwchess::ChessPosition const&> M_signal_illegal;

  //@}

  /** @name 'Edit Position' mode related variables */
  //@{

  protected:
    //! The square that a new piece is being placed on with the popup menu, in 'Edit Position' mode.
    cwchess::Index M_placepiece_index;
    //! A pointer to a drawable used for it's colormap (for the icons in the popup menu).
    Gtk::Window* M_drawable;
    //! An instance of the popup menu to place new pieces.
    Gtk::Menu* M_MenuPopup;
    //! Reference to a UIManager.
    Glib::RefPtr<Gtk::UIManager> M_refUIManager;
    //! Reference to a ActionGroup.
    Glib::RefPtr<Gtk::ActionGroup> M_refActionGroup;
    //! Reference to a IconFactory.
    Glib::RefPtr<Gtk::IconFactory> M_refIconFactory;
    //! Reference to RadioAction for ToMoveWhite.
    Glib::RefPtr<Gtk::RadioAction> M_refToMoveWhite_action;
    //! Reference to RadioAction for ToMoveBlack.
    Glib::RefPtr<Gtk::RadioAction> M_refToMoveBlack_action;
    //! Reference to ToggleAction for PieceHasMoved.
    Glib::RefPtr<Gtk::ToggleAction> M_refPieceHasMoved_action;
    //! Reference to ToggleAction for AllowEnPassantCapture.
    Glib::RefPtr<Gtk::ToggleAction> M_refAllowEnPassantCapture_action;
    //! The HasMoved ToggleAction connection.
    sigc::connection M_PieceHasMoved_connection;
    //! The AllowEnPassantCapture ToggleAction connection.
    sigc::connection M_AllowEnPassantCapture_connection;

  //@}

  public:
  /** @name Constructor */
  //@{

    /** @brief Constructor.
     *
     * @param drawable : A drawable, usually the main window of the application.
     * @param promotion : An object derived from Promotion that handles pawn promotions.
     */
    ChessPositionWidget(Gtk::Window* drawable, Glib::RefPtr<cwchess::Promotion> promotion = Glib::RefPtr<cwchess::Promotion>(new cwchess::Promotion)) :
        M_floating_piece_handle(-1), M_widget_mode(mode_edit_position), M_promotion(promotion), M_MenuPopup(NULL), M_drawable(drawable), M_trying_primary(false)
    {
      // Continue initialization when realized.
      drawable->signal_realize().connect(sigc::mem_fun(this, &ChessPositionWidget::initialize_menus));
    }

  //@}

  private:
    void initialize_menus(void);
    bool popup_menu(GdkEventButton* event, int col, int row);
    void popup_deactivated(void);
    void update_paste_status(void);

  protected:
    virtual void on_menu_placepiece_black_pawn(void);
    virtual void on_menu_placepiece_black_rook(void);
    virtual void on_menu_placepiece_black_knight(void);
    virtual void on_menu_placepiece_black_bishop(void);
    virtual void on_menu_placepiece_black_queen(void);
    virtual void on_menu_placepiece_black_king(void);
    virtual void on_menu_placepiece_white_pawn(void);
    virtual void on_menu_placepiece_white_rook(void);
    virtual void on_menu_placepiece_white_knight(void);
    virtual void on_menu_placepiece_white_bishop(void);
    virtual void on_menu_placepiece_white_queen(void);
    virtual void on_menu_placepiece_white_king(void);
    virtual void on_menu_placepiece_nothing(void);
    virtual void on_menu_allow_en_passant_capture(void);
    virtual void on_menu_piece_has_moved(void);
    virtual void on_menu_copy_FEN(void);
    virtual void on_menu_paste_FEN(void);
    virtual void on_menu_swap_colors(void);
    virtual void on_menu_initial_position(void);
    virtual void on_menu_clear_board(void);
    virtual void on_menu_to_move_white(void);
    virtual void on_menu_to_move_black(void);
    virtual void on_clipboard_get(Gtk::SelectionData& selection_data, guint info);
    virtual void on_clipboard_clear(void);
    virtual void on_clipboard_received(Glib::ustring const& text);
    virtual void on_clipboard_received_targets(Glib::StringArrayHandle const& targets_array);

  /** @name ChessPostion methods : Hidden base class members.
   *
   * ChessboardWidget::set_square : Use \link ChessPositionWidget::place place \endlink instead.
   * ChessboardWidget::get_square : Use \link cwchess::ChessPosition::piece_at piece_at \endlink instead.
   * ChessboardWidget::set_active_turn_indicator : Use \link ChessPositionWidget::to_move(cwchess::Color const&) to_move \endlink instead.
   * ChessboardWidget::get_active_turn_indicator : Use \link cwchess::ChessPosition::to_move() to_move \endlink instead.
   */
  //@{

  protected:
    using ChessboardWidget::set_square;
    using ChessboardWidget::get_square;
    using ChessboardWidget::set_active_turn_indicator;
    using ChessboardWidget::get_active_turn_indicator;

  //@}

  public:
    using ChessboardWidget::trackable;
    using ChessboardWidget::ColorHandle;
    using ChessboardWidget::code_t;
    using ChessboardWidget::colrow2xy;
    using ChessboardWidget::x2col;
    using ChessboardWidget::y2row;
    using ChessboardWidget::is_inside_board;

  /** @name ChessPostion methods : Position setup
   *
   * For a description of these methods, see the member functions
   * with the same name in cwchess::ChessPosition.
   */
  //@{
    
    using ChessPosition::set_half_move_clock;
    using ChessPosition::set_full_move_number;
    using ChessPosition::set_has_moved;
    using ChessPosition::clear_has_moved;

    //! See cwchess::ChessPosition::clear.
    void clear(void) { ChessPosition::clear(); sync(); }
    //! See cwchess::ChessPosition::initial_position.
    void initial_position(void) { ChessPosition::initial_position(); sync(); }
    //! See cwchess::ChessPosition::skip_move.
    bool skip_move(void) { bool result = ChessPosition::skip_move(); set_active_turn_indicator(to_move().is_white()); return result; }
    //! See cwchess::ChessPosition::to_move.
    void to_move(cwchess::Color const& color) { ChessPosition::to_move(color); set_active_turn_indicator(to_move().is_white()); }
    //! See cwchess::ChessPosition::set_en_passant.
    bool set_en_passant(cwchess::Index const& index) { ChessPosition::set_en_passant(index); set_active_turn_indicator(to_move().is_white()); }
    //! See cwchess::ChessPosition::swap_colors.
    void swap_colors(void) { ChessPosition::swap_colors(); sync(); }
    //! See cwchess::ChessPosition::place.
    bool place(cwchess::Code const& code, cwchess::Index const& index) { if (ChessPosition::place(code, index)) set_square(index.col(), index.row(), code); }
    //! See cwchess::ChessPosition::load_FEN.
    bool load_FEN(std::string const& FEN);

  //@}

  /** @name ChessPostion methods : Accessors
   *
   * For a description of these methods, see the member functions
   * with the same name in cwchess::ChessPosition.
   */
  //@{

  public:
    using ChessPosition::piece_at;
    using ChessPosition::to_move;
    using ChessPosition::half_move_clock;
    using ChessPosition::full_move_number;
    using ChessPosition::castle_flags;
    using ChessPosition::en_passant;
    using ChessPosition::all;

  //@}

  /** @name ChessPostion methods : Visitors
   *
   * For a description of these methods, see the member functions
   * with the same name in cwchess::ChessPosition.
   */
  //@{

  public:
    using ChessPosition::FEN;
    using ChessPosition::candidates_table_offset;
    using ChessPosition::candidates;
    using ChessPosition::reachables;
    using ChessPosition::defendables;
    using ChessPosition::index_of_king;
    using ChessPosition::check;
    using ChessPosition::double_check;
    using ChessPosition::moves;
    using ChessPosition::legal;
    
  //@}

  /** @name ChessPostion methods : Iterators
   *
   * For a description of these methods, see the member functions
   * with the same name in cwchess::ChessPosition.
   */
  //@{

  public:
    using ChessPosition::piece_begin;
    using ChessPosition::piece_end;
    using ChessPosition::move_begin;
    using ChessPosition::move_end;
    
  //@}

  /** @name ChessPostion methods : Game play
   *
   * For a description of these methods, see the member functions
   * with the same name in cwchess::ChessPosition.
   */
  //@{

    //! See cwchess::ChessPosition::execute.
    bool execute(cwchess::Move const& move);
    
  //@}

  /** @name ChessboardWidget methods : Accessors
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::sside;
    using ChessboardWidget::top_left_a1_x;
    using ChessboardWidget::top_left_a1_y;

  //@}
    
  /** @name ChessboardWidget methods : Border
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::set_draw_border;
    using ChessboardWidget::get_draw_border;
    using ChessboardWidget::set_draw_turn_indicators;
    using ChessboardWidget::get_draw_turn_indicators;
    using ChessboardWidget::set_flip_board;
    using ChessboardWidget::get_flip_board;
    using ChessboardWidget::set_calc_board_border_width ;

  //@}

  /** @name ChessboardWidget methods : Border
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

 public:
   using ChessboardWidget::set_dark_square_color;
   using ChessboardWidget::set_light_square_color;
   using ChessboardWidget::set_border_color;
   using ChessboardWidget::set_white_fill_color;
   using ChessboardWidget::set_white_line_color;
   using ChessboardWidget::set_black_fill_color;
   using ChessboardWidget::set_black_line_color;
   using ChessboardWidget::get_dark_square_color;
   using ChessboardWidget::get_light_square_color;
   using ChessboardWidget::get_border_color;
   using ChessboardWidget::get_white_fill_color;
   using ChessboardWidget::get_white_line_color;
   using ChessboardWidget::get_black_fill_color;
   using ChessboardWidget::get_black_line_color;
   using ChessboardWidget::allocate_color_handle_rgb;
   using ChessboardWidget::allocate_color_handle;
   using ChessboardWidget::free_color_handle;
   using ChessboardWidget::set_background_color;
   using ChessboardWidget::get_background_color;
   using ChessboardWidget::set_background_colors;
   using ChessboardWidget::get_background_colors;

  //@}

  /** @name ChessboardWidget methods : Floating Pieces
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::add_floating_piece;
    using ChessboardWidget::move_floating_piece;
    using ChessboardWidget::remove_floating_piece;
    using ChessboardWidget::get_floating_piece;

  //@}

  /** @name ChessboardWidget methods : HUD Layers
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::enable_hud_layer;
    using ChessboardWidget::disable_hud_layer;

  //@}

  /** @name ChessboardWidget methods : Markers
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::set_marker_color;
    using ChessboardWidget::get_marker_color;
    using ChessboardWidget::set_marker_thickness;
    using ChessboardWidget::get_marker_thickness;
    using ChessboardWidget::set_marker_level;

  //@}

  /** @name ChessboardWidget methods : Cursor
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::show_cursor;
    using ChessboardWidget::hide_cursor ;
    using ChessboardWidget::set_cursor_thickness;
    using ChessboardWidget::get_cursor_thickness;
    using ChessboardWidget::set_cursor_color;
    using ChessboardWidget::get_cursor_color;

  //@}

  /** @name ChessboardWidget methods : Arrows
   *
   * For a description of these methods, see the member functions
   * with the same name in cwmm::ChessboardWidget.
   */
  //@{

  public:
    using ChessboardWidget::add_arrow;
    using ChessboardWidget::remove_arrow;

  //@}

  /** @name Position setup */
  //@{

    /** Copy a new position from \a chess_position.
     *
     * @param chess_position : The new position to use.
     */
    void set_position(cwchess::ChessPosition const& chess_position) { *static_cast<cwchess::ChessPosition*>(this) = chess_position; sync(); }

    //! Paste a position from the clipboard.
    void clipboard_paste(void);

  //@}

  /** @name Accessors */
  //@{

    //! Return a const reference to the current position.
    ChessPosition const& get_position(void) const { return *this; }

    //! Copy a position to the clipboard.
    void clipboard_copy(void) const;

  //@}

  /** @name Events */
  //@{

    /** @brief Called when a mouse button is pressed while the mouse is on the chessboard.
     *
     * @param col : The column of the square the mouse is on.
     * @param row : The row of the square the mouse is on.
     * @param event : The button press event.
     *
     * Useful members of the event are:
     * - <code>event->button</code> : The number of the button that is pressed.
     * - <code>event->type</code> : <code>GDK_BUTTON_PRESS</code> for a normal click, <code>GDK_2BUTTON_PRESS</code> for a double click.
     * - <code>event->x</code>, <code>event->y</code> : The exact pixel coordinates where was clicked.
     *
     * The default does nothing but return false.
     *
     * This function should normally return false. If it returns true then
     * the event is considered to be completely handled and nothing else
     * will be done.
     *
     * In case of a double click, this function is called three times.
     * First with <code>event->type == GDK_BUTTON_PRESS</code>, followed by a call to #on_button_release.
     * Next again with <code>event->type == GDK_BUTTON_PRESS</code> and then with <code>event->type == GDK_2BUTTON_PRESS</code>.
     * Finally #on_button_release will be called a second time. Therefore, a call to this
     * function with <code>event->type == GDK_BUTTON_PRESS</code> and a subsequent on_button_release at
     * the same coordinates should be a non-operation (if you want to use double clicks too).
     */
    virtual bool on_button_press(gint col, gint row, GdkEventButton const* event) { return false; }

    /** @brief Called when a mouse button is released.
     *
     * @param col : The column of the square the mouse is on, or -1 if the mouse is outside the chessboard.
     * @param row : The row of the square the mouse is on, or -1 if the mouse is outside the chessboard.
     * @param event : The button release event.
     *
     * Useful members of the event are:
     * - <code>event->button</code> : The number of the button that is released.
     * - <code>event->x</code>, <code>event->y</code> : The exact pixel coordinates where it is released.
     *
     * The default does nothing but return false.
     *
     * This function should normally return false. If it returns true then
     * the event is considered to be completely handled and nothing else
     * will be done.
     */
    virtual bool on_button_release(gint col, gint row, GdkEventButton const* event) { return false; }

    /** @brief Called when the mouse pointer left the chessboard.
     *
     * The default does nothing.
     */
    virtual void on_cursor_left_chessboard(gint prev_col, gint prev_row) { }

    /** @brief Called when the mouse pointer entered a new square.
     *
     * The default does nothing.
     */
    virtual void on_cursor_entered_square(gint prev_col, gint prev_row, gint col, gint row) { }

  //@}

  /** @name Signals */
  //@{

    /** @brief Return handler for events signaling that the user picked up a piece.
     *
     * This signal is generated whenever the user picks up a piece.
     * It can happen in both 'Position Setup' as well as 'Game Play' mode.
     *
     * Note that a double-click on a piece will cause the piece to be picked
     * up and dropped, then picked up again, then the double click event
     * will be generated and upon final release of the mouse button the
     * piece will be dropped again. You have to take into account that this
     * happens when dealing with double clicks.
     */
    sigc::signal<void, cwchess::Index const&, cwchess::ChessPosition const&>& signal_picked_up(void) { return M_signal_picked_up; }

    /** @brief Return handler for events signaling that the user dropped a piece.
     *
     * This signal is generated whenever the mouse button is released after
     * picking up a piece.
     */
    sigc::signal<void, gint, gint, cwchess::ChessPosition const&>& signal_dropped(void) { return M_signal_dropped; }

    /** @brief Return handler for events signaling that the user did a (legal) chess move.
     *
     * This signal is only generated while the widget is in 'Game Play' mode.
     * If the user picks up a piece and drops it on a different square and
     * this is a legal move, then the move is executed (the ChessPosition is updated)
     * after which this signal is generated.
     *
     * Be aware that callback functions should return promptly or the GUI would freeze.
     * If, for example, you want the computer to play the other color then the
     * callback function would merely put the widget in 'Disabled' mode, set
     * a flag that it is the turn of the computer and return immediately.
     * In general, calculating a reply move would be done in a seperate thread.
     */
    sigc::signal<void, cwchess::Move const&, cwchess::ChessPosition const&, cwchess::ChessPosition const&>& signal_moved(void) { return M_signal_moved; }

    /** @brief Return handler for events signaling that the user attempted an illegal move.
     *
     * This signal is only generated while the widget is in 'Game Play' mode.
     * Note that dropping a piece on the square where it was picked up, or
     * dropping it outside the board is <em>not</em> an illegal move, but
     * is considered to be an abort of 'pick up and move' action.
     */
    sigc::signal<void, cwchess::Move const&, cwchess::ChessPosition const&>& signal_illegal(void) { return M_signal_illegal; }

  //@}

  protected_notdocumented:
    virtual bool on_button_press_event(GdkEventButton* event);
    virtual bool on_button_release_event(GdkEventButton* event);

  private:
    void sync(void);
};

} // namespace cwmm

#endif	// CHESSPOSITIONWIDGET_H
