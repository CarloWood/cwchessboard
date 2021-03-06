// cwchessboard -- A C++ chessboard tool set
//
//! @file MoveIterator.inl This file contains the inline definitions of class MoveIterator.
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

namespace cwchess {

inline Type MoveIterator::initial_type(Index const& index) const
{
  Piece const& piece(M_chess_position->piece_at(index));
  if (piece != pawn)
    return nothing;
  return (index.row() == ((piece == white) ? 6 : 1)) ? queen : nothing;
}

inline MoveIterator::MoveIterator(ChessPosition const* chess_position, Index const& index) :
    M_chess_position(chess_position), M_targets(chess_position->moves(index)), M_current_move(index, get_first_bitindex(), initial_type(index)) { }

} // namespace cwchess

