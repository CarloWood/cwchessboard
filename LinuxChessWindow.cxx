#include "sys.h"
#include "LinuxChessWindow.h"
#include "LinuxChessMenuBar.h"

LinuxChessWindow::LinuxChessWindow(LinuxChessApplication* application) : m_chessboard_widget(this), m_application(application)
{
  DoutEntering(dc::notice, "LinuxChessWindow::LinuxChessWindow(" << application << ")");

  set_title("LinuxChess");
  set_default_size(500, 800);

  m_menubar = new LinuxChessMenuBar(this);
  m_grid.attach(*m_menubar, 0, 0);

  m_chessboard_widget.set_hexpand(true);
  m_chessboard_widget.set_halign(Gtk::ALIGN_FILL);
  m_chessboard_widget.set_vexpand(true);
  m_chessboard_widget.set_valign(Gtk::ALIGN_FILL);

  m_grid.attach(m_chessboard_widget, 0, 1);

  add(m_grid);

  show_all_children();

#if 0
  // Record moves.
  m_chessboard_widget.signal_moved().connect(sigc::mem_fun(this, &LinuxChessWindow::moved));

  // Update en passant arrow.
  m_chessboard_widget.signal_moved().connect(sigc::mem_fun(m_chessboard_widget, &LinuxChessboardWidget::moved));

  // Just show that we can catch this event too.
  m_chessboard_widget.signal_illegal().connect(sigc::mem_fun(m_chessboard_widget, &LinuxChessboardWidget::illegal));

  // Show cursor, reachables and pinning as function of whether we picked up a piece or not.
  m_chessboard_widget.signal_picked_up().connect(sigc::mem_fun(m_chessboard_widget, &LinuxChessboardWidget::picked_up));
  m_chessboard_widget.signal_dropped().connect(sigc::mem_fun(m_chessboard_widget, &LinuxChessboardWidget::dropped));
#endif
}

LinuxChessWindow::~LinuxChessWindow()
{
  Dout(dc::notice, "Calling LinuxChessWindow::~LinuxChessWindow()");
}

void LinuxChessWindow::append_menu_entries(LinuxChessMenuBar* menubar)
{
  using namespace menu_keys;
  using namespace Gtk::Stock;
#define ADD(top, entry) \
  menubar->append_menu_entry({top, entry},   this, &LinuxChessWindow::on_menu_##top##_##entry)

  ADD(File, OPEN);
  ADD(File, SAVE);
  ADD(Game, NEW);
  ADD(Game, CLEAR);
  ADD(Game, Export);
  ADD(Game, UNDO);
  ADD(Game, Flip);
  ADD(Mode, EditPosition);
  ADD(Mode, EditGame);
}

void LinuxChessWindow::moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& current_position)
{
}

void LinuxChessWindow::on_menu_File_OPEN()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_File_OPEN()");
}

void LinuxChessWindow::on_menu_File_SAVE()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_File_SAVE()");
}

void LinuxChessWindow::on_menu_Game_NEW()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Game_NEW()");
}

void LinuxChessWindow::on_menu_Game_CLEAR()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Game_CLEAR()");
}

void LinuxChessWindow::on_menu_Game_Export()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Game_Export()");
}

void LinuxChessWindow::on_menu_Game_UNDO()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Game_UNDO()");
}

void LinuxChessWindow::on_menu_Game_Flip()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Game_Flip()");
}
