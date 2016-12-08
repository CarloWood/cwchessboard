// cwchessboard -- A C++ chessboard tool set
//
//! @file PieceIterator.h This file contains the definition of class PieceIterator.
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

#ifndef PIECEITERATOR_H
#define PIECEITERATOR_H

#ifndef USE_PCH
#include <iterator>
#endif

#include "Piece.h"

namespace cwchess {

class ChessPosition;

/** @brief Non-mutable iterator over selective chess pieces in a chess position.
 *
 * This iterator iterates over bits in a given BitBoard, passed during creation,
 * and returns the Piece at the given position when dereferenced.
 */
class PieceIterator : public std::iterator<std::bidirectional_iterator_tag, Piece> {
  protected:
    ChessPosition const* M_chess_position;	//!< The underlaying chess position.
    BitBoard M_pieces;				//!< The pieces that the iterator will iterate over.
    Index M_current_index;			//!< The index to the current piece, or 64 if this iterator points one passed the end.
  
  public:
  /** @name Constructors */
  //@{

    // Default Constructible.
    /** @brief Construct the corresponding one-passed-the-end iterator.
     *
     * @sa ChessPosition::piece_end(void)
     */
    PieceIterator(void) : M_chess_position(NULL), M_current_index(index_end) { }

    /** @brief Construct the corresponding one-before-the-beginning iterator.
     */
    PieceIterator(int) : M_chess_position(NULL), M_current_index(index_pre_begin) { }

    // Assignable.
    //! @brief Copy-constructor.
    PieceIterator(PieceIterator const& iter) :
        M_chess_position(iter.M_chess_position), M_pieces(iter.M_pieces), M_current_index(iter.M_current_index) { }

    /** @brief Construct a fully initialized PieceIterator.
     *
     * @param chess_position : The ChessPosition that we will retrieve the Pieces from. It is only used when the iterator is dereferenced.
     * @param pieces : A BitBoard with bits set for each square that the iterator should visit.
     *
     * This iterator is initialized to point at the beginning (the least significant bit).
     * A typical loop would look as follows:
     *
     * \code
     * PieceIterator const piece_end;
     * for (PieceIterator piece_iter(chess_position, bitboard); piece_iter != piece_end; ++piece_iter)
     * {
     *   // Use piece_iter.index(), the square that the piece is standing on, or
     *   // access the Piece directly through piece_iter->.
     * }
     * \endcode
     *
     * which will run over all bits set in bitboard.
     *
     * @sa ChessPosition::piece_begin, ChessPosition::all
     */
    PieceIterator(ChessPosition const* chess_position, BitBoard pieces) :
        M_chess_position(chess_position), M_pieces(pieces), M_current_index(get_first_bitindex()) { }

    /** @brief Construct a fully initialized PieceIterator.
     *
     * @param chess_position : The ChessPosition that we will retrieve the Pieces from.
     * @param pieces : The pieces, a BitBoard with bits set for each square that the iterator should visit.
     *
     * This iterator is initialized to point at the end rather than the beginning.
     * A typical loop would look as follows:
     *
     * \code
     * PieceIterator const piece_end(0);
     * for (PieceIterator piece_iter(chess_position, bitboard, 0); piece_iter != piece_end; --piece_iter)
     * {
     *   // Use piece_iter.index(), the square that the piece is standing on, or
     *   // access the Piece directly through piece_iter->.
     * }
     * \endcode
     */
    PieceIterator(ChessPosition const* chess_position, BitBoard pieces, int) :
        M_chess_position(chess_position), M_pieces(pieces), M_current_index(get_last_bitindex()) { }

  //@}

  /** @name Assignment operator */
  //@{

    // Assignable.
    //! @brief Assign from another PieceIterator.
    PieceIterator& operator=(PieceIterator const& iter)
        { M_chess_position = iter.M_chess_position; M_pieces = iter.M_pieces; M_current_index = iter.M_current_index; return *this; }

  //@}

  /** @name Comparison operators */
  //@{

    // Equality Comparable.
    //! Return TRUE if the current index of this PieceIterator and \a iter are equal.
    bool operator==(PieceIterator const& iter) const { return M_current_index == iter.M_current_index; }

    //! Return TRUE if the current index of this PieceIterator and \a iter differ.
    bool operator!=(PieceIterator const& iter) const { return M_current_index != iter.M_current_index; }

  //@}

  /** @name Accessors */
  //@{

    // Dereferencable.
    //! Return the Piece that stands on the current index.
    Piece operator*() const;

    //! Return a pointer to the Piece standing on the current index.
    Piece const* operator->(void) const;

    //! Return the current index.
    Index const& index(void) const { return M_current_index; }

  //@}

  /** @name Increment and decrement operators */
  //@{

    // Bi-directional iterator.
    PieceIterator& operator++() { M_current_index.next_bit_in(M_pieces()); return *this; }
    PieceIterator operator++(int) { PieceIterator result(*this); operator++(); return result; }
    PieceIterator& operator--() { M_current_index.prev_bit_in(M_pieces()); return *this; }
    PieceIterator operator--(int) { PieceIterator result(*this); operator--(); return result; }

  //@}

  private:
    Index get_first_bitindex(void) const { Index result(index_pre_begin); result.next_bit_in(M_pieces()); return result; }
    Index get_last_bitindex(void) const { Index result(index_end); result.prev_bit_in(M_pieces()); return result; }
};

} // namespace cwchess

#endif	// PIECEITERATOR_H
