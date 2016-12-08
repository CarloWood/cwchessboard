// cwchessboard -- A C++ chessboard tool set
//
//! @file Move.h This file contains the definition of class Move.
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

#ifndef MOVE_H
#define MOVE_H

#ifndef USE_PCH
#endif

#include "Index.h"
#include "Type.h"

namespace cwchess {

/** @brief A chess move in a particular chess position.
 *
 * This class represents a move as can be done in a particular chess position.
 * That means that it cannot be used to do the same move in another position
 * where that move is possible too. The reason for that is that it's internal
 * representation only stores the squares, and not piece information.
 *
 * A Move is therefore always used in combination with a ChessPosition.
 */
class Move {
  private:
    Index M_from;		//!< Where the piece comes from.
    Index M_to;			//!< Where the piece moves to.
    Type M_promotion_type;	//!< Set to 'nothing' if not a promotion.

  public:
  /** @name Constructors */
  //@{

    //! Construct an uninitialized Move.
    Move(void) { }

    /** @brief Construct a Move from square \a from to square \a to.
     *
     * If this move represents a pawn promotion then \a promotion
     * must be one of queen, rook, knight, bishop. Otherwise it
     * must be empty.
     */
    Move(Index from, Index to, Type promotion) : M_from(from), M_to(to), M_promotion_type(promotion) { }

    //! Copy-constructor.
    Move(Move const& move) : M_from(move.M_from), M_to(move.M_to), M_promotion_type(move.M_promotion_type) { }

  //@}

  /** @name Assignment operator */
  //@{

    Move& operator=(Move const& move) { M_from = move.M_from; M_to = move.M_to; M_promotion_type = move.M_promotion_type; return *this; }

  //@}

  /** @name Comparision operators */
  //@{

    bool operator==(Move const& move) const
    {
      if (move.M_to == index_end) return M_to == index_end;
      return M_from == move.M_from && M_to == move.M_to && M_promotion_type == move.M_promotion_type;
    }

    bool operator!=(Move const& move) const
    {
      if (move.M_to == index_end) return M_to != index_end;
      return M_to != move.M_to || M_from != move.M_from || M_promotion_type != move.M_promotion_type;
    }

  //@}

  /** @name Accessors */
  //@{

    //! Return TRUE if this move is a pawn promotion.
    bool is_promotion(void) const { return M_promotion_type != nothing; }

    //! Return the square the piece moves from.
    Index from(void) const { return M_from; }

    //! Return the square the piece moves to.
    Index to(void) const { return M_to; }

    //! Return the promotion type. Returns empty if this isn't a promotion.
    Type promotion_type(void) const { return M_promotion_type; }

  //@}

  /** @name Manipulators */
  //@{

    //! Set a different promotion type.
    void set_promotion(Type promotion) { M_promotion_type = promotion; }

    //! Set different target square.
    void set_to(Index to) { M_to = to; }

    //! Set from, to and promotion type.
    void set_move(Index from, Index to, Type promotion) { M_from = from; M_to = to; M_promotion_type = promotion; }

  //@}
};

} // namespace cwchess

#endif	// MOVE_H
