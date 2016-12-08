// cwchessboard -- A C++ chessboard tool set
//
//! @file BitBoardTest.h Testsuite header for class BitBoard.
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

#ifndef BITBOARD_TEST_H
#define BITBOARD_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "BitBoard.h"

namespace testsuite {

using namespace cwchess;

class BitBoardTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BitBoardTest);

  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testCopyconstructorEquality);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST(testAssignment);
  CPPUNIT_TEST(test2mask);
  CPPUNIT_TEST(testConsts);
  CPPUNIT_TEST(testReset);
  CPPUNIT_TEST(testSet);
  CPPUNIT_TEST(testToggle);
  CPPUNIT_TEST(testTest);
  CPPUNIT_TEST(testOrAndXor);

  CPPUNIT_TEST_SUITE_END();

  private:
    BitBoard M_default_constructed;		// Default constructed.
    BitBoard M_const_file_d;
    BitBoard M_const_file_e;
    BitBoard M_const_rank_4;
    BitBoard M_const_rank_5;

  public:
    BitBoardTest() : M_const_file_d(file_d), M_const_file_e(file_e), M_const_rank_4(rank_4), M_const_rank_5(rank_5) { }

    void setUp();
    void tearDown();

    void testConstruction();
    void testCopyconstructorEquality();
    void testEquality();
    void testAssignment();
    void test2mask();
    void testConsts();
    void testReset();
    void testSet();
    void testToggle();
    void testTest();
    void testOrAndXor();
};

} // namespace testsuite

#endif // BITBOARD_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

// This line can't be used.
CPPUNIT_TEST_SUITE_REGISTRATION(BitBoardTest);

void BitBoardTest::setUp()
{
}

void BitBoardTest::tearDown()
{
}

void BitBoardTest::testConstruction()
{
  // Test basic construction.
  CPPUNIT_ASSERT(!(a1 == b1));
  BitBoard bitboard_a1(a1);
  CPPUNIT_ASSERT(bitboard_a1 == a1);
  CPPUNIT_ASSERT(bitboard_a1 == bitboard_a1);
  BitBoard bitboard_b1(b1);
  CPPUNIT_ASSERT(bitboard_b1 == b1);
  CPPUNIT_ASSERT(!(bitboard_a1 == bitboard_b1));
}

void BitBoardTest::testCopyconstructorEquality()
{
  CPPUNIT_ASSERT(!(M_const_file_d == M_const_file_e));
  BitBoard bitboard_file_d(M_const_file_d);
  CPPUNIT_ASSERT(bitboard_file_d == M_const_file_d);
  BitBoard bitboard_file_e(M_const_file_e);
  CPPUNIT_ASSERT(bitboard_file_e == M_const_file_e);
  CPPUNIT_ASSERT(!(bitboard_file_d == bitboard_file_e));
}

void BitBoardTest::testEquality()
{
  BitBoardData data1, data2;
  for (mask_t mask1 = 1; mask1 != 0; mask1 <<= 1)
  {
    data1.M_bitmask = mask1 - 1;
    BitBoard bitboard1(data1);
    for (mask_t mask2 = 1; mask2 != 0; mask2 <<= 1)
    {
      data2.M_bitmask = mask2 - 1;
      BitBoard bitboard2(data2);
      CPPUNIT_ASSERT((bitboard1 == bitboard2) == (mask1 == mask2));
      CPPUNIT_ASSERT((bitboard1 == data2) == (mask1 == mask2));
      CPPUNIT_ASSERT((data1 == bitboard2) == (mask1 == mask2));
      CPPUNIT_ASSERT((data1 == data2) == (mask1 == mask2));
      CPPUNIT_ASSERT((bitboard1 != bitboard2) == !(mask1 == mask2));
      CPPUNIT_ASSERT((bitboard1 != data2) == !(mask1 == mask2));
      CPPUNIT_ASSERT((data1 != bitboard2) == !(mask1 == mask2));
      CPPUNIT_ASSERT((data1 != data2) == !(mask1 == mask2));
    }
  }
}

