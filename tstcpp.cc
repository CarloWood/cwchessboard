// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file tstcpp.cc A full-blown test application used for debugging %cwchess::%ChessPosition.
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
// cwmm::ChessboardWidget usage example in C++ code.
//

#include "sys.h"
#include <iomanip>
#include <gtk/gtk.h>
#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/stock.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/menu.h>
#include <gtkmm/iconfactory.h>
#include <cstring>
#include <stack>
#include <fstream>
#include "ChessPositionWidget.h"
#include "debug.h"
#include "ChessNotation.h"

enum mode_type {
  mode_show_candidates,
  mode_show_reachables,
  mode_show_attacked,
  mode_show_defendables,
  mode_show_defended_black,
  mode_show_defended_white,
  mode_show_moves,
  mode_popup_menu
};

// The chessboard.
class MyChessboardWidget : public cwmm::ChessPositionWidget {
  private:
    gint M_arrow_begin_col;
    gint M_arrow_begin_row;
    ColorHandle M_colors[10];
    gpointer M_en_passant_arrow;
    cwchess::Index M_en_passant_arrow_index;
    mode_type M_mode;

  protected:
    virtual void draw_hud_layer(cairo_t* cr, gint sside, guint hud);
    virtual bool on_button_press(gint col, gint row, GdkEventButton const* event);
    virtual bool on_button_release(gint col, gint row, GdkEventButton const* event);

  public:
    void on_menu_mode_editposition(void)
    {
      if (get_widget_mode() != mode_edit_position)
	set_widget_mode(mode_edit_position);
    }
    void on_menu_mode_editgame(void)
    {
      if (get_widget_mode() != mode_edit_game)
	set_widget_mode(mode_edit_game);
    }
    void on_menu_mode_showcandidates(void) { M_mode = mode_show_candidates; }
    void on_menu_mode_showreachables(void) { M_mode = mode_show_reachables; }
    void on_menu_mode_showattacked(void) { M_mode = mode_show_attacked; }
    void on_menu_mode_showdefendables(void) { M_mode = mode_show_defendables; }
    void on_menu_mode_showdefended_black(void) { M_mode = mode_show_defended_black; }
    void on_menu_mode_showdefended_white(void) { M_mode = mode_show_defended_white; }
    void on_menu_mode_showmoves(void) { M_mode = mode_show_moves; }
    void on_menu_mode_placepieces(void) { M_mode = mode_popup_menu; }

    void picked_up(cwchess::Index const& index, cwchess::ChessPosition const& chess_position);
    void dropped(gint col, gint row, cwchess::ChessPosition const& chess_position);
    void moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& current_position);
    void illegal(cwchess::Move const& move, cwchess::ChessPosition const& chess_position);

  public:
    MyChessboardWidget(Gtk::Window* drawable);

  private:
    void show_reachables(int col, int row, mode_type mode);
    void update_en_passant_arrow(void);
    void show_pinning();

  private:
    enum colors_t {
      background = 0,
      red,
      green,
      blue,
      brown
    };

  private:
    void init_colors(void);
    ColorHandle get_color_handle(int index, colors_t color);
};

