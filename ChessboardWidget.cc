// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file ChessboardWidget.cc This file contains the implementation of the gtkmm class ChessboardWidget.
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

#ifndef USE_PCH
#include "sys.h"
#endif

#include "ChessboardWidget.h"
#include "CwChessboardCodes.h"

namespace cwmm {

void ChessboardWidget::S_draw_turn_indicator_hook(CwChessboard* chessboard, gboolean white, gboolean on)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_turn_indicator(white, on);
}

void ChessboardWidget::S_draw_border_hook(CwChessboard* chessboard)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_border();
}

void ChessboardWidget::S_draw_hud_layer_hook(CwChessboard* chessboard, cairo_t* cr, gint sside, guint hud) 
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_hud_layer(cr, sside, hud);
}

gboolean ChessboardWidget::S_draw_hud_square_hook(CwChessboard* chessboard, cairo_t* cr, gint col, gint row, gint sside, guint hud)
{
  return static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_hud_square(cr, col, row, sside, hud);
}

void ChessboardWidget::S_draw_pawn_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_pawn(cr, x, y, sside, white);
}

void ChessboardWidget::S_draw_rook_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_rook(cr, x, y, sside, white);
}

void ChessboardWidget::S_draw_knight_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_knight(cr, x, y, sside, white);
}

void ChessboardWidget::S_draw_bishop_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_bishop(cr, x, y, sside, white);
}

void ChessboardWidget::S_draw_queen_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_queen(cr, x, y, sside, white);
}

void ChessboardWidget::S_draw_king_hook(CwChessboard* chessboard, cairo_t* cr, gdouble x, gdouble y, gdouble sside, gboolean white)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->draw_king(cr, x, y, sside, white);
}

void ChessboardWidget::S_on_cursor_left_chessboard_hook(CwChessboard* chessboard, gint prev_col, gint prev_row)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->on_cursor_left_chessboard(prev_col, prev_row);
}

void ChessboardWidget::S_on_cursor_entered_square_hook(CwChessboard* chessboard, gint prev_col, gint prev_row, gint col, gint row)
{
  static_cast<ChessboardWidget*>(chessboard->gtkmm_widget)->on_cursor_entered_square(prev_col, prev_row, col, row);
}

ChessboardWidget::ChessboardWidget() : Gtk::DrawingArea(&M_chessboard->parent)
{
  // Initialize casting pointer.
  M_chessboard->gtkmm_widget = static_cast<void*>(this);

  // Connect the events to our virtual functions.
  signal_button_press_event().connect(sigc::mem_fun(*this, &ChessboardWidget::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &ChessboardWidget::on_button_release_event));
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_turn_indicator = S_draw_turn_indicator_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->cursor_left_chessboard = S_on_cursor_left_chessboard_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->cursor_entered_square = S_on_cursor_entered_square_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_border = S_draw_border_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_hud_layer = S_draw_hud_layer_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_hud_square = S_draw_hud_square_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_piece[(::white_pawn >> 1) - 1] = S_draw_pawn_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_piece[(::white_rook >> 1) - 1] = S_draw_rook_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_piece[(::white_knight >> 1) - 1] = S_draw_knight_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_piece[(::white_bishop >> 1) - 1] = S_draw_bishop_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_piece[(::white_queen >> 1) - 1] = S_draw_queen_hook;
  CW_CHESSBOARD_GET_CLASS(M_chessboard)->draw_piece[(::white_king >> 1) - 1] = S_draw_king_hook;
}

ChessboardWidget::~ChessboardWidget()
{
}

} // namespace cwmm