void BitBoardTest::testAssignment()
{
  BitBoardData data1, data2;
  for (mask_t mask1 = 1; mask1 != 0; mask1 <<= 1)
  {
    data1.M_bitmask = mask1 - 1;	// This can be equal to rank 1, but not file a.
    BitBoard tmp(data1);
    BitBoard bitboard1(file_a);
    CPPUNIT_ASSERT(bitboard1 != data1);
    bitboard1 = data1;
    CPPUNIT_ASSERT(bitboard1 == data1);
    bitboard1 = file_a;
    CPPUNIT_ASSERT(bitboard1 != tmp);
    bitboard1 = tmp;
    CPPUNIT_ASSERT(bitboard1 == tmp);
    for (mask_t mask2 = 1; mask2 != 0; mask2 <<= 1)
    {
      data2.M_bitmask = mask2 - 1;
      BitBoard bitboard2(file_a);
      CPPUNIT_ASSERT(bitboard2 != data2);
      bitboard2 = data2;
      CPPUNIT_ASSERT(bitboard2 == data2);
      CPPUNIT_ASSERT((bitboard1 == bitboard2) == (mask1 == mask2));
    }
  }
}

void BitBoardTest::test2mask()
{
  for (int col = 0; col < 8; ++col)
  {
    for (int row = 0; row < 8; ++row)
    {
      Index index(col, row);
      mask_t mask1 = index2mask(index);
      mask_t mask2 = colrow2mask(col, row);
      CPPUNIT_ASSERT(mask1 == mask2);
      BitBoardData data;
      data.M_bitmask = mask1;
      BitBoard bitboard(data);
      CPPUNIT_ASSERT((bitboard == a1) == (col == 0 && row == 0));
      CPPUNIT_ASSERT((bitboard == b1) == (col == 1 && row == 0));
      CPPUNIT_ASSERT((bitboard == c1) == (col == 2 && row == 0));
      CPPUNIT_ASSERT((bitboard == d1) == (col == 3 && row == 0));
      CPPUNIT_ASSERT((bitboard == e1) == (col == 4 && row == 0));
      CPPUNIT_ASSERT((bitboard == f1) == (col == 5 && row == 0));
      CPPUNIT_ASSERT((bitboard == g1) == (col == 6 && row == 0));
      CPPUNIT_ASSERT((bitboard == h1) == (col == 7 && row == 0));
      CPPUNIT_ASSERT((bitboard == a2) == (col == 0 && row == 1));
      CPPUNIT_ASSERT((bitboard == b2) == (col == 1 && row == 1));
      CPPUNIT_ASSERT((bitboard == c2) == (col == 2 && row == 1));
      CPPUNIT_ASSERT((bitboard == d2) == (col == 3 && row == 1));
      CPPUNIT_ASSERT((bitboard == e2) == (col == 4 && row == 1));
      CPPUNIT_ASSERT((bitboard == f2) == (col == 5 && row == 1));
      CPPUNIT_ASSERT((bitboard == g2) == (col == 6 && row == 1));
      CPPUNIT_ASSERT((bitboard == h2) == (col == 7 && row == 1));
      CPPUNIT_ASSERT((bitboard == a3) == (col == 0 && row == 2));
      CPPUNIT_ASSERT((bitboard == b3) == (col == 1 && row == 2));
      CPPUNIT_ASSERT((bitboard == c3) == (col == 2 && row == 2));
      CPPUNIT_ASSERT((bitboard == d3) == (col == 3 && row == 2));
      CPPUNIT_ASSERT((bitboard == e3) == (col == 4 && row == 2));
      CPPUNIT_ASSERT((bitboard == f3) == (col == 5 && row == 2));
      CPPUNIT_ASSERT((bitboard == g3) == (col == 6 && row == 2));
      CPPUNIT_ASSERT((bitboard == h3) == (col == 7 && row == 2));
      CPPUNIT_ASSERT((bitboard == a4) == (col == 0 && row == 3));
      CPPUNIT_ASSERT((bitboard == b4) == (col == 1 && row == 3));
      CPPUNIT_ASSERT((bitboard == c4) == (col == 2 && row == 3));
      CPPUNIT_ASSERT((bitboard == d4) == (col == 3 && row == 3));
      CPPUNIT_ASSERT((bitboard == e4) == (col == 4 && row == 3));
      CPPUNIT_ASSERT((bitboard == f4) == (col == 5 && row == 3));
      CPPUNIT_ASSERT((bitboard == g4) == (col == 6 && row == 3));
      CPPUNIT_ASSERT((bitboard == h4) == (col == 7 && row == 3));
      CPPUNIT_ASSERT((bitboard == a5) == (col == 0 && row == 4));
      CPPUNIT_ASSERT((bitboard == b5) == (col == 1 && row == 4));
      CPPUNIT_ASSERT((bitboard == c5) == (col == 2 && row == 4));
      CPPUNIT_ASSERT((bitboard == d5) == (col == 3 && row == 4));
      CPPUNIT_ASSERT((bitboard == e5) == (col == 4 && row == 4));
      CPPUNIT_ASSERT((bitboard == f5) == (col == 5 && row == 4));
      CPPUNIT_ASSERT((bitboard == g5) == (col == 6 && row == 4));
      CPPUNIT_ASSERT((bitboard == h5) == (col == 7 && row == 4));
      CPPUNIT_ASSERT((bitboard == a6) == (col == 0 && row == 5));
      CPPUNIT_ASSERT((bitboard == b6) == (col == 1 && row == 5));
      CPPUNIT_ASSERT((bitboard == c6) == (col == 2 && row == 5));
      CPPUNIT_ASSERT((bitboard == d6) == (col == 3 && row == 5));
      CPPUNIT_ASSERT((bitboard == e6) == (col == 4 && row == 5));
      CPPUNIT_ASSERT((bitboard == f6) == (col == 5 && row == 5));
      CPPUNIT_ASSERT((bitboard == g6) == (col == 6 && row == 5));
      CPPUNIT_ASSERT((bitboard == h6) == (col == 7 && row == 5));
      CPPUNIT_ASSERT((bitboard == a7) == (col == 0 && row == 6));
      CPPUNIT_ASSERT((bitboard == b7) == (col == 1 && row == 6));
      CPPUNIT_ASSERT((bitboard == c7) == (col == 2 && row == 6));
      CPPUNIT_ASSERT((bitboard == d7) == (col == 3 && row == 6));
      CPPUNIT_ASSERT((bitboard == e7) == (col == 4 && row == 6));
      CPPUNIT_ASSERT((bitboard == f7) == (col == 5 && row == 6));
      CPPUNIT_ASSERT((bitboard == g7) == (col == 6 && row == 6));
      CPPUNIT_ASSERT((bitboard == h7) == (col == 7 && row == 6));
      CPPUNIT_ASSERT((bitboard == a8) == (col == 0 && row == 7));
      CPPUNIT_ASSERT((bitboard == b8) == (col == 1 && row == 7));
      CPPUNIT_ASSERT((bitboard == c8) == (col == 2 && row == 7));
      CPPUNIT_ASSERT((bitboard == d8) == (col == 3 && row == 7));
      CPPUNIT_ASSERT((bitboard == e8) == (col == 4 && row == 7));
      CPPUNIT_ASSERT((bitboard == f8) == (col == 5 && row == 7));
      CPPUNIT_ASSERT((bitboard == g8) == (col == 6 && row == 7));
      CPPUNIT_ASSERT((bitboard == h8) == (col == 7 && row == 7));
    }
  }
}

