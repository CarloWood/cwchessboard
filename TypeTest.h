// cwchessboard -- A C++ chessboard tool set
//
//! @file TypeTest.h Testsuite header for class Type.
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

#ifndef TYPE_TEST_H
#define TYPE_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "Type.h"

namespace testsuite {

using namespace cwchess;

class TypeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TypeTest);

  CPPUNIT_TEST(testCopyconstructorEquality); 
  CPPUNIT_TEST(testEquality); 
  CPPUNIT_TEST(testAssignment); 
  CPPUNIT_TEST(testTests); 

  CPPUNIT_TEST_SUITE_END();

  private:
    Type M_default_constructed;		// Default constructed.
    Type M_const_nothing;		// Constructed from constant [nothing].
    Type M_const_pawn;			// Constructed from constant [pawn].
    Type M_const_rook;			// Constructed from constant [rook].
    Type M_const_knight;		// Constructed from constant [knight].
    Type M_const_bishop;		// Constructed from constant [bishop].
    Type M_const_queen;			// Constructed from constant [queen].
    Type M_const_king;			// Constructed from constant [king].

  public:
    TypeTest() :
        M_const_nothing(nothing),
	M_const_pawn(pawn),
	M_const_rook(rook),
	M_const_knight(knight),
	M_const_bishop(bishop),
	M_const_queen(queen),
	M_const_king(king){ }

    void setUp();
    void tearDown();

    void testCopyconstructorEquality();
    void testEquality();
    void testAssignment();
    void testTests();
};

} // namespace testsuite

#endif // TYPE_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

CPPUNIT_TEST_SUITE_REGISTRATION(TypeTest);

void TypeTest::setUp()
{
}

void TypeTest::tearDown()
{
}

void TypeTest::testCopyconstructorEquality()
{
  Type type_nothing(M_const_nothing);			// Copy constructor
  CPPUNIT_ASSERT(type_nothing == M_const_nothing);	// Copy-constructed Type must be equal to prototype. [nothing]
  CPPUNIT_ASSERT(!(type_nothing != M_const_nothing));	// operator!= but return the opposite. [nothing]
  Type type_pawn(M_const_pawn);				// Copy constructor
  CPPUNIT_ASSERT(type_pawn == M_const_pawn);		// Copy-constructed Type must be equal to prototype. [pawn]
  CPPUNIT_ASSERT(!(type_pawn != M_const_pawn));		// operator!= but return the opposite. [pawn]
  Type type_rook(M_const_rook);				// Copy constructor
  CPPUNIT_ASSERT(type_rook == M_const_rook);		// Copy-constructed Type must be equal to prototype. [rook]
  CPPUNIT_ASSERT(!(type_rook != M_const_rook));		// operator!= but return the opposite. [rook]
  Type type_knight(M_const_knight);			// Copy constructor
  CPPUNIT_ASSERT(type_knight == M_const_knight);	// Copy-constructed Type must be equal to prototype. [knight]
  CPPUNIT_ASSERT(!(type_knight != M_const_knight));	// operator!= but return the opposite. [knight]
  Type type_bishop(M_const_bishop);			// Copy constructor
  CPPUNIT_ASSERT(type_bishop == M_const_bishop);	// Copy-constructed Type must be equal to prototype. [bishop]
  CPPUNIT_ASSERT(!(type_bishop != M_const_bishop));	// operator!= but return the opposite. [bishop]
  Type type_queen(M_const_queen);			// Copy constructor
  CPPUNIT_ASSERT(type_queen == M_const_queen);		// Copy-constructed Type must be equal to prototype. [queen]
  CPPUNIT_ASSERT(!(type_queen != M_const_queen));	// operator!= but return the opposite. [queen]
  Type type_king(M_const_king);				// Copy constructor
  CPPUNIT_ASSERT(type_king == M_const_king);		// Copy-constructed Type must be equal to prototype. [king]
  CPPUNIT_ASSERT(!(type_king != M_const_king));		// operator!= but return the opposite. [king]
}

