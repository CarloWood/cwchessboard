// cwchessboard -- A C++ chessboard tool set
//
//! @file IndexTest.h Testsuite header for class Index.
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

#ifndef INDEX_TEST_H
#define INDEX_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "Index.h"

namespace testsuite {

using namespace cwchess;

class IndexTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(IndexTest);

  CPPUNIT_TEST(testCopyconstructorEquality); 
  CPPUNIT_TEST(testConstruction); 
  CPPUNIT_TEST(testConstants); 
  CPPUNIT_TEST(testEquality); 
  CPPUNIT_TEST(testAssignment); 
  CPPUNIT_TEST(testIndex); 
  CPPUNIT_TEST(testBitindex);

  CPPUNIT_TEST_SUITE_END();

  private:
    Index M_default_constructed;		// Default constructed.
    Index M_const_ia1;				// Constructed from constant [ia1].
    Index M_const_ia8;				// Constructed from constant [ia8].
    Index M_const_ih1;				// Constructed from constant [ih1].
    Index M_const_ih8;				// Constructed from constant [ih8].

  public:
    IndexTest() :
	M_const_ia1(ia1),
	M_const_ia8(ia8),
	M_const_ih1(ih1),
	M_const_ih8(ih8) { }

    void setUp();
    void tearDown();

    void testCopyconstructorEquality();
    void testConstruction();
    void testConstants();
    void testEquality();
    void testAssignment();
    void testIndex();
    void testBitindex();
};

} // namespace testsuite

#endif // INDEX_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

// This line cannot be used.
CPPUNIT_TEST_SUITE_REGISTRATION(IndexTest);

void IndexTest::setUp()
{
}

void IndexTest::tearDown()
{
}

void IndexTest::testCopyconstructorEquality()
{
  Index index_a1(M_const_ia1);				// Copy constructor
  CPPUNIT_ASSERT(index_a1 == M_const_ia1);		// Copy-constructed Index must be equal to prototype. [ia1]
  CPPUNIT_ASSERT(!(index_a1 != M_const_ia1));		// operator!= but return the opposite. [ia1]
  Index index_a8(M_const_ia8);				// Copy constructor
  CPPUNIT_ASSERT(index_a8 == M_const_ia8);		// Copy-constructed Index must be equal to prototype. [ia8]
  CPPUNIT_ASSERT(!(index_a8 != M_const_ia8));		// operator!= but return the opposite. [ia8]
  Index index_h1(M_const_ih1);				// Copy constructor
  CPPUNIT_ASSERT(index_h1 == M_const_ih1);		// Copy-constructed Index must be equal to prototype. [ih1]
  CPPUNIT_ASSERT(!(index_h1 != M_const_ih1));		// operator!= but return the opposite. [ih1]
  Index index_h8(M_const_ih8);				// Copy constructor
  CPPUNIT_ASSERT(index_h8 == M_const_ih8);		// Copy-constructed Index must be equal to prototype. [ih8]
  CPPUNIT_ASSERT(!(index_h8 != M_const_ih8));		// operator!= but return the opposite. [ih8]
}

void IndexTest::testConstruction()
{
  Index pre_begin(index_pre_begin);
  Index end(index_end);
  CPPUNIT_ASSERT(pre_begin != end);
  for (int col1 = 0; col1 < 8; ++col1)
  {
    for (int row1 = 0; row1 < 8; ++row1)
    {
      Index index1(col1, row1);
      CPPUNIT_ASSERT(index1.col() == col1);
      CPPUNIT_ASSERT(index1.row() == row1);
      CPPUNIT_ASSERT(index1 != pre_begin && index1 != end);
    }
  }
}