void BitBoardTest::testConsts()
{
  BitBoard bitboard;
  for (int file = 1; file <= 8; ++file)
  {
    int col = file - 1;
    bitboard.reset();
    for (int row = 0; row < 8; ++row)
      bitboard |= BitBoard(col, row);
    switch (file)
    {
      case 1:
        CPPUNIT_ASSERT(bitboard == file_a);
	break;
      case 2:
        CPPUNIT_ASSERT(bitboard == file_b);
	break;
      case 3:
        CPPUNIT_ASSERT(bitboard == file_c);
	break;
      case 4:
        CPPUNIT_ASSERT(bitboard == file_d);
	break;
      case 5:
        CPPUNIT_ASSERT(bitboard == file_e);
	break;
      case 6:
        CPPUNIT_ASSERT(bitboard == file_f);
	break;
      case 7:
        CPPUNIT_ASSERT(bitboard == file_g);
	break;
      case 8:
        CPPUNIT_ASSERT(bitboard == file_h);
	break;
    }
  }
  for (int rank = 1; rank <= 8; ++rank)
  {
    int row = rank - 1;
    bitboard.reset();
    for (int col = 0; col < 8; ++col)
      bitboard |= BitBoard(col, row);
    switch (rank)
    {
      case 1:
        CPPUNIT_ASSERT(bitboard == rank_1);
        break;
      case 2:
        CPPUNIT_ASSERT(bitboard == rank_2);
        break;
      case 3:
        CPPUNIT_ASSERT(bitboard == rank_3);
        break;
      case 4:
        CPPUNIT_ASSERT(bitboard == rank_4);
        break;
      case 5:
        CPPUNIT_ASSERT(bitboard == rank_5);
        break;
      case 6:
        CPPUNIT_ASSERT(bitboard == rank_6);
        break;
      case 7:
        CPPUNIT_ASSERT(bitboard == rank_7);
        break;
      case 8:
        CPPUNIT_ASSERT(bitboard == rank_8);
        break;
    }
  }
  for (int scol = 7, count = 1; count <= 8; ++count, scol -= 2)
  {
    bitboard.reset();
    for (int row = 7; row >= 0; --row)
    {
      int col = scol + (7 - row);
      if (col < 0 || col > 7)
        continue;
      bitboard |= BitBoard(col, row);
    }
    switch (count)
    {
      case 1:
        CPPUNIT_ASSERT(bitboard == bdl_1);
        break;
      case 2:
        CPPUNIT_ASSERT(bitboard == bdl_2);
        break;
      case 3:
        CPPUNIT_ASSERT(bitboard == bdl_3);
        break;
      case 4:
        CPPUNIT_ASSERT(bitboard == bdl_4);
        break;
      case 5:
        CPPUNIT_ASSERT(bitboard == bdl_5);
        break;
      case 6:
        CPPUNIT_ASSERT(bitboard == bdl_6);
        break;
      case 7:
        CPPUNIT_ASSERT(bitboard == bdl_7);
        break;
      case 8:
        CPPUNIT_ASSERT(bitboard == bdl_8);
        break;
    }
  }
  for (int scol = 1, count = 1; count <= 7; ++count, scol += 2)
  {
    bitboard.reset();
    for (int row = 7; row >= 0; --row)
    {
      int col = scol - (7 - row);
      if (col < 0 || col > 7)
        continue;
      bitboard |= BitBoard(col, row);
    }
    switch (count)
    {
      case 1:
        CPPUNIT_ASSERT(bitboard == bdr_1);
        break;
      case 2:
        CPPUNIT_ASSERT(bitboard == bdr_2);
        break;
      case 3:
        CPPUNIT_ASSERT(bitboard == bdr_3);
        break;
      case 4:
        CPPUNIT_ASSERT(bitboard == bdr_4);
        break;
      case 5:
        CPPUNIT_ASSERT(bitboard == bdr_5);
        break;
      case 6:
        CPPUNIT_ASSERT(bitboard == bdr_6);
        break;
      case 7:
        CPPUNIT_ASSERT(bitboard == bdr_7);
        break;
    }
  }
  for (int scol = 0, count = 1; count <= 8; ++count, scol += 2)
  {
    bitboard.reset();
    for (int row = 7; row >= 0; --row)
    {
      int col = scol - (7 - row);
      if (col < 0 || col > 7)
        continue;
      bitboard |= BitBoard(col, row);
    }
    switch (count)
    {
      case 1:
        CPPUNIT_ASSERT(bitboard == wdr_1);
        break;
      case 2:
        CPPUNIT_ASSERT(bitboard == wdr_2);
        break;
      case 3:
        CPPUNIT_ASSERT(bitboard == wdr_3);
        break;
      case 4:
        CPPUNIT_ASSERT(bitboard == wdr_4);
        break;
      case 5:
        CPPUNIT_ASSERT(bitboard == wdr_5);
        break;
      case 6:
        CPPUNIT_ASSERT(bitboard == wdr_6);
        break;
      case 7:
        CPPUNIT_ASSERT(bitboard == wdr_7);
        break;
      case 8:
        CPPUNIT_ASSERT(bitboard == wdr_8);
        break;
    }
  }
  for (int scol = 6, count = 1; count <= 7; ++count, scol -= 2)
  {
    bitboard.reset();
    for (int row = 7; row >= 0; --row)
    {
      int col = scol + (7 - row);
      if (col < 0 || col > 7)
        continue;
      bitboard |= BitBoard(col, row);
    }
    switch (count)
    {
      case 1:
        CPPUNIT_ASSERT(bitboard == wdl_1);
        break;
      case 2:
        CPPUNIT_ASSERT(bitboard == wdl_2);
        break;
      case 3:
        CPPUNIT_ASSERT(bitboard == wdl_3);
        break;
      case 4:
        CPPUNIT_ASSERT(bitboard == wdl_4);
        break;
      case 5:
        CPPUNIT_ASSERT(bitboard == wdl_5);
        break;
      case 6:
        CPPUNIT_ASSERT(bitboard == wdl_6);
        break;
      case 7:
        CPPUNIT_ASSERT(bitboard == wdl_7);
        break;
    }
  }
}

