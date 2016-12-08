// cwchessboard -- A C++ chessboard tool set
//
//! @file FlagsTest.h Testsuite header for class Flags.
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

#ifndef FLAGS_TEST_H
#define FLAGS_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "Flags.h"

namespace testsuite {

using namespace cwchess;

class FlagsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FlagsTest);

  CPPUNIT_TEST(testCopyconstructorEquality); 
  CPPUNIT_TEST(testEquality); 
  CPPUNIT_TEST(testAssignment); 

  CPPUNIT_TEST_SUITE_END();

  private:
    Flags M_default_constructed;		// Default constructed.
    Flags M_const_pawn_can_take_queen_side;		// Constructed from constant [fl_pawn_can_take_queen_side].
    Flags M_const_pawn_is_not_blocked;		// Constructed from constant [fl_pawn_is_not_blocked].
    Flags M_const_pawn_can_take_king_side;		// Constructed from constant [fl_pawn_can_take_king_side].
    Flags M_const_pawn_can_move_two_squares;		// Constructed from constant [fl_pawn_can_move_two_squares].

  public:
    FlagsTest() :
	M_const_pawn_can_take_queen_side(fl_pawn_can_take_queen_side),
	M_const_pawn_is_not_blocked(fl_pawn_is_not_blocked),
	M_const_pawn_can_take_king_side(fl_pawn_can_take_king_side),
	M_const_pawn_can_move_two_squares(fl_pawn_can_move_two_squares) { }

    void setUp();
    void tearDown();

    void testCopyconstructorEquality();
    void testEquality();
    void testAssignment();
};

} // namespace testsuite

#endif // INDEX_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

CPPUNIT_TEST_SUITE_REGISTRATION(FlagsTest);

void FlagsTest::setUp()
{
}

void FlagsTest::tearDown()
{
}

void FlagsTest::testCopyconstructorEquality()
{
  Flags flags_pawn_can_take_queen_side(M_const_pawn_can_take_queen_side);				// Copy constructor
  CPPUNIT_ASSERT(flags_pawn_can_take_queen_side == M_const_pawn_can_take_queen_side);		// Copy-constructed Flags must be equal to prototype. [fl_pawn_can_take_queen_side]
  CPPUNIT_ASSERT(!(flags_pawn_can_take_queen_side != M_const_pawn_can_take_queen_side));		// operator!= but return the opposite. [fl_pawn_can_take_queen_side]
  Flags flags_pawn_is_not_blocked(M_const_pawn_is_not_blocked);				// Copy constructor
  CPPUNIT_ASSERT(flags_pawn_is_not_blocked == M_const_pawn_is_not_blocked);			// Copy-constructed Flags must be equal to prototype. [fl_pawn_is_not_blocked]
  CPPUNIT_ASSERT(!(flags_pawn_is_not_blocked != M_const_pawn_is_not_blocked));		// operator!= but return the opposite. [fl_pawn_is_not_blocked]
  Flags flags_pawn_can_take_king_side(M_const_pawn_can_take_king_side);					// Copy constructor
  CPPUNIT_ASSERT(flags_pawn_can_take_king_side == M_const_pawn_can_take_king_side);				// Copy-constructed Flags must be equal to prototype. [fl_pawn_can_take_king_side]
  CPPUNIT_ASSERT(!(flags_pawn_can_take_king_side != M_const_pawn_can_take_king_side));			// operator!= but return the opposite. [fl_pawn_can_take_king_side]
  Flags flags_pawn_can_move_two_squares(M_const_pawn_can_move_two_squares);				// Copy constructor
  CPPUNIT_ASSERT(flags_pawn_can_move_two_squares == M_const_pawn_can_move_two_squares);			// Copy-constructed Flags must be equal to prototype. [fl_pawn_can_move_two_squares]
  CPPUNIT_ASSERT(!(flags_pawn_can_move_two_squares != M_const_pawn_can_move_two_squares));		// operator!= but return the opposite. [fl_pawn_can_move_two_squares]
}

void FlagsTest::testEquality()
{
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side == M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side != M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side != M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side != M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked != M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked == M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked != M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked != M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side != M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side != M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side == M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side != M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares != M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares != M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares != M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares == M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side != M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side == M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side == M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side == M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked == M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked != M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked == M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked == M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side == M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side == M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side != M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side == M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares == M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares == M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares == M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares != M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side == fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side != fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side != fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_queen_side != fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked != fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked == fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked != fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_is_not_blocked != fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side != fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side != fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side == fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_can_take_king_side != fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares != fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares != fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares != fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_const_pawn_can_move_two_squares == fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side != fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side == fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side == fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_queen_side == fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked == fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked != fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked == fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_is_not_blocked == fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side == fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side == fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side != fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_take_king_side == fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares == fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares == fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares == fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(M_const_pawn_can_move_two_squares != fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side == M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side != M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side != M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side != M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked != M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked == M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked != M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked != M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side != M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side != M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side == M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side != M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares != M_const_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares != M_const_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares != M_const_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares == M_const_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side != M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side == M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side == M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side == M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked == M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked != M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked == M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked == M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side == M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side == M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side != M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side == M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares == M_const_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares == M_const_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares == M_const_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares != M_const_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side == fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side != fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side != fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_queen_side != fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked != fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked == fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked != fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_is_not_blocked != fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side != fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side != fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side == fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_can_take_king_side != fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares != fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares != fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares != fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(fl_pawn_can_move_two_squares == fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side != fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side == fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side == fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_queen_side == fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked == fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked != fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked == fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_is_not_blocked == fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side == fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side == fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side != fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_take_king_side == fl_pawn_can_move_two_squares));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares == fl_pawn_can_take_queen_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares == fl_pawn_is_not_blocked));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares == fl_pawn_can_take_king_side));
  CPPUNIT_ASSERT(!(fl_pawn_can_move_two_squares != fl_pawn_can_move_two_squares));
}

void FlagsTest::testAssignment()
{
  M_default_constructed = M_const_pawn_can_take_queen_side;
  CPPUNIT_ASSERT(M_default_constructed == M_const_pawn_can_take_queen_side);
  M_default_constructed = M_const_pawn_is_not_blocked;
  CPPUNIT_ASSERT(M_default_constructed == M_const_pawn_is_not_blocked);
  M_default_constructed = M_const_pawn_can_take_king_side;
  CPPUNIT_ASSERT(M_default_constructed == M_const_pawn_can_take_king_side);
  M_default_constructed = M_const_pawn_can_move_two_squares;
  CPPUNIT_ASSERT(M_default_constructed == M_const_pawn_can_move_two_squares);
  M_default_constructed = fl_pawn_can_take_queen_side;
  CPPUNIT_ASSERT(M_default_constructed == fl_pawn_can_take_queen_side);
  M_default_constructed = fl_pawn_is_not_blocked;
  CPPUNIT_ASSERT(M_default_constructed == fl_pawn_is_not_blocked);
  M_default_constructed = fl_pawn_can_take_king_side;
  CPPUNIT_ASSERT(M_default_constructed == fl_pawn_can_take_king_side);
  M_default_constructed = fl_pawn_can_move_two_squares;
  CPPUNIT_ASSERT(M_default_constructed == fl_pawn_can_move_two_squares);
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