void IndexTest::testConstants()
{
  Index index_a1(0, 0);
  Index index_b1(1, 0);
  Index index_c1(2, 0);
  Index index_d1(3, 0);
  Index index_e1(4, 0);
  Index index_f1(5, 0);
  Index index_g1(6, 0);
  Index index_h1(7, 0);
  Index index_a2(0, 1);
  Index index_b2(1, 1);
  Index index_c2(2, 1);
  Index index_d2(3, 1);
  Index index_e2(4, 1);
  Index index_f2(5, 1);
  Index index_g2(6, 1);
  Index index_h2(7, 1);
  Index index_a3(0, 2);
  Index index_b3(1, 2);
  Index index_c3(2, 2);
  Index index_d3(3, 2);
  Index index_e3(4, 2);
  Index index_f3(5, 2);
  Index index_g3(6, 2);
  Index index_h3(7, 2);
  Index index_a4(0, 3);
  Index index_b4(1, 3);
  Index index_c4(2, 3);
  Index index_d4(3, 3);
  Index index_e4(4, 3);
  Index index_f4(5, 3);
  Index index_g4(6, 3);
  Index index_h4(7, 3);
  Index index_a5(0, 4);
  Index index_b5(1, 4);
  Index index_c5(2, 4);
  Index index_d5(3, 4);
  Index index_e5(4, 4);
  Index index_f5(5, 4);
  Index index_g5(6, 4);
  Index index_h5(7, 4);
  Index index_a6(0, 5);
  Index index_b6(1, 5);
  Index index_c6(2, 5);
  Index index_d6(3, 5);
  Index index_e6(4, 5);
  Index index_f6(5, 5);
  Index index_g6(6, 5);
  Index index_h6(7, 5);
  Index index_a7(0, 6);
  Index index_b7(1, 6);
  Index index_c7(2, 6);
  Index index_d7(3, 6);
  Index index_e7(4, 6);
  Index index_f7(5, 6);
  Index index_g7(6, 6);
  Index index_h7(7, 6);
  Index index_a8(0, 7);
  Index index_b8(1, 7);
  Index index_c8(2, 7);
  Index index_d8(3, 7);
  Index index_e8(4, 7);
  Index index_f8(5, 7);
  Index index_g8(6, 7);
  Index index_h8(7, 7);
  CPPUNIT_ASSERT(index_a1 == ia1);
  CPPUNIT_ASSERT(index_b1 == ib1);
  CPPUNIT_ASSERT(index_c1 == ic1);
  CPPUNIT_ASSERT(index_d1 == id1);
  CPPUNIT_ASSERT(index_e1 == ie1);
  CPPUNIT_ASSERT(index_f1 == if1);
  CPPUNIT_ASSERT(index_g1 == ig1);
  CPPUNIT_ASSERT(index_h1 == ih1);
  CPPUNIT_ASSERT(index_a2 == ia2);
  CPPUNIT_ASSERT(index_b2 == ib2);
  CPPUNIT_ASSERT(index_c2 == ic2);
  CPPUNIT_ASSERT(index_d2 == id2);
  CPPUNIT_ASSERT(index_e2 == ie2);
  CPPUNIT_ASSERT(index_f2 == if2);
  CPPUNIT_ASSERT(index_g2 == ig2);
  CPPUNIT_ASSERT(index_h2 == ih2);
  CPPUNIT_ASSERT(index_a3 == ia3);
  CPPUNIT_ASSERT(index_b3 == ib3);
  CPPUNIT_ASSERT(index_c3 == ic3);
  CPPUNIT_ASSERT(index_d3 == id3);
  CPPUNIT_ASSERT(index_e3 == ie3);
  CPPUNIT_ASSERT(index_f3 == if3);
  CPPUNIT_ASSERT(index_g3 == ig3);
  CPPUNIT_ASSERT(index_h3 == ih3);
  CPPUNIT_ASSERT(index_a4 == ia4);
  CPPUNIT_ASSERT(index_b4 == ib4);
  CPPUNIT_ASSERT(index_c4 == ic4);
  CPPUNIT_ASSERT(index_d4 == id4);
  CPPUNIT_ASSERT(index_e4 == ie4);
  CPPUNIT_ASSERT(index_f4 == if4);
  CPPUNIT_ASSERT(index_g4 == ig4);
  CPPUNIT_ASSERT(index_h4 == ih4);
  CPPUNIT_ASSERT(index_a5 == ia5);
  CPPUNIT_ASSERT(index_b5 == ib5);
  CPPUNIT_ASSERT(index_c5 == ic5);
  CPPUNIT_ASSERT(index_d5 == id5);
  CPPUNIT_ASSERT(index_e5 == ie5);
  CPPUNIT_ASSERT(index_f5 == if5);
  CPPUNIT_ASSERT(index_g5 == ig5);
  CPPUNIT_ASSERT(index_h5 == ih5);
  CPPUNIT_ASSERT(index_a6 == ia6);
  CPPUNIT_ASSERT(index_b6 == ib6);
  CPPUNIT_ASSERT(index_c6 == ic6);
  CPPUNIT_ASSERT(index_d6 == id6);
  CPPUNIT_ASSERT(index_e6 == ie6);
  CPPUNIT_ASSERT(index_f6 == if6);
  CPPUNIT_ASSERT(index_g6 == ig6);
  CPPUNIT_ASSERT(index_h6 == ih6);
  CPPUNIT_ASSERT(index_a7 == ia7);
  CPPUNIT_ASSERT(index_b7 == ib7);
  CPPUNIT_ASSERT(index_c7 == ic7);
  CPPUNIT_ASSERT(index_d7 == id7);
  CPPUNIT_ASSERT(index_e7 == ie7);
  CPPUNIT_ASSERT(index_f7 == if7);
  CPPUNIT_ASSERT(index_g7 == ig7);
  CPPUNIT_ASSERT(index_h7 == ih7);
  CPPUNIT_ASSERT(index_a8 == ia8);
  CPPUNIT_ASSERT(index_b8 == ib8);
  CPPUNIT_ASSERT(index_c8 == ic8);
  CPPUNIT_ASSERT(index_d8 == id8);
  CPPUNIT_ASSERT(index_e8 == ie8);
  CPPUNIT_ASSERT(index_f8 == if8);
  CPPUNIT_ASSERT(index_g8 == ig8);
  CPPUNIT_ASSERT(index_h8 == ih8);
}

