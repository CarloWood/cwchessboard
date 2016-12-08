// cwchessboard -- A C++ chessboard tool set
//
//! @file Index.h This file contains the declaration of class Index.
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

#ifndef INDEX_H
#define INDEX_H

#ifndef USE_PCH
#include <stdint.h>
#endif

#define DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK 0

#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
#include <cassert>
#endif

namespace cwchess {

/** @brief The POD base type of class Index.
 *
 * The index of square a1 is \link cwchess::ia1 ia1 \endlink, and so on till \link cwchess::ih8 ih8 \endlink.
 *
 * @sa Index, index_pre_begin, index_begin, index_end
 */ 
struct IndexData {
  uint8_t M_bits;  	//!< 00RRRCCC, where RRR is the row and CCC the column.
};

uint8_t const col_mask = 0x07;	//!< A mask for the bits used for the column in IndexData.
uint8_t const row_mask = 0x38;	//!< A mask for the bits used for the row in IndexData.

//! A constant representing the index to square a1.
IndexData const ia1 = { 0 };
//! A constant representing the index to square b1.
IndexData const ib1 = { 1 };
//! A constant representing the index to square c1.
IndexData const ic1 = { 2 };
//! A constant representing the index to square d1.
IndexData const id1 = { 3 };
//! A constant representing the index to square e1.
IndexData const ie1 = { 4 };
//! A constant representing the index to square f1.
IndexData const if1 = { 5 };
//! A constant representing the index to square g1.
IndexData const ig1 = { 6 };
//! A constant representing the index to square h1.
IndexData const ih1 = { 7 };
//! A constant representing the index to square a2.
IndexData const ia2 = { 8 };
//! A constant representing the index to square b2.
IndexData const ib2 = { 9 };
//! A constant representing the index to square c2.
IndexData const ic2 = { 10 };
//! A constant representing the index to square d2.
IndexData const id2 = { 11 };
//! A constant representing the index to square e2.
IndexData const ie2 = { 12 };
//! A constant representing the index to square f2.
IndexData const if2 = { 13 };
//! A constant representing the index to square g2.
IndexData const ig2 = { 14 };
//! A constant representing the index to square h2.
IndexData const ih2 = { 15 };
//! A constant representing the index to square a3.
IndexData const ia3 = { 16 };
//! A constant representing the index to square b3.
IndexData const ib3 = { 17 };
//! A constant representing the index to square c3.
IndexData const ic3 = { 18 };
//! A constant representing the index to square d3.
IndexData const id3 = { 19 };
//! A constant representing the index to square e3.
IndexData const ie3 = { 20 };
//! A constant representing the index to square f3.
IndexData const if3 = { 21 };
//! A constant representing the index to square g3.
IndexData const ig3 = { 22 };
//! A constant representing the index to square h3.
IndexData const ih3 = { 23 };
//! A constant representing the index to square a4.
IndexData const ia4 = { 24 };
//! A constant representing the index to square b4.
IndexData const ib4 = { 25 };
//! A constant representing the index to square c4.
IndexData const ic4 = { 26 };
//! A constant representing the index to square d4.
IndexData const id4 = { 27 };
//! A constant representing the index to square e4.
IndexData const ie4 = { 28 };
//! A constant representing the index to square f4.
IndexData const if4 = { 29 };
//! A constant representing the index to square g4.
IndexData const ig4 = { 30 };
//! A constant representing the index to square h4.
IndexData const ih4 = { 31 };
//! A constant representing the index to square a5.
IndexData const ia5 = { 32 };
//! A constant representing the index to square b5.
IndexData const ib5 = { 33 };
//! A constant representing the index to square c5.
IndexData const ic5 = { 34 };
//! A constant representing the index to square d5.
IndexData const id5 = { 35 };
//! A constant representing the index to square e5.
IndexData const ie5 = { 36 };
//! A constant representing the index to square f5.
IndexData const if5 = { 37 };
//! A constant representing the index to square g5.
IndexData const ig5 = { 38 };
//! A constant representing the index to square h5.
IndexData const ih5 = { 39 };
//! A constant representing the index to square a6.
IndexData const ia6 = { 40 };
//! A constant representing the index to square b6.
IndexData const ib6 = { 41 };
//! A constant representing the index to square c6.
IndexData const ic6 = { 42 };
//! A constant representing the index to square d6.
IndexData const id6 = { 43 };
//! A constant representing the index to square e6.
IndexData const ie6 = { 44 };
//! A constant representing the index to square f6.
IndexData const if6 = { 45 };
//! A constant representing the index to square g6.
IndexData const ig6 = { 46 };
//! A constant representing the index to square h6.
IndexData const ih6 = { 47 };
//! A constant representing the index to square a7.
IndexData const ia7 = { 48 };
//! A constant representing the index to square b7.
IndexData const ib7 = { 49 };
//! A constant representing the index to square c7.
IndexData const ic7 = { 50 };
//! A constant representing the index to square d7.
IndexData const id7 = { 51 };
//! A constant representing the index to square e7.
IndexData const ie7 = { 52 };
//! A constant representing the index to square f7.
IndexData const if7 = { 53 };
//! A constant representing the index to square g7.
IndexData const ig7 = { 54 };
//! A constant representing the index to square h7.
IndexData const ih7 = { 55 };
//! A constant representing the index to square a8.
IndexData const ia8 = { 56 };
//! A constant representing the index to square b8.
IndexData const ib8 = { 57 };
//! A constant representing the index to square c8.
IndexData const ic8 = { 58 };
//! A constant representing the index to square d8.
IndexData const id8 = { 59 };
//! A constant representing the index to square e8.
IndexData const ie8 = { 60 };
//! A constant representing the index to square f8.
IndexData const if8 = { 61 };
//! A constant representing the index to square g8.
IndexData const ig8 = { 62 };
//! A constant representing the index to square h8.
IndexData const ih8 = { 63 };

//! A constant representing 'one before the start'.
IndexData const index_pre_begin = { 255 };
//! A constant representing the 'first' index.
IndexData const index_begin = { 0 };
//! A constant representing 'one past the end'.
IndexData const index_end = { 64 };

// Compare constants (this should never be needed, but why not add it).
inline bool operator==(IndexData i1, IndexData i2) { return i1.M_bits == i2.M_bits; }
inline bool operator!=(IndexData i1, IndexData i2) { return i1.M_bits != i2.M_bits; }

#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
uint64_t const index_initialization_magic = CW_MASK_T_CONST(1515151515151515151);
uint64_t const index_destruction_magic = CW_MASK_T_CONST(6666666666666666666);
#endif

/** @brief The index of a chess square.
 *
 * This class represents the index to a square on the chessboard.
 * It's value runs from 0 till 63, where 0 corresponds to a1 and 63 to h8.
 * In addition there are two out of band values: index_pre_begin (255) and index_end (64).
 *
 * The three least significant bits represent the column, which runs from 0 to 7.
 * The next three bits represent the row, which also runs from 0 to 7.
 *
 * Hence column 0 corresponds to file a, column 1 to file b etc,
 * while row 0 corresponds to rank 1, row 1 to rank 2 etc.
 *
 * @sa IndexData, index_pre_begin, index_begin, index_end,
 */
class Index : protected IndexData {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
  private:
    uint64_t M_initialized;
#endif

