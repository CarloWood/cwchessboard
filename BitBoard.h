// cwchessboard -- A C++ chessboard tool set
//
//! @file BitBoard.h This file contains the declaration of class BitBoard.
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

#ifndef BITBOARD_H
#define BITBOARD_H

#ifndef USE_PCH
#include <cstdint>
#endif

#include "Index.h"

#define DEBUG_BITBOARD_INITIALIZATION 0

#if DEBUG_BITBOARD_INITIALIZATION
#include <cassert>
#endif

/** @namespace cwchess
 *  @brief A namespace for all chess related objects that are not related to the GUI.
 */
namespace cwchess {

/** @brief The type of an internal BitBoard mask.
 *
 * Don't use this, use \link cwchess::BitBoard BitBoard \endlink.
 */
typedef uint64_t mask_t;

/** @brief Convert Index to a mask_t. */
inline mask_t index2mask(Index index)
{
  return CW_MASK_T_CONST(1) << index();
}

/** @brief Convert a \a col, \a row pair to a mask_t. */
inline mask_t colrow2mask(int col, int row)
{
  return CW_MASK_T_CONST(1) << (col + (row << 3));
}

/** @brief Convert a single bit mask into it's Index.
 *
 * The provided \a mask should have a single bit set.
 * However, if that is not the case, then this function
 * returns either 64 if no bit is set, or the index
 * of the first (least significant) bit that is set.
 */
inline Index mask2index(mask_t mask)
{
  Index result(index_pre_begin);
  result.next_bit_in(mask);
  return result;
}

/** @brief The POD base type of class BitBoard.
 *
 * This class contains a 64-bit unsigned integer
 * where each bit represents one square on the chessboard.
 *
 * It may not have constructors or destructors, because
 * it is needed that this struct is POD in order for
 * optimisation to work.
 *
 * @sa BitBoard
 */
struct BitBoardData {
    /** @brief A one-bit-per-square chessboard mask.
     *
     * The least significant bit represents square a1.
     * The next bit b1, c1, ... h1, a2, b2, ... etcetera till h8.
     */
    mask_t M_bitmask;
};

BitBoardData const a1 = { CW_MASK_T_CONST(0x1) };			//!< The square a1.
BitBoardData const b1 = { CW_MASK_T_CONST(0x2) };			//!< The square b1.
BitBoardData const c1 = { CW_MASK_T_CONST(0x4) };			//!< The square c1.
BitBoardData const d1 = { CW_MASK_T_CONST(0x8) };			//!< The square d1.
BitBoardData const e1 = { CW_MASK_T_CONST(0x10) };			//!< The square e1.
BitBoardData const f1 = { CW_MASK_T_CONST(0x20) };			//!< The square f1.
BitBoardData const g1 = { CW_MASK_T_CONST(0x40) };			//!< The square g1.
BitBoardData const h1 = { CW_MASK_T_CONST(0x80) };			//!< The square h1.
BitBoardData const a2 = { CW_MASK_T_CONST(0x100) };			//!< The square a2.
BitBoardData const b2 = { CW_MASK_T_CONST(0x200) };			//!< The square b2.
BitBoardData const c2 = { CW_MASK_T_CONST(0x400) };			//!< The square c2.
BitBoardData const d2 = { CW_MASK_T_CONST(0x800) };			//!< The square d2.
BitBoardData const e2 = { CW_MASK_T_CONST(0x1000) };			//!< The square e2.
BitBoardData const f2 = { CW_MASK_T_CONST(0x2000) };			//!< The square f2.
BitBoardData const g2 = { CW_MASK_T_CONST(0x4000) };			//!< The square g2.
BitBoardData const h2 = { CW_MASK_T_CONST(0x8000) };			//!< The square h2.
BitBoardData const a3 = { CW_MASK_T_CONST(0x10000) };			//!< The square a3.
BitBoardData const b3 = { CW_MASK_T_CONST(0x20000) };			//!< The square b3.
BitBoardData const c3 = { CW_MASK_T_CONST(0x40000) };			//!< The square c3.
BitBoardData const d3 = { CW_MASK_T_CONST(0x80000) };			//!< The square d3.
BitBoardData const e3 = { CW_MASK_T_CONST(0x100000) };			//!< The square e3.
BitBoardData const f3 = { CW_MASK_T_CONST(0x200000) };			//!< The square f3.
BitBoardData const g3 = { CW_MASK_T_CONST(0x400000) };			//!< The square g3.
BitBoardData const h3 = { CW_MASK_T_CONST(0x800000) };			//!< The square h3.
BitBoardData const a4 = { CW_MASK_T_CONST(0x1000000) };		//!< The square a4.
BitBoardData const b4 = { CW_MASK_T_CONST(0x2000000) };		//!< The square b4.
BitBoardData const c4 = { CW_MASK_T_CONST(0x4000000) };		//!< The square c4.
BitBoardData const d4 = { CW_MASK_T_CONST(0x8000000) };		//!< The square d4.
BitBoardData const e4 = { CW_MASK_T_CONST(0x10000000) };		//!< The square e4.
BitBoardData const f4 = { CW_MASK_T_CONST(0x20000000) };		//!< The square f4.
BitBoardData const g4 = { CW_MASK_T_CONST(0x40000000) };		//!< The square g4.
BitBoardData const h4 = { CW_MASK_T_CONST(0x80000000) };		//!< The square h4.
BitBoardData const a5 = { CW_MASK_T_CONST(0x100000000) };		//!< The square a5.
BitBoardData const b5 = { CW_MASK_T_CONST(0x200000000) };		//!< The square b5.
BitBoardData const c5 = { CW_MASK_T_CONST(0x400000000) };		//!< The square c5.
BitBoardData const d5 = { CW_MASK_T_CONST(0x800000000) };		//!< The square d5.
BitBoardData const e5 = { CW_MASK_T_CONST(0x1000000000) };		//!< The square e5.
BitBoardData const f5 = { CW_MASK_T_CONST(0x2000000000) };		//!< The square f5.
BitBoardData const g5 = { CW_MASK_T_CONST(0x4000000000) };		//!< The square g5.
BitBoardData const h5 = { CW_MASK_T_CONST(0x8000000000) };		//!< The square h5.
BitBoardData const a6 = { CW_MASK_T_CONST(0x10000000000) };		//!< The square a6.
BitBoardData const b6 = { CW_MASK_T_CONST(0x20000000000) };		//!< The square b6.
BitBoardData const c6 = { CW_MASK_T_CONST(0x40000000000) };		//!< The square c6.
BitBoardData const d6 = { CW_MASK_T_CONST(0x80000000000) };		//!< The square d6.
BitBoardData const e6 = { CW_MASK_T_CONST(0x100000000000) };		//!< The square e6.
BitBoardData const f6 = { CW_MASK_T_CONST(0x200000000000) };		//!< The square f6.
BitBoardData const g6 = { CW_MASK_T_CONST(0x400000000000) };		//!< The square g6.
BitBoardData const h6 = { CW_MASK_T_CONST(0x800000000000) };		//!< The square h6.
BitBoardData const a7 = { CW_MASK_T_CONST(0x1000000000000) };		//!< The square a7.
BitBoardData const b7 = { CW_MASK_T_CONST(0x2000000000000) };		//!< The square b7.
BitBoardData const c7 = { CW_MASK_T_CONST(0x4000000000000) };		//!< The square c7.
BitBoardData const d7 = { CW_MASK_T_CONST(0x8000000000000) };		//!< The square d7.
BitBoardData const e7 = { CW_MASK_T_CONST(0x10000000000000) };		//!< The square e7.
BitBoardData const f7 = { CW_MASK_T_CONST(0x20000000000000) };		//!< The square f7.
BitBoardData const g7 = { CW_MASK_T_CONST(0x40000000000000) };		//!< The square g7.
BitBoardData const h7 = { CW_MASK_T_CONST(0x80000000000000) };		//!< The square h7.
BitBoardData const a8 = { CW_MASK_T_CONST(0x100000000000000) };	//!< The square a8.
BitBoardData const b8 = { CW_MASK_T_CONST(0x200000000000000) };	//!< The square b8.
BitBoardData const c8 = { CW_MASK_T_CONST(0x400000000000000) };	//!< The square c8
BitBoardData const d8 = { CW_MASK_T_CONST(0x800000000000000) };	//!< The square d8
BitBoardData const e8 = { CW_MASK_T_CONST(0x1000000000000000) };	//!< The square e8.
BitBoardData const f8 = { CW_MASK_T_CONST(0x2000000000000000) };	//!< The square f8.
BitBoardData const g8 = { CW_MASK_T_CONST(0x4000000000000000) };	//!< The square g8.
BitBoardData const h8 = { CW_MASK_T_CONST(0x8000000000000000) };	//!< The square h8.

// Compare constants (this should never be needed, but why not add it).
inline bool operator==(BitBoardData b1, BitBoardData b2) { return b1.M_bitmask == b2.M_bitmask; }
inline bool operator!=(BitBoardData b1, BitBoardData b2) { return b1.M_bitmask != b2.M_bitmask; }

/** @brief Calculate the union of two bit board constants. */
inline BitBoardData operator|(BitBoardData x, BitBoardData y)
{
  BitBoardData result;
  result.M_bitmask = x.M_bitmask | y.M_bitmask;
  return result;
}

/** @brief Calculate the intersection of two bit board constants. */
inline BitBoardData operator&(BitBoardData x, BitBoardData y)
{
  BitBoardData result;
  result.M_bitmask = x.M_bitmask & y.M_bitmask;
  return result;
}

/** @brief Calculate the union minus the intersection of two bit board constants. */
inline BitBoardData operator^(BitBoardData x, BitBoardData y)
{
  BitBoardData result;
  result.M_bitmask = x.M_bitmask ^ y.M_bitmask;
  return result;
}

/** @brief Calculate the inverse of the bitboard constant. */
inline BitBoardData operator~(BitBoardData x)
{
  BitBoardData result;
  result.M_bitmask = ~x.M_bitmask;
  return result;
}

BitBoardData const file_a = a1|a2|a3|a4|a5|a6|a7|a8;	//!< The a-file.
BitBoardData const file_b = b1|b2|b3|b4|b5|b6|b7|b8;	//!< The b-file.
BitBoardData const file_c = c1|c2|c3|c4|c5|c6|c7|c8;	//!< The c-file.
BitBoardData const file_d = d1|d2|d3|d4|d5|d6|d7|d8;	//!< The d-file.
BitBoardData const file_e = e1|e2|e3|e4|e5|e6|e7|e8;	//!< The e-file.
BitBoardData const file_f = f1|f2|f3|f4|f5|f6|f7|f8;	//!< The f-file.
BitBoardData const file_g = g1|g2|g3|g4|g5|g6|g7|g8;	//!< The g-file.
BitBoardData const file_h = h1|h2|h3|h4|h5|h6|h7|h8;	//!< The h-file.

BitBoardData const rank_1 = a1|b1|c1|d1|e1|f1|g1|h1;	//!< The first rank.
BitBoardData const rank_2 = a2|b2|c2|d2|e2|f2|g2|h2;	//!< The second rank.
BitBoardData const rank_3 = a3|b3|c3|d3|e3|f3|g3|h3;	//!< The third rank.
BitBoardData const rank_4 = a4|b4|c4|d4|e4|f4|g4|h4;	//!< The fourth rank.
BitBoardData const rank_5 = a5|b5|c5|d5|e5|f5|g5|h5;	//!< The fifth rank.
BitBoardData const rank_6 = a6|b6|c6|d6|e6|f6|g6|h6;	//!< The sixth rank.
BitBoardData const rank_7 = a7|b7|c7|d7|e7|f7|g7|h7;	//!< The seventh rank.
BitBoardData const rank_8 = a8|b8|c8|d8|e8|f8|g8|h8;	//!< The eighth rank.

BitBoardData const bdl_1 =                      h8;	//!< The black diagonal from h8 to h8.
BitBoardData const bdl_2 =                f8|g7|h6;	//!< The black diagonal from f8 to h6.
BitBoardData const bdl_3 =          d8|e7|f6|g5|h4;	//!< The black diagonal from d8 to h4.
BitBoardData const bdl_4 =    b8|c7|d6|e5|f4|g3|h2;	//!< The black diagonal from b8 to h2.
BitBoardData const bdl_5 = a7|b6|c5|d4|e3|f2|g1;	//!< The black diagonal from a7 to g1.
BitBoardData const bdl_6 = a5|b4|c3|d2|e1;		//!< The black diagonal from a5 to e1.
BitBoardData const bdl_7 = a3|b2|c1;			//!< The black diagonal from a3 to c1.
BitBoardData const bdl_8 = a1;				//!< The black diagonal from a1 to a1.

BitBoardData const bdr_1 = a7|b8;			//!< The black diagonal from a7 to b8.
BitBoardData const bdr_2 = a5|b6|c7|d8;			//!< The black diagonal from a5 to d8.
BitBoardData const bdr_3 = a3|b4|c5|d6|e7|f8;		//!< The black diagonal from a3 to f8.
BitBoardData const bdr_4 = a1|b2|c3|d4|e5|f6|g7|h8;	//!< The black diagonal from a1 to h8.
BitBoardData const bdr_5 =       c1|d2|e3|f4|g5|h6;	//!< The black diagonal from c1 to h6.
BitBoardData const bdr_6 =             e1|f2|g3|h4;	//!< The black diagonal from e1 to h4.
BitBoardData const bdr_7 =                   g1|h2;	//!< The black diagonal from g1 ti h2.

BitBoardData const wdr_1 = a8;				//!< The white diagonal from a8 to a8.
BitBoardData const wdr_2 = a6|b7|c8;			//!< The white diagonal from a6 to c8.
BitBoardData const wdr_3 = a4|b5|c6|d7|e8;		//!< The white diagonal from a4 to e8.
BitBoardData const wdr_4 = a2|b3|c4|d5|e6|f7|g8;	//!< The white diagonal from a2 to g8.
BitBoardData const wdr_5 =    b1|c2|d3|e4|f5|g6|h7;	//!< The white diagonal from b1 to h7.
BitBoardData const wdr_6 =          d1|e2|f3|g4|h5;	//!< The white diagonal from d1 to h5.
BitBoardData const wdr_7 =                f1|g2|h3;	//!< The white diagonal from f1 to h3.
BitBoardData const wdr_8 =                      h1;	//!< The white diagonal from h1 to h1.

BitBoardData const wdl_1 =                   g8|h7;	//!< The white diagonal from g8 to h7.
BitBoardData const wdl_2 =             e8|f7|g6|h5;	//!< The white diagonal from e8 to h5.
BitBoardData const wdl_3 =       c8|d7|e6|f5|g4|h3;	//!< The white diagonal from c8 to h3.
BitBoardData const wdl_4 = a8|b7|c6|d5|e4|f3|g2|h1;	//!< The white diagonal from a8 to h1.
BitBoardData const wdl_5 = a6|b5|c4|d3|e2|f1;		//!< The white diagonal from a6 to f1.
BitBoardData const wdl_6 = a4|b3|c2|d1;			//!< The white diagonal from a4 to d1.
BitBoardData const wdl_7 = a2|b1;			//!< The white diagonal from a2 to b1.

#if DEBUG_BITBOARD_INITIALIZATION
uint64_t const bitboard_initialization_magic = 1212121212121212121;
uint64_t const bitboard_destructed_magic = 555555555555555555;
#endif

/** @brief A one-boolean-per-square chessboard.
 *
 * This class defines the interface for access to the bit-board.
 *
 * Normally one should be hidden from the bit-level implemention (mask_t thus).
 * Instead use BitBoardData constant:
 *
 * \link cwchess::a1 a1 \endlink, ..., \link cwchess::h8 h8 \endlink<br>
 * \link cwchess::file_a file_a \endlink, ..., \link cwchess::file_h file_h \endlink<br>
 * \link cwchess::rank_1 rank_1 \endlink, ..., \link cwchess::rank_8 rank_8 \endlink<br>
 * and others.
 */
class BitBoard : protected BitBoardData {
#if DEBUG_BITBOARD_INITIALIZATION
  private:
    uint64_t M_initialized;
#endif

