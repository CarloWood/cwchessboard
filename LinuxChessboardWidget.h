#pragma once

#include "ChessPositionWidget.h"

class LinuxChessMenuBar;

enum mode_type {
  mode_show_nothing,
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
class LinuxChessboardWidget : public cwmm::ChessPositionWidget
{
 private:
  gint M_arrow_begin_col;
  gint M_arrow_begin_row;
  ColorHandle M_colors[10];
  gpointer M_en_passant_arrow;
  cwchess::Index M_en_passant_arrow_index;
  mode_type M_mode;
  bool m_showing;

  //Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;

 protected:
  void draw_hud_layer(Cairo::RefPtr<Cairo::Context> const& crmm, gint sside, guint hud) override;
  bool on_button_press(gint col, gint row, GdkEventButton const* event) override;
  bool on_button_release(gint col, gint row, GdkEventButton const* event) override;
  void initialize_menu() override;
  void on_cursor_entered_square(gint prev_col, gint prev_row, gint col, gint row) override;
  void on_cursor_left_chessboard(gint prev_col, gint prev_row) override;

 public:
  void picked_up(cwchess::Index const& index, cwchess::ChessPosition const& chess_position);
  void dropped(gint col, gint row, cwchess::ChessPosition const& chess_position);
  void moved(cwchess::Move const& move, cwchess::ChessPosition const& previous_position, cwchess::ChessPosition const& current_position);
  void illegal(cwchess::Move const& move, cwchess::ChessPosition const& chess_position);

 public:
  LinuxChessboardWidget(Gtk::Window* main_window, Glib::RefPtr<cwchess::Promotion> promotion = Glib::RefPtr<cwchess::Promotion>(new cwchess::Promotion));
  virtual ~LinuxChessboardWidget() { DoutEntering(dc::notice, "LinuxChessboardWidget::~LinuxChessboardWidget()"); }

  void append_menu_entries(LinuxChessMenuBar* menubar);

 protected:
  void on_menu_Mode_ShowCandidates() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowCandidates()"); M_mode = mode_show_candidates; }
  void on_menu_Mode_ShowReachables() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowReachables()"); M_mode = mode_show_reachables; }
  void on_menu_Mode_ShowAttacked() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowAttacked()"); M_mode = mode_show_attacked; }
  void on_menu_Mode_ShowDefendables() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowDefendables()"); M_mode = mode_show_defendables; }
  void on_menu_Mode_ShowDefendedBlack() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowDefendedBlack()"); M_mode = mode_show_defended_black; }
  void on_menu_Mode_ShowDefendedWhite() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowDefendedWhite()"); M_mode = mode_show_defended_white; }
  void on_menu_Mode_ShowMoves() { DoutEntering(dc::notice, "LinuxChessboardWidget::on_menu_Mode_ShowMoves()"); M_mode = mode_show_moves; }
  void on_menu_Mode_PlacePieces();

 private:
  void show_reachables(int col, int row, mode_type mode);
  void update_en_passant_arrow();
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
  void init_colors();
  ColorHandle get_color_handle(int index, colors_t color);
};
