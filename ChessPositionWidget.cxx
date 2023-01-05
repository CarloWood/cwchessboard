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
  if (code.is_a(cwchess::pawn))
  {
    EnPassant const& en_passant(this->en_passant());
    if (en_passant.exists() && en_passant.index() == move.to())
      set_square(en_passant.pawn_index().col(), en_passant.pawn_index().row(), empty_square);
  }
  bool draw_by_50_moves_rule = ChessPosition::execute(move);
  set_square(move.from().col(), move.from().row(), empty_square);
  if (move.is_promotion())
    code = move.promotion_type();
  set_square(move.to().col(), move.to().row(), code);
  uint8_t col_diff = move.to().col() - move.from().col();
  if (code.is_a(cwchess::king) && col_diff && !(col_diff & 1))
  {
    IndexData rook_from_data = { static_cast<uint8_t>(move.from()() - 4 + 7 * (2 + move.to()() - move.from()()) / 4) };
    IndexData rook_to_data = { static_cast<uint8_t>(move.from()() + (move.to()() - move.from()()) / 2) };
    Index rook_from(rook_from_data);
    Index rook_to(rook_to_data);
    set_square(rook_from.col(), rook_from.row(), empty_square);
    set_square(rook_to.col(), rook_to.row(), (code.color() == white) ? ::white_rook : ::black_rook);
  }
  set_active_turn_indicator(to_move().is_white());
  return draw_by_50_moves_rule;
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
  {
    set_position(tmp);
    M_signal_position_editted.emit();
  }
  return result;
}

bool ChessPositionWidget::popup_menu(GdkEventButton* event, int col, int row)
{
  DoutEntering(dc::notice|dc::clipboard, "ChessPositionWidget::popup_menu(" << col << ", " << row << ")");
  m_placepiece_index = cwchess::Index(col, row);
  if (m_menuPopup)
  {
    Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PlacepieceBlackPawn"))->set_enabled(row != 0 && row != 7);
    Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PlacepieceWhitePawn"))->set_enabled(row != 0 && row != 7);
    Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PlacepieceBlackKing"))->set_enabled(!all(cwchess::black_king).test());
    Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PlacepieceWhiteKing"))->set_enabled(!all(cwchess::white_king).test());
    Glib::ustring const parameter(to_move().is_white() ? "'white'" : "'black'");
    m_refToMove_action->change_state(parameter);
    Piece const& piece(piece_at(m_placepiece_index));
    m_PlacepieceNothing->set_visible(piece != nothing);
    bool can_be_taken_en_passant =
        (piece == cwchess::white_pawn && m_placepiece_index.row() == 3 &&
	    piece_at(m_placepiece_index + south.offset) == nothing &&
	    piece_at(m_placepiece_index + 2 * south.offset) == nothing) ||
        (piece == cwchess::black_pawn && m_placepiece_index.row() == 4 &&
	    piece_at(m_placepiece_index + north.offset) == nothing &&
	    piece_at(m_placepiece_index + 2 * north.offset) == nothing);
    m_AllowEnPassantCapture->set_visible(can_be_taken_en_passant);
    if (can_be_taken_en_passant)
    {
      M_AllowEnPassantCapture_connection.block();
      Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("AllowEnPassantCapture"))->set_enabled(en_passant().exists() && en_passant().pawn_index() == m_placepiece_index);
//      m_refAllowEnPassantCapture_action->set_active(en_passant().exists() && en_passant().pawn_index() == m_placepiece_index);
      M_AllowEnPassantCapture_connection.unblock();
    }
    bool is_castle_piece =
        (piece == cwchess::white_king && m_placepiece_index == ie1) ||
	(piece == cwchess::black_king && m_placepiece_index == ie8) ||
	(piece == cwchess::white_rook && (m_placepiece_index == ia1 || m_placepiece_index == ih1)) ||
	(piece == cwchess::black_rook && (m_placepiece_index == ia8 || m_placepiece_index == ih8));
    Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PieceHasMoved"))->set_enabled(is_castle_piece);
    if (is_castle_piece)
    {
      M_PieceHasMoved_connection.block();
//      M_refPieceHasMoved_action->set_active(has_moved(m_placepiece_index));
      Glib::RefPtr<Gtk::ToggleAction>::cast_dynamic(m_refActionGroup->lookup_action("PieceHasMoved"))->set_active(has_moved(m_placepiece_index));
      M_PieceHasMoved_connection.unblock();
    }
    update_paste_status();
    if (!m_menuPopup->get_attach_widget())
      m_menuPopup->attach_to_widget(*this);
    m_menuPopup->popup_at_pointer((GdkEvent*)event);
    return true;	// Signal that popup_at_pointer() was called.
  }
  return false;
}

