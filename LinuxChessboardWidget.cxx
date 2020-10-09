#include "sys.h"
#include "LinuxChessboardWidget.h"
#include "LinuxChessMenuBar.h"
#ifdef CWDEBUG
#include "ChessNotation.h"
#endif
#include "debug.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <gtk/gtk.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/menu.h>
#include <gtkmm/iconfactory.h>
#pragma GCC diagnostic pop

void LinuxChessboardWidget::append_menu_entries(LinuxChessMenuBar* menubar)
{
  using namespace menu_keys;
  using namespace Gtk::Stock;
#define ADD(top, entry) \
  menubar->append_menu_entry({top, entry},   this, &LinuxChessboardWidget::on_menu_##top##_##entry)

  Gtk::RadioButtonGroup show;
#define ADD_RADIO(top, entry) \
  menubar->append_radio_menu_entry(show, {top, entry},   this, &LinuxChessboardWidget::on_menu_##top##_##entry)

  menubar->append_separator(Mode);
  ADD_RADIO(Mode, ShowCandidates);
  ADD_RADIO(Mode, ShowReachables);
  ADD_RADIO(Mode, ShowAttacked);
  ADD_RADIO(Mode, ShowDefendables);
  ADD_RADIO(Mode, ShowDefendedBlack);
  ADD_RADIO(Mode, ShowDefendedWhite);
  ADD_RADIO(Mode, ShowMoves);
  ADD_RADIO(Mode, PlacePieces);

  // This wasn't called for some reason.
  on_menu_Mode_ShowCandidates();
}

void LinuxChessboardWidget::draw_hud_layer(Cairo::RefPtr<Cairo::Context> const& cr, gint sside, guint hud)
{
  DoutEntering(dc::notice, "draw_hud_layer(cr, " << sside << ", " << hud << ")");
  cr->select_font_face("Sans", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
  cr->set_font_size(20);
  Cairo::TextExtents extents;
  cr->get_text_extents("3", extents);
  double x, y;
  for (int col = 0; col < 8; ++col)
  {
    for (int row = 0; row < 8; ++row)
    {
      static char const* number[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
      cwchess::Index i(col, row);
      int count = get_defended()[cwchess::white].count(i);
      char const* utf8 = number[count % 10];
      x = col * sside + 1;
      y = (7 - row) * sside + extents.height + 2;
      cr->move_to(x, y);
      if (count > 9)
	cr->show_text(number[1]);
      cr->show_text(utf8);
    }
  }
}

bool LinuxChessboardWidget::on_button_press(gint col, gint row, GdkEventButton const* event)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::on_button_press(" << col << ", " << row << ", " << event << ")");

  // Ignore double clicks.
  if (event->type == GDK_2BUTTON_PRESS)
    return false;
  if (event->button == 2)
  {
    // Inside board?
    if (col != -1)
    {
      show_cursor();
      // Mark the start square.
      set_marker_color(col, row, 1);
      M_arrow_begin_col = col;
      M_arrow_begin_row = row;
    }
  }
  else if (event->button == 3)
  {
    // Inside board?
    if (col != -1)
    {
      switch (M_mode)
      {
        case mode_show_candidates:
        case mode_show_reachables:
        case mode_show_attacked:
        case mode_show_defendables:
        case mode_show_defended_black:
        case mode_show_defended_white:
        case mode_show_moves:
          show_reachables(col, row, M_mode);
	  show_cursor();
	  return true;		// Don't show the popup menu.
	case mode_popup_menu:
	  // The default of ChessPositionWidget already does this.
          break;
      }
      show_cursor();
    }
  }
  else if (event->button == 10)
  {
    enable_hud_layer(1);
    show_reachables(0, 0, mode_show_defended_white);
  }
  return false;
}

bool LinuxChessboardWidget::on_button_release(gint col, gint row, GdkEventButton const* event)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::on_button_release(" << col << ", " << row << ", " << event << ")");

  hide_cursor();
  if (event->button == 2)
  {
    GdkColor color = { 0, 0, 0, 65535 };
    // Clear the marker on the start square again.
    set_marker_color(M_arrow_begin_col, M_arrow_begin_row, 0);
    // Draw an arrow.
    if ((M_arrow_begin_col != col || M_arrow_begin_row != row) &&
	is_inside_board(M_arrow_begin_col, M_arrow_begin_row) &&
	is_inside_board(col, row))
      add_arrow(M_arrow_begin_col, M_arrow_begin_row, col, row, color);
  }
  return false;
}

void LinuxChessboardWidget::picked_up(cwchess::Index const& index, cwchess::ChessPosition const& chess_position)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::picked_up(" << cwchess::ChessNotation(*this, index) << ", ...)");
  if (get_widget_mode() == mode_edit_game)
  {
    // If we pick up a piece to do a move, show all legal moves and the cursor.
    show_reachables(index.col(), index.row(), mode_show_moves);
    show_cursor();
  }
  else if (get_widget_mode() == mode_edit_position)
    // In 'edit position' mode, show the pinning information instead.
    show_pinning();
}

void LinuxChessboardWidget::dropped(gint col, gint row, cwchess::ChessPosition const& chess_position)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::dropped(" << col << ", " << row << ", ...)");
  // The piece that was picked up is dropped again. Return to regular standing.
  hide_cursor();
  show_pinning();
}

