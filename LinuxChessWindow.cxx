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
#endif

  // Show cursor, reachables and pinning as function of whether we picked up a piece or not.
  m_chessboard_widget.signal_picked_up().connect(sigc::mem_fun(m_chessboard_widget, &LinuxChessboardWidget::picked_up));
  m_chessboard_widget.signal_dropped().connect(sigc::mem_fun(m_chessboard_widget, &LinuxChessboardWidget::dropped));
  m_chessboard_widget.signal_position_editted().connect(sigc::mem_fun(this, &LinuxChessWindow::position_editted));
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

  Gtk::RadioButtonGroup mode;
#define ADD_RADIO(top, entry) \
  menubar->append_radio_menu_entry(mode, {top, entry},   this, &LinuxChessWindow::on_menu_##top##_##entry)

  ADD(File, OPEN);
  ADD(File, SAVE);
  ADD(Game, NEW);
  ADD(Game, CLEAR);
  ADD(Game, Export);
  ADD(Game, UNDO);
  ADD(Game, Flip);
  ADD_RADIO(Mode, EditPosition);
  ADD_RADIO(Mode, EditGame);
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

void LinuxChessWindow::on_menu_Mode_EditPosition()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Mode_EditPosition()");
  if (m_chessboard_widget.get_widget_mode() != cwmm::ChessPositionWidget::mode_edit_position)
    m_chessboard_widget.set_widget_mode(cwmm::ChessPositionWidget::mode_edit_position);
  //M_ModePlacePieces_action->set_active(true);
}

void LinuxChessWindow::on_menu_Mode_EditGame()
{
  DoutEntering(dc::notice, "LinuxChessWindow::on_menu_Mode_EditGame()");
  if (m_chessboard_widget.get_widget_mode() != cwmm::ChessPositionWidget::mode_edit_game)
  {
    if (m_chessboard_widget.check(m_chessboard_widget.to_move().opposite()))
    {
      // Refuse to switch when both sides are in check.
      if (m_chessboard_widget.check(m_chessboard_widget.to_move()))
      {
        using namespace menu_keys;
        m_menubar->activate({Mode, EditPosition});
        return;
      }
      m_chessboard_widget.to_move(m_chessboard_widget.to_move().opposite());
    }
    m_chessboard_widget.set_widget_mode(cwmm::ChessPositionWidget::mode_edit_game);
  }
}

void LinuxChessWindow::position_editted()
{
  DoutEntering(dc::notice, "LinuxChessWindow::position_editted()");
  using namespace cwchess;
  using namespace cwmm;
  // We get here while in Edit Position mode and something was changed.
  // This function is meant to gray out switching to Edit Game when
  // the current position is illegal.
  if (m_chessboard_widget.get_widget_mode() == ChessPositionWidget::mode_edit_position)
  {
    using namespace menu_keys;
    bool illegal_position = m_chessboard_widget.index_of_king(black) == index_end ||                    // Missing black king.
                            m_chessboard_widget.index_of_king(white) == index_end ||                    // Missing white king.
                            (m_chessboard_widget.check(black) && m_chessboard_widget.check(white));     // Both kings are in check.
    m_menubar->set_sensitive(!illegal_position, {Mode, EditGame});
  }
}
