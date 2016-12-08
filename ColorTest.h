// cwchessboard -- A C++ chessboard tool set
//
//! @file ColorTest.h Testsuite header for class Color.
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

#ifndef COLOR_TEST_H
#define COLOR_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "Color.h"

namespace testsuite {

using namespace cwchess;

class ColorTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ColorTest);

  CPPUNIT_TEST(testCopyconstructorEquality); 
  CPPUNIT_TEST(testEquality); 
  CPPUNIT_TEST(testAssignment); 
  CPPUNIT_TEST(testTests); 
  CPPUNIT_TEST(testAccessors); 

  CPPUNIT_TEST_SUITE_END();

  private:
    Color M_default_constructed;	// Default constructed.
    Color M_const_black;		// Constructed from constant [black].
    Color M_const_white;		// Constructed from constant [white].

  public:
    ColorTest() : M_const_black(black), M_const_white(white) { }

    void setUp();
    void tearDown();

    void testCopyconstructorEquality();
    void testEquality();
    void testAssignment();
    void testTests();
    void testAccessors(); 
};

} // namespace testsuite

#endif // COLOR_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

CPPUNIT_TEST_SUITE_REGISTRATION(ColorTest);

void ColorTest::setUp()
{
}

void ColorTest::tearDown()
{
}

void ColorTest::testCopyconstructorEquality()
{
  Color color_black(M_const_black);			// Copy constructor
  CPPUNIT_ASSERT(color_black == M_const_black);		// Copy-constructed Color must be equal to prototype. [black]
  CPPUNIT_ASSERT(!(color_black != M_const_black));	// operator!= but return the opposite. [black]
  Color color_white(M_const_white);			// Copy constructor
  CPPUNIT_ASSERT(color_white == M_const_white);		// Copy-constructed Color must be equal to prototype. [white]
  CPPUNIT_ASSERT(!(color_white != M_const_white));	// operator!= but return the opposite. [white]
}

void ColorTest::testEquality()
{
  CPPUNIT_ASSERT(M_const_black == M_const_black);
  CPPUNIT_ASSERT(M_const_black != M_const_white);
  CPPUNIT_ASSERT(M_const_white != M_const_black);
  CPPUNIT_ASSERT(M_const_white == M_const_white);
  CPPUNIT_ASSERT(M_const_black == black);
  CPPUNIT_ASSERT(black == M_const_black);
  CPPUNIT_ASSERT(M_const_black != white);
  CPPUNIT_ASSERT(white != M_const_black);
  CPPUNIT_ASSERT(M_const_white != black);
  CPPUNIT_ASSERT(black != M_const_white);
  CPPUNIT_ASSERT(M_const_white == white);
  CPPUNIT_ASSERT(white == M_const_white);
  CPPUNIT_ASSERT(black == black);
  CPPUNIT_ASSERT(black != white);
  CPPUNIT_ASSERT(white != black);
  CPPUNIT_ASSERT(white == white);
}

void ColorTest::testAssignment()
{
  M_default_constructed = M_const_black;
  CPPUNIT_ASSERT(M_default_constructed == M_const_black);
  M_default_constructed = M_const_white;
  CPPUNIT_ASSERT(M_default_constructed == M_const_white);
  Color color;
  color = black;
  CPPUNIT_ASSERT(color == black);
  color = white;
  CPPUNIT_ASSERT(color == white);
}

void ColorTest::testTests()
{
  CPPUNIT_ASSERT(M_const_black.is_black());
  CPPUNIT_ASSERT(!M_const_black.is_white());
  CPPUNIT_ASSERT(!M_const_white.is_black());
  CPPUNIT_ASSERT(M_const_white.is_white());
}

void ColorTest::testAccessors()
{
  CPPUNIT_ASSERT(M_const_black() == 0);
  CPPUNIT_ASSERT(M_const_white() == 8);
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