  public:
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
    Index(void) : M_initialized(0) { }
    ~Index() { M_initialized = index_destruction_magic; }

    bool is_initialized(void) const { return M_initialized == index_initialization_magic; }
#else
  /** @name Constructors */
  //@{

    //! Construct an uninitialized Index object.
    Index(void) { }
#endif

    //! Copy-constructor.
    Index(Index const& index)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(index.is_initialized());
      M_initialized = index_initialization_magic;
#endif
      M_bits = index.M_bits;
    }

    //! Construct an Index object from a constant.
    Index(IndexData index)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(index.M_bits < 64 || index == index_end || index == index_pre_begin);
      M_initialized = index_initialization_magic;
#endif
      M_bits = index.M_bits;
    }

    //! Construct an Index for column \a col and row \a row.
    Index(int col, int row)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(col >= 0 && col <= 7 && row >= 0 && row <= 7);
      M_initialized = index_initialization_magic;
#endif
      M_bits = ((uint8_t)row << 3) | (uint8_t)col;
    }

  //@}

  /** @name Assignment operators */
  //@{

    Index& operator=(Index const& index)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(index.is_initialized());
      M_initialized = index_initialization_magic;
#endif
      M_bits = index.M_bits;
      return *this;
    }

    Index& operator=(IndexData index)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(index.M_bits < 64 || index == index_end || index == index_pre_begin);
      M_initialized = index_initialization_magic;