MyChessboardWidget::MyChessboardWidget(Gtk::Window* drawable) : cwmm::ChessPositionWidget(drawable), M_en_passant_arrow(NULL)
{
  init_colors();
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

void MyChessboardWidget::init_colors(void)
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

MyChessboardWidget::ColorHandle MyChessboardWidget::get_color_handle(int index, colors_t color)
{
  int const dark = 0;
  int const light = 1;
  int offset = ((index & 1) == ((index >> 3) & 1)) ? dark : light;
  return M_colors[2 * color + offset];
}

void MyChessboardWidget::show_pinning(void)
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

void MyChessboardWidget::show_reachables(int col, int row, mode_type mode)
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
      cwchess::Color color(to_move());
      cwchess::Piece piece(piece_at(col, row));
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

void MyChessboardWidget::update_en_passant_arrow(void)
{
  cwchess::EnPassant const& en_passant(this->en_passant());
  if (en_passant.exists())
  {
    if (M_en_passant_arrow_index != en_passant.index())
    {
      remove_arrow(M_en_passant_arrow);
      M_en_passant_arrow = NULL;
    }
    if (!M_en_passant_arrow)
    {
      cwchess::Index from(en_passant.from_index());
      cwchess::Index to(en_passant.pawn_index());
      GdkColor color = { 0, 0, 65535, 65535 };
      M_en_passant_arrow = add_arrow(from.col(), from.row(), to.col(), to.row(), color);
      M_en_passant_arrow_index = en_passant.index();
    }
  }
  else if (M_en_passant_arrow)
  {
    remove_arrow(M_en_passant_arrow);
    M_en_passant_arrow = NULL;
  }
}

bool MyChessboardWidget::on_button_press(gint col, gint row, GdkEventButton const* event)
{
  // Ignore double clicks.
  if (event->type == GDK_2BUTTON_PRESS)
    return false;
  if (event->button == 2)
  {
    // Inside board?
    if (col != -1)
    {
      show_cursor();
      // Marker the start square.
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

bool MyChessboardWidget::on_button_release(gint col, gint row, GdkEventButton const* event)
{
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

void MyChessboardWidget::moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& UNUSED(current_position))
{
  DoutEntering(dc::notice, "MyChessboardWidget::moved(" << cwchess::ChessNotation(previous_position, move) << ", ...)");
  // Having executed a move, we might need to draw or erase the en passant arrow.
  update_en_passant_arrow();
}

void MyChessboardWidget::illegal(cwchess::Move const& move, cwchess::ChessPosition const& UNUSED(chess_position))
{
  DoutEntering(dc::notice, "MyChessboardWidget::illegal(" << cwchess::ChessNotation(*this, move) << ", ...)");
}

void MyChessboardWidget::picked_up(cwchess::Index const& index, cwchess::ChessPosition const& UNUSED(chess_position))
{
  DoutEntering(dc::notice, "MyChessboardWidget::picked_up(" << cwchess::ChessNotation(*this, index) << ", ...)");
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

void MyChessboardWidget::dropped(gint col, gint row, cwchess::ChessPosition const& UNUSED(chess_position))
{
  DoutEntering(dc::notice, "MyChessboardWidget::dropped(" << col << ", " << row << ", ...)");
  // The piece that was picked up is dropped again. Return to regular standing.
  hide_cursor();
  show_pinning();
}

void MyChessboardWidget::draw_hud_layer(cairo_t* cr, gint sside, guint hud)
{
  DoutEntering(dc::notice, "draw_hud_layer(" << cr << ", " << sside << ", " << hud << ")");
  cairo_text_extents_t extents;
  cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 20);
  cairo_text_extents(cr, "3", &extents);
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
      cairo_move_to(cr, x, y);
      if (count > 9)
	cairo_show_text(cr, number[1]);
      cairo_show_text(cr, utf8);
    }
  }
}

// The main application.
class GtkTest : public Gtk::Window {
  public:
    GtkTest(int width, int height);
    virtual ~GtkTest() { }

    // Accessors for the chessboard widget.
    MyChessboardWidget& chessboard_widget(void) { return m_chessboard_widget; }
    MyChessboardWidget const& chessboard_widget(void) const { return m_chessboard_widget; }

  private:
    void setup_menu(void);

  protected:
    //Signal handlers:
    virtual void on_menu_file_quit();
    virtual void on_menu_file_open();
    virtual void on_menu_file_save();
    virtual void on_menu_file_new();
    virtual void on_menu_file_clear();
    virtual void on_menu_file_export();
    virtual void on_menu_file_undo();
    virtual void on_menu_file_flip();
    virtual void on_menu_mode_editposition(void)
    {
      m_chessboard_widget.on_menu_mode_editposition();
      if (m_chessboard_widget.get_widget_mode() == cwmm::ChessPositionWidget::mode_edit_game)
	M_ModePlacePieces_action->set_active(true);
      m_chessboard_widget.set_widget_mode(cwmm::ChessPositionWidget::mode_edit_position);
    }
    virtual void on_menu_mode_editgame(void)
    {
      m_chessboard_widget.on_menu_mode_editgame();
      m_chessboard_widget.set_widget_mode(cwmm::ChessPositionWidget::mode_edit_game);
    }

    void moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& current_position);

    //Member widgets:
    Gtk::VBox m_vbox;
    MyChessboardWidget m_chessboard_widget;

    // Menu and tool bar stuff.
    Glib::RefPtr<Gtk::UIManager> m_refUIManager;
    Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
    Glib::RefPtr<Gtk::RadioAction> M_ModePlacePieces_action;
    Glib::RefPtr<Gtk::RadioAction> M_ModeEditPosition_action;

    std::stack<cwchess::ChessPosition> M_history;
};

void GtkTest::moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& UNUSED(current_position))
{
  DoutEntering(dc::notice, "GtkTest::moved(" << cwchess::ChessNotation(previous_position, move) << ", ...)");
  M_history.push(previous_position);
}

