// cwchessboard -- A C++ chessboard tool set
//
//! @file Flags.h This file contains the declaration of class Flags.
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

#ifndef FLAGS_H
#define FLAGS_H

#ifndef USE_PCH
#include <stdint.h>
#endif

namespace cwchess {

/** @brief The POD base type of class Flags.
 *
 * @sa Flags, fl_pawn_can_take_queen_side, fl_pawn_is_not_blocked, fl_pawn_can_take_king_side, fl_pawn_can_move_two_squares
 */ 
struct FlagsData {
  uint8_t M_bits;	//!< 0TKNQ000, T=can move two squares, K=can take king side, N=is not blocked, Q=can take queen side.
};

FlagsData const fl_none = { 0 };

uint8_t const fl_pawn_mask = { 120 };

// Do not change these values, their exact value has a deeper meaning.
FlagsData const fl_pawn_can_take_queen_side = { 8 };	//!< A constant representing the flag 'pawn can take queen side'.
FlagsData const fl_pawn_is_not_blocked = { 16 };	//!< A constant representing the flag 'pawn is not blocked'.
FlagsData const fl_pawn_can_take_king_side = { 32 };	//!< A constant representing the flag 'pawn can take king side'.
FlagsData const fl_pawn_can_move_two_squares = { 64 };	//!< A constant representing the flag 'pawn can move two squares'.

// Compare constants (this should never be needed, but why not add it).
inline bool operator==(FlagsData f1, FlagsData f2) { return f1.M_bits == f2.M_bits; }
inline bool operator!=(FlagsData f1, FlagsData f2) { return f1.M_bits != f2.M_bits; }

/** @brief Calculate the union of two Flags constants. */
inline FlagsData operator|(FlagsData x, FlagsData y)
{
  FlagsData result;
  result.M_bits = x.M_bits | y.M_bits;
  return result;
}

/** @brief Calculate the intersection of two Flags constants. */
inline FlagsData operator&(FlagsData x, FlagsData y)
{
  FlagsData result;
  result.M_bits = x.M_bits & y.M_bits;
  return result;
}

/** @brief Calculate the union minus the intersection of two Flags constants. */
inline FlagsData operator^(FlagsData x, FlagsData y)
{
  FlagsData result;
  result.M_bits = x.M_bits ^ y.M_bits;
  return result;
}

/** @brief %Flags representing the state of a piece on the chessboard.
 *
 * This class is for internal use. You should not need it.
 *
 * @sa FlagsData
 */
class Flags : protected FlagsData {
  public:
  /** @name Constructors */
  //@{

    //! Construct an uninitialized Flags object.
    Flags(void) { }

    //! Copy-constructor.
    Flags(Flags const& flags) { M_bits = flags.M_bits; }

    //! Construct a Flags object from a constant.
    Flags(FlagsData flags) { M_bits = flags.M_bits; }

  //@}

  /** @name Assignment operators */
  //@{

    //! Assign from another Flags object.
    Flags& operator=(Flags const& flags) { M_bits = flags.M_bits; return *this; }

    //! Assign from a constant.
    Flags& operator=(FlagsData flags) { M_bits = flags.M_bits; return *this; }

    //! Set all flags to 0.
    void clear(void) { M_bits = 0; }

  //@}

  /** @name Comparision operators */
  //@{

    friend bool operator==(Flags const& f1, Flags const& f2) { return f1.M_bits == f2.M_bits; }
    friend bool operator==(Flags const& f1, FlagsData f2) { return f1.M_bits == f2.M_bits; }
    friend bool operator==(FlagsData f1, Flags const& f2) { return f1.M_bits == f2.M_bits; }
    friend bool operator!=(Flags const& f1, Flags const& f2) { return f1.M_bits != f2.M_bits; }
    friend bool operator!=(Flags const& f1, FlagsData f2) { return f1.M_bits != f2.M_bits; }
    friend bool operator!=(FlagsData f1, Flags const& f2) { return f1.M_bits != f2.M_bits; }

  //@}

  /** @name Bit fiddling */
  //@{

    //! Set all bits that are set in \a flags.
    Flags& operator|=(Flags const& flags) { M_bits |= flags.M_bits; return *this; }

    //! Set all bits that are set in \a flags.
    Flags& operator|=(FlagsData flags) { M_bits |= flags.M_bits; return *this; }

    //! Reset all bits that are not set in \a flags.
    Flags& operator&=(Flags const& flags) { M_bits &= flags.M_bits; return *this; }

    //! Reset all bits that are not set in \a flags.
    Flags& operator&=(FlagsData flags) { M_bits &= flags.M_bits; return *this; }

    //! Toggle all bits that are set in \a flags.
    Flags& operator^=(Flags const& flags) { M_bits ^= flags.M_bits; return *this; }

    //! Toggle all bits that are set in \a flags.
    Flags& operator^=(FlagsData flags) { M_bits ^= flags.M_bits; return *this; }

    //! Set the bits that are set in \a flags.
    void set(FlagsData flags) { M_bits |= flags.M_bits; }

    //! Clear the bits that are set in \a flags.
    void reset(FlagsData flags) { M_bits &= ~flags.M_bits; }

  //@}

  /** @name Special functions */
  //@{

    //! Set the fl_pawn_can_move_two_squares bit iff fl_pawn_is_not_blocked bit is already set.
    void set_can_move_two_squares_if_not_blocked(void) { M_bits |= (M_bits & 16) << 2; }

  //@}

  /** @name Accessors */
  //@{

    //! Return the underlaying integral value.
    uint8_t operator()(void) const { return M_bits; }

  //@}

};

} // namespace cwchess

#endif	// FLAGS_H