#endif
      M_bits = index.M_bits;
      return *this;
    }

  //@}

  /** @name Comparision operators */
  //@{

    friend bool operator==(Index const& i1, Index const& i2) { return i1.M_bits == i2.M_bits; }
    friend bool operator==(Index const& i1, IndexData i2) { return i1.M_bits == i2.M_bits; }
    friend bool operator==(IndexData i1, Index const& i2) { return i1.M_bits == i2.M_bits; }
    friend bool operator!=(Index const& i1, Index const& i2) { return i1.M_bits != i2.M_bits; }
    friend bool operator!=(Index const& i1, IndexData i2) { return i1.M_bits != i2.M_bits; }
    friend bool operator!=(IndexData i1, Index const& i2) { return i1.M_bits != i2.M_bits; }

    friend bool operator<(Index const& index1, Index const& index2) { return index1.M_bits < index2.M_bits; }
    friend bool operator<(Index const& index1, IndexData const& index2) { return index1.M_bits < index2.M_bits; }
    friend bool operator<(IndexData const& index1, Index const& index2) { return index1.M_bits < index2.M_bits; }
    friend bool operator<=(Index const& index1, Index const& index2) { return index1.M_bits <= index2.M_bits; }
    friend bool operator<=(Index const& index1, IndexData const& index2) { return index1.M_bits <= index2.M_bits; }
    friend bool operator<=(IndexData const& index1, Index const& index2) { return index1.M_bits <= index2.M_bits; }
    friend bool operator>(Index const& index1, Index const& index2) { return index1.M_bits > index2.M_bits; }
    friend bool operator>(Index const& index1, IndexData const& index2) { return index1.M_bits > index2.M_bits; }
    friend bool operator>(IndexData const& index1, Index const& index2) { return index1.M_bits > index2.M_bits; }
    friend bool operator>=(Index const& index1, Index const& index2) { return index1.M_bits >= index2.M_bits; }
    friend bool operator>=(Index const& index1, IndexData const& index2) { return index1.M_bits >= index2.M_bits; }
    friend bool operator>=(IndexData const& index1, Index const& index2) { return index1.M_bits >= index2.M_bits; }

  //@}

  /** @name Manipulators */
  //@{

    Index const& operator+=(int offset) { M_bits += offset; return *this; }
    friend Index operator+(Index const& index, int offset) { Index result(index); return result += offset; }
    friend Index operator+(int offset, Index const& index) { Index result(index); return result += offset; }
    Index const& operator-=(int offset) { M_bits -= offset; return *this; }
    friend Index operator-(Index const& index, int offset) { Index result(index); return result -= offset; }
    friend Index operator-(int offset, Index const& index) { Index result(index); return result -= offset; }

    Index& operator++(void) { ++M_bits; return *this; }
    Index operator++(int) { Index result(*this); operator++(); return result; }
    Index& operator--(void) { ++M_bits; return *this; }
    Index operator--(int) { Index result(*this); operator--(); return result; }

  //@}

  /** @name Accessors */
  //@{

    //! Returns the row.
    int row(void) const { return M_bits >> 3; }

    //! Returns the column.
    int col(void) const { return M_bits & 7; }

    //! Return the unlaying integral value.
    uint8_t operator()(void) const { return M_bits; }

  //@}

  /** @name Special functions */
  //@{

    /*! @brief Advance the index to the next bit that is set in mask.
     *
     * Index may be 0xff, in which case it will be set to
     * the first bit that is set in the mask (0...63) if any,
     * or 64 if no bit is set.
     *
     * Otherwise Index must be in the range [0, 63],
     * in which case a value is returned larger than the
     * current value. If no more bits could be found, Index is
     * set to 64.
     *
     * @param mask : The bitmask.
     */
    void next_bit_in(uint64_t mask)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(is_initialized());
      assert(M_bits < 64 || M_bits == index_pre_begin.M_bits);
