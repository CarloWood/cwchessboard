// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file from_to_table.cxx Program used to generate cwchess::from_to_table[].
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
#include <cstdlib>
#include <cstdint>

typedef uint64_t mask_t;

void output(int from, int to, mask_t mask)
{
  static int count = 0;
  if (count == 0)
    std::cout << " ";
  std::cout << " {CW_MASK_T_CONST(0x" << std::hex << std::setfill('0') << std::setw(16) << mask << std::dec << ")},";
  if (++count == 4)
  {
    std::cout << '\n';
    count = 0;
  }
}

int main()
{
  for (int from = 0; from < 64; ++from)
  {
    int from_col = from & 7;
    int from_row = from >> 3;
    for (int to = 0; to < 64; ++to)
    {
      if (from == to)
      {
        output(from, to, CW_MASK_T_CONST(0));
	continue;
      }
      int to_col = to & 7;
      int to_row = to >> 3;
      int col_diff = abs(from_col - to_col);
      int row_diff = abs(from_row - to_row);
      if (col_diff != 0 && row_diff != 0 && col_diff != row_diff)
      {
        output(from, to, CW_MASK_T_CONST(0));
	continue;
      }
      int dcol = (to_col - from_col);
      if (dcol)
        dcol /= col_diff;
      int drow = (to_row - from_row);
      if (drow)
        drow /= row_diff;
      mask_t mask = 0;
      int c = from_col;
      int r = from_row;
      do
      {
	mask |= CW_MASK_T_CONST(1) << ((r << 3) | c);
        c += dcol;
	r += drow;
      }
      while (c != to_col || r != to_row);
      output(from, to, mask);
    }
  }
}
