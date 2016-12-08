// cwchessboard -- A C++ chessboard tool set
//
//! @file ChessNotation.cc This file contains the implementation of class ChessNotation.
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
#include <iostream>
#endif

#include "ChessNotation.h"

namespace cwchess {

std::ostream& operator<<(std::ostream& os, ChessNotation const& chess_notation)
{
  if (chess_notation.M_type)
    chess_notation.print_on(os, *chess_notation.M_type);
  if (chess_notation.M_piece)
    chess_notation.print_on(os, *chess_notation.M_piece);
  if (chess_notation.M_index)
    chess_notation.print_on(os, *chess_notation.M_index);
  if (chess_notation.M_move)
    chess_notation.print_on(os, *chess_notation.M_move);
  return os;
}

void ChessNotation::print_on(std::ostream& os, Piece const& piece) const
{
  print_on(os, piece.type());
}

void ChessNotation::print_on(std::ostream& os, Type const& type) const
{
  switch(type())
  {
    case knight_bits:
      os << 'N';
      break;
    case king_bits:
      os << 'K';
      break;
    case bishop_bits:
      os << 'B';
      break;
    case rook_bits:
      os << 'R';
      break;
    case queen_bits:
      os << 'Q';
      break;
  }
}

void ChessNotation::print_on(std::ostream& os, Index const& index) const
{
  char column = 'a' + index.col();
  char rank = '1' + index.row();
  os << column << rank;
}

void ChessNotation::print_on(std::ostream& os, Move const& move) const
{
  Piece const& piece(M_chess_position.piece_at(move.from()));
  int col_diff = move.from().col() - move.to().col();
  if (piece == king && (col_diff == 2 || col_diff == -2))
  {
    if (col_diff == 2)
      os << "0-0-0";
    else
      os << "0-0";
  }
  else
  {
    print_on(os, piece);
    print_on(os, move.from());
    bool target_square_empty = M_chess_position.piece_at(move.to()) == nothing;
    bool en_passant = col_diff != 0 && piece == pawn && target_square_empty;
    if (target_square_empty && !en_passant)
      os << '-';
    else
      os << 'x';
    print_on(os, move.to());
    if (en_passant)
      os << " e.p.";
    if (move.is_promotion())
    {
      os << '(';
      print_on(os, move.promotion_type());
      os << ')';
    }
  }
  Debug(dc::place.off());
  ChessPosition tmp(M_chess_position);
  if (!tmp.legal(move))
    os << " illegal move!";
  else
  {
    bool draw = tmp.execute(move);
    // Find the number of remaining possible moves.
    int moves = 0;
    for (PieceIterator piece_iter = tmp.piece_begin(tmp.to_move()); piece_iter != tmp.piece_end(); ++piece_iter)
    {
      MoveIterator move_end(tmp.move_end());
      for (MoveIterator iter = tmp.move_begin(piece_iter.index()); iter != move_end; ++iter)
	++moves;
    }
    bool check = tmp.check();
    bool check_mate = false;
    bool stale_mate = false;
    if (moves == 0)
    {
      if (check)
      {
        check_mate = true;
	draw = false;
      }
      else
      {
        stale_mate = true;
        draw = true;
      }
    }
    if (check_mate)
      os << '#';
    else if (stale_mate)
      os << " stale mate";
    if (check_mate)
    {
      if (tmp.to_move() == black)
        os << " 1-0";
      else
        os << " 0-1";
    }
    else if (check)
      os << '+';
    if (draw)
      os << " 1/2-1/2";
  }
  Debug(dc::place.on());
}

} // namespace cwchess