  public:
#if DEBUG_BITBOARD_INITIALIZATION
    BitBoard() : M_initialized(0) { }    
    ~BitBoard() { M_initialized = bitboard_destructed_magic; }

    bool is_initialized() const { return M_initialized == bitboard_initialization_magic; }
#else
  //! @name Constructors
  //@{

    //! Construct an uninitialized BitBoard.
    BitBoard() { }    

#endif

    //! Construct a BitBoard from another BitBoard.
    BitBoard(BitBoard const& other)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      assert(other.is_initialized());
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = other.M_bitmask;
    }

    //! Construct a BitBoard with a single bit set at \a index.
    BitBoard(Index const& index)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = index2mask(index);
    }

    //! Construct a BitBoard from a constant.
    BitBoard(BitBoardData data)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = data.M_bitmask;
    }

    //! Construct a BitBoard with a single bit set at \a col, \a row.
    BitBoard(int col, int row)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = colrow2mask(col, row);
    }

    //! Construct a BitBoard from a constant or mask (for internal use only).
    explicit BitBoard(mask_t bitmask)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = bitmask;
    }

  //@}

  //! @name Assignment operators
  //@{

    //! Assignment from other BitBoard.
    BitBoard& operator=(BitBoard const& bitboard)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      assert(bitboard.is_initialized());
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = bitboard.M_bitmask;
      return *this;
    }

    //! Assignment from a constant.
    BitBoard& operator=(BitBoardData bitboard)
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = bitboard.M_bitmask;
      return *this;
    }

  //@}

  //! @name Comparison operators
  //@{

    friend bool operator==(BitBoard const& b1, BitBoard const& b2) { return b1.M_bitmask == b2.M_bitmask; }
    friend bool operator==(BitBoard const& b1, BitBoardData b2) { return b1.M_bitmask == b2.M_bitmask; }
    friend bool operator==(BitBoardData b1, BitBoard const& b2) { return b1.M_bitmask == b2.M_bitmask; }
    friend bool operator!=(BitBoard const& b1, BitBoard const& b2) { return b1.M_bitmask != b2.M_bitmask; }
    friend bool operator!=(BitBoard const& b1, BitBoardData b2) { return b1.M_bitmask != b2.M_bitmask; }
    friend bool operator!=(BitBoardData b1, BitBoard const& b2) { return b1.M_bitmask != b2.M_bitmask; }

  //@}

  //! @name Initialization
  //@{

    //! Set all values to FALSE.
    void reset()
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = 0; 
    }

    //! Set all values to TRUE.
    void set()
    {
#if DEBUG_BITBOARD_INITIALIZATION
      M_initialized = bitboard_initialization_magic;
#endif
      M_bitmask = CW_MASK_T_CONST(0xffffffffffffffff);
    }

  //@}

  //! @name Bit fiddling
  //@{

    //! @brief Reset the bit at \a col, \a row.
    void reset(int col, int row) { M_bitmask &= ~colrow2mask(col, row); }

    //! @brief Reset the bit at \a index.
    void reset(Index const& index) { M_bitmask &= ~index2mask(index); }

    //! @brief Reset the bits from \a mask.
    void reset(mask_t mask) { M_bitmask &= ~mask; }

    //! @brief Reset the bits from \a bitboard.
    void reset(BitBoardData bitboard) { M_bitmask &= ~bitboard.M_bitmask; }

    //! @brief Reset the bits from \a bitboard.
    void reset(BitBoard const& bitboard) { M_bitmask &= ~bitboard.M_bitmask; }

    //! @brief Set the bit at \a col, \a row.
    void set(int col, int row) { M_bitmask |= colrow2mask(col, row); }

    //! @brief Set the bit at \a index.
    void set(Index const& index) { M_bitmask |= index2mask(index); }

    //! @brief Set the bits from \a mask.
    void set(mask_t mask) { M_bitmask |= mask; }

    //! @brief Set the bits from \a bitboard.
    void set(BitBoardData bitboard) { M_bitmask |= bitboard.M_bitmask; }

    //! @brief Set the bits from \a bitboard.
    void set(BitBoard const& bitboard) { M_bitmask |= bitboard.M_bitmask; }

    //! @brief Toggle the bit at \a col, \a row.
    void toggle(int col, int row) { M_bitmask ^= colrow2mask(col, row); }

    //! @brief Toggle the bit at \a index.
    void toggle(Index const& index) { M_bitmask ^= index2mask(index); }

    //! @brief Toggle the bits from \a mask.
    void toggle(mask_t mask) { M_bitmask ^= mask; }

    //! @brief Toggle the bits from \a bitboard.
    void toggle(BitBoardData bitboard) { M_bitmask ^= bitboard.M_bitmask; }

    //! @brief Toggle the bits from \a bitboard.
    void toggle(BitBoard const& bitboard) { M_bitmask ^= bitboard.M_bitmask; }

  //@}

  //! @name Accessors
  //@{

    //! @brief Test if any bit is set at all.
    bool test() const { return M_bitmask; }

    //! @brief Test if the bit at \a col, \a row is set.
    bool test(int col, int row) const { return M_bitmask & colrow2mask(col, row); }

    //! @brief Test if the bit at \a index is set.
    bool test(Index const& index) const { return M_bitmask & index2mask(index); }

    //! @brief Test if any bit in \a mask is set.
    bool test(mask_t mask) const { return M_bitmask & mask; }

    //! @brief Test if any bit in \a bitboard is set.
    bool test(BitBoardData bitboard) const { return M_bitmask & bitboard.M_bitmask; }

    //! @brief Test if any bit in \a bitboard is set.
    bool test(BitBoard const& bitboard) const { return M_bitmask & bitboard.M_bitmask; }

    //! @brief Return the inverse of the bitboard.
    BitBoard operator~() const { return BitBoard(~M_bitmask); }

    //! @brief Return TRUE if the bitboard is not empty.
