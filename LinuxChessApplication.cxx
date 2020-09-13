#include "sys.h"
#include "LinuxChessApplication.h"
#include "LinuxChessWindow.h"
#include "LinuxChessMenuBar.h"
#include "debug.h"

LinuxChessApplication::LinuxChessApplication() : Gtk::Application("info.linuxchess")
{
  DoutEntering(dc::notice, "LinuxChessApplication::LinuxChessApplication()");
  Glib::set_application_name("LinuxChess");
}

LinuxChessApplication::~LinuxChessApplication()
{
  Dout(dc::notice, "Calling LinuxChessApplication::~LinuxChessApplication()");
}

Glib::RefPtr<LinuxChessApplication> LinuxChessApplication::create()
{
  return Glib::RefPtr<LinuxChessApplication>(new LinuxChessApplication);
}

void LinuxChessApplication::on_startup()
{
  DoutEntering(dc::notice, "LinuxChessApplication::on_startup()");

  // Call the base class's implementation.
  Gtk::Application::on_startup();

  // Layout the actions in a menubar and an application menu.
  Glib::ustring ui_info =
    "<interface>"
    "  <menu id='menu-linuxchess'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_File</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Load</attribute>"
    "          <attribute name='action'>win.FileLoad</attribute>"                       // LinuxChessWindow::on_menu_file_load
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Save</attribute>"
    "          <attribute name='action'>win.FileSave</attribute>"                       // LinuxChessWindow::on_menu_file_save
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Exit</attribute>"
    "          <attribute name='action'>app.FileExit</attribute>"                       // LinuxChessApplication::on_menu_file_quit
    "        </item>"
    "      </section>"
    "    </submenu>"
    "  </menu>"
    "</interface>";

  // Layout the actions in a menubar and an application menu.
  Glib::ustring ui_info2 =
    "<interface>"
    "  <menu id='menu-linuxchess'>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_Game</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>New</attribute>"
    "          <attribute name='action'>win.GameNew</attribute>"                        // LinuxChessWindow::on_menu_game_new
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Clear</attribute>"
    "          <attribute name='action'>win.GameClear</attribute>"                      // LinuxChessWindow::on_menu_game_clear
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Export</attribute>"
    "          <attribute name='action'>win.GameExport</attribute>"                     // LinuxChessWindow::on_menu_game_export
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Undo</attribute>"
    "          <attribute name='action'>win.GameUndo</attribute>"                       // LinuxChessWindow::on_menu_game_undo
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Flip</attribute>"
    "          <attribute name='action'>win.GameFlip</attribute>"                       // LinuxChessWindow::on_menu_game_flip
    "        </item>"
    "      </section>"
    "    </submenu>"
    "    <submenu>"
    "      <attribute name='label' translatable='yes'>_Mode</attribute>"
    "      <section>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Edit position</attribute>"
    "          <attribute name='action'>win.ModeEditPosition</attribute>"               // LinuxChessWindow::on_menu_mode_editposition
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Edit game</attribute>"
    "          <attribute name='action'>win.ModeEditGame</attribute>"                   // LinuxChessWindow::on_menu_mode_editgame
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show candidates</attribute>"
    "          <attribute name='action'>chessboard.ModeShowCandidates</attribute>"      // LinuxChessboardWidget::on_menu_mode_showcandidates
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show reachables</attribute>"
    "          <attribute name='action'>chessboard.ModeShowReachables</attribute>"      // LinuxChessboardWidget::on_menu_mode_showreachables
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show attacked</attribute>"
    "          <attribute name='action'>chessboard.ModeShowAttacked</attribute>"        // LinuxChessboardWidget::on_menu_mode_showattacked
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show defendables</attribute>"
    "          <attribute name='action'>chessboard.ModeShowDefendables</attribute>"     // LinuxChessboardWidget::on_menu_mode_showdefendables
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show defended by black</attribute>"
    "          <attribute name='action'>chessboard.ModeShowDefendedBlack</attribute>"   // LinuxChessboardWidget::on_menu_mode_showdefended_black
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show defended by white</attribute>"
    "          <attribute name='action'>chessboard.ModeShowDefendedWhite</attribute>"   // LinuxChessboardWidget::on_menu_mode_showdefended_white
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Show moves</attribute>"
    "          <attribute name='action'>chessboard.ModeShowMoves</attribute>"           // LinuxChessboardWidget::on_menu_mode_showmoves
    "        </item>"
    "        <item>"
    "          <attribute name='label' translatable='yes'>Place pieces</attribute>"
    "          <attribute name='action'>chessboard.ModePlacePieces</attribute>"         // LinuxChessboardWidget::on_menu_mode_placepieces
    "        </item>"
    "      </section>"
    "    </submenu>"
    "  </menu>"
    "</interface>";
}

void LinuxChessApplication::on_activate()
{
  DoutEntering(dc::notice, "LinuxChessApplication::on_activate()");

  // The application has been started, create and show the main window.
  m_main_window = create_window();
}

LinuxChessWindow* LinuxChessApplication::create_window()
{
  LinuxChessWindow* main_window = new LinuxChessWindow(this);

  // Make sure that the application runs as long this window is still open.
  add_window(*main_window);
  std::vector<Gtk::Window*> windows = get_windows();
  Dout(dc::notice, "The application has " << windows.size() << " windows.");
  ASSERT(G_IS_OBJECT(main_window->gobj()));

  // Delete the window when it is hidden.
  main_window->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &LinuxChessApplication::on_window_hide), main_window));

  main_window->show_all();
  return main_window;
}

void LinuxChessApplication::on_window_hide(Gtk::Window* window)
{
  DoutEntering(dc::notice, "LinuxChessApplication::on_window_hide(" << window << ")");
  // There is only one window, no?
  ASSERT(window == m_main_window);

  // Hiding the window removed it from the application.
  // Set our pointer to nullptr, just to be sure we won't access it again.
  // Delete the window.
  if (window == m_main_window)
  {
    m_main_window = nullptr;
    delete window;
  }

  Dout(dc::notice, "Leaving LinuxChessApplication::on_window_hide()");
}

void LinuxChessApplication::append_menu_entries(LinuxChessMenuBar* menubar)
{
  using namespace menu_keys;
  using namespace Gtk::Stock;
#define ADD(top, entry) \
  menubar->append_menu_entry({top, entry},   this, &LinuxChessApplication::on_menu_##top##_##entry)

  ADD(File, QUIT);
}

void LinuxChessApplication::on_menu_File_QUIT()
{
  DoutEntering(dc::notice, "LinuxChessApplication::on_menu_File_QUIT()");

  quit(); // Not really necessary, when Gtk::Widget::hide() is called.

  // Gio::Application::quit() will make Gio::Application::run() return,
  // but it's a crude way of ending the program. The window is not removed
  // from the application. Neither the window's nor the application's
  // destructors will be called, because there will be remaining reference
  // counts in both of them. If we want the destructors to be called, we
  // must remove the window from the application. One way of doing this
  // is to hide the window.
  std::vector<Gtk::Window*> windows = get_windows();
  Dout(dc::notice, "The application has " << windows.size() << " windows.");
  for (auto window : windows)
    window->hide();

  Dout(dc::notice, "Leaving LinuxChessApplication::on_menu_File_QUIT()");
}