void TypeTest::testEquality()
{
  CPPUNIT_ASSERT(M_const_nothing == M_const_nothing);
  CPPUNIT_ASSERT(M_const_nothing != M_const_pawn);
  CPPUNIT_ASSERT(M_const_nothing != M_const_rook);
  CPPUNIT_ASSERT(M_const_nothing != M_const_knight);
  CPPUNIT_ASSERT(M_const_nothing != M_const_bishop);
  CPPUNIT_ASSERT(M_const_nothing != M_const_queen);
  CPPUNIT_ASSERT(M_const_nothing != M_const_king);
  CPPUNIT_ASSERT(M_const_pawn != M_const_nothing);
  CPPUNIT_ASSERT(M_const_pawn == M_const_pawn);
  CPPUNIT_ASSERT(M_const_pawn != M_const_rook);
  CPPUNIT_ASSERT(M_const_pawn != M_const_knight);
  CPPUNIT_ASSERT(M_const_pawn != M_const_bishop);
  CPPUNIT_ASSERT(M_const_pawn != M_const_queen);
  CPPUNIT_ASSERT(M_const_pawn != M_const_king);
  CPPUNIT_ASSERT(M_const_rook != M_const_nothing);
  CPPUNIT_ASSERT(M_const_rook != M_const_pawn);
  CPPUNIT_ASSERT(M_const_rook == M_const_rook);
  CPPUNIT_ASSERT(M_const_rook != M_const_knight);
  CPPUNIT_ASSERT(M_const_rook != M_const_bishop);
  CPPUNIT_ASSERT(M_const_rook != M_const_queen);
  CPPUNIT_ASSERT(M_const_rook != M_const_king);
  CPPUNIT_ASSERT(M_const_knight != M_const_nothing);
  CPPUNIT_ASSERT(M_const_knight != M_const_pawn);
  CPPUNIT_ASSERT(M_const_knight != M_const_rook);
  CPPUNIT_ASSERT(M_const_knight == M_const_knight);
  CPPUNIT_ASSERT(M_const_knight != M_const_bishop);
  CPPUNIT_ASSERT(M_const_knight != M_const_queen);
  CPPUNIT_ASSERT(M_const_knight != M_const_king);
  CPPUNIT_ASSERT(M_const_bishop != M_const_nothing);
  CPPUNIT_ASSERT(M_const_bishop != M_const_pawn);
  CPPUNIT_ASSERT(M_const_bishop != M_const_rook);
  CPPUNIT_ASSERT(M_const_bishop != M_const_knight);
  CPPUNIT_ASSERT(M_const_bishop == M_const_bishop);
  CPPUNIT_ASSERT(M_const_bishop != M_const_queen);
  CPPUNIT_ASSERT(M_const_bishop != M_const_king);
  CPPUNIT_ASSERT(M_const_queen != M_const_nothing);
  CPPUNIT_ASSERT(M_const_queen != M_const_pawn);
  CPPUNIT_ASSERT(M_const_queen != M_const_rook);
  CPPUNIT_ASSERT(M_const_queen != M_const_knight);
  CPPUNIT_ASSERT(M_const_queen != M_const_bishop);
  CPPUNIT_ASSERT(M_const_queen == M_const_queen);
  CPPUNIT_ASSERT(M_const_queen != M_const_king);
  CPPUNIT_ASSERT(M_const_king != M_const_nothing);
  CPPUNIT_ASSERT(M_const_king != M_const_pawn);
  CPPUNIT_ASSERT(M_const_king != M_const_rook);
  CPPUNIT_ASSERT(M_const_king != M_const_knight);
  CPPUNIT_ASSERT(M_const_king != M_const_bishop);
  CPPUNIT_ASSERT(M_const_king != M_const_queen);
  CPPUNIT_ASSERT(M_const_king == M_const_king);
  CPPUNIT_ASSERT(!(M_const_nothing != M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_nothing == M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_nothing == M_const_rook));
  CPPUNIT_ASSERT(!(M_const_nothing == M_const_knight));
  CPPUNIT_ASSERT(!(M_const_nothing == M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_nothing == M_const_queen));
  CPPUNIT_ASSERT(!(M_const_nothing == M_const_king));
  CPPUNIT_ASSERT(!(M_const_pawn == M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_pawn != M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_pawn == M_const_rook));
  CPPUNIT_ASSERT(!(M_const_pawn == M_const_knight));
  CPPUNIT_ASSERT(!(M_const_pawn == M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_pawn == M_const_queen));
  CPPUNIT_ASSERT(!(M_const_pawn == M_const_king));
  CPPUNIT_ASSERT(!(M_const_rook == M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_rook == M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_rook != M_const_rook));
  CPPUNIT_ASSERT(!(M_const_rook == M_const_knight));
  CPPUNIT_ASSERT(!(M_const_rook == M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_rook == M_const_queen));
  CPPUNIT_ASSERT(!(M_const_rook == M_const_king));
  CPPUNIT_ASSERT(!(M_const_knight == M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_knight == M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_knight == M_const_rook));
  CPPUNIT_ASSERT(!(M_const_knight != M_const_knight));
  CPPUNIT_ASSERT(!(M_const_knight == M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_knight == M_const_queen));
  CPPUNIT_ASSERT(!(M_const_knight == M_const_king));
  CPPUNIT_ASSERT(!(M_const_bishop == M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_bishop == M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_bishop == M_const_rook));
  CPPUNIT_ASSERT(!(M_const_bishop == M_const_knight));
  CPPUNIT_ASSERT(!(M_const_bishop != M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_bishop == M_const_queen));
  CPPUNIT_ASSERT(!(M_const_bishop == M_const_king));
  CPPUNIT_ASSERT(!(M_const_queen == M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_queen == M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_queen == M_const_rook));
  CPPUNIT_ASSERT(!(M_const_queen == M_const_knight));
  CPPUNIT_ASSERT(!(M_const_queen == M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_queen != M_const_queen));
  CPPUNIT_ASSERT(!(M_const_queen == M_const_king));
  CPPUNIT_ASSERT(!(M_const_king == M_const_nothing));
  CPPUNIT_ASSERT(!(M_const_king == M_const_pawn));
  CPPUNIT_ASSERT(!(M_const_king == M_const_rook));
  CPPUNIT_ASSERT(!(M_const_king == M_const_knight));
  CPPUNIT_ASSERT(!(M_const_king == M_const_bishop));
  CPPUNIT_ASSERT(!(M_const_king == M_const_queen));
  CPPUNIT_ASSERT(!(M_const_king != M_const_king));
  CPPUNIT_ASSERT(nothing == M_const_nothing);
  CPPUNIT_ASSERT(nothing != M_const_pawn);
  CPPUNIT_ASSERT(nothing != M_const_rook);
  CPPUNIT_ASSERT(nothing != M_const_knight);
  CPPUNIT_ASSERT(nothing != M_const_bishop);
  CPPUNIT_ASSERT(nothing != M_const_queen);
  CPPUNIT_ASSERT(nothing != M_const_king);
  CPPUNIT_ASSERT(pawn != M_const_nothing);
  CPPUNIT_ASSERT(pawn == M_const_pawn);
  CPPUNIT_ASSERT(pawn != M_const_rook);
  CPPUNIT_ASSERT(pawn != M_const_knight);
  CPPUNIT_ASSERT(pawn != M_const_bishop);
  CPPUNIT_ASSERT(pawn != M_const_queen);
  CPPUNIT_ASSERT(pawn != M_const_king);
  CPPUNIT_ASSERT(rook != M_const_nothing);
  CPPUNIT_ASSERT(rook != M_const_pawn);
  CPPUNIT_ASSERT(rook == M_const_rook);
  CPPUNIT_ASSERT(rook != M_const_knight);
  CPPUNIT_ASSERT(rook != M_const_bishop);
  CPPUNIT_ASSERT(rook != M_const_queen);
  CPPUNIT_ASSERT(rook != M_const_king);
  CPPUNIT_ASSERT(knight != M_const_nothing);
  CPPUNIT_ASSERT(knight != M_const_pawn);
  CPPUNIT_ASSERT(knight != M_const_rook);
  CPPUNIT_ASSERT(knight == M_const_knight);
  CPPUNIT_ASSERT(knight != M_const_bishop);
  CPPUNIT_ASSERT(knight != M_const_queen);
  CPPUNIT_ASSERT(knight != M_const_king);
  CPPUNIT_ASSERT(bishop != M_const_nothing);
  CPPUNIT_ASSERT(bishop != M_const_pawn);
  CPPUNIT_ASSERT(bishop != M_const_rook);
  CPPUNIT_ASSERT(bishop != M_const_knight);
  CPPUNIT_ASSERT(bishop == M_const_bishop);
  CPPUNIT_ASSERT(bishop != M_const_queen);
  CPPUNIT_ASSERT(bishop != M_const_king);
  CPPUNIT_ASSERT(queen != M_const_nothing);
  CPPUNIT_ASSERT(queen != M_const_pawn);
  CPPUNIT_ASSERT(queen != M_const_rook);
  CPPUNIT_ASSERT(queen != M_const_knight);
  CPPUNIT_ASSERT(queen != M_const_bishop);
  CPPUNIT_ASSERT(queen == M_const_queen);
  CPPUNIT_ASSERT(queen != M_const_king);
  CPPUNIT_ASSERT(king != M_const_nothing);
  CPPUNIT_ASSERT(king != M_const_pawn);
  CPPUNIT_ASSERT(king != M_const_rook);
  CPPUNIT_ASSERT(king != M_const_knight);
  CPPUNIT_ASSERT(king != M_const_bishop);
  CPPUNIT_ASSERT(king != M_const_queen);
  CPPUNIT_ASSERT(king == M_const_king);
  CPPUNIT_ASSERT(!(nothing != M_const_nothing));
  CPPUNIT_ASSERT(!(nothing == M_const_pawn));
  CPPUNIT_ASSERT(!(nothing == M_const_rook));
  CPPUNIT_ASSERT(!(nothing == M_const_knight));
  CPPUNIT_ASSERT(!(nothing == M_const_bishop));
  CPPUNIT_ASSERT(!(nothing == M_const_queen));
  CPPUNIT_ASSERT(!(nothing == M_const_king));
  CPPUNIT_ASSERT(!(pawn == M_const_nothing));
  CPPUNIT_ASSERT(!(pawn != M_const_pawn));
  CPPUNIT_ASSERT(!(pawn == M_const_rook));
  CPPUNIT_ASSERT(!(pawn == M_const_knight));
  CPPUNIT_ASSERT(!(pawn == M_const_bishop));
  CPPUNIT_ASSERT(!(pawn == M_const_queen));
  CPPUNIT_ASSERT(!(pawn == M_const_king));
  CPPUNIT_ASSERT(!(rook == M_const_nothing));
  CPPUNIT_ASSERT(!(rook == M_const_pawn));
  CPPUNIT_ASSERT(!(rook != M_const_rook));
  CPPUNIT_ASSERT(!(rook == M_const_knight));
  CPPUNIT_ASSERT(!(rook == M_const_bishop));
  CPPUNIT_ASSERT(!(rook == M_const_queen));
  CPPUNIT_ASSERT(!(rook == M_const_king));
  CPPUNIT_ASSERT(!(knight == M_const_nothing));
  CPPUNIT_ASSERT(!(knight == M_const_pawn));
  CPPUNIT_ASSERT(!(knight == M_const_rook));
  CPPUNIT_ASSERT(!(knight != M_const_knight));
  CPPUNIT_ASSERT(!(knight == M_const_bishop));
  CPPUNIT_ASSERT(!(knight == M_const_queen));
  CPPUNIT_ASSERT(!(knight == M_const_king));
  CPPUNIT_ASSERT(!(bishop == M_const_nothing));
  CPPUNIT_ASSERT(!(bishop == M_const_pawn));
  CPPUNIT_ASSERT(!(bishop == M_const_rook));
  CPPUNIT_ASSERT(!(bishop == M_const_knight));
  CPPUNIT_ASSERT(!(bishop != M_const_bishop));
  CPPUNIT_ASSERT(!(bishop == M_const_queen));
  CPPUNIT_ASSERT(!(bishop == M_const_king));
  CPPUNIT_ASSERT(!(queen == M_const_nothing));
  CPPUNIT_ASSERT(!(queen == M_const_pawn));
  CPPUNIT_ASSERT(!(queen == M_const_rook));
  CPPUNIT_ASSERT(!(queen == M_const_knight));
  CPPUNIT_ASSERT(!(queen == M_const_bishop));
  CPPUNIT_ASSERT(!(queen != M_const_queen));
  CPPUNIT_ASSERT(!(queen == M_const_king));
  CPPUNIT_ASSERT(!(king == M_const_nothing));
  CPPUNIT_ASSERT(!(king == M_const_pawn));
  CPPUNIT_ASSERT(!(king == M_const_rook));
  CPPUNIT_ASSERT(!(king == M_const_knight));
  CPPUNIT_ASSERT(!(king == M_const_bishop));
  CPPUNIT_ASSERT(!(king == M_const_queen));
  CPPUNIT_ASSERT(M_const_nothing == nothing);
  CPPUNIT_ASSERT(M_const_nothing != pawn);
  CPPUNIT_ASSERT(M_const_nothing != rook);
  CPPUNIT_ASSERT(M_const_nothing != knight);
  CPPUNIT_ASSERT(M_const_nothing != bishop);
  CPPUNIT_ASSERT(M_const_nothing != queen);
  CPPUNIT_ASSERT(M_const_nothing != king);
  CPPUNIT_ASSERT(M_const_pawn != nothing);
  CPPUNIT_ASSERT(M_const_pawn == pawn);
  CPPUNIT_ASSERT(M_const_pawn != rook);
  CPPUNIT_ASSERT(M_const_pawn != knight);
  CPPUNIT_ASSERT(M_const_pawn != bishop);
  CPPUNIT_ASSERT(M_const_pawn != queen);
  CPPUNIT_ASSERT(M_const_pawn != king);
  CPPUNIT_ASSERT(M_const_rook != nothing);
  CPPUNIT_ASSERT(M_const_rook != pawn);
  CPPUNIT_ASSERT(M_const_rook == rook);
  CPPUNIT_ASSERT(M_const_rook != knight);
  CPPUNIT_ASSERT(M_const_rook != bishop);
  CPPUNIT_ASSERT(M_const_rook != queen);
  CPPUNIT_ASSERT(M_const_rook != king);
  CPPUNIT_ASSERT(M_const_knight != nothing);
  CPPUNIT_ASSERT(M_const_knight != pawn);
  CPPUNIT_ASSERT(M_const_knight != rook);
  CPPUNIT_ASSERT(M_const_knight == knight);
  CPPUNIT_ASSERT(M_const_knight != bishop);
  CPPUNIT_ASSERT(M_const_knight != queen);
  CPPUNIT_ASSERT(M_const_knight != king);
  CPPUNIT_ASSERT(M_const_bishop != nothing);
  CPPUNIT_ASSERT(M_const_bishop != pawn);
  CPPUNIT_ASSERT(M_const_bishop != rook);
  CPPUNIT_ASSERT(M_const_bishop != knight);
  CPPUNIT_ASSERT(M_const_bishop == bishop);
  CPPUNIT_ASSERT(M_const_bishop != queen);
  CPPUNIT_ASSERT(M_const_bishop != king);
  CPPUNIT_ASSERT(M_const_queen != nothing);
  CPPUNIT_ASSERT(M_const_queen != pawn);
  CPPUNIT_ASSERT(M_const_queen != rook);
  CPPUNIT_ASSERT(M_const_queen != knight);
  CPPUNIT_ASSERT(M_const_queen != bishop);
  CPPUNIT_ASSERT(M_const_queen == queen);
  CPPUNIT_ASSERT(M_const_queen != king);
  CPPUNIT_ASSERT(M_const_king != nothing);
  CPPUNIT_ASSERT(M_const_king != pawn);
  CPPUNIT_ASSERT(M_const_king != rook);
  CPPUNIT_ASSERT(M_const_king != knight);
  CPPUNIT_ASSERT(M_const_king != bishop);
  CPPUNIT_ASSERT(M_const_king != queen);
  CPPUNIT_ASSERT(M_const_king == king);
  CPPUNIT_ASSERT(!(M_const_nothing != nothing));
  CPPUNIT_ASSERT(!(M_const_nothing == pawn));
  CPPUNIT_ASSERT(!(M_const_nothing == rook));
  CPPUNIT_ASSERT(!(M_const_nothing == knight));
  CPPUNIT_ASSERT(!(M_const_nothing == bishop));
  CPPUNIT_ASSERT(!(M_const_nothing == queen));
  CPPUNIT_ASSERT(!(M_const_nothing == king));
  CPPUNIT_ASSERT(!(M_const_pawn == nothing));
  CPPUNIT_ASSERT(!(M_const_pawn != pawn));
  CPPUNIT_ASSERT(!(M_const_pawn == rook));
  CPPUNIT_ASSERT(!(M_const_pawn == knight));
  CPPUNIT_ASSERT(!(M_const_pawn == bishop));
  CPPUNIT_ASSERT(!(M_const_pawn == queen));
  CPPUNIT_ASSERT(!(M_const_pawn == king));
  CPPUNIT_ASSERT(!(M_const_rook == nothing));
  CPPUNIT_ASSERT(!(M_const_rook == pawn));
  CPPUNIT_ASSERT(!(M_const_rook != rook));
  CPPUNIT_ASSERT(!(M_const_rook == knight));
  CPPUNIT_ASSERT(!(M_const_rook == bishop));
  CPPUNIT_ASSERT(!(M_const_rook == queen));
  CPPUNIT_ASSERT(!(M_const_rook == king));
  CPPUNIT_ASSERT(!(M_const_knight == nothing));
  CPPUNIT_ASSERT(!(M_const_knight == pawn));
  CPPUNIT_ASSERT(!(M_const_knight == rook));
  CPPUNIT_ASSERT(!(M_const_knight != knight));
  CPPUNIT_ASSERT(!(M_const_knight == bishop));
  CPPUNIT_ASSERT(!(M_const_knight == queen));
  CPPUNIT_ASSERT(!(M_const_knight == king));
  CPPUNIT_ASSERT(!(M_const_bishop == nothing));
  CPPUNIT_ASSERT(!(M_const_bishop == pawn));
  CPPUNIT_ASSERT(!(M_const_bishop == rook));
  CPPUNIT_ASSERT(!(M_const_bishop == knight));
  CPPUNIT_ASSERT(!(M_const_bishop != bishop));
  CPPUNIT_ASSERT(!(M_const_bishop == queen));
  CPPUNIT_ASSERT(!(M_const_bishop == king));
  CPPUNIT_ASSERT(!(M_const_queen == nothing));
  CPPUNIT_ASSERT(!(M_const_queen == pawn));
  CPPUNIT_ASSERT(!(M_const_queen == rook));
  CPPUNIT_ASSERT(!(M_const_queen == knight));
  CPPUNIT_ASSERT(!(M_const_queen == bishop));
  CPPUNIT_ASSERT(!(M_const_queen != queen));
  CPPUNIT_ASSERT(!(M_const_queen == king));
  CPPUNIT_ASSERT(!(M_const_king == nothing));
  CPPUNIT_ASSERT(!(M_const_king == pawn));
  CPPUNIT_ASSERT(!(M_const_king == rook));
  CPPUNIT_ASSERT(!(M_const_king == knight));
  CPPUNIT_ASSERT(!(M_const_king == bishop));
  CPPUNIT_ASSERT(!(M_const_king == queen));
  CPPUNIT_ASSERT(!(M_const_king != king));
  CPPUNIT_ASSERT(nothing == nothing);
  CPPUNIT_ASSERT(nothing != pawn);
  CPPUNIT_ASSERT(nothing != rook);
  CPPUNIT_ASSERT(nothing != knight);
  CPPUNIT_ASSERT(nothing != bishop);
  CPPUNIT_ASSERT(nothing != queen);
  CPPUNIT_ASSERT(nothing != king);
  CPPUNIT_ASSERT(pawn != nothing);
  CPPUNIT_ASSERT(pawn == pawn);
  CPPUNIT_ASSERT(pawn != rook);
  CPPUNIT_ASSERT(pawn != knight);
  CPPUNIT_ASSERT(pawn != bishop);
  CPPUNIT_ASSERT(pawn != queen);
  CPPUNIT_ASSERT(pawn != king);
  CPPUNIT_ASSERT(rook != nothing);
  CPPUNIT_ASSERT(rook != pawn);
  CPPUNIT_ASSERT(rook == rook);
  CPPUNIT_ASSERT(rook != knight);
  CPPUNIT_ASSERT(rook != bishop);
  CPPUNIT_ASSERT(rook != queen);
  CPPUNIT_ASSERT(rook != king);
  CPPUNIT_ASSERT(knight != nothing);
  CPPUNIT_ASSERT(knight != pawn);
  CPPUNIT_ASSERT(knight != rook);
  CPPUNIT_ASSERT(knight == knight);
  CPPUNIT_ASSERT(knight != bishop);
  CPPUNIT_ASSERT(knight != queen);
  CPPUNIT_ASSERT(knight != king);
  CPPUNIT_ASSERT(bishop != nothing);
  CPPUNIT_ASSERT(bishop != pawn);
  CPPUNIT_ASSERT(bishop != rook);
  CPPUNIT_ASSERT(bishop != knight);
  CPPUNIT_ASSERT(bishop == bishop);
  CPPUNIT_ASSERT(bishop != queen);
  CPPUNIT_ASSERT(bishop != king);
  CPPUNIT_ASSERT(queen != nothing);
  CPPUNIT_ASSERT(queen != pawn);
  CPPUNIT_ASSERT(queen != rook);
  CPPUNIT_ASSERT(queen != knight);
  CPPUNIT_ASSERT(queen != bishop);
  CPPUNIT_ASSERT(queen == queen);
  CPPUNIT_ASSERT(queen != king);
  CPPUNIT_ASSERT(king != nothing);
  CPPUNIT_ASSERT(king != pawn);
  CPPUNIT_ASSERT(king != rook);
  CPPUNIT_ASSERT(king != knight);
  CPPUNIT_ASSERT(king != bishop);
  CPPUNIT_ASSERT(king != queen);
  CPPUNIT_ASSERT(king == king);
  CPPUNIT_ASSERT(!(nothing != nothing));
  CPPUNIT_ASSERT(!(nothing == pawn));
  CPPUNIT_ASSERT(!(nothing == rook));
  CPPUNIT_ASSERT(!(nothing == knight));
  CPPUNIT_ASSERT(!(nothing == bishop));
  CPPUNIT_ASSERT(!(nothing == queen));
  CPPUNIT_ASSERT(!(nothing == king));
  CPPUNIT_ASSERT(!(pawn == nothing));
  CPPUNIT_ASSERT(!(pawn != pawn));
  CPPUNIT_ASSERT(!(pawn == rook));
  CPPUNIT_ASSERT(!(pawn == knight));
  CPPUNIT_ASSERT(!(pawn == bishop));
  CPPUNIT_ASSERT(!(pawn == queen));
  CPPUNIT_ASSERT(!(pawn == king));
  CPPUNIT_ASSERT(!(rook == nothing));
  CPPUNIT_ASSERT(!(rook == pawn));
  CPPUNIT_ASSERT(!(rook != rook));
  CPPUNIT_ASSERT(!(rook == knight));
  CPPUNIT_ASSERT(!(rook == bishop));
  CPPUNIT_ASSERT(!(rook == queen));
  CPPUNIT_ASSERT(!(rook == king));
  CPPUNIT_ASSERT(!(knight == nothing));
  CPPUNIT_ASSERT(!(knight == pawn));
  CPPUNIT_ASSERT(!(knight == rook));
  CPPUNIT_ASSERT(!(knight != knight));
  CPPUNIT_ASSERT(!(knight == bishop));
  CPPUNIT_ASSERT(!(knight == queen));
  CPPUNIT_ASSERT(!(knight == king));
  CPPUNIT_ASSERT(!(bishop == nothing));
  CPPUNIT_ASSERT(!(bishop == pawn));
  CPPUNIT_ASSERT(!(bishop == rook));
  CPPUNIT_ASSERT(!(bishop == knight));
  CPPUNIT_ASSERT(!(bishop != bishop));
  CPPUNIT_ASSERT(!(bishop == queen));
  CPPUNIT_ASSERT(!(bishop == king));
  CPPUNIT_ASSERT(!(queen == nothing));
  CPPUNIT_ASSERT(!(queen == pawn));
  CPPUNIT_ASSERT(!(queen == rook));
  CPPUNIT_ASSERT(!(queen == knight));
  CPPUNIT_ASSERT(!(queen == bishop));
  CPPUNIT_ASSERT(!(queen != queen));
  CPPUNIT_ASSERT(!(queen == king));
  CPPUNIT_ASSERT(!(king == nothing));
  CPPUNIT_ASSERT(!(king == pawn));
  CPPUNIT_ASSERT(!(king == rook));
  CPPUNIT_ASSERT(!(king == knight));
  CPPUNIT_ASSERT(!(king == bishop));
  CPPUNIT_ASSERT(!(king == queen));
  CPPUNIT_ASSERT(!(king != king));
  CPPUNIT_ASSERT(!(king != king));
}

