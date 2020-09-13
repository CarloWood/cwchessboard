#pragma once

#include <gtkmm.h>

class LinuxChessWindow;
class LinuxChessMenuBar;

class LinuxChessApplication : public Gtk::Application
{
 private:
  LinuxChessWindow* m_main_window;

 protected:
  LinuxChessApplication();
  ~LinuxChessApplication() override;

 public:
  static Glib::RefPtr<LinuxChessApplication> create();

  void append_menu_entries(LinuxChessMenuBar* menubar);

 protected:
  void on_startup() override;
  void on_activate() override;

 private:
  LinuxChessWindow* create_window();

  void on_menu_File_QUIT();

  void on_window_hide(Gtk::Window* window);
};
