// cwchessboard -- A C++ chessboard tool set
//
//! @file EnPassant.h This file contains the declaration of class EnPassant.
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

#ifndef ENPASSANT_H
#define ENPASSANT_H

#ifndef USE_PCH
#endif

#define DEBUG_ENPASSANT_EXISTS 0

#if DEBUG_ENPASSANT_EXISTS
#include <cassert>
#endif

namespace cwchess {

/** @brief An object representing en passant information.
 *
 * This is an internal class, you should not normally need.
 *
 * If EnPassant::exists() returns true, then a pawn exists that can be taken en passant,
 * or could be taken en passant if an enemy pawn was standing next to it.
 * This means that in 'position setup mode', placing a pawn next to the en passant
 * pawn (on index EnPassant::pawn_index()) will be able to take the en passant pawn.
 *
 * @sa ChessPosition::en_passant
 */
class EnPassant {
  private:
    //! @brief En passant information.
    //
    // PEIIIIII, where 'IIIIII' is the index of the square that a pawn just passed by moving two squares forward,
    // and bit 'P' is a flag indicating that a pawn is not allowed to take en passant because it is pinned.
    // If 'E' is set then all I must be 0 and the index is 'index_end'.
    //
    // If there exists no en passant pawn then 'P' must be 0.
    uint8_t M_bits;

  public:
#ifndef DOXYGEN
    //! @brief Construct an uninitialized EnPassant object.
    EnPassant() { }

    //! @brief Construct EnPassant object with index \a index.
    EnPassant(Index const& index) : M_bits(index()) { }

    //! @brief Assignment operator.
//    EnPassant& operator=(EnPassant const& en_passant) { M_bits = en_passant.M_bits; return *this; }

    //! @brief Clear the EnPassant object.
    void clear() { M_bits = index_end.M_bits; }

    //! @brief Return the FEN field with en passant information.
    std::string FEN4() const;
#endif

    //! @brief Return TRUE if the last move was a pawn advancing two squares.
    bool exists() const { return M_bits != index_end.M_bits; }

  /** @name Accessors */
  //@{

    //! @brief Return the index of the square that was passed.
    Index index() const
    {
#if DEBUG_ENPASSANT_EXISTS
      assert(exists());
#endif
      IndexData result = { static_cast<uint8_t>(M_bits & 0x7f) };
      return result;
    }

    //! @brief Return the index of the pawn that just advanced two squares.
    Index pawn_index() const
    {
#if DEBUG_ENPASSANT_EXISTS
      assert(exists());
#endif
      IndexData result = { static_cast<uint8_t>((M_bits & 0x7f) ^ 8) };
      return result;
    }

    //! @brief Return the index of the square that pawn came from.
    Index from_index() const
    {
#if DEBUG_ENPASSANT_EXISTS
      assert(exists());
#endif
      IndexData result = { static_cast<uint8_t>((M_bits & 0x7f) ^ 24) };
      return result;
    }

    //! @brief Return TRUE if taking en passant is not allowed due to <em>horizontal</em> pinning.
    bool pinned() const
    {
#if DEBUG_ENPASSANT_EXISTS
      assert(exists());
#endif
      return (M_bits & 0x80);
    }

  //@}

#ifndef DOXYGEN
    //! @brief Taking en passant is not allowed because it is pinned.
    void pinned_set()
    {
#if DEBUG_ENPASSANT_EXISTS
      assert(exists());
#endif
      M_bits |= 0x80;
    }

    //! @brief Taking en passant is allowed again, because it is unpinned.
    void pinned_reset() { M_bits &= 0x7f; }
#endif

};

} // namespace cwchess

#endif	// ENPASSANT_H