GtkTest::GtkTest(int width, int height) : m_vbox(FALSE, 0), m_chessboard_widget(this)
{
  // Set a title.
  set_title("ChessboardWidget Test");

  // Sets the border width of the window.
  set_border_width(10);

  // Set a default width and height.
  set_default_size(width, height);

  // A vbox to put a menu and chessboard in.
  add(m_vbox);

  // Set up the menu.
  setup_menu();

  // Add the chessboard to m_vbox.
  m_vbox.add(m_chessboard_widget);

  // Show everything.
  show_all_children(); 

  // Record moves.
  m_chessboard_widget.signal_moved().connect(sigc::mem_fun(this, &GtkTest::moved));

  // Update en passant arrow.
  m_chessboard_widget.signal_moved().connect(sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::moved));

  // Just show that we can catch this event too.
  m_chessboard_widget.signal_illegal().connect(sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::illegal));

  // Show cursor, reachables and pinning as function of whether we picked up a piece or not.
  m_chessboard_widget.signal_picked_up().connect(sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::picked_up));
  m_chessboard_widget.signal_dropped().connect(sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::dropped));
}

void GtkTest::on_menu_file_quit(void)
{
  hide();
}

void GtkTest::on_menu_file_save(void)
{
  std::ofstream file;
  file.open("FEN.out");
  file << chessboard_widget().FEN() << std::endl;
  file.close();
}

void GtkTest::on_menu_file_undo(void)
{
  if (M_history.empty())
    return;
  chessboard_widget().set_position(M_history.top());
  M_history.pop();
}

void GtkTest::on_menu_file_new(void)
{
  chessboard_widget().initial_position();
}

void GtkTest::on_menu_file_clear(void)
{
  chessboard_widget().clear();
  Dout(dc::notice, "Calling M_ModeEditPosition_action->set_active(true)");
  M_ModeEditPosition_action->set_active(true); 
}

void GtkTest::on_menu_file_flip(void)
{
  chessboard_widget().set_flip_board(!chessboard_widget().get_flip_board());
}

void GtkTest::on_menu_file_export(void)
{
  using namespace cwchess;
  std::cout << "  chess_position.load_FEN(\"" << chessboard_widget().FEN() << "\");\n";
  std::cout << "  static mask_t moves1[] = {\n";
  int color_count = 0;
  bool first = true;
  for (Color color(black); color_count < 2; ++color_count, color = white)
  {
    for (PieceIterator piece_iter = chessboard_widget().piece_begin(color); piece_iter != chessboard_widget().piece_end(); ++piece_iter)
    {
      if (first)
      {
        std::cout << "    ";
        first = false;
      }
      else
        std::cout << ", ";
      std::cout << std::hex << "0x" << chessboard_widget().moves(piece_iter.index())() << std::dec;
    }
  }
  std::cout << "\n  };" << std::endl;
}

void GtkTest::on_menu_file_open(void)
{
  std::ifstream file;
  file.open("FEN.out");
  std::string fen;
  std::getline(file, fen);
  file.close();
  chessboard_widget().load_FEN(fen);
}

