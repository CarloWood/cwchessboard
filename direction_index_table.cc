// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file direction_index_table.cc Program used to generate cwchess::direction_index_table[].
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

#include <iostream>
#include <iomanip>
#include <cstdlib>

void output(int val)
{
  static int count = 0;
  if (count == 0)
    std::cout << ' ';
  std::cout << " 0x" << std::setfill('0') << std::setw(2) << std::hex << val << std::dec << ',';
  if (++count == 16)
  {
    std::cout << '\n';
    count = 0;
  }
}

int main()
{
  for (int i = 0; i < 256; ++i)
  {
    int col_diff = i & 0xf;
    int row_diff = i >> 4;
    if (col_diff == 8 || row_diff == 8)
    {
      output(8);
      continue;
    }
    if (col_diff > 8)
      col_diff -= 16;
    if (row_diff > 8)
      row_diff -= 16;
    if (col_diff == 0 && row_diff == 0)
    {
      output(8);
      continue;
    }
    if (col_diff != 0 && row_diff != 0 && std::abs(col_diff) != std::abs(row_diff))
    {
      output(8);
      continue;
    }
    if (col_diff == 0 && row_diff < 0)
      output(1);	// south
    else if (col_diff == 0 && row_diff > 0)
      output(6);	// north
    else if (col_diff < 0 && row_diff == 0)
      output(3);	// west
    else if (col_diff > 0 && row_diff == 0)
      output(4);	// east
    else if (col_diff < 0 && row_diff < 0)
      output(0);	// south-west
    else if (col_diff < 0 && row_diff > 0)
      output(5);	// north-west
    else if (col_diff > 0 && row_diff < 0)
      output(2);	// south-east
    else if (col_diff > 0 && row_diff > 0)
      output(7);	// north-east
  }
  std::cout << '\n';
}
