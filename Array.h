// cwchessboard -- A C++ chessboard tool set
//
//! @file Array.h This file contains the definition of class Array.
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

#ifndef ARRAY_H
#define ARRAY_H

#ifndef USE_PCH
#endif

#include "Color.h"
#include "Index.h"

#define DEBUG_ARRAY_RANGE_CHECK 0

#if DEBUG_ARRAY_RANGE_CHECK
#include <cassert>
#endif

namespace cwchess {

template<typename T>
struct ArrayCode {
  T M_array[16];

  T& operator[](Code const& code)
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(code() < 16);
#endif
    return M_array[code()];
  }
  T const& operator[](Code const& code) const
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(code() < 16);
#endif
    return M_array[code()];
  }

  T& operator[](Color const& color)
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(color() == 0 || color() == 8);
#endif
    return M_array[color()];
  }
  T const& operator[](Color const& color) const
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(color() == 0 || color() == 8);
#endif
    return M_array[color()];
  }
};

template<typename T>
struct ArrayColor {
  T M_array[2];

  T& operator[](Code const& code)
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(code() < 16);
#endif
    return M_array[code() >> 3];
  }
  T const& operator[](Code const& code) const
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(code() < 16);
#endif
    return M_array[code() >> 3];
  }

  T& operator[](Color const& color)
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(color.index() < 2);
#endif
    return M_array[color.index()];
  }
  T const& operator[](Color const& color) const
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(color.index() < 2);
#endif
    return M_array[color.index()];
  }
};

template<typename T>
struct ArrayIndex {
  T M_array[64];

  T& operator[](Index const& index)
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(index() < 64);
#endif
    return M_array[index()];
  }
  T const& operator[](Index const& index) const
  {
#if DEBUG_ARRAY_RANGE_CHECK
    assert(index() < 64);
#endif
    return M_array[index()];
  }
};

} // namespace cwchess

#endif	// ARRAY_H
