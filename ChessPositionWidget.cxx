// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file ChessPositionWidget.cxx This file contains the implementation of class ChessPositionWidget.
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

#include "sys.h"
#include "ChessPositionWidget.h"
#include "CwChessboardCodes.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtkmm/radioaction.h>
#include <gtkmm/stock.h>
#pragma GCC diagnostic pop

using namespace cwchess;

namespace cwmm {

bool ChessPositionWidget::execute(Move const& move)
{
  Code code(piece_at(move.from()).code());
  if (code.is_a(pawn))
  {
    EnPassant const& en_passant(this->en_passant());
    if (en_passant.exists() && en_passant.index() == move.to())
      set_square(en_passant.pawn_index().col(), en_passant.pawn_index().row(), empty_square);
  }
  bool result = ChessPosition::execute(move);
  set_square(move.from().col(), move.from().row(), empty_square);
  if (move.is_promotion())
    code = move.promotion_type();
  set_square(move.to().col(), move.to().row(), code);
  uint8_t col_diff = move.to().col() - move.from().col();
  if (code.is_a(king) && col_diff && !(col_diff & 1))
  {
    IndexData rook_from_data = { static_cast<uint8_t>(move.from()() - 4 + 7 * (2 + move.to()() - move.from()()) / 4) };
    IndexData rook_to_data = { static_cast<uint8_t>(move.from()() + (move.to()() - move.from()()) / 2) };
    Index rook_from(rook_from_data);
    Index rook_to(rook_to_data);
    set_square(rook_from.col(), rook_from.row(), empty_square);
    set_square(rook_to.col(), rook_to.row(), (code.color() == white) ? ::white_rook : ::black_rook);
  }
  set_active_turn_indicator(to_move().is_white());
}

void ChessPositionWidget::sync()
{
  // Clear the board.
  for (Index index = index_begin; index != index_end; ++index)
    set_square(index.col(), index.row(), empty_square);

  // Set up a new position.
  for (Index index = index_begin; index != index_end; ++index)
    set_square(index.col(), index.row(), piece_at(index).code());

  // Draw the correct turn indicator.
  set_active_turn_indicator(to_move().is_white());
}

bool ChessPositionWidget::load_FEN(std::string const& FEN)
{
  cwchess::ChessPosition tmp(*static_cast<ChessPosition*>(this));
  bool result = tmp.load_FEN(FEN);
  if (result)
    set_position(tmp);
  return result;
}

bool ChessPositionWidget::popup_menu(GdkEventButton* event, int col, int row)
{
  DoutEntering(dc::notice|dc::clipboard, "ChessPositionWidget::popup_menu(" << col << ", " << row << ")");
  M_placepiece_index = cwchess::Index(col, row);
  if (M_MenuPopup)
  {
    M_refActionGroup->get_action("PlacepieceBlackPawn")->set_sensitive(row != 0 && row != 7);
    M_refActionGroup->get_action("PlacepieceWhitePawn")->set_sensitive(row != 0 && row != 7);
    M_refActionGroup->get_action("PlacepieceBlackKing")->set_sensitive(!all(cwchess::black_king).test());
    M_refActionGroup->get_action("PlacepieceWhiteKing")->set_sensitive(!all(cwchess::white_king).test());
    if (to_move().is_white())
      M_refToMoveWhite_action->set_active(true);
    else
      M_refToMoveBlack_action->set_active(true);
    Piece const& piece(piece_at(M_placepiece_index));
    M_refActionGroup->get_action("PlacepieceNothing")->set_visible(piece != nothing);
    bool can_be_taken_en_passant =
        (piece == cwchess::white_pawn && M_placepiece_index.row() == 3 &&
	    piece_at(M_placepiece_index + south.offset) == nothing &&
	    piece_at(M_placepiece_index + 2 * south.offset) == nothing) ||
        (piece == cwchess::black_pawn && M_placepiece_index.row() == 4 &&
	    piece_at(M_placepiece_index + north.offset) == nothing &&
	    piece_at(M_placepiece_index + 2 * north.offset) == nothing);
    M_refActionGroup->get_action("AllowEnPassantCapture")->set_visible(can_be_taken_en_passant);
    if (can_be_taken_en_passant)
    {
      M_AllowEnPassantCapture_connection.block();
      M_refAllowEnPassantCapture_action->set_active(en_passant().exists() && en_passant().pawn_index() == M_placepiece_index);
      M_AllowEnPassantCapture_connection.unblock();
    }
    bool is_castle_piece =
        (piece == cwchess::white_king && M_placepiece_index == ie1) ||
	(piece == cwchess::black_king && M_placepiece_index == ie8) ||
	(piece == cwchess::white_rook && (M_placepiece_index == ia1 || M_placepiece_index == ih1)) ||
	(piece == cwchess::black_rook && (M_placepiece_index == ia8 || M_placepiece_index == ih8));
    M_refActionGroup->get_action("PieceHasMoved")->set_visible(is_castle_piece);
    if (is_castle_piece)
    {
      M_PieceHasMoved_connection.block();
      M_refPieceHasMoved_action->set_active(has_moved(M_placepiece_index));
      M_PieceHasMoved_connection.unblock();
    }
    update_paste_status();
    M_MenuPopup->popup(event->button, event->time);
    return true;	// Signal that popup() was called.
  }
  return false;
}

void ChessPositionWidget::on_menu_placepiece_black_pawn()
{
  place(cwchess::black_pawn, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_black_rook()
{
  place(cwchess::black_rook, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_black_knight()
{
  place(cwchess::black_knight, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_black_bishop()
{
  place(cwchess::black_bishop, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_black_queen()
{
  place(cwchess::black_queen, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_black_king()
{
  place(cwchess::black_king, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_white_pawn()
{
  place(cwchess::white_pawn, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_white_rook()
{
  place(cwchess::white_rook, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_white_knight()
{
  place(cwchess::white_knight, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_white_bishop()
{
  place(cwchess::white_bishop, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_white_queen()
{
  place(cwchess::white_queen, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_white_king()
{
  place(cwchess::white_king, M_placepiece_index);
}

void ChessPositionWidget::on_menu_placepiece_nothing()
{
  place(cwchess::Code(), M_placepiece_index);
}

void ChessPositionWidget::on_menu_allow_en_passant_capture()
{
  bool en_passant_allowed = en_passant().exists() && en_passant().pawn_index() == M_placepiece_index;
  reset_en_passant();
  if (!en_passant_allowed)
  {
    Index passed_square(M_placepiece_index);
    if (piece_at(M_placepiece_index) == black)
      passed_square += north.offset;
    else
      passed_square += south.offset;
    set_en_passant(passed_square);
  }
}

void ChessPositionWidget::on_menu_piece_has_moved()
{
  if (has_moved(M_placepiece_index))
    clear_has_moved(M_placepiece_index);
  else
    set_has_moved(M_placepiece_index);
}

void ChessPositionWidget::on_menu_copy_FEN()
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::on_menu_copy_FEN");
  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
  std::list<Gtk::TargetEntry> targets;
  targets.push_back(Gtk::TargetEntry("UTF8_STRING"));
  Glib::ustring clipboard_store = FEN();
  // Set the content of the clipboard here, because I'm not sure if set() can cause an immediate call to on_clipboard_get.
  M_clipboard_content = clipboard_store;
  // This call can clear M_clipboard_content (because it calls ChessPositionWidget::on_clipboard_clear if it was called before.
  if (!refClipboard->set(targets,
      sigc::mem_fun(*this, &ChessPositionWidget::on_clipboard_get),
      sigc::mem_fun(*this, &ChessPositionWidget::on_clipboard_clear)))
    g_warning("ChessPositionWidget::on_menu_copy_FEN(): setting the clipboard data failed.");
  // Therefore we set the clipboard content again, here.
  M_clipboard_content = clipboard_store;
  Dout(dc::clipboard, "Storing \"" << M_clipboard_content << "\".");
}

void ChessPositionWidget::on_menu_paste_FEN()
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::on_menu_paste_FEN");
  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
  refClipboard->request_text(sigc::mem_fun(*this, &ChessPositionWidget::on_clipboard_received));
}

void ChessPositionWidget::on_clipboard_get(Gtk::SelectionData& selection_data, guint)
{
  std::string const target = selection_data.get_target();

  DoutEntering(dc::clipboard, "ChessPositionWidget::on_clipboard_get(...) with target data \"" << target << "\".");

  if (target == "UTF8_STRING")
  {
    Dout(dc::clipboard, "Passing back \"" << M_clipboard_content << "\".");
    selection_data.set_text(M_clipboard_content);
  }
  else
    g_warning("ChessPositionWidget::on_clipboard_get(): Unexpected clipboard target format.");
}

void ChessPositionWidget::on_clipboard_clear()
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::on_clipboard_clear()");
  M_clipboard_content.clear();
}

void ChessPositionWidget::on_clipboard_received(Glib::ustring const& text)
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::on_clipboard_received(\"" << text << "\").");

  if (!load_FEN(text) && !M_trying_primary)
  {
    Dout(dc::clipboard, "No FEN in CLIPBOARD. Trying PRIMARY.");
    Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get(GDK_SELECTION_PRIMARY);
    M_trying_primary = true;
    refClipboard->request_text(sigc::mem_fun(*this, &ChessPositionWidget::on_clipboard_received));
  }
  else
    M_trying_primary = false;
}

void ChessPositionWidget::update_paste_status()
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::update_paste_status()");
  M_refActionGroup->get_action("PasteFEN")->set_sensitive(false);
  Dout(dc::clipboard, "paste disabled.");
  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
  refClipboard->request_targets(sigc::mem_fun(*this, &ChessPositionWidget::on_clipboard_received_targets));
}

void ChessPositionWidget::on_clipboard_received_targets(Glib::StringArrayHandle const& targets_array)
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::on_clipboard_received_targets(...).");

  typedef std::list<Glib::ustring> targets_type;
  targets_type targets = targets_array;
  if (std::find(targets.begin(), targets.end(), "UTF8_STRING") != targets.end())
  {
    Dout(dc::clipboard, "Target found; paste enabled.");
    M_refActionGroup->get_action("PasteFEN")->set_sensitive(true);
  }
}

void ChessPositionWidget::on_menu_swap_colors()
{
  swap_colors();
}

void ChessPositionWidget::on_menu_initial_position()
{
  initial_position();
}

void ChessPositionWidget::on_menu_clear_board()
{
  clear();
}

void ChessPositionWidget::on_menu_to_move_white()
{
  to_move(white);
}

void ChessPositionWidget::on_menu_to_move_black()
{
  to_move(black);
}

void ChessPositionWidget::initialize_menus()
{
  DoutEntering(dc::notice, "ChessPositionWidget::initialize_menus");

  M_refIconFactory = Gtk::IconFactory::create();
  M_refIconFactory->add_default();

  GdkColor light_square_color;
  get_light_square_color(light_square_color);

  int color_count = 0;
  for (bool iswhite = false; color_count < 2; ++color_count, iswhite = true)
  {
    using namespace cwchess;
    Type type(nothing);
    do
    {
      Glib::RefPtr<Gdk::Pixmap> icon_pixmap = Gdk::Pixmap::create(M_drawable->get_window(), 16, 16);
      Cairo::RefPtr<Cairo::Context> cairo_context = icon_pixmap->create_cairo_context();
      cairo_t* cr = cairo_context->cobj();
      cairo_set_source_rgb(cr, light_square_color.red / 65535.0, light_square_color.green / 65535.0, light_square_color.blue / 65535.0);
      cairo_paint(cr);
      std::string icon_str(iswhite ? "white_" : "black_");
      switch (type())
      {
        case nothing_bits:
	  icon_str += "nothing";
          break;
        case pawn_bits:
	  draw_pawn(cr, 8, 8, 16, iswhite);
	  icon_str += "pawn";
          break;
        case rook_bits:
	  draw_rook(cr, 8, 8, 16, iswhite);
	  icon_str += "rook";
          break;
        case knight_bits:
	  draw_knight(cr, 8, 8, 16, iswhite);
	  icon_str += "knight";
          break;
        case bishop_bits:
	  draw_bishop(cr, 8, 8, 16, iswhite);
	  icon_str += "bishop";
          break;
        case queen_bits:
	  draw_queen(cr, 8, 8, 16, iswhite);
	  icon_str += "queen";
          break;
        case king_bits:
	  draw_king(cr, 8, 8, 16, iswhite);
	  icon_str += "king";
          break;
      }
      icon_str += "_icon";
      Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf = Gdk::Pixbuf::create(Glib::RefPtr<Gdk::Drawable>::cast_static(icon_pixmap), 0, 0, 16, 16);
      M_refIconFactory->add(Gtk::StockID(icon_str.c_str()), Gtk::IconSet(icon_pixbuf));

      do { TypeData type_data = { static_cast<uint8_t>(type() + 1) }; type = type_data; } while (type() == 4);
    }
    while (type() != 8);
  }

  M_refActionGroup = Gio::SimpleActionGroup::create();
  M_refActionGroup->add(Gtk::Action::create("PlacepieceMenu", "Placepiece Menu"));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceBlackPawn", Gtk::StockID("black_pawn_icon"), "Black Pawn"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_pawn));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceBlackRook", Gtk::StockID("black_rook_icon"), "Black Rook"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_rook));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceBlackKnight", Gtk::StockID("black_knight_icon"), "Black Knight"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_knight));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceBlackBishop", Gtk::StockID("black_bishop_icon"), "Black Bishop"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_bishop));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceBlackQueen", Gtk::StockID("black_queen_icon"), "Black Queen"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_queen));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceBlackKing", Gtk::StockID("black_king_icon"), "Black King"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_king));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceWhitePawn", Gtk::StockID("white_pawn_icon"), "White Pawn"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_pawn));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceWhiteRook", Gtk::StockID("white_rook_icon"), "White Rook"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_rook));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceWhiteKnight", Gtk::StockID("white_knight_icon"), "White Knight"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_knight));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceWhiteBishop", Gtk::StockID("white_bishop_icon"), "White Bishop"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_bishop));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceWhiteQueen", Gtk::StockID("white_queen_icon"), "White Queen"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_queen));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceWhiteKing", Gtk::StockID("white_king_icon"), "White King"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_king));
  M_refActionGroup->add(Gtk::Action::create("PlacepieceNothing", Gtk::StockID("black_nothing_icon"), "Clear square"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_nothing));
  M_refAllowEnPassantCapture_action = Gtk::ToggleAction::create("AllowEnPassantCapture", "Allow e.p. capture");
  M_refActionGroup->add(M_refAllowEnPassantCapture_action);
  M_AllowEnPassantCapture_connection =
      M_refAllowEnPassantCapture_action->signal_toggled().connect(sigc::mem_fun(*this, &ChessPositionWidget::on_menu_allow_en_passant_capture));
  M_refPieceHasMoved_action = Gtk::ToggleAction::create("PieceHasMoved", "Has moved");
  M_refActionGroup->add(M_refPieceHasMoved_action);
  M_PieceHasMoved_connection =
      M_refPieceHasMoved_action->signal_toggled().connect(sigc::mem_fun(*this, &ChessPositionWidget::on_menu_piece_has_moved));
  M_refActionGroup->add(Gtk::Action::create("CopyFEN", Gtk::Stock::COPY, "Copy FEN"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_copy_FEN));
  M_refActionGroup->add(Gtk::Action::create("PasteFEN", Gtk::Stock::PASTE, "Paste FEN"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_paste_FEN));
  M_refActionGroup->add(Gtk::Action::create("SwapColors", "Swap colors"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_swap_colors));
  M_refActionGroup->add(Gtk::Action::create("InitialPosition", "Initial position"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_initial_position));
  M_refActionGroup->add(Gtk::Action::create("ClearBoard", "Clear board"),
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_clear_board));
  Gtk::RadioAction::Group group_to_move;
  M_refToMoveWhite_action = Gtk::RadioAction::create(group_to_move, "ToMoveWhite", "White to play");
  M_refActionGroup->add(M_refToMoveWhite_action,
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_to_move_white));
  M_refToMoveBlack_action = Gtk::RadioAction::create(group_to_move, "ToMoveBlack", "Black to play");
  M_refActionGroup->add(M_refToMoveBlack_action,
      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_to_move_black));

  M_refUIManager = Gtk::UIManager::create();
  M_refUIManager->insert_action_group(M_refActionGroup);

  //add_accel_group(M_refUIManager->get_accel_group());

  //Layout the actions in a menubar and toolbar:
  Glib::ustring ui_info =
        "<ui>"
        "  <popup name='PopupMenu'>"
        "    <menuitem action='PlacepieceWhitePawn'/>"
        "    <menuitem action='PlacepieceWhiteRook'/>"
        "    <menuitem action='PlacepieceWhiteKnight'/>"
        "    <menuitem action='PlacepieceWhiteBishop'/>"
        "    <menuitem action='PlacepieceWhiteQueen'/>"
        "    <menuitem action='PlacepieceWhiteKing'/>"
	"    <separator/>"
        "    <menuitem action='PlacepieceBlackPawn'/>"
        "    <menuitem action='PlacepieceBlackRook'/>"
        "    <menuitem action='PlacepieceBlackKnight'/>"
        "    <menuitem action='PlacepieceBlackBishop'/>"
        "    <menuitem action='PlacepieceBlackQueen'/>"
        "    <menuitem action='PlacepieceBlackKing'/>"
	"    <separator/>"
	"    <menuitem action='AllowEnPassantCapture'/>"
	"    <menuitem action='PieceHasMoved'/>"
        "    <menuitem action='PlacepieceNothing'/>"
	"    <separator/>"
	"    <menuitem action='ToMoveWhite'/>"
	"    <menuitem action='ToMoveBlack'/>"
	"    <separator/>"
	"    <menuitem action='CopyFEN'/>"
	"    <menuitem action='PasteFEN'/>"
	"    <menuitem action='SwapColors'/>"
	"    <menuitem action='InitialPosition'/>"
	"    <menuitem action='ClearBoard'/>"
        "  </popup>"
        "</ui>";
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    M_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
#else
  std::auto_ptr<Glib::Error> ex;
  M_refUIManager->add_ui_from_string(ui_info, ex);
  if(ex.get())
  {
    std::cerr << "building menus failed: " <<  ex->what();
  }
#endif //GLIBMM_EXCEPTIONS_ENABLED

  // Get the menu:
  M_MenuPopup = dynamic_cast<Gtk::Menu*>(M_refUIManager->get_widget("/PopupMenu"));
  if (!M_MenuPopup)
    g_warning("menu not found");
  else
    M_MenuPopup->signal_deactivate().connect(sigc::mem_fun(this, &ChessPositionWidget::popup_deactivated));
}

void ChessPositionWidget::popup_deactivated()
{
  Dout(dc::notice, "Calling ChessPositionWidget::popup_deactivated()");
  hide_cursor();
}

bool ChessPositionWidget::on_button_press_event(GdkEventButton* event)
{
#ifdef CWDEBUG
  if (debug::channels::dc::event.is_on())
    Dout(dc::event, "Entering ChessPositionWidget::on_button_press_event(" << event << ")");
  else
    Dout(dc::notice|dc::clipboard, "Entering ChessPositionWidget::on_button_press_event()");
  debug::Indent __cwmm_chesspositionwidget_debug_indent(2);
#endif

  // Find the square under the mouse, if any.
  gint col = x2col(event->x);
  gint row = y2row(event->y);

  // Ignore button presses outside the chessboard.
  if (!is_inside_board(col, row))
    return false;

  // Inform the user that a button press event occured.
  if (on_button_press(col, row, event))
    return true;

  // If the edit mode isn't one of the builtin modes, then we're done.
  // We're also done if it was a double click.
  if (event->type != GDK_BUTTON_PRESS ||
      (M_widget_mode != mode_edit_position && M_widget_mode != mode_edit_game))
    return false;

  // Handle picking up a piece.
  if (event->button == 1)
  {
    M_floating_piece_handle = -1;
    code_t code = get_square(col, row);
    // Ignore clicks on empty squares.
    // The least significant bit reflects the color of the piece. A code <= 1 means an empty square.
    if (code > 1)
    {
      bool pickup = true;
      if (M_widget_mode == mode_edit_position)
      {
	// Remove the piece under the mouse pointer.
	place(Code(), Index(col, row));
      }
      else // M_widget_mode == mode_edit_game
      {
	// Remember were the piece came from if we're going to do a move.
	M_move_from = Index(col, row);
	if (piece_at(M_move_from).color() != to_move())
	  // Refuse to pick up a piece of the wrong color.
	  pickup = false;
	else
	{
          // Erase the piece in the widget only.
	  set_square(col, row, empty_square);
        }
      }
      if (pickup)
      {
	// Put the piece under the mouse pointer.
	double hsside = 0.5 * sside();
	double fraction = hsside - (gint)hsside;
	M_floating_piece_handle = add_floating_piece(code, event->x - fraction, event->y - fraction, TRUE);
	M_signal_picked_up.emit(M_move_from, *this);
	// We picked up a piece; the event was handled.
        return true;
      }
    }
  }
  // Handle placing new pieces.
  else if (event->button == 3 && M_widget_mode == mode_edit_position)
  {
    if (popup_menu(event, col, row))
      return true;			// Ignore the event completely: we won't get a release event either.
  }

  // Returning FALSE means that the next event handler will be called.
  return false;
}

bool ChessPositionWidget::on_button_release_event(GdkEventButton* event)
{
#ifdef CWDEBUG
  if (debug::channels::dc::event.is_on())
    Dout(dc::event, "Entering ChessPositionWidget::on_button_release_event(" << event << ")");
  else
    Dout(dc::notice, "Entering ChessPositionWidget::on_button_release_event()");
  debug::Indent __cwmm_chesspositionwidget_debug_indent(2);
#endif

  // Find the square under the mouse, if any.
  gint col = x2col(event->x);
  gint row = y2row(event->y);

  // Inform the user that a button press event occured.
  if (!is_inside_board(col, row))
    col = row = -1;
  if (on_button_release(col, row, event))
    return true;

  // If the edit mode isn't one of the builtin modes, then we're done.
  if (M_widget_mode != mode_edit_position && M_widget_mode != mode_edit_game)
    return false;

  // Handle dropping a piece.
  if (event->button == 1 && M_floating_piece_handle != -1)
  {
    if (M_widget_mode == mode_edit_game || col != -1)
    {
      // Get the code of the floating piece.
      code_t code = get_floating_piece(M_floating_piece_handle);
      if (M_widget_mode == mode_edit_position)
      {
	// Put the piece on the new square.
	place(cwchess::Code(code), cwchess::Index(col, row));
      }
      else // M_widget_mode == mode_edit_game
      {
        // Where are we trying to move to?
	cwchess::Index to(col, row);
	if (to == M_move_from || col == -1)
	{
	  // The piece was put back where it was picked up. Just draw it again in the widget and do nothing.
	  set_square(M_move_from.col(), M_move_from.row(), code);
	}
	else
	{
	  // Create the move object.
	  cwchess::Move move(M_move_from, to, cwchess::nothing);
	  cwchess::Piece piece(piece_at(M_move_from));
	  cwchess::Color color(piece.color());
	  if (((color == cwchess::black && row == 0) || (color == cwchess::white && row == 7)) && piece.code().is_a(cwchess::pawn))
	    move. set_promotion(M_promotion->type(this, move));
	  if (legal(move))
	  {
	    cwchess::ChessPosition previous_position(*this);
	    execute(move);
	    M_signal_moved.emit(move, previous_position, *this);
	  }
	  else
	  {
	    // The move was illegal. Just draw the piece back where it was and do nothing.
	    set_square(M_move_from.col(), M_move_from.row(), code);
	    M_signal_illegal.emit(move, *this);
	  }
	}
      }
    }
    // Remove the floating piece.
    remove_floating_piece(M_floating_piece_handle);
    M_floating_piece_handle = -1;
    M_signal_dropped.emit(col, row, *this);
    // We just dropped the piece; the event was handled.
    return true;
  }

  return false;
}

} // namespace cwmm

