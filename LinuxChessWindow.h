#pragma once

#include "LinuxChessboardWidget.h"
#include <gtkmm.h>

class LinuxChessMenuBar;
class LinuxChessApplication;

class LinuxChessWindow : public Gtk::Window
{
 private:
  LinuxChessApplication* m_application;         // The appliction that was passed to the constructor.

 public:
  LinuxChessWindow(LinuxChessApplication* application);
  ~LinuxChessWindow() override;

  // Accessors for the chessboard widget.
//  LinuxChessboardWidget& chessboard_widget() { return m_chessboard_widget; }
//  LinuxChessboardWidget const& chessboard_widget() const { return m_chessboard_widget; }

  void append_menu_entries(LinuxChessMenuBar* menubar);

 protected:
  friend class LinuxChessMenuBar;       // Calls append_menu_entries on the two objects returned by the following accessors.
  LinuxChessApplication& application() const { return *m_application; }
  LinuxChessboardWidget& chessboard_widget() { return m_chessboard_widget; }

  void moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& current_position);

  void on_menu_File_OPEN();
  void on_menu_File_SAVE();
  void on_menu_Game_NEW();
  void on_menu_Game_CLEAR();
  void on_menu_Game_Export();
  void on_menu_Game_UNDO();
  void on_menu_Game_Flip();
  void on_menu_Mode_EditPosition();
  void on_menu_Mode_EditGame();

  void position_editted();

  // Child widgets.
  Gtk::Grid m_grid;
  LinuxChessMenuBar* m_menubar;
  LinuxChessboardWidget m_chessboard_widget;

  std::stack<cwchess::ChessPosition> M_history;
};