void ChessPositionWidget::on_menu_placepiece_black_pawn()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_black_pawn()");
  if (place(cwchess::black_pawn, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_black_rook()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_black_rook()");
  if (place(cwchess::black_rook, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_black_knight()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_black_knight()");
  if (place(cwchess::black_knight, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_black_bishop()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_black_bishop()");
  if (place(cwchess::black_bishop, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_black_queen()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_black_queen()");
  if (place(cwchess::black_queen, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_black_king()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_black_king()");
  if (place(cwchess::black_king, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_white_pawn()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_white_pawn()");
  if (place(cwchess::white_pawn, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_white_rook()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_white_rook()");
  if (place(cwchess::white_rook, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_white_knight()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_white_knight()");
  if (place(cwchess::white_knight, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_white_bishop()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_white_bishop()");
  if (place(cwchess::white_bishop, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_white_queen()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_white_queen()");
  if (place(cwchess::white_queen, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_white_king()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_white_king()");
  if (place(cwchess::white_king, m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_placepiece_nothing()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_placepiece_nothing()");
  if (place(cwchess::Code(), m_placepiece_index))
    M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_allow_en_passant_capture()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_allow_en_passant_capture()");
  bool en_passant_allowed = en_passant().exists() && en_passant().pawn_index() == m_placepiece_index;
  reset_en_passant();
  if (!en_passant_allowed)
  {
    Index passed_square(m_placepiece_index);
    if (piece_at(m_placepiece_index) == black)
      passed_square += north.offset;
    else
      passed_square += south.offset;
    set_en_passant(passed_square);
  }
}

void ChessPositionWidget::on_menu_piece_has_moved()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_piece_has_moved()");
  if (has_moved(m_placepiece_index))
    clear_has_moved(m_placepiece_index);
  else
    set_has_moved(m_placepiece_index);
}

void ChessPositionWidget::on_menu_copy_FEN()
{
  DoutEntering(dc::clipboard, "ChessPositionWidget::on_menu_copy_FEN");
  Glib::RefPtr<Gtk::Clipboard> refClipboard = Gtk::Clipboard::get();
  std::vector<Gtk::TargetEntry> targets;
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
  M_signal_position_editted.emit();
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
  Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PasteFEN"))->set_enabled(false);
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
    Glib::RefPtr<Gio::SimpleAction>::cast_dynamic(m_refActionGroup->lookup_action("PasteFEN"))->set_enabled(true);
  }
}

void ChessPositionWidget::on_menu_swap_colors()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_swap_colors()");
  swap_colors();
  M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_initial_position()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_initial_position()");
  initial_position();
  M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_clear_board()
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_clear_board()");
  clear();
  M_signal_position_editted.emit();
}

void ChessPositionWidget::on_menu_to_move(Glib::ustring const& parameter)
{
  DoutEntering(dc::notice, "ChessPositionWidget::on_menu_to_move()");
  to_move(parameter == "'white'" ? white : black);
}

void ChessPositionWidget::initialize_menu()
{
  DoutEntering(dc::notice, "ChessPositionWidget::initialize_menu()");

  M_refIconFactory = Gtk::IconFactory::create();
  M_refIconFactory->add_default();

  int color_count = 0;
  for (bool iswhite = false; color_count < 2; ++color_count, iswhite = true)
  {
    using namespace cwchess;
    Type type(nothing);
    do
    {
      // Create cairo surface and context to draw on.
      cairo_surface_t* icon_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 16, 16);
      cairo_t* cr = cairo_create(icon_surface);

      cairo_set_source_rgb(cr, m_light_square_color.red, m_light_square_color.green, m_light_square_color.blue);
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
      // Make an icon ID.
      Gtk::StockID const icon_id(icon_str.c_str());
      Cairo::RefPtr<Cairo::Surface> icon_surface_refptr(new Cairo::Surface(icon_surface));
      // Convert cairo surface to Pixbuf and add it to the IconFactory.
      Glib::RefPtr<Gdk::Pixbuf> icon_pixbuf = Gdk::Pixbuf::create(icon_surface_refptr, 0, 0, 16, 16);
      M_refIconFactory->add(icon_id, Gtk::IconSet::create(icon_pixbuf));

      do { TypeData type_data = { static_cast<uint8_t>(type() + 1) }; type = type_data; } while (type() == 4);

      cairo_surface_destroy(icon_surface);
    }
    while (type() != 8);
  }

  // Set up the popup menu.
  m_refActionGroup = Gio::SimpleActionGroup::create();
  insert_action_group("PlacepieceMenu", m_refActionGroup);
  m_refActionGroup->add_action("PlacepieceBlackPawn", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_pawn));
  m_refActionGroup->add_action("PlacepieceBlackRook", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_rook));
  m_refActionGroup->add_action("PlacepieceBlackKnight", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_knight));
  m_refActionGroup->add_action("PlacepieceBlackBishop", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_bishop));
  m_refActionGroup->add_action("PlacepieceBlackQueen", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_queen));
  m_refActionGroup->add_action("PlacepieceBlackKing", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_black_king));
  m_refActionGroup->add_action("PlacepieceWhitePawn", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_pawn));
  m_refActionGroup->add_action("PlacepieceWhiteRook", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_rook));
  m_refActionGroup->add_action("PlacepieceWhiteKnight", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_knight));
  m_refActionGroup->add_action("PlacepieceWhiteBishop", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_bishop));
  m_refActionGroup->add_action("PlacepieceWhiteQueen", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_queen));
  m_refActionGroup->add_action("PlacepieceWhiteKing", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_white_king));
  m_refActionGroup->add_action("PlacepieceNothing", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_placepiece_nothing));

  //m_refAllowEnPassantCapture_action = Gtk::ToggleAction::create("AllowEnPassantCapture", "Allow e.p. capture");
  //m_refActionGroup->add(m_refAllowEnPassantCapture_action);
  m_refActionGroup->add_action("AllowEnPassantCapture", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_allow_en_passant_capture));
  //M_AllowEnPassantCapture_connection =
  //    m_refAllowEnPassantCapture_action->signal_toggled().connect(sigc::mem_fun(*this, &ChessPositionWidget::on_menu_allow_en_passant_capture));

//  M_refPieceHasMoved_action = Gtk::ToggleAction::create("PieceHasMoved", "Has moved");
//  m_refActionGroup->add(M_refPieceHasMoved_action);
//  M_PieceHasMoved_connection =
//      M_refPieceHasMoved_action->signal_toggled().connect(sigc::mem_fun(*this, &ChessPositionWidget::on_menu_piece_has_moved));
  m_refActionGroup->add_action("PieceHasMoved", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_piece_has_moved));
//  m_refActionGroup->add(Gtk::Action::create("CopyFEN", Gtk::Stock::COPY, "Copy FEN"),
//      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_copy_FEN));
  m_refActionGroup->add_action("CopyFEN", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_copy_FEN));
//  m_refActionGroup->add(Gtk::Action::create("PasteFEN", Gtk::Stock::PASTE, "Paste FEN"),
//      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_paste_FEN));
  m_refActionGroup->add_action("PasteFEN", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_paste_FEN));
//  m_refActionGroup->add(Gtk::Action::create("SwapColors", "Swap colors"),
//      sigc::mem_fun(*this, &ChessPositionWidget::on_menu_swap_colors));
  m_refActionGroup->add_action("SwapColors", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_swap_colors));
  m_refActionGroup->add_action("InitialPosition", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_initial_position));
  m_refActionGroup->add_action("ClearBoard", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_clear_board));

  m_refToMove_action = m_refActionGroup->add_action_radio_string("ToMove", sigc::mem_fun(*this, &ChessPositionWidget::on_menu_to_move), "'black'");

