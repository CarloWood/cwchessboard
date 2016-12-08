// cwchessboard -- A C++ chessboard tool set
//
//! @file Piece.h This file contains the declaration of class Piece.
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

#ifndef PIECE_H
#define PIECE_H

#ifndef USE_PCH
#endif

#include "Code.h"
#include "Flags.h"

namespace cwchess {

/** @brief A particular piece on the board.
 *
 * This class represents pieces on the chessboard
 * (in the broadest sense of 'piece', including pawn, queen, king and even nothing).
 *
 * Since the flags are hidden and not of the users concern, you can
 * consider a Piece to be equivalent with Code.
 *
 * @sa ChessPosition::piece_at
 */
class Piece {
  protected:
    Code M_code;	//!< The Code of this piece.
    Flags M_flags;	//!< Flags for this piece.

  public:

  /** @name Constructors */
  //@{

    //! Construct an empty piece (nothing).
    Piece(void) : M_code(), M_flags(fl_none) { }

    /** @brief Construct a fully initialized Piece.
     *
     * @param color : The color of the piece.
     * @param type  : The type of the piece.
     * @param flags : All flags.
     */
    Piece(Color const& color, Type const& type, Flags const& flags) : M_code(color, type), M_flags(flags) { }

    /** @brief Construct a fully initialized Piece with all Flags reset.
     *
     * @param color : The color of the piece.
     * @param type  : The type of the piece.
     */
    Piece(Color const& color, Type const& type) : M_code(color, type), M_flags(fl_none) { }

    /** @brief Construct a fully initialized Piece.
     *
     * @param code  : The chess piece Code.
     * @param flags : All flags.
     */
    Piece(Code const& code, Flags const& flags) : M_code(code), M_flags(flags) { }
   
    //! Copy-constructor.
    Piece(Piece const& piece) : M_code(piece.M_code), M_flags(piece.M_flags) { }

  //@}

  /** @name Assignment operators */
  //@{

    //! Assign from another Piece.
    Piece& operator=(Piece const& piece) { M_code = piece.M_code; M_flags = piece.M_flags; return *this; }

    //! Just change the color.
    Piece& operator=(Color const& color) { M_code = color; return *this; }

    //! Just change the type.
    Piece& operator=(Type const& type) { M_code = type; return *this; }

    //! Just change the code.
    Piece& operator=(Code const& code) { M_code = code; return *this; }

    //! Just change the flags.
    Piece& operator=(Flags const& flags) { M_flags = flags; return *this; }

  //@}

  /** @name Accessors */
  //@{

    /** @brief Return the color of the piece.
     *
     * Only valid if the type is not 'nothing'.
     */
    Color color(void) const { return M_code.color(); }

    //! The type of this piece.
    Type type(void) const { return M_code.type(); }

    //! The flags of this piece.
    Flags flags(void) const { return M_flags; }

    //! The code of this piece.
    Code code(void) const { return M_code; }

  //@}

  private:
    friend class ChessPosition;
#ifdef PIECE_TEST_H
    friend class testsuite::PieceTest;
#endif

    //! Change the flags of this piece.
    void set_flags(Flags const& flags) { M_flags = flags; }

    //! Change the type of this piece.
    void set_type(Type const& type) { if (type == nothing) { M_code.clear(); M_flags.clear(); } else M_code = type; }

    //! Reset the 'can take queen side' flag, if any.
    void reset_can_take_queen_side(void) { M_flags.reset(fl_pawn_can_take_queen_side); }

    //! Reset the 'can take king side' flag, if any.
    void reset_can_take_king_side(void) { M_flags.reset(fl_pawn_can_take_king_side); }

    //! Reset the 'is not blocked' flag, if any.
    void reset_is_not_blocked(void) { M_flags.reset(fl_pawn_is_not_blocked|fl_pawn_can_move_two_squares); }

    //! Reset the 'can move two squares' flag, if any.
    void reset_can_move_two_squares(void) { M_flags.reset(fl_pawn_can_move_two_squares); }

    //! Set the 'can take queen side' flag.
    void set_can_take_queen_side(void) { M_flags.set(fl_pawn_can_take_queen_side); }

    //! Set the 'can take king side' flag.
    void set_can_take_king_side(void) { M_flags.set(fl_pawn_can_take_king_side); }

    //! Set the 'is not blocked' flag.
    void set_is_not_blocked(void) { M_flags.set(fl_pawn_is_not_blocked); }

    //! Set the 'can move two squares' flag.
    void set_can_move_two_squares(void) { M_flags.set(fl_pawn_can_move_two_squares); }

    //! Copy the 'is not blocked' flag to the 'can move two squares' flag, if set.
    void set_can_move_two_squares_if_not_blocked(void) { M_flags.set_can_move_two_squares_if_not_blocked(); }

  public:
  /** @name Comparison operators */
  //@{

    /** @brief Return TRUE if the Code of the pieces are equal.
     *
     * This operator ignores the flags.
     */
    friend bool operator==(Piece const& piece1, Piece const& piece2) { return piece1.M_code == piece2.M_code; }

    /** @brief Return TRUE if the Code of the pieces are unequal.
     *
     * This operator ignores the flags.
     */
    friend bool operator!=(Piece const& piece1, Piece const& piece2) { return piece1.M_code != piece2.M_code; }

    //! Return TRUE if \a piece is a \a type.
    friend bool operator==(Piece const& piece, Type const& type) { return piece.M_code.is_a(type); }

    //! Return TRUE if \a piece is a \a type.
    friend bool operator==(Type const& type, Piece const& piece) { return piece.M_code.is_a(type); }

    //! Return TRUE if \a piece is not a \a type.
    friend bool operator!=(Piece const& piece, Type const& type) { return !piece.M_code.is_a(type); }

    //! Return TRUE if \a piece is not a \a type.
    friend bool operator!=(Type const& type, Piece const& piece) { return !piece.M_code.is_a(type); }

    //! Return TRUE if \a piece has color \a color.
    friend bool operator==(Piece const& piece, Color const& color) { return piece.M_code.is(color); }

    //! Return TRUE if \a piece has color \a color.
    friend bool operator==(Color const& color, Piece const& piece) { return piece.M_code.is(color); }

    //! Return TRUE if \a piece does not have color \a color.
    friend bool operator!=(Piece const& piece, Color const& color) { return !piece.M_code.is(color); }

    //! Return TRUE if \a piece does not have color \a color.
    friend bool operator!=(Color const& color, Piece const& piece) { return !piece.M_code.is(color); }

    //! Return TRUE if \a piece has code \a code.
    friend bool operator==(Piece const& piece, Code const& code) { return piece.M_code == code; }

    //! Return TRUE if \a piece has code \a code.
    friend bool operator==(Code const& code, Piece const& piece) { return piece.M_code == code; }

    //! Return TRUE if \a piece does not have code \a code.
    friend bool operator!=(Piece const& piece, Code const& code) { return piece.M_code != code; }

    //! Return TRUE if \a piece does not have code \a code.
    friend bool operator!=(Code const& code, Piece const& piece) { return piece.M_code != code; }

  //@}
};

} // namespace cwchess

#endif	// PIECE_H
