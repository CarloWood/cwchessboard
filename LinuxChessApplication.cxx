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