void BitBoardTest::testReset()
{
  BitBoard const zero((mask_t)0);
  BitBoard const center(d4|e4|d5|e5);
  BitBoard const corners(a1|a8|h1|h8);
  BitBoard const center_plus_corners(a1|a8|h1|h8|d4|e4|d5|e5);
  BitBoard tmp(center_plus_corners);
  CPPUNIT_ASSERT(tmp != zero);
  tmp.reset();
  CPPUNIT_ASSERT(tmp == zero);
  tmp = center_plus_corners;
  CPPUNIT_ASSERT(tmp != center && tmp != corners);
  tmp.reset(3, 3);
  tmp.reset(3, 4);
  tmp.reset(4, 3);
  tmp.reset(4, 4);
  CPPUNIT_ASSERT(tmp == corners);
  tmp.reset(0, 0);
  tmp.reset(7, 0);
  tmp.reset(0, 7);
  tmp.reset(7, 7);
  CPPUNIT_ASSERT(tmp == zero);
  tmp = center_plus_corners;
  tmp.reset(Index(0, 0));
  tmp.reset(Index(7, 0));
  tmp.reset(Index(0, 7));
  tmp.reset(Index(7, 7));
  CPPUNIT_ASSERT(tmp == center);
  tmp = center_plus_corners;
  tmp.reset((a1|a8|h1|h8).M_bitmask);
  CPPUNIT_ASSERT(tmp == center);
  tmp = center_plus_corners;
  tmp.reset(center);
  CPPUNIT_ASSERT(tmp == corners);
}