//  insert_action_group("examplepopup", refActionGroup);

  m_refBuilder = Gtk::Builder::create();
  try
  {
    Dout(dc::notice, "Loading PlacePiece.glade");
    m_refBuilder->add_from_file("/home/carlo/projects/cwchessboard/PlacePiece.glade");
  }
  catch (Glib::Error const& error)
  {
    DoutFatal(dc::core, "building menus failed: " <<  error.what());
  }

  // Get the menu:
  m_menuPopup = nullptr;
  m_refBuilder->get_widget<Gtk::Menu>("PlacePiece", m_menuPopup);
  m_menuPopup->signal_deactivate().connect(sigc::mem_fun(*this, &ChessPositionWidget::popup_deactivated));
  // Get certain menu items:
  m_PlacepieceNothing = nullptr;
  m_refBuilder->get_widget<Gtk::MenuItem>("PlacepieceNothing", m_PlacepieceNothing);
  m_AllowEnPassantCapture = nullptr;
  m_refBuilder->get_widget<Gtk::MenuItem>("AllowEnPassantCapture", m_AllowEnPassantCapture);

  initial_position();
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
        if (M_widget_mode == mode_edit_position)
          M_signal_position_editted.emit();
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

  bool handled = false;

  if (on_button_release(col, row, event))
    handled = true;

  // If the edit mode isn't one of the builtin modes, then we're done.
  else if (M_widget_mode != mode_edit_position && M_widget_mode != mode_edit_game)
    handled = false;

  // Handle dropping a piece.
  else if (event->button == 1 && M_floating_piece_handle != -1)
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
    if (M_widget_mode == mode_edit_position)
      M_signal_position_editted.emit();
    // We just dropped the piece; the event was handled.
    handled = true;
  }

  return handled;
}

} // namespace cwmm

