// cwchessboard -- A C++ chessboard tool set
//
//! @file Type.h This file contains the declaration of class Type.
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

#ifndef TYPE_H
#define TYPE_H

#ifndef USE_PCH
#include <stdint.h>
#endif

#include "Color.h"

namespace cwchess {

class Code;

/** @brief The POD base type of class Type.
 *
 * This class uses the same internal type as Code to store the type bits.
 * It even uses the same bits (the three least significant bits).
 * All other bits are garanteed zero.
 *
 * If the third bit is set then the object represents a sliding piece (bishop, rook or queen).
 * If in addition the first bit is set it can moves like a bishop (bishop and queen),
 * or if the second bit is set it can move like a rook (rook and queen).
 *
 * @sa Type, nothing, pawn, knight, king, bishop, rook, queen
 */ 
struct TypeData {
  uint8_t M_bits;	//!< 00000STT, where STT is the type. If S == 1 then the piece is a slider.
};

uint8_t const nothing_bits = 0;		//!< The underlaying integral value of type 'nothing'.
uint8_t const pawn_bits = 1;		//!< The underlaying integral value of type 'pawn'.
uint8_t const knight_bits = 2;		//!< The underlaying integral value of type 'knight'.
uint8_t const king_bits = 3;		//!< The underlaying integral value of type 'king'.
uint8_t const bishop_bits = 5;		//!< The underlaying integral value of type 'bishop'.
uint8_t const rook_bits = 6;		//!< The underlaying integral value of type 'rook'.
uint8_t const queen_bits = 7;		//!< The underlaying integral value of type 'queen'.
uint8_t const type_mask = 7;		//!< A mask for the bits used for the type of a piece.

//! A constant representing the absence of a piece.
TypeData const nothing = { nothing_bits };
//! A constant representing a pawn.
TypeData const pawn = { pawn_bits };
//! A constant representing a knight.
TypeData const knight = { knight_bits };
//! A constant representing a king.
TypeData const king = { king_bits };
//! A constant representing a bishop.
TypeData const bishop = { bishop_bits };
//! A constant representing a rook.
TypeData const rook = { rook_bits };
//! A constant representing a queen.
TypeData const queen = { queen_bits };

// Compare constants (this should never be needed, but why not add it).
inline bool operator==(TypeData t1, TypeData t2) { return t1.M_bits == t2.M_bits; }
inline bool operator!=(TypeData t1, TypeData t2) { return t1.M_bits != t2.M_bits; }

/** @brief A chess piece type.
 *
 * This class represents a chess piece type.
 *
 * The class Code is a friend of this class.
 */
class Type : protected TypeData {
  public:
  /** @name Constructors */
  //@{

    //! Construct an uninitialized Type.
    Type(void) { }

    //! Copy-constructor.
    Type(Type const& type) { M_bits = type.M_bits; }

    //! Construct a Type from a constant.
    Type(TypeData type) { M_bits = type.M_bits; }

  //@}

  /** @name Assignment operators */
  //@{

    //! Assign from another Type.
    Type& operator=(Type const& type) { M_bits = type.M_bits; return *this; }

    //! Assign from a constant.
    Type& operator=(TypeData type) { M_bits = type.M_bits; return *this; }

  //@}

  /** @name Comparison operators */
  //@{

    friend bool operator==(Type const& t1, Type const& t2) { return t1.M_bits == t2.M_bits; }
    friend bool operator==(Type const& t1, TypeData t2) { return t1.M_bits == t2.M_bits; }
    friend bool operator==(TypeData t1, Type const& t2) { return t1.M_bits == t2.M_bits; }
    friend bool operator!=(Type const& t1, Type const& t2) { return t1.M_bits != t2.M_bits; }
    friend bool operator!=(Type const& t1, TypeData t2) { return t1.M_bits != t2.M_bits; }
    friend bool operator!=(TypeData t1, Type const& t2) { return t1.M_bits != t2.M_bits; }

  //@}

  /** @name Accessors */
  //@{

    //! Returns TRUE if the type is a bishop, rook or queen.
    bool is_a_slider(void) const { return M_bits > 4; }

    //! Returns TRUE if the type is a rook or queen.
    bool is_a_rookmover(void) const { return (M_bits & rook_bits) == rook_bits; }

    //! Returns TRUE if the type is a bishop or queen.
    bool is_a_bishopmover(void) const { return (M_bits & bishop_bits) == bishop_bits; }
    
    //! Return the underlaying integral value.
    uint8_t operator()(void) const { return M_bits; }

  //@}

  private:
    friend class Code;
    // Constructor for class Code.
    explicit Type(uint8_t bits) { M_bits = bits; }
};

} // namespace cwchess

#endif	// TYPE_H