void LinuxChessboardWidget::moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& current_position)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::moved(" << cwchess::ChessNotation(previous_position, move) << ", ...)");
  // Having executed a move, we might need to draw or erase the en passant arrow.
  update_en_passant_arrow();
}

void LinuxChessboardWidget::illegal(cwchess::Move const& move, cwchess::ChessPosition const& chess_position)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::illegal(" << cwchess::ChessNotation(*this, move) << ", ...)");
}

LinuxChessboardWidget::LinuxChessboardWidget(Gtk::Window* window, Glib::RefPtr<cwchess::Promotion> promotion) : cwmm::ChessPositionWidget(window, promotion), M_en_passant_arrow(nullptr)
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::LinuxChessboardWidget()");
  init_colors();
  //m_refActionGroup = Gio::SimpleActionGroup::create();
}

void LinuxChessboardWidget::initialize_menu()
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::initialize_menu()");
  ChessPositionWidget::initialize_menu();
}

struct rgb_t { gdouble red; gdouble green; gdouble blue; };

rgb_t mix(rgb_t col1, gdouble alpha1, rgb_t col2, gdouble alpha2)
{
  rgb_t result;
  result.red = (col1.red * alpha1 + col2.red * alpha2) / sqrt(alpha1 * alpha1 + alpha2 * alpha2);
  result.green = (col1.green * alpha1 + col2.green * alpha2) / sqrt(alpha1 * alpha1 + alpha2 * alpha2);
  result.blue = (col1.blue * alpha1 + col2.blue * alpha2) / sqrt(alpha1 * alpha1 + alpha2 * alpha2);
  return result;
}

void LinuxChessboardWidget::init_colors()
{
  gdouble alpha = 0.5;

  rgb_t rgb_red = { 1.0, 0, 0 };
  rgb_t rgb_green = { 0, 1.0, 0 };
  rgb_t rgb_blue = { 0, 0, 1.0 };
  rgb_t rgb_brown = { 1.0, 0, 1.0 };

  GdkColor dark_background;
  GdkColor light_background;

  get_dark_square_color(dark_background);
  get_light_square_color(light_background);

  rgb_t rgb_background_dark = { dark_background.red / 65535.0, dark_background.green / 65535.0, dark_background.blue / 65535.0 };
  rgb_t rgb_background_light = { light_background.red / 65535.0, light_background.green / 65535.0, light_background.blue / 65535.0 };

  rgb_t tmp;

  int const dark = 0;
  int const light = 1;
  M_colors[2 * background + dark] = 0;
  M_colors[2 * background + light] = 0;
  tmp = mix(rgb_background_dark, 1.0, rgb_red, alpha);
  M_colors[2 * red + dark] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_light, 1.0, rgb_red, alpha);
  M_colors[2 * red + light] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_dark, 1.0, rgb_green, alpha);
  M_colors[2 * green + dark] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_light, 1.0, rgb_green, alpha);
  M_colors[2 * green + light] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_dark, 1.0, rgb_blue, alpha);
  M_colors[2 * blue + dark] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_light, 1.0, rgb_blue, alpha);
  M_colors[2 * blue + light] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_dark, 1.0, rgb_brown, alpha);
  M_colors[2 * brown + dark] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
  tmp = mix(rgb_background_light, 1.0, rgb_brown, alpha);
  M_colors[2 * brown + light] = allocate_color_handle_rgb(tmp.red, tmp.green, tmp.blue);
}