void TypeTest::testAssignment()
{
  M_default_constructed = M_const_nothing;
  CPPUNIT_ASSERT(M_default_constructed == M_const_nothing);
  Type type;
  type = knight;
  CPPUNIT_ASSERT(type == knight);
  type = bishop;
  CPPUNIT_ASSERT(type == bishop);
  M_default_constructed = type;
  CPPUNIT_ASSERT(M_default_constructed == bishop);
}

void TypeTest::testTests()
{
  CPPUNIT_ASSERT(!M_const_nothing.is_a_slider());
  CPPUNIT_ASSERT(!M_const_pawn.is_a_slider());
  CPPUNIT_ASSERT(M_const_rook.is_a_slider());
  CPPUNIT_ASSERT(!M_const_knight.is_a_slider());
  CPPUNIT_ASSERT(M_const_bishop.is_a_slider());
  CPPUNIT_ASSERT(M_const_queen.is_a_slider());
  CPPUNIT_ASSERT(!M_const_king.is_a_slider());
  CPPUNIT_ASSERT(!M_const_nothing.is_a_rookmover());
  CPPUNIT_ASSERT(!M_const_pawn.is_a_rookmover());
  CPPUNIT_ASSERT(M_const_rook.is_a_rookmover());
  CPPUNIT_ASSERT(!M_const_knight.is_a_rookmover());
  CPPUNIT_ASSERT(!M_const_bishop.is_a_rookmover());
  CPPUNIT_ASSERT(M_const_queen.is_a_rookmover());
  CPPUNIT_ASSERT(!M_const_king.is_a_rookmover());
  CPPUNIT_ASSERT(!M_const_nothing.is_a_bishopmover());
  CPPUNIT_ASSERT(!M_const_pawn.is_a_bishopmover());
  CPPUNIT_ASSERT(!M_const_rook.is_a_bishopmover());
  CPPUNIT_ASSERT(!M_const_knight.is_a_bishopmover());
  CPPUNIT_ASSERT(M_const_bishop.is_a_bishopmover());
  CPPUNIT_ASSERT(M_const_queen.is_a_bishopmover());
  CPPUNIT_ASSERT(!M_const_king.is_a_bishopmover());
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
