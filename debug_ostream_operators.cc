// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file debug_ostream_operators.cc Debug ostream inserter functions.
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
#include "sys.h"        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.
#endif

#ifdef CWDEBUG

#ifndef USE_PCH
#include <iostream>
#include "debug.h"
#endif

#include "Color.h"
#include "Index.h"
#include "ChessPosition.h"
#include "ChessNotation.h"

namespace cwchess {

std::ostream& operator<<(std::ostream& os, Color const& color)
{
  if (color.is_white())
    os << "white";
  else
    os << "black";
  return os;
}

std::ostream& operator<<(std::ostream& os, Index const& index)
{
  return os << ChessNotation(ChessPosition(), index);
}

std::ostream& operator<<(std::ostream& os, Code const& code)
{
  os << '{';
  if (!code.is_nothing())
    os << code.color() << ' ';
  return os << code.type() << '}';
}

std::ostream& operator<<(std::ostream& os, Type const& type)
{
  switch(type())
  {
    case nothing_bits:
      os << "nothing";
      break;
    case pawn_bits:
      os << "pawn";
      break;
    case rook_bits:
      os << "rook";
      break;
    case knight_bits:
      os << "knight";
      break;
    case bishop_bits:
      os << "bishop";
      break;
    case queen_bits:
      os << "queen";
      break;
    case king_bits:
      os << "king";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, BitBoard const& bitboard)
{
  bool first = true;
  os << '[';
  for (Index index = index_begin; index != index_end; ++index)
    if ((bitboard() & index2mask(index)))
    {
      int col = index.col();
      int row = index.row();
      if (!first)
        os << ", ";
      else
        first = false;
      os << (char)('a' + col) << (char)('1' + row);
    }
  return os << ']';
}

} // namespace cwchess

#endif // CWDEBUG
