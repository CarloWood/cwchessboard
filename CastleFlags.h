// cwchessboard -- A C++ chessboard tool set
//
//! @file CastleFlags.h This file contains the declaration of class CastleFlags.
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

#ifndef CASTLEFLAGS_H
#define CASTLEFLAGS_H

#ifndef USE_PCH
#endif

#include "Code.h"
#include "Index.h"
#include "Piece.h"

namespace cwchess {

uint8_t const black_rook_queen_side_moved = 1;
uint8_t const black_rook_king_side_moved = 2;
uint8_t const black_king_moved = 4;
uint8_t const black_king_in_check = 8;
uint8_t const white_king_in_check = 16;
uint8_t const white_rook_queen_side_moved = 32;
uint8_t const white_rook_king_side_moved = 64;
uint8_t const white_king_moved = 128;

class ChessPosition;

/** @brief A class to keep track of castling rights.
 *
 * This class is for internal use. ChessPosition takes cares
 * of keeping track of castling rights and the user should
 * never have to access or use this class.
 *
 * Each ChessPosition has one CastleFlags member that keeps
 * track of which rook and king moved. Castling is only
 * allowed when neither, the king nor the rook used to castle,
 * did move.
 *
 * This object has no notion of temporary reasons why castling
 * might not be allowed. However, two unused bits are used
 * to store whether or not white/black is in check (that is not
 * used for the castling part though).
 */
class CastleFlags {
  private:
    uint8_t M_bits;

    friend class ChessPosition;

    CastleFlags(void) : M_bits(0) { }
    CastleFlags& operator=(uint8_t bits) { M_bits = bits; return *this; }

    // Called when all pieces are removed from the board.
    void clear(void) { M_bits = 231; }

    // Called if \a code was removed from \a index.
    void update_removed(Code const& code, Index const& index)
    {
      if (code == white_rook)
      {
	if (index == ia1)
	  M_bits |= white_rook_queen_side_moved;
	else if (index == ih1)
	  M_bits |= white_rook_king_side_moved;
      }
      else if (code == black_rook)
      {
	if (index == ia8)
	  M_bits |= black_rook_queen_side_moved;
	else if (index == ih8)
	  M_bits |= black_rook_king_side_moved;
      }
      else if(code == white_king)
      {
	if (index == ie1)
	  M_bits |= white_king_moved;
      }
      else if (code == black_king)
      {
	if (index == ie8)
	  M_bits |= black_king_moved;
      }
    }

    // Called if \a code was placed at \a index.
    void update_placed(Code const& code, Index const& index)
    {
      if (code == white_rook)
      {
	if (index == ia1)
	  M_bits &= ~white_rook_queen_side_moved;
	else if (index == ih1)
	  M_bits &= ~white_rook_king_side_moved;
      }
      else if (code == black_rook)
      {
	if (index == ia8)
	  M_bits &= ~black_rook_queen_side_moved;
	else if (index == ih8)
	  M_bits &= ~black_rook_king_side_moved;
      }
      else if (code == white_king)
      {
	if (index == ie1)
	  M_bits &= ~white_king_moved;
      }
      else if (code == black_king)
      {
	if (index == ie8)
	  M_bits &= ~black_king_moved;
      }
    }

    // Called if the king or rook \a piece (initial position \a from) moved.
    void piece_moved_from(Piece const& piece, Index const& from);

  public:
    //! Return TRUE if \a color is still allowed to castle at all (not taking into account checks).
    bool can_castle(Color const& color) const { return ((M_bits >> ((color == black) ? 0 : 5)) & 7) < 3; }

    //! Return TRUE if \a color is still allowed to castle short (not taking into account checks).
    bool can_castle_short(Color const& color) const
    {
      // Speed up for the case that both, the black and the white king already moved.
      if (__builtin_expect((M_bits & (black_king_moved | white_king_moved)) == (black_king_moved | white_king_moved), 1))
	return false;
      // Calculate mask: the king and the appropriate rook.
      uint8_t mask = color.is_white() ? (white_king_moved | white_rook_king_side_moved) : (black_king_moved | black_rook_king_side_moved);
      // Neither may have moved.
      return !(M_bits & mask);
    }

    //! Return TRUE if \a color is still allowed to castle long (not taking into account checks).
    bool can_castle_long(Color const& color) const
    {
      // Speed up for the case that both, the black and the white king already moved.
      if (__builtin_expect((M_bits & (black_king_moved | white_king_moved)) == (black_king_moved | white_king_moved), 1))
	return false;
      // Calculate mask: the king and the appropriate rook.
      uint8_t mask = color.is_white() ? (white_king_moved | white_rook_queen_side_moved) : (black_king_moved | black_rook_queen_side_moved);
      // Neither may have moved.
      return !(M_bits & mask);
    }

    // Set the 'in_check' bit.
    void set_check(Color const& color, bool check) { uint8_t flag = 8 + color(); if (check) M_bits |= flag; else M_bits &= ~flag; }

    // Retrieve the 'in_check' bit.
    bool in_check(Color const& color) const { uint8_t flag = 8 + color(); return (M_bits & flag); }

    //! Return TRUE if \a code at \a index is marked as having moved.
    bool has_moved(Code const& code, Index const& index);
};

} // namespace cwchess

#endif	// CASTLEFLAGS_H