LinuxChessboardWidget::ColorHandle LinuxChessboardWidget::get_color_handle(int index, colors_t color)
{
  int const dark = 0;
  int const light = 1;
  int offset = ((index & 1) == ((index >> 3) & 1)) ? dark : light;
  return M_colors[2 * color + offset];
}

void LinuxChessboardWidget::show_reachables(int col, int row, mode_type mode)
{
  using namespace cwchess;

  Index index(col, row);
  BitBoard bb;
  switch (mode)
  {
    case mode_show_candidates:
      bb = candidates(index);
      Dout(dc::notice, "candidates: 0x" << std::hex << bb() << std::dec << " " << bb);
      break;
    case mode_show_reachables:
      bb = reachables(index);
      Dout(dc::notice, "reachables: 0x" << std::hex << bb() << std::dec << " " << bb);
      break;
    case mode_show_attacked:
      bb = reachables(index, true);
      Dout(dc::notice, "attacked reachables: 0x" << std::hex << bb() << std::dec << " " << bb);
      break;
    case mode_show_defendables:
    {
      bool battery = false;
      bb = defendables(piece_at(index).code(), index, battery);
      Dout(dc::notice, "defendables: 0x" << std::hex << bb() << std::dec << " " << bb);
      break;
    }
    case mode_show_defended_black:
    {
      bb = get_defended()[black].any();
      for (int row = 7; row >= 0; --row)
      {
        for (int col = 0; col <= 7; ++col)
        {
	  Index i(col, row);
	  std::cout << get_defended()[black].count(i) << " ";
	}
	std::cout << '\n';
      }
      break;
    }
    case mode_show_defended_white:
    {
      bb = get_defended()[white].any();
      for (int row = 7; row >= 0; --row)
      {
        for (int col = 0; col <= 7; ++col)
        {
	  Index i(col, row);
	  std::cout << get_defended()[white].count(i) << " ";
	}
	std::cout << '\n';
      }
      break;
    }
    default:
    {
      Color color(to_move());
      Piece piece(piece_at(col, row));
      if (!piece.code().is_nothing())
	to_move(piece.color());
      bb = moves(index);
      if (get_widget_mode() == mode_edit_game)
	to_move(color);
      Dout(dc::notice, "moves: 0x" << std::hex << bb() << std::dec << " " << bb);
      break;
    }
  }
  if (mode != mode_show_defended_black && mode != mode_show_defended_white)
    disable_hud_layer(1);
  ColorHandle handles[64];
  std::memset(handles, 0, sizeof(handles));
  for (Index index = index_begin; index != index_end; ++index)
    if (bb.test(index))
      handles[index()] = get_color_handle(index(), red);
  set_background_colors(handles);
  update_en_passant_arrow();
}

void LinuxChessboardWidget::update_en_passant_arrow()
{
  DoutEntering(dc::notice, "LinuxChessboardWidget::update_en_passant_arrow()");
  using namespace cwchess;
  EnPassant const& en_passant(this->en_passant());
  if (en_passant.exists())
  {
    if (M_en_passant_arrow &&
        M_en_passant_arrow_index != en_passant.index())
    {
      remove_arrow(M_en_passant_arrow);
      M_en_passant_arrow = nullptr;
    }
    if (!M_en_passant_arrow)
    {
      Index from(en_passant.from_index());
      Index to(en_passant.pawn_index());
      GdkColor color = { 0, 0, 65535, 65535 };
      M_en_passant_arrow = add_arrow(from.col(), from.row(), to.col(), to.row(), color);
      M_en_passant_arrow_index = en_passant.index();
    }
  }
  else if (M_en_passant_arrow)
  {
    remove_arrow(M_en_passant_arrow);
    M_en_passant_arrow = nullptr;
  }
}

void LinuxChessboardWidget::show_pinning()
{
  using namespace cwchess;
  BitBoard bb1 = attackers(black) | attackers(white);
  BitBoard bb2 = pinned(black) | pinned(white);
  ColorHandle handles[64];
  std::memset(handles, 0, sizeof(handles));
  for (Index index = index_begin; index != index_end; ++index)
    if (bb1.test(index) && !bb2.test(index))
      handles[index()] = get_color_handle(index(), blue);
    else if (bb2.test(index) && !bb1.test(index))
      handles[index()] = get_color_handle(index(), red);
    else if (bb1.test(index) && bb2.test(index))
      handles[index()] = get_color_handle(index(), brown);
  set_background_colors(handles);
}