#endif
      if (__builtin_expect(++M_bits == 64, 0))
        return;
      mask >>= M_bits; 
#ifdef __x86_64
      uint64_t indx = 64 - M_bits;
      __asm__ __volatile__(
	  "bsfq  %1, %0"	// mask, indx
	: "=&r" (indx)
	: "r" (mask), "0" (indx)
	: "cc"
      );
      M_bits += indx;
#else
      union conversion {
	uint64_t mask;
	struct {
	  uint32_t low;
	  uint32_t high;
	};
      };
      conversion tmp;
      tmp.mask = mask;
      uint32_t indx;
      if (tmp.low)
      {
	__asm__ __volatile__(
	    "bsf  %1, %0"       // low, indx
	  : "=&r" (indx)
	  : "r" (tmp.low), "0" (indx)
	  : "cc"
	);
	M_bits += indx;
      }
      else
      {
	indx = 32 - M_bits;
	__asm__ __volatile__(
	    "bsf  %1, %0"       // high, indx
	  : "=&r" (indx)
	  : "r" (tmp.high), "0" (indx)
	  : "cc"
	);
	M_bits += indx + 32;
      }
#endif
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(M_bits <= 64);
#endif
    }

    /*! @brief Retreat Index to the previous bit that is set.
     *
     * Index may be 64, in which case it will be set to the last
     * bit that is set in the BitBoard (0...63) if any, or 0xff
     * if no bit is set.
     *
     * Otherwise Index must be in the range <0, 63], in which
     * case a value is returned smaller than the current value.
     * If no more bits could be found, Index is set to 0xff.
     *
     * Normally a value of 0 should always return 0xff,
     * but that is not the case. Don't call this function
     * if the Index is 0.
     *
     * @param mask : The bitmask.
     */
    void prev_bit_in(uint64_t mask)
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(is_initialized());
      assert(M_bits > 0 && M_bits != index_pre_begin.M_bits);
#endif
      M_bits = 64 - M_bits;
      mask <<= M_bits;
#ifdef __x86_64
      uint64_t indx = 0xff + M_bits;
      __asm__ __volatile__(
	  "bsrq  %1, %0"	// mask, indx
	: "=&r" (indx)
	: "r" (mask), "0" (indx)
	: "cc"
      );
      M_bits = indx - M_bits;
#else
      union conversion {
	uint64_t mask;
	struct {
	  uint32_t low;
	  uint32_t high;
	};
      };
      conversion tmp;
      tmp.mask = mask;
      uint32_t indx;
      if (tmp.high)
      {
	__asm__ __volatile__(
	    "bsr  %1, %0"       // high, indx
	  : "=&r" (indx)
	  : "r" (tmp.high), "0" (indx)
	  : "cc"
	);
	M_bits = indx - M_bits + 32;
      }
      else
      {
	indx = 0xff + M_bits;
	__asm__ __volatile__(
	    "bsr  %1, %0"       // low, indx
	  : "=&r" (indx)
	  : "r" (tmp.low), "0" (indx)
	  : "cc"
	);
	M_bits = indx - M_bits;
      }
#endif
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(M_bits < 64 || M_bits == index_pre_begin.M_bits);
#endif
    }

    //! Return TRUE if index is not index_pre_begin and also not 0.
    bool may_call_prev_bit_in(void) const
    {
#if DEBUG_INDEX_INITIALIZATION_AND_RANGE_CHECK
      assert(is_initialized());
#endif
      return (int8_t)M_bits > 0;
    }

  //@}

};

} // namespace cwchess

#endif	// INDEX_H
