// cwchessboard -- A C++ chessboard tool set
//
//! @file Color.h This file contains the declaration of class Color.
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

#ifndef COLOR_H
#define COLOR_H

#ifndef USE_PCH
#include <stdint.h>
#endif

namespace cwchess {

class Code;

/** @brief The POD base type of class Color.
 *
 * This class uses the same internal type as Code to store the color bit.
 * It even uses the same bit (the fourth bit). All other bits are garanteed zero.
 *
 * If the bit is set then the object represents the color white.
 *
 * @sa Color, black, white
 */ 
struct ColorData {
  uint8_t M_bits;		//!< 0000C000, where C=0 means black and C=1 means white.
};

uint8_t const black_bits = 0;	//!< The underlaying integral value of color 'black'.
uint8_t const white_bits = 8;	//!< The underlaying integral value of color 'white'.
uint8_t const color_mask = 8;	//!< A mask for the bits used for the color of a piece. 

//! A constant representing the color black.
ColorData const black = { black_bits };
//! A constant representing the color white.
ColorData const white = { white_bits };

// Compare constants (this should never be needed, but why not add it).
inline bool operator==(ColorData c1, ColorData c2) { return c1.M_bits == c2.M_bits; }
inline bool operator!=(ColorData c1, ColorData c2) { return c1.M_bits != c2.M_bits; }

/** @brief A color (black or white).
 *
 * This class represents a chess color.
 *
 * See ColorData for defined constants.
 */
class Color : protected ColorData {
  public:

  /** @name Constructors */
  //@{

    //! Construct an uninitialized Color object.
    Color(void) { }

    //! Copy-constructor.
    Color(Color const& color) { M_bits = color.M_bits; }

    //! Construct a Color object from a constant.
    Color(ColorData color) { M_bits = color.M_bits; }

  //@}

  /** @name Assignment operators */
  //@{

    //! Assign from another Color object.
    Color& operator=(Color const& color) { M_bits = color.M_bits; return *this; }

    //! Assign from a constant.
    Color& operator=(ColorData color) { M_bits = color.M_bits; return *this; }

  //@}

  /** @name Comparison operators */
  //@{

    friend bool operator==(Color const& c1, Color const& c2) { return c1.M_bits == c2.M_bits; }
    friend bool operator==(Color const& c1, ColorData c2) { return c1.M_bits == c2.M_bits; }
    friend bool operator==(ColorData c1, Color const& c2) { return c1.M_bits == c2.M_bits; }
    friend bool operator!=(Color const& c1, Color const& c2) { return c1.M_bits != c2.M_bits; }
    friend bool operator!=(Color const& c1, ColorData c2) { return c1.M_bits != c2.M_bits; }
    friend bool operator!=(ColorData c1, Color const& c2) { return c1.M_bits != c2.M_bits; }

  //@}
    
  /** @name Accessors */
  //@{

    //! Return TRUE if this color is white.
    bool is_white(void) const { return M_bits; }

    //! Return TRUE if this color is black.
    bool is_black(void) const { return !M_bits; }

    //! Return the underlaying integral value.
    uint8_t operator()(void) const { return M_bits; }

  //@}

  /** @name Special functions */
  //@{

    //! Change the color from black to white or vica versa.
    void toggle(void) { M_bits ^= color_mask; }

  //@}

  /** @name Visitors */
  //@{

    //! Return a Color object with the opposite color of this object.
    Color opposite(void) const { ColorData data; data.M_bits = M_bits ^ color_mask; return Color(data); }

    //! Return a number that can be used as array index.
    uint8_t index(void) const { return M_bits >> 3; }

    //! Return the index offset that advances one square in the direction of the pawns of this color.
    uint8_t forward_index_offset(void) const { return (M_bits << 1) - 8; }
    
  //@}

#ifndef DOXYGEN
  private:
    friend class Code;
    // Constructor for class Code.
    explicit Color(uint8_t bits) { M_bits = bits; }
#endif
};

} // namespace cwchess

#endif	// COLOR_H