void BitBoardTest::testSet()
{
  BitBoard const zero((mask_t)0);
  BitBoard const ones((mask_t)-1);
  BitBoard const center(d4|e4|d5|e5);
  BitBoard const corners(a1|a8|h1|h8);
  BitBoard const center_plus_corners(a1|a8|h1|h8|d4|e4|d5|e5);
  BitBoard tmp(zero);
  tmp.set();
  CPPUNIT_ASSERT(tmp == ones);
  tmp.reset();
  tmp.set(3, 3);
  tmp.set(3, 4);
  tmp.set(4, 3);
  tmp.set(4, 4);
  CPPUNIT_ASSERT(tmp == center);
  tmp.set(0, 0);
  tmp.set(0, 7);
  tmp.set(7, 0);
  tmp.set(7, 7);
  CPPUNIT_ASSERT(tmp == center_plus_corners);
  tmp = corners;
  tmp.set(Index(3, 3));
  tmp.set(Index(3, 4));
  tmp.set(Index(4, 3));
  tmp.set(Index(4, 4));
  CPPUNIT_ASSERT(tmp == center_plus_corners);
  tmp = corners;
  tmp.set(d4|e4|d5|e5);
  CPPUNIT_ASSERT(tmp == center_plus_corners);
  tmp = corners;
  tmp.set(center);
  CPPUNIT_ASSERT(tmp == center_plus_corners);
}

void BitBoardTest::testToggle()
{
  BitBoard const zero((mask_t)0);
  BitBoard const ones((mask_t)-1);
  BitBoard const center(d4|e4|d5|e5);
  BitBoard const corners(a1|a8|h1|h8);
  BitBoard const center_plus_corners(a1|a8|h1|h8|d4|e4|d5|e5);
  BitBoard tmp(center);
  tmp.toggle(0, 0);
  tmp.toggle(3, 3);
  tmp.toggle(0, 7);
  tmp.toggle(3, 4);
  tmp.toggle(7, 0);
  tmp.toggle(4, 3);
  tmp.toggle(7, 7);
  tmp.toggle(4, 4);
  CPPUNIT_ASSERT(tmp == corners);
  tmp.toggle(Index(0, 0));
  tmp.toggle(Index(3, 3));
  tmp.toggle(Index(0, 7));
  tmp.toggle(Index(3, 4));
  tmp.toggle(Index(7, 0));
  tmp.toggle(Index(4, 3));
  tmp.toggle(Index(7, 7));
  tmp.toggle(Index(4, 4));
  CPPUNIT_ASSERT(tmp == center);
  tmp.toggle((a1|a8|h1|h8|d4|e4|d5|e5).M_bitmask);
  CPPUNIT_ASSERT(tmp == corners);
  tmp.toggle(a1|a8|h1|h8|d4|e4|d5|e5);
  CPPUNIT_ASSERT(tmp == center);
  tmp.toggle(center_plus_corners);
  CPPUNIT_ASSERT(tmp == corners);
}

