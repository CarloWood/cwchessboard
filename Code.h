// cwchessboard -- A C++ chessboard tool set
//
//! @file Code.h This file contains the declaration of class Code.
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

#ifndef CODE_H
#define CODE_H

#ifndef USE_PCH
#include <stdint.h>
#endif

#include "Color.h"
#include "Type.h"
#include "Direction.h"

namespace cwchess {

/** @brief The POD base type of class Code.
 *
 * This class represents a chess piece, either
 * pawn, rook, knight, bishop, queen or king,
 * and it's color, white or black.
 *
 * See ColorData and TypeData for the encoding of the bits.
 *
 * @sa Code, white_pawn, white_rook, white_knight, white_bishop, white_queen, white_king, black_pawn, black_rook, black_knight, black_bishop, black_queen, black_king
 *
 * In order to refer to 'nothing', use the default constructor Code().
 */
struct CodeData {
  uint8_t M_bits;	//!< 0000CTTT, where C is the color and TTT the type.
};

//! A constant representing a white pawn.
CodeData const white_pawn = { white_bits | pawn_bits };
//! A constant representing a white rook.
CodeData const white_rook = { white_bits | rook_bits };
//! A constant representing a white knight.
CodeData const white_knight = { white_bits | knight_bits };
//! A constant representing a white bishop.
CodeData const white_bishop = { white_bits | bishop_bits };
//! A constant representing a white queen.
CodeData const white_queen = { white_bits | queen_bits };
//! A constant representing a white king.
CodeData const white_king = { white_bits | king_bits };
//! A constant representing a black pawn.
CodeData const black_pawn = { black_bits | pawn_bits };
//! A constant representing a black rook;
CodeData const black_rook = { black_bits | rook_bits };
//! A constant representing a black knight;
CodeData const black_knight = { black_bits | knight_bits };
//! A constant representing a black bishop;
CodeData const black_bishop = { black_bits | bishop_bits };
//! A constant representing a black queen;
CodeData const black_queen = { black_bits | queen_bits };
//! A constant representing a black king;
CodeData const black_king = { black_bits | king_bits };

// Compare constants (this should never be needed, but why not add it).
inline bool operator==(CodeData c1, CodeData c2) { return c1.M_bits == c2.M_bits; }
inline bool operator!=(CodeData c1, CodeData c2) { return c1.M_bits != c2.M_bits; }

// Used to cast Code to CwChessboardCode.
typedef uint16_t CwChessboardCode;

/** @brief A chess piece type including color.
 *
 * This class represents a code for a chess piece
 * that includes it's Type as well as it's Color.
 *
 * See CodeData for defined constants.
 */
class Code : protected CodeData {
  public:

  /** @name Constructors */
  //@{

    //! Construct a Code object initialized as 'nothing'.
    Code(void) { M_bits = 0; }

    //! Copy-constructor.
    Code(Code const& code) { M_bits = code.M_bits; }

    //! Construct a Code object from a constant.
    Code(CodeData code) { M_bits = code.M_bits; }

    //! Construct a Code object with color \a color and type \a type.
    Code(Color const& color, Type const& type) { M_bits = color.M_bits | type.M_bits; }

    //! Explicit conversion from CwChessboardCode to Code.
    explicit Code(CwChessboardCode code) { M_bits = CwChessboardCode_to_Code[code].M_bits; }

  //@}

  /** @name Assigment operators */
  //@{

    //! Assign from another Code.
    Code& operator=(Code const& code) { M_bits = code.M_bits; return *this; }

    //! Assign from a constant.
    Code& operator=(CodeData code) { M_bits = code.M_bits; return *this; }

    //! Change the type to \a type. Type may not be nothing (use clear() instead).
    Code& operator=(Type const& type) { M_bits &= ~type_mask; M_bits |= type.M_bits; return *this; }

    //! Change the color to \a color.
    Code& operator=(Color const& color) { M_bits &= ~color_mask; M_bits |= color.M_bits; return *this; }

    //! Set the type to nothing.
    void clear(void) { M_bits = 0; }

  //@}

  /** @name Comparison operators */
  //@{

    friend bool operator==(Code const& c1, Code const& c2) { return c1.M_bits == c2.M_bits; }
    friend bool operator==(Code const& c1, CodeData c2) { return c1.M_bits == c2.M_bits; }
    friend bool operator==(CodeData c1, Code const& c2) { return c1.M_bits == c2.M_bits; }
    friend bool operator!=(Code const& c1, Code const& c2) { return c1.M_bits != c2.M_bits; }
    friend bool operator!=(Code const& c1, CodeData c2) { return c1.M_bits != c2.M_bits; }
    friend bool operator!=(CodeData c1, Code const& c2) { return c1.M_bits != c2.M_bits; }

  //@}

  /** @name Accessors */
  //@{

    //! Returns TRUE if the type is a bishop, rook or queen.
    bool is_a_slider(void) const { return (M_bits & type_mask) > 4; }

    //! Returns TRUE if the type is a rook or queen.
    bool is_a_rookmover(void) const { return (M_bits & rook_bits) == rook_bits; }

    //! Returns TRUE if the type is a bishop or queen.
    bool is_a_bishopmover(void) const { return (M_bits & bishop_bits) == bishop_bits; }

    //! Returns TRUE if the code represents 'nothing'.
    bool is_nothing(void) const { return M_bits == 0; }

    //! Returns TRUE if the type is equal.
    bool is_a(Type const& type) const { return (M_bits & type_mask) == type.M_bits; }

    //! Returns TRUE if the type is equal.
    bool is_a(TypeData type) const { return (M_bits & type_mask) == type.M_bits; }

    //! Return TRUE if the color is equal.
    bool is(Color const& color) const { return (M_bits & color_mask) == color.M_bits; }

    //! Return TRUE if the color is equal.
    bool is(ColorData color) const { return (M_bits & color_mask) == color.M_bits; }

    //! Return TRUE if the colors are different.
    bool has_opposite_color_of(Code const& code) { return (M_bits ^ code.M_bits) & color_mask; }

    //! Return TRUE if this piece moves along \a direction.
    bool moves_along(Direction const& direction) { return (M_bits & direction.mover_flags().M_bits) == direction.mover_flags().M_bits; }

    //! Return the Type of this Code.
    Type type(void) const { return Type(M_bits & type_mask); }

    //! Return the Color of this Code.
    Color color(void) const { return Color(M_bits & color_mask); }

    //! Return the unlaying integral value.
    uint8_t operator()(void) const { return M_bits; }

  //@}

  /** @name Special functions */
  //@{

    //! Toggle the color. May not be used on type 'nothing'.
    void toggle_color(void) { M_bits ^= color_mask; }

    //! Casting operator.
    operator CwChessboardCode(void) const { return Code_to_CwChessboardCode[M_bits]; }
  
  //@}

  private:
    static CwChessboardCode Code_to_CwChessboardCode[16];
    static CodeData CwChessboardCode_to_Code[14];
};

} // namespace cwchess

#endif	// CODE_H
