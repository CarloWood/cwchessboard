// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file candidates_table.cc Program used to generate cwchess::candidates_table[].
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

#include "sys.h"
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;

struct Direction {
  int x;
  int y;
};

struct Piece {
  char const* name;
  int depth;
  int nrdirs;
  Direction direction[4];
};

Piece piece[] = {
  { "Knight", 1, 4, { { 1, 2 }, { 2, 1 }, { 2, -1, }, { 1, -2 } } },
  { "King", 1, 4, { { 0, 1 }, { 1, 0 }, { 1, 1 }, { 1, -1 } } },
  { "Bishop", 8, 2, { { 1, 1 }, { 1, -1 } } },
  { "Rook", 8, 2, { { 0, 1 }, { 1, 0 } } },
  { "Queen", 8, 4, { { 0, 1 }, { 1, 0 }, { 1, 1 }, { 1, -1 } } }
};

uint64_t colrow2mask(int col, int row)
{
  return CW_MASK_T_CONST(1) << ((col << 3) + row);
}

int main()
{
  for (int p = 0; p < 5; ++p)
  {
    cout << "// " << piece[p].name << '\n';
    for (int col = 0; col < 8; ++col)
    {
      for (int row = 0; row < 8; ++row)
      {
        uint64_t mask = 0;
        for (int dir = 0; dir < piece[p].nrdirs; ++dir)
	{
	  for (int depth = -piece[p].depth; depth <= piece[p].depth; ++depth)
	  {
	    if (depth == 0)
	      continue;
	    int tcol = col + depth * piece[p].direction[dir].x;
	    if (tcol < 0 || tcol > 7)
	      continue;
	    int trow = row + depth * piece[p].direction[dir].y;
	    if (trow < 0 || trow > 7)
	      continue;
	    mask |= colrow2mask(tcol, trow);
	  }
	}
	std::cout << std::hex << "{ CW_MASK_T_CONST(0x" << std::setfill('0') << std::setw(16) << mask << ") }, ";
	if (row == 3)
	  std::cout << '\n';
      }
      std::cout << '\n';
    }
  }
}