void BitBoardTest::testTest()
{
  BitBoard bitboard;
  bitboard.reset();
  CPPUNIT_ASSERT(!bitboard.test(CW_MASK_T_CONST(0xffffffffffffffff)));
  for (int col = 0; col < 8; ++col)
    for (int row = 1; row < 8; ++row)
      CPPUNIT_ASSERT(!bitboard.test(col, row));
  bitboard.set();
  for (int col = 0; col < 8; ++col)
    for (int row = 1; row < 8; ++row)
    {
      CPPUNIT_ASSERT(bitboard.test(col, row));
      if ((col + row) % 2 == 1)
        bitboard.toggle(col, row);
    }
  for (int col = 0; col < 8; ++col)
    for (int row = 1; row < 8; ++row)
    {
      CPPUNIT_ASSERT(bitboard.test(col, row) == ((col + row) % 2 == 0));
      CPPUNIT_ASSERT(bitboard.test(Index(col, row)) == ((col + row) % 2 == 0));
    }
  CPPUNIT_ASSERT(bitboard.test(a1) && !bitboard.test(a2));
  CPPUNIT_ASSERT(bitboard.test(BitBoard(b2)) && !bitboard.test(BitBoard(b3)));
}

void BitBoardTest::testOrAndXor()
{
  mask_t masks[] = {
    CW_MASK_T_CONST(0x0000000000000000),
    CW_MASK_T_CONST(0xffffffff00000000),
    CW_MASK_T_CONST(0x0000ffff0000ffff),
    CW_MASK_T_CONST(0xff00ff00ff00ff00),
    CW_MASK_T_CONST(0x0f0f0f0f0f0f0f0f),
    CW_MASK_T_CONST(0xffffffffffffffff)
  };
  BitBoardData data1, data2;
  BitBoard bitboard1, bitboard2;
  for (int op = 0; op < 2; ++op)
  {
    for (int mask1 = 0; mask1 < 6; ++mask1)
    {
      for (int mask2 = 0; mask2 < 6; ++mask2)
      {
        mask_t result(0);
	data1.M_bitmask = masks[mask1];
	bitboard1 = data1;
	data2.M_bitmask = masks[mask2];
	bitboard2 = data2;
	BitBoard result1(bitboard1);
	BitBoard result2(bitboard1);
	BitBoard result3(bitboard1);
	BitBoard result4(0), result5(0), result6(0), result7(0);
	switch (op)
	{
	  case 0:
	    result = masks[mask1] | masks[mask2];
            result1 |= bitboard2;
	    result2 |= data2;
	    result3 |= masks[mask2];
	    result4 = bitboard1 | bitboard2;
	    result5 = bitboard1 | data2;
	    result6 = data1 | bitboard2;
	    result7 = data1 | data2;
	    break;
	  case 1:
	    result = masks[mask1] & masks[mask2];
            result1 &= bitboard2;
	    result2 &= data2;
	    result3 &= masks[mask2];
	    result4 = bitboard1 & bitboard2;
	    result5 = bitboard1 & data2;
	    result6 = data1 & bitboard2;
	    result7 = data1 & data2;
	    break;
	  case 2:
	    result = masks[mask1] ^ masks[mask2];
            result1 ^= bitboard2;
	    result2 ^= data2;
	    result3 ^= masks[mask2];
	    result4 = bitboard1 ^ bitboard2;
	    result5 = bitboard1 ^ data2;
	    result6 = data1 ^ bitboard2;
	    result7 = data1 ^ data2;
	    break;
	}
	CPPUNIT_ASSERT(result1() == result);
	CPPUNIT_ASSERT(result2() == result);
	CPPUNIT_ASSERT(result3() == result);
	CPPUNIT_ASSERT(result4() == result);
	CPPUNIT_ASSERT(result5() == result);
	CPPUNIT_ASSERT(result6() == result);
	CPPUNIT_ASSERT(result7() == result);
	CPPUNIT_ASSERT(!result1 == !result);
      }
    }
  }
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