void GtkTest::setup_menu(void)
{
  // Create the action group.
  m_refActionGroup = Gtk::ActionGroup::create();

  // File menu:
  m_refActionGroup->add(Gtk::Action::create("FileMenu", "File"));
  // Add the QUIT action.
  m_refActionGroup->add(Gtk::Action::create("FileQuit", Gtk::Stock::QUIT),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_quit));
  // Add the OPEN action.
  m_refActionGroup->add(Gtk::Action::create("FileOpen", Gtk::Stock::OPEN, "Load", "Load position from file FEN.out"),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_open));
  // Add the SAVE action.
  m_refActionGroup->add(Gtk::Action::create("FileSave", Gtk::Stock::SAVE, "Save", "Save position to file FEN.out"),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_save));
  // Add the CONVERT action.
  m_refActionGroup->add(Gtk::Action::create("FileConvert", Gtk::Stock::CONVERT, "Export"),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_export));
  // Add the CLEAR action.
  m_refActionGroup->add(Gtk::Action::create("FileClear", Gtk::Stock::CLEAR, "Clear", "Remove all pieces and switch to Mode 'Edit position'."),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_clear));
  // Add the NEW action.
  m_refActionGroup->add(Gtk::Action::create("FileNew", Gtk::Stock::NEW, "New", "Set up initial position."),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_new));
  // Add the UNDO action.
  m_refActionGroup->add(Gtk::Action::create("FileUndo", Gtk::Stock::UNDO, "Undo"),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_undo));
  // Add the FLIP action.
  m_refActionGroup->add(Gtk::Action::create("FileFlip", Gtk::Stock::REFRESH, "Flip"),
      sigc::mem_fun(*this, &GtkTest::on_menu_file_flip));

  // Mode menu:
  m_refActionGroup->add(Gtk::Action::create("ModeMenu", "Mode"));
  Gtk::RadioAction::Group group_widget_mode;
  Gtk::RadioAction::Group group_rightclick_mode;
  // Add SET UP POSITION action.
  M_ModeEditPosition_action = Gtk::RadioAction::create(group_widget_mode, "ModeEditPosition", "Edit position");
  m_refActionGroup->add(M_ModeEditPosition_action, sigc::mem_fun(*this, &GtkTest::on_menu_mode_editposition));
  // Add EDIT GAME action.
  Glib::RefPtr<Gtk::RadioAction> ModeEditGame_action(Gtk::RadioAction::create(group_widget_mode, "ModeEditGame", "Edit game"));
  m_refActionGroup->add(ModeEditGame_action, sigc::mem_fun(*this, &GtkTest::on_menu_mode_editgame));
  // Add the SHOW CANDIDATES action.
  m_refActionGroup->add(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowCandidates", "Show candidates"),
      sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showcandidates));
  // Add the SHOW REACHABLES action.
  m_refActionGroup->add(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowReachables", "Show reachables"),
      sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showreachables));
  // Add the SHOW ATTACKED action.
  m_refActionGroup->add(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowAttacked", "Show attacked"),
      sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showattacked));
  // Add the SHOW DEFENDABLES action.
  m_refActionGroup->add(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowDefendables", "Show defendables"),
      sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showdefendables));
  // Add the SHOW DEFENDED BLACK action.
  m_refActionGroup->add(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowDefendedBlack", "Show defended by black"),
      sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showdefended_black));
  // Add the SHOW DEFENDED WHITE action.
  m_refActionGroup->add(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowDefendedWhite", "Show defended by white"),
      sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showdefended_white));
  // Add the SHOW MOVES action.
  Glib::RefPtr<Gtk::RadioAction> ModeShowMoves_action(Gtk::RadioAction::create(group_rightclick_mode, "ModeShowMoves", "Show moves"));
  m_refActionGroup->add(ModeShowMoves_action, sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_showmoves));
  // Add the PLACE PIECES action.
  M_ModePlacePieces_action = Gtk::RadioAction::create(group_rightclick_mode, "ModePlacePieces", "Place pieces");
  m_refActionGroup->add(M_ModePlacePieces_action, sigc::mem_fun(m_chessboard_widget, &MyChessboardWidget::on_menu_mode_placepieces));

  // Default.
#if 1
  Dout(dc::notice, "Calling ModeEditGame_action->set_active(true)");
  ModeEditGame_action->set_active(true);
  Dout(dc::notice, "Calling ModeShowMoves_action->set_active(true)");
  ModeShowMoves_action->set_active(true);
