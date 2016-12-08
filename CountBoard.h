// cwchessboard -- A C++ chessboard tool set
//
//! @file CountBoard.h This file contains the declaration of class CountBoard.
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

#ifndef COUNTBOARD_H
#define COUNTBOARD_H

#ifndef USE_PCH
#include <cstring>
#ifndef DOXYGEN
#include "debug.h"
#endif
#endif

#include "BitBoard.h"

namespace cwchess {

class CountBoard {
  private:
    BitBoard M_bits[4];
    BitBoard M_any;

  public:
    CountBoard(void) { }
    CountBoard(CountBoard const& count_board) { std::memcpy(this, &count_board, sizeof(CountBoard)); }
    CountBoard& operator=(CountBoard const& count_board) { std::memcpy(this, &count_board, sizeof(CountBoard)); return *this; }

    void add(BitBoard const& bit_board)
    {
#ifdef CWDEBUG
      if (debug::channels::dc::countboard.is_on())
      {
	// Get the location that we were called from.
	libcwd::location_ct location((char*)__builtin_return_address(0) + libcwd::builtin_return_address_offset);
	// Demangle the function name of the location that we were called from.
	std::string demangled_function_name;
	libcwd::demangle_symbol(location.mangled_function_name(), demangled_function_name);
	// Print it.
	Dout(dc::countboard, "CountBoard::add() with this = " << (void*)this << " [called from " << demangled_function_name << '(' << location << ")], adds:");
	for (int row = 7; row >= 0; --row)
	{
	  for (int col = 0; col <= 7; ++col)
	  {
	    if (bit_board.test(col, row))
	      std::cout << "1 ";
	    else
	      std::cout << "0 ";
	  }
	  std::cout << '\n';
	}
      }
#endif
      // This whole function takes typically 4 clock cycles on a QX6700.
      BitBoard input(bit_board);
      M_any |= input;
      BitBoard bits_and1(M_bits[0] & input);
      M_bits[0] ^= input;
      BitBoard bits_and2(M_bits[1] & bits_and1);
      M_bits[1] ^= bits_and1;
      bits_and1 = M_bits[2] & bits_and2;
      M_bits[2] ^= bits_and2;
      M_bits[3] ^= bits_and1;
    }

    void sub(BitBoard const& bit_board)
    {
#ifdef CWDEBUG
      if (debug::channels::dc::countboard.is_on())
      {
	// Get the location that we were called from.
	libcwd::location_ct location((char*)__builtin_return_address(0) + libcwd::builtin_return_address_offset);
	// Demangle the function name of the location that we were called from.
	std::string demangled_function_name;
	libcwd::demangle_symbol(location.mangled_function_name(), demangled_function_name);
	// Print it.
	Dout(dc::countboard, "CountBoard::sub() with this = " << (void*)this << " [called from " << demangled_function_name << '(' << location << ")], subtracts:");
	for (int row = 7; row >= 0; --row)
	{
	  for (int col = 0; col <= 7; ++col)
	  {
	    if (bit_board.test(col, row))
	      std::cout << "1 ";
	    else
	      std::cout << "0 ";
	  }
	  std::cout << '\n';
	}
      }
#endif
      // This whole function takes typically 5 clock cycles on a QX6700.
      BitBoard input(bit_board);
      BitBoard bits_and1(~M_bits[0] & input);
      BitBoard collect(M_bits[0] ^= input);
      BitBoard bits_and2(~M_bits[1] & bits_and1);
      collect |= (M_bits[1] ^= bits_and1);
      bits_and1 = ~M_bits[2] & bits_and2;
      collect |= (M_bits[2] ^= bits_and2);
      collect |= (M_bits[3] ^= bits_and1);
      M_any = collect;
    }

    void reset(void)
    {
      std::memset(this, 0, sizeof(CountBoard));
#if DEBUG_BITBOARD_INITIALIZATION
      M_bits[0].reset();
      M_bits[1].reset();
      M_bits[2].reset();
      M_bits[3].reset();
      M_any.reset();
#endif
    }

    BitBoard any(void) const { return M_any; }

    int count(BitBoard const& pos) const
    {
      return ((M_bits[0] & pos) ? 1 : 0) | ((M_bits[1] & pos) ? 2 : 0) | ((M_bits[2] & pos) ? 4 : 0) | ((M_bits[3] & pos) ? 8 : 0);
    }

    int count(Index const& index) const
    {
      BitBoard pos(index);
      return count(pos);
    }
};

} // namespace cwchess

#endif	// COUNTBOARD_H
