// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file direction_table.cc Program used to generate cwchess::direction_table[].
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

Direction dirs[] = {
  { -1, -1 },
  { 0, -1 },
  { 1, -1 },
  { -1, 0 },
  { 1, 0 },
  { -1, 1 },
  { 0, 1 },
  { 1, 1 }
};

int main()
{
  int count = 0;
  for (int row = 0; row < 8; ++row)
  {
    for (int col = 0; col < 8; ++col)
    {
      for (int d = 0; d < 8; ++d)
      {
	int r = row;
	int c = col;
	uint64_t mask = 0;
	for(;;)
	{
	  c += dirs[d].x;
	  r += dirs[d].y;
	  if (c < 0 || c > 7 || r < 0 || r > 7)
	    break;
	  mask |= CW_MASK_T_CONST(1) << ((r << 3) + c);
	}
	if ((count % 4) == 0)
	  cout << "  ";
	cout << "{CW_MASK_T_CONST(0x" << std::hex << std::setfill('0') << std::setw(16) << mask << ")}, ";
	if ((++count % 4) == 0)
	  cout << '\n';
      }
    }
  }
}