#else
  Dout(dc::notice, "Calling M_ModeEditPosition_action->set_active(true)");
  M_ModeEditPosition_action->set_active(true); 
  Dout(dc::notice, "Calling M_ModePlacePieces_action->set_active(true)");
  M_ModePlacePieces_action->set_active(true);
#endif

  // Create a UIManager and add the ActionGroup to the UIManager.
  m_refUIManager = Gtk::UIManager::create();
  m_refUIManager->insert_action_group(m_refActionGroup);

  // Tell the parent window to respond to the specified keyboard shortcuts.
  add_accel_group(m_refUIManager->get_accel_group());

  // Layout the actions in a menubar and toolbar.
  Glib::ustring ui_info = 
        "<ui>"
        "  <menubar name='MenuBar'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='FileOpen'/>"
        "      <menuitem action='FileSave'/>"
        "      <menuitem action='FileQuit'/>"
        "    </menu>"
        "    <menu action='ModeMenu'>"
        "      <menuitem action='ModeEditPosition'/>"
        "      <menuitem action='ModeEditGame'/>"
	"      <separator/>"
        "      <menuitem action='ModeShowCandidates'/>"
        "      <menuitem action='ModeShowReachables'/>"
        "      <menuitem action='ModeShowAttacked'/>"
        "      <menuitem action='ModeShowDefendables'/>"
        "      <menuitem action='ModeShowDefendedBlack'/>"
        "      <menuitem action='ModeShowDefendedWhite'/>"
        "      <menuitem action='ModeShowMoves'/>"
        "      <menuitem action='ModePlacePieces'/>"
        "    </menu>"
        "  </menubar>"
        "  <toolbar  name='ToolBar'>"
        "    <toolitem action='FileQuit'/>"
        "    <toolitem action='FileOpen'/>"
        "    <toolitem action='FileSave'/>"
        "    <toolitem action='FileNew'/>"
        "    <toolitem action='FileClear'/>"
        "    <toolitem action='FileConvert'/>"
        "    <toolitem action='FileUndo'/>"
        "    <toolitem action='FileFlip'/>"
        "  </toolbar>"
        "</ui>";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
#else
  std::auto_ptr<Glib::Error> ex;
  m_refUIManager->add_ui_from_string(ui_info, ex);
  if(ex.get())
  {
    std::cerr << "building menus failed: " <<  ex->what();
  }
#endif // GLIBMM_EXCEPTIONS_ENABLED

  // Get the menubar and toolbar widgets, and add them to a container widget.
  Gtk::Widget* pMenubar = m_refUIManager->get_widget("/MenuBar");
  if(pMenubar)
    m_vbox.pack_start(*pMenubar, Gtk::PACK_SHRINK);

  Gtk::Widget* pToolbar = m_refUIManager->get_widget("/ToolBar") ;
  if(pToolbar)
    m_vbox.pack_start(*pToolbar, Gtk::PACK_SHRINK);
}

int main(int argc, char* argv[])
{
  using namespace cwchess;

  Debug(NAMESPACE_DEBUG::init());
  Dout(dc::notice, "Entered main()");

  Gtk::Main kit(argc, argv);

  // Create the application window, passing a default size.
  GtkTest application(500, 532);

  // Get a reference to the chessboard widget.
  MyChessboardWidget& chessboard_widget(application.chessboard_widget());

  ChessPosition chess_position;
  //chess_position.initial_position();
  chess_position.load_FEN("rnbqkbnr/4p1pp/1p6/p1p1PpP1/1P1p4/2P5/P2P1P1P/RNBQKBNR w KQkq f6 0 7");
#if 0
  std::ifstream file;
  file.open("FEN.out");
  std::string fen;
  std::getline(file, fen);
  file.close();
  chess_position.load_FEN(fen);
#endif
  //chess_position.swap_colors();

  chessboard_widget.set_position(chess_position);

  // Set the border color.
  uint32_t border_color_html = 0x597359;
  GdkColor brown = { 0, static_cast<guint16>((border_color_html & 0xff0000) >> 8), static_cast<guint16>(border_color_html & 0xff00), static_cast<guint16>((border_color_html & 0xff) << 8) };
  chessboard_widget.set_border_color(brown);

  //open_gdb = true;
  Gtk::Main::run(application);

  return 0;
}
