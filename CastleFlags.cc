// cwchessboard -- A C++ chessboard tool set
//
//! @file CastleFlags.cc This file contains the implementation of class CastleFlags.
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
#include "debug.h"
#endif

#include "CastleFlags.h"

namespace cwchess {

void CastleFlags::piece_moved_from(Piece const& piece, Index const& from)
{
  switch (piece.code()())
  {
    case (black_bits|rook_bits):
      if (from == ia8)
	M_bits |= black_rook_queen_side_moved;
      else if (from == ih8)
	M_bits |= black_rook_king_side_moved;
      break;
    case (black_bits|king_bits):
      M_bits |= black_king_moved;
      break;
    case (white_bits|rook_bits):
      if (from == ia1)
	M_bits |= white_rook_queen_side_moved;
      else if (from == ih1)
	M_bits |= white_rook_king_side_moved;
      break;
    case (white_bits|king_bits):
      M_bits |= white_king_moved;
      break;
    default:
      return;
  }
}

bool CastleFlags::has_moved(Code const& code, Index const& index)
{
  if (index == ia1 && code == white_rook)
    return M_bits & white_rook_queen_side_moved;
  else if (index == ie1 && code == white_king)
    return M_bits & white_king_moved;
  else if (index == ih1 && code == white_rook)
    return M_bits & white_rook_king_side_moved;
  else if (index == ia8 && code == black_rook)
    return M_bits & black_rook_queen_side_moved;
  else if (index == ie8 && code == black_king)
    return M_bits & black_king_moved;
  else if (index == ih8 && code == black_rook)
    return M_bits & black_rook_king_side_moved;
  // We should normally never get here. Return some random value.
  return false;
}

} // namespace cwchess
