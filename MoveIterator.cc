// cwchessboard -- A C++ chessboard tool set
//
//! @file MoveIterator.cc This file contains the implementation of class MoveIterator.
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

#include "MoveIterator.h"

namespace cwchess {

bool MoveIterator::next_promotion(void)
{
  Type type = rook;
  Type promotion_type = M_current_move.promotion_type();
  // The piece is a queen when we get here first.
  // Order: queen -> rook -> knight -> bishop -> return true.
  if (promotion_type == bishop)
    return true;		// We tried all types.
  else if (promotion_type == rook)
    type = knight;
  else if (promotion_type == knight)
    type = bishop;
  M_current_move.set_promotion(type);
  return false;
}

bool MoveIterator::prev_promotion(void)
{
  Type type = queen;
  Type promotion_type = M_current_move.promotion_type();
  // Order: bishop -> knight -> rook --> queen --> return true.
  if (promotion_type == queen)
    return true;		// We tried all types.
  else if (promotion_type == bishop)
    type = knight;
  else if (promotion_type == knight)
    type = rook;
  M_current_move.set_promotion(type);
  return false;
}

} // namespace cwchess