class IndexWithAccess : public Index {
  public:
    IndexData const& index_data(void) const { return *static_cast<IndexData const*>(this); }
};

void IndexTest::testEquality()
{
  for (int col1 = 0; col1 < 8; ++col1)
  {
    for (int row1 = 0; row1 < 8; ++row1)
    {
      Index index1(col1, row1);
      IndexData const& data1(static_cast<IndexWithAccess*>(&index1)->index_data());
      for (int col2 = 0; col2 < 8; ++col2)
      {
	for (int row2 = 0; row2 < 8; ++row2)
	{
	  Index index2(col2, row2);
	  IndexData const& data2(static_cast<IndexWithAccess*>(&index2)->index_data());
          CPPUNIT_ASSERT((index1 == index2) == (col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((index1 != index2) == !(col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((index1 == data2) == (col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((index1 != data2) == !(col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((data1 == index2) == (col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((data1 != index2) == !(col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((data1 == data2) == (col1 == col2 && row1 == row2));
          CPPUNIT_ASSERT((data1 != data2) == !(col1 == col2 && row1 == row2));
	}
      }
    }
  }
}

void IndexTest::testAssignment()
{
  Index index;
  for (int col1 = 0; col1 < 8; ++col1)
  {
    for (int row1 = 0; row1 < 8; ++row1)
    {
      Index index1(col1, row1);
      IndexData const& data1(static_cast<IndexWithAccess*>(&index1)->index_data());
      index = index1;
      CPPUNIT_ASSERT(index == index1);
      index = data1;
      CPPUNIT_ASSERT(index == data1);
    }
  }
}

void IndexTest::testIndex()
{
  Index index;
  for (int col1 = 0; col1 < 8; ++col1)
  {
    for (int row1 = 0; row1 < 8; ++row1)
    {
      Index index1(col1, row1);
      CPPUNIT_ASSERT((index1() % 8) == col1);
      CPPUNIT_ASSERT((index1() / 8) == row1);
    }
  }
}

void IndexTest::testBitindex()
{
  uint64_t const bitboard1111111111111111111111111111111111111111111111111111111111111111 = CW_MASK_T_CONST(0xffffffffffffffff);
  uint64_t const bitboard1001100110011001100110011001100110011001100110011001100110011001 = CW_MASK_T_CONST(0x9999999999999999);
  uint64_t const bitboard0000000000000000000000000000000000000000000000000000000000000000 = CW_MASK_T_CONST(0x0000000000000000);
  uint64_t const bitboard0000000000000000000000000000000000000000000000000000000000000001 = CW_MASK_T_CONST(0x0000000000000001);
  uint64_t const bitboard0000000000000000000000000000000000000000000000000000000000000010 = CW_MASK_T_CONST(0x0000000000000002);
  uint64_t const bitboard0100000000000000000000000000000000000000000000000000000000000000 = CW_MASK_T_CONST(0x4000000000000000);
  uint64_t const bitboard0100000000000000000000000000000000000000000000000000000000000001 = CW_MASK_T_CONST(0x4000000000000001);
  uint64_t const bitboard0100000000000000000000000000000000000000000000000000000000000010 = CW_MASK_T_CONST(0x4000000000000002);
  uint64_t const bitboard1000000000000000000000000000000000000000000000000000000000000000 = CW_MASK_T_CONST(0x8000000000000000);
  uint64_t const bitboard1000000000000000000000000000000000000000000000000000000000000001 = CW_MASK_T_CONST(0x8000000000000001);
  uint64_t const bitboard1000000000000000000000000000000000000000000000000000000000000010 = CW_MASK_T_CONST(0x8000000000000002);
  for (uint8_t start_index = 0xff; start_index != 64; ++start_index)
  {
    uint8_t start_index_plus_one = start_index + 1;
    IndexData const data = { start_index };
    Index bitindex(data);
    bitindex.next_bit_in(bitboard1111111111111111111111111111111111111111111111111111111111111111);
    CPPUNIT_ASSERT(bitindex() == start_index_plus_one);
    bitindex = data;
    bitindex.next_bit_in(bitboard1001100110011001100110011001100110011001100110011001100110011001);
    CPPUNIT_ASSERT(bitindex() == (start_index_plus_one - (start_index_plus_one % 4) + ((start_index_plus_one % 4) ? 3 : 0)));
    bitindex = data;
    bitindex.next_bit_in(bitboard0000000000000000000000000000000000000000000000000000000000000000);
    CPPUNIT_ASSERT(bitindex() == 64);
    bitindex = data;
    bitindex.next_bit_in(bitboard0000000000000000000000000000000000000000000000000000000000000001);
    CPPUNIT_ASSERT(bitindex() == ((start_index == 0xff) ? 0 : 64));
    bitindex = data;
    bitindex.next_bit_in(bitboard0000000000000000000000000000000000000000000000000000000000000010);
    CPPUNIT_ASSERT(bitindex() == ((start_index == 0xff || start_index == 0) ? 1 : 64));
    bitindex = data;
    bitindex.next_bit_in(bitboard0100000000000000000000000000000000000000000000000000000000000000);
    CPPUNIT_ASSERT(bitindex() == ((start_index_plus_one > 62) ? 64 : 62));
    bitindex = data;
    bitindex.next_bit_in(bitboard0100000000000000000000000000000000000000000000000000000000000001);
    CPPUNIT_ASSERT(bitindex() == ((start_index_plus_one > 62) ? 64 : (start_index == 0xff) ? 0 : 62));
    bitindex = data;
    bitindex.next_bit_in(bitboard0100000000000000000000000000000000000000000000000000000000000010);
    CPPUNIT_ASSERT(bitindex() == ((start_index_plus_one > 62) ? 64 : (start_index == 0xff || start_index == 0) ? 1 : 62));
    bitindex = data;
    bitindex.next_bit_in(bitboard1000000000000000000000000000000000000000000000000000000000000000);
    CPPUNIT_ASSERT(bitindex() == ((start_index == 63) ? 64 : 63));
    bitindex = data;
    bitindex.next_bit_in(bitboard1000000000000000000000000000000000000000000000000000000000000001);
    CPPUNIT_ASSERT(bitindex() == ((start_index == 63) ? 64 : (start_index == 0xff) ? 0 : 63));
    bitindex = data;
    bitindex.next_bit_in(bitboard1000000000000000000000000000000000000000000000000000000000000010);
    CPPUNIT_ASSERT(bitindex() == ((start_index == 63) ? 64 : (start_index == 0xff || start_index == 0) ? 1 : 63));
  }
  for (uint8_t start_index = 1; start_index <= 64; ++start_index)
  {
    IndexData const data = { start_index };
    Index bitindex(data);
    bitindex.prev_bit_in(bitboard1111111111111111111111111111111111111111111111111111111111111111);
    CPPUNIT_ASSERT(bitindex() == start_index - 1);
    bitindex = data;
    bitindex.prev_bit_in(bitboard1001100110011001100110011001100110011001100110011001100110011001);
    CPPUNIT_ASSERT(bitindex() == (start_index - 1 - ((start_index - 1) % 4) + (((start_index - 1) % 4 == 3) ? 3 : 0)));
    bitindex = data;
    bitindex.prev_bit_in(bitboard0000000000000000000000000000000000000000000000000000000000000000);
    CPPUNIT_ASSERT(bitindex() == 0xff);
    bitindex = data;
    bitindex.prev_bit_in(bitboard0000000000000000000000000000000000000000000000000000000000000001);
    CPPUNIT_ASSERT(bitindex() == 0);
    bitindex = data;
    bitindex.prev_bit_in(bitboard0000000000000000000000000000000000000000000000000000000000000010);
    CPPUNIT_ASSERT(bitindex() == ((start_index == 1) ? 0xff : 1));
    bitindex = data;
    bitindex.prev_bit_in(bitboard0100000000000000000000000000000000000000000000000000000000000000);
    CPPUNIT_ASSERT(bitindex() == ((start_index > 62) ? 62 : 0xff));
    bitindex = data;
    bitindex.prev_bit_in(bitboard0100000000000000000000000000000000000000000000000000000000000001);
    CPPUNIT_ASSERT(bitindex() == ((start_index > 62) ? 62 : 0));
    bitindex = data;
    bitindex.prev_bit_in(bitboard0100000000000000000000000000000000000000000000000000000000000010);
    CPPUNIT_ASSERT(bitindex() == ((start_index > 62) ? 62 : ((start_index == 1) ? 0xff : 1)));
    bitindex = data;
    bitindex.prev_bit_in(bitboard1000000000000000000000000000000000000000000000000000000000000000);
    CPPUNIT_ASSERT(bitindex() == ((start_index > 63) ? 63 : 0xff));
    bitindex = data;
    bitindex.prev_bit_in(bitboard1000000000000000000000000000000000000000000000000000000000000001);
    CPPUNIT_ASSERT(bitindex() == ((start_index > 63) ? 63 : 0));
    bitindex = data;
    bitindex.prev_bit_in(bitboard1000000000000000000000000000000000000000000000000000000000000010);
    CPPUNIT_ASSERT(bitindex() == ((start_index > 63) ? 63 : ((start_index == 1) ? 0xff : 1)));
  }
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