#ifdef __x86_64
    operator void*() const { return reinterpret_cast<void*>(M_bitmask); }
#else
    // Casting to void* would only give us the lower 32 bits.
    operator void*() const { return M_bitmask ? (void*)1 : 0; }
#endif

    //! @brief Return the underlaying bitmask.
    mask_t operator()() const { return M_bitmask; }

  //@}

  //! @name Bit-wise OR operators with another BitBoard
  //@{
    BitBoard& operator|=(BitBoard const& bitboard) { M_bitmask |= bitboard.M_bitmask; return *this; }
    BitBoard& operator|=(BitBoardData bitboard) { M_bitmask |= bitboard.M_bitmask; return *this; }
    BitBoard& operator|=(mask_t bitmask) { M_bitmask |= bitmask; return *this; }
    friend BitBoard operator|(BitBoard const& bitboard1, BitBoard const& bitboard2) { return BitBoard(bitboard1.M_bitmask | bitboard2.M_bitmask); }
    friend BitBoard operator|(BitBoard const& bitboard1, BitBoardData bitboard2) { return BitBoard(bitboard1.M_bitmask | bitboard2.M_bitmask); }
    friend BitBoard operator|(BitBoardData bitboard1, BitBoard const& bitboard2) { return BitBoard(bitboard1.M_bitmask | bitboard2.M_bitmask); }
  //@}

  //! @name Bit-wise AND operators with another BitBoard
  //@{
    BitBoard& operator&=(BitBoard const& bitboard) { M_bitmask &= bitboard.M_bitmask; return *this; }
    BitBoard& operator&=(BitBoardData bitboard) { M_bitmask &= bitboard.M_bitmask; return *this; }
    BitBoard& operator&=(mask_t bitmask) { M_bitmask &= bitmask; return *this; }
    friend BitBoard operator&(BitBoard bitboard1, BitBoard bitboard2) { return BitBoard(bitboard1.M_bitmask & bitboard2.M_bitmask); }
    friend BitBoard operator&(BitBoard bitboard1, BitBoardData bitboard2) { return BitBoard(bitboard1.M_bitmask & bitboard2.M_bitmask); }
    friend BitBoard operator&(BitBoardData bitboard1, BitBoard bitboard2) { return BitBoard(bitboard1.M_bitmask & bitboard2.M_bitmask); }
  //@}

  //! @name Bit-wise XOR operators with another BitBoard
  //@{
    BitBoard& operator^=(BitBoard const& bitboard) { M_bitmask ^= bitboard.M_bitmask; return *this; }
    BitBoard& operator^=(BitBoardData bitboard) { M_bitmask ^= bitboard.M_bitmask; return *this; }
    BitBoard& operator^=(mask_t bitmask) { M_bitmask ^= bitmask; return *this; }
    friend BitBoard operator^(BitBoard bitboard1, BitBoard bitboard2) { return BitBoard(bitboard1.M_bitmask ^ bitboard2.M_bitmask); }
    friend BitBoard operator^(BitBoard bitboard1, BitBoardData bitboard2) { return BitBoard(bitboard1.M_bitmask ^ bitboard2.M_bitmask); }
    friend BitBoard operator^(BitBoardData bitboard1, BitBoard bitboard2) { return BitBoard(bitboard1.M_bitmask ^ bitboard2.M_bitmask); }
  //@}

};

} // namespace cwchess

#endif	// BITBOARD_H
