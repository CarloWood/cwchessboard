// cwchessboard -- A C++ chessboard tool set
//
//! @file Direction.h This file contains the declaration of class Direction.
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

#ifndef DIRECTION_H
#define DIRECTION_H

#ifndef USE_PCH
#endif

#include "BitBoard.h"
#include "Type.h"
#include "Flags.h"

namespace cwchess {

uint8_t const df_rook_mover = rook_bits;
uint8_t const df_bishop_mover = bishop_bits;
uint8_t const df_pinned_horizontally = 0 << 3;
uint8_t const df_pinned_vertically = 1 << 3;
uint8_t const df_pinned_slashy = 2 << 3;
uint8_t const df_pinned_backslashy = 3 << 3;
uint8_t const df_pinned_mask = 3 << 3;

struct Direction {
  int8_t const shift;
  int8_t const offset;
  int8_t const index;
  uint8_t const flags;

  FlagsData mover_flags(void) const { FlagsData result; result.M_bits = flags & type_mask; return result; }
  FlagsData pinning_flags(void) const { FlagsData result; result.M_bits = flags >> 3; return result; }
  bool is_horizontal(void) const { return shift == 1; }
  bool matches(Type const& type) const { return (type() & flags) == (type_mask & flags); }

  //! @brief Return a BitBoard with all squares in this direction.
  BitBoard from(Index const &index) const { return direction_table[((static_cast<int>(index())) << 3) + this->index]; }

  friend Index operator+(Index const& index, Direction const& direction) { Index result(index); result += direction.offset; return result; }
  friend Index operator+(Direction const& direction, Index const& index) { Index result(index); result += direction.offset; return result; }
  friend Index operator-(Index const& index, Direction const& direction) { Index result(index); result -= direction.offset; return result; }
  friend Index operator-(Direction const& direction, Index const& index) { Index result(index); result -= direction.offset; return result; }

  private:
    static BitBoardData const direction_table[64 * 8];
};

Direction const south_west = { 9, -9, 0, df_bishop_mover | df_pinned_slashy };
Direction const south = { 8, -8, 1, df_rook_mover | df_pinned_vertically };
Direction const south_east = { 7, -7, 2, df_bishop_mover | df_pinned_backslashy };
Direction const west = { 1, -1, 3, df_rook_mover | df_pinned_horizontally };
Direction const east = { 1, 1, 4, df_rook_mover | df_pinned_horizontally };
Direction const north_west = { 7, 7, 5, df_bishop_mover | df_pinned_backslashy };
Direction const north = { 8, 8, 6, df_rook_mover | df_pinned_vertically };
Direction const north_east = { 9, 9, 7, df_bishop_mover | df_pinned_slashy };

extern BitBoardData const from_to_table[64 * 64];

inline BitBoard squares_from_to(Index const& from, Index const& to)
{
  return from_to_table[(from() << 6) | to()];
}

extern uint8_t const direction_index_table[256];

inline Direction const& direction_from_to(Index const& from, Index const& to)
{
  uint8_t from_bits(from());
  uint8_t to_bits(to());
  uint8_t from_col_bits = from_bits & 0x07;
  from_bits <<= 1;
  uint8_t from_row_bits = from_bits & 0x70;
  uint8_t to_col_bits = to_bits & 0x07;
  to_bits <<= 1;
  uint8_t to_row_bits = to_bits & 0x70;
  uint8_t row_diff = to_row_bits - from_row_bits;
  uint8_t col_diff = (to_col_bits - from_col_bits) & 0x0f;
  static Direction const directions[9] = { south_west, south, south_east, west, east, north_west, north, north_east, {0,0,0,0} };
  return directions[direction_index_table[row_diff | col_diff]];
}

} // namespace cwchess

#endif	// DIRECTION_H
