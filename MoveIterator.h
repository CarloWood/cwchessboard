// cwchessboard -- A C++ chessboard tool set
//
//! @file MoveIterator.h This file contains the declaration of class MoveIterator.
//
// Copyright (C) 2008 - 2010, by
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

#ifndef MOVEITERATOR_H
#define MOVEITERATOR_H

#ifndef USE_PCH
#include <iterator>
#endif

#include "Move.h"
#include "BitBoard.h"
#include "Piece.h"
#ifdef GTKMM
#include <glib.h>
#endif

#ifndef G_LIKELY
#define G_LIKELY(condition) __builtin_expect(condition, true)
#endif

namespace cwchess {

class ChessPosition;

/** @brief Non-mutable iterator over all moves of a given chess piece.
 *
 * This iterator generates the Move objects while advancing to the next
 * move, storing the result in a member variable. The lifetime of the reference
 * or pointer to the Move is therefore equal to the lifetime of the iterator:
 * calling the increment or decrement operators overwrites the previous move.
 *
 * Usage example:
 *
 * \code
 * for (MoveIterator move_iter = chess_position.move_begin(index); move_iter != chess_position.move_end(); ++move_iter)
 * {
 *   Move const& move(*move_iter);
 *   // Use 'move'.
 * }
 * \endcode
 */
class MoveIterator : public std::iterator<std::bidirectional_iterator_tag, Move> {
  protected:
    ChessPosition const* M_chess_position;	//!< The underlaying chess position.
    BitBoard M_targets;				//!< The targets that this piece can move to.
    Move M_current_move;			//!< The actual move that is returned when dereferenced.

  public:
  /** @name Constructors */
  //@{

    // Default Constructible.
    /** @brief Construct a one-past-the-end MoveIterator.
     *
     * @sa ChessPosition::move_end()
     */
    MoveIterator(void) : M_current_move(index_end, index_end, nothing) { }

    // Assignable.
    //! @brief Copy-Constructor.
    MoveIterator(MoveIterator const& iter) :
        M_chess_position(iter.M_chess_position), M_targets(iter.M_targets), M_current_move(iter.M_current_move) { }

    /** @brief Construct a fully initialized MoveIterator.
     *
     * @param chess_position : The ChessPosition that we're generating moves for.
     * @param index : The index of the piece that we're generating moves for.
     *
     * @sa ChessPosition::move_begin(Index const&)
     */
    MoveIterator(ChessPosition const* chess_position, Index const& index);

  //@}

  /** @name Assignment operator */
  //@{

    //! @brief Assignment operator.
    MoveIterator& operator=(MoveIterator const& iter)
        { M_chess_position = iter.M_chess_position; M_targets = iter.M_targets; M_current_move = iter.M_current_move; return *this; }

  //@}

  /** @name Comparision operators */
  //@{

    // Equality Comparable.
    bool operator==(MoveIterator const& iter) const { return M_current_move == iter.M_current_move; }
    bool operator!=(MoveIterator const& iter) const { return M_current_move != iter.M_current_move; }

  //@}

  /** @name Accessors */
  //@{

    // Dereferencable.
    Move const& operator*() const { return M_current_move; }
    Move const* operator->(void) const { return &M_current_move; }

  //@}

  /** @name Increment and decrement operators */
  //@{

    // Bi-directional iterator.
    MoveIterator& operator++()
	{
	  if (G_LIKELY(!M_current_move.is_promotion()) || next_promotion())
	  {
	    Index current_index(M_current_move.to());
	    current_index.next_bit_in(M_targets());
	    M_current_move.set_to(current_index);
	  }
	  return *this;
	}

    MoveIterator operator++(int) { MoveIterator result(*this); operator++(); return result; }

    MoveIterator& operator--()
	{
	  if (G_LIKELY(!M_current_move.is_promotion()) || prev_promotion())
	  {
	    Index current_index(M_current_move.to());
	    current_index.prev_bit_in(M_targets());
	    M_current_move.set_to(current_index);
	  }
	  return *this;
	}

    MoveIterator operator--(int) { MoveIterator result(*this); operator--(); return result; }

  //@}

  public_notdocumented:
    // Indexing.
    uint32_t index(void) const { return M_current_move.to()(); } 

  private:
    bool next_promotion(void);
    bool prev_promotion(void);

    Index get_first_bitindex(void) const { Index result(index_pre_begin); result.next_bit_in(M_targets()); return result; }

    Type initial_type(Index const& index) const;
};

} // namespace cwchess

#endif	// MOVEITERATOR_H
