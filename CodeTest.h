// cwchessboard -- A C++ chessboard tool set
//
//! @file CodeTest.h Testsuite header for class Code.
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

#ifndef CODE_TEST_H
#define CODE_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "Code.h"

namespace testsuite {

using namespace cwchess;

class CodeTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CodeTest);

  CPPUNIT_TEST(testDefaultconstructor); 
  CPPUNIT_TEST(testCopyconstructorEquality); 
  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testEquality); 
  CPPUNIT_TEST(testAssignment); 
  CPPUNIT_TEST(testInspection); 
  CPPUNIT_TEST(testManipulation); 
  CPPUNIT_TEST(testTests); 

  CPPUNIT_TEST_SUITE_END();

  private:
    Code M_default_constructed;		// Default constructed [nothing]
    Code M_const_black_pawn;		// Constructed from constant [black_pawn].
    Code M_const_black_rook;		// Constructed from constant [black_rook].
    Code M_const_black_knight;		// Constructed from constant [black_knight].
    Code M_const_black_bishop;		// Constructed from constant [black_bishop].
    Code M_const_black_queen;		// Constructed from constant [black_queen].
    Code M_const_black_king;		// Constructed from constant [black_king].
    Code M_const_white_pawn;		// Constructed from constant [white_pawn].
    Code M_const_white_rook;		// Constructed from constant [white_rook].
    Code M_const_white_knight;		// Constructed from constant [white_knight].
    Code M_const_white_bishop;		// Constructed from constant [white_bishop].
    Code M_const_white_queen;		// Constructed from constant [white_queen].
    Code M_const_white_king;		// Constructed from constant [white_king].

    Type types[7];
    Color colors[2];

  public:
    CodeTest() :
	M_const_black_pawn(black_pawn),
	M_const_black_rook(black_rook),
	M_const_black_knight(black_knight),
	M_const_black_bishop(black_bishop),
	M_const_black_queen(black_queen),
	M_const_black_king(black_king),
	M_const_white_pawn(white_pawn),
	M_const_white_rook(white_rook),
	M_const_white_knight(white_knight),
	M_const_white_bishop(white_bishop),
	M_const_white_queen(white_queen),
	M_const_white_king(white_king) { }

    void setUp();
    void tearDown();

    void testDefaultconstructor();
    void testCopyconstructorEquality();
    void testConstruction();
    void testEquality();
    void testAssignment();
    void testInspection();
    void testManipulation();
    void testTests();
};

} // namespace testsuite

#endif // CODE_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

CPPUNIT_TEST_SUITE_REGISTRATION(CodeTest);

void CodeTest::setUp()
{
  types[0] = nothing;
  types[1] = pawn;
  types[2] = rook;
  types[3] = knight;
  types[4] = bishop;
  types[5] = queen;
  types[6] = king;

  colors[0] = black;
  colors[1] = white;
}

void CodeTest::tearDown()
{
}

void CodeTest::testDefaultconstructor()
{
  // Attempt to create something that we perceive as 'nothing'.
  Code code_cleared;
  code_cleared.clear();
  CPPUNIT_ASSERT(code_cleared.color() == black);			// This is illegal, but we do it anyway to make sure that clear() worked.
  CPPUNIT_ASSERT(code_cleared.type() == nothing);
  CPPUNIT_ASSERT(code_cleared.is_nothing());

  CPPUNIT_ASSERT(code_cleared == M_default_constructed);		// Default-constructed Code must be equal to nothing. [nothing]
  CPPUNIT_ASSERT(!(code_cleared != M_default_constructed));		// operator!= but return the opposite. [nothing]
}

void CodeTest::testCopyconstructorEquality()
{
  Code code_nothing(M_default_constructed);				// Copy constructor
  CPPUNIT_ASSERT(code_nothing.is_nothing());				// Copy-constructed Code must be equal to prototype. [nothing]
  Code code_black_pawn(M_const_black_pawn);				// Copy constructor
  CPPUNIT_ASSERT(code_black_pawn == M_const_black_pawn);		// Copy-constructed Code must be equal to prototype. [black_pawn]
  CPPUNIT_ASSERT(!(code_black_pawn != M_const_black_pawn));		// operator!= but return the opposite. [black_pawn]
  Code code_black_rook(M_const_black_rook);				// Copy constructor
  CPPUNIT_ASSERT(code_black_rook == M_const_black_rook);		// Copy-constructed Code must be equal to prototype. [black_rook]
  CPPUNIT_ASSERT(!(code_black_rook != M_const_black_rook));		// operator!= but return the opposite. [black_rook]
  Code code_black_knight(M_const_black_knight);				// Copy constructor
  CPPUNIT_ASSERT(code_black_knight == M_const_black_knight);		// Copy-constructed Code must be equal to prototype. [black_knight]
  CPPUNIT_ASSERT(!(code_black_knight != M_const_black_knight));		// operator!= but return the opposite. [black_knight]
  Code code_black_bishop(M_const_black_bishop);				// Copy constructor
  CPPUNIT_ASSERT(code_black_bishop == M_const_black_bishop);		// Copy-constructed Code must be equal to prototype. [black_bishop]
  CPPUNIT_ASSERT(!(code_black_bishop != M_const_black_bishop));		// operator!= but return the opposite. [black_bishop]
  Code code_black_queen(M_const_black_queen);				// Copy constructor
  CPPUNIT_ASSERT(code_black_queen == M_const_black_queen);		// Copy-constructed Code must be equal to prototype. [black_queen]
  CPPUNIT_ASSERT(!(code_black_queen != M_const_black_queen));		// operator!= but return the opposite. [black_queen]
  Code code_black_king(M_const_black_king);				// Copy constructor
  CPPUNIT_ASSERT(code_black_king == M_const_black_king);		// Copy-constructed Code must be equal to prototype. [black_king]
  CPPUNIT_ASSERT(!(code_black_king != M_const_black_king));		// operator!= but return the opposite. [black_king]
  Code code_white_pawn(M_const_white_pawn);				// Copy constructor
  CPPUNIT_ASSERT(code_white_pawn == M_const_white_pawn);		// Copy-constructed Code must be equal to prototype. [white_pawn]
  CPPUNIT_ASSERT(!(code_white_pawn != M_const_white_pawn));		// operator!= but return the opposite. [white_pawn]
  Code code_white_rook(M_const_white_rook);				// Copy constructor
  CPPUNIT_ASSERT(code_white_rook == M_const_white_rook);		// Copy-constructed Code must be equal to prototype. [white_rook]
  CPPUNIT_ASSERT(!(code_white_rook != M_const_white_rook));		// operator!= but return the opposite. [white_rook]
  Code code_white_knight(M_const_white_knight);				// Copy constructor
  CPPUNIT_ASSERT(code_white_knight == M_const_white_knight);		// Copy-constructed Code must be equal to prototype. [white_knight]
  CPPUNIT_ASSERT(!(code_white_knight != M_const_white_knight));		// operator!= but return the opposite. [white_knight]
  Code code_white_bishop(M_const_white_bishop);				// Copy constructor
  CPPUNIT_ASSERT(code_white_bishop == M_const_white_bishop);		// Copy-constructed Code must be equal to prototype. [white_bishop]
  CPPUNIT_ASSERT(!(code_white_bishop != M_const_white_bishop));		// operator!= but return the opposite. [white_bishop]
  Code code_white_queen(M_const_white_queen);				// Copy constructor
  CPPUNIT_ASSERT(code_white_queen == M_const_white_queen);		// Copy-constructed Code must be equal to prototype. [white_queen]
  CPPUNIT_ASSERT(!(code_white_queen != M_const_white_queen));		// operator!= but return the opposite. [white_queen]
  Code code_white_king(M_const_white_king);				// Copy constructor
  CPPUNIT_ASSERT(code_white_king == M_const_white_king);		// Copy-constructed Code must be equal to prototype. [white_king]
  CPPUNIT_ASSERT(!(code_white_king != M_const_white_king));		// operator!= but return the opposite. [white_king]
}

void CodeTest::testConstruction()
{
  for (int color_index = 0; color_index <= 1; ++color_index)
  {
    for (int type_index = 1; type_index < 7; ++type_index)
    {
      Code code(colors[color_index], types[type_index]);
      switch (2 * type_index + color_index)
      {
	case 2:
	  CPPUNIT_ASSERT(code == black_pawn);
	  break;
	case 3:
	  CPPUNIT_ASSERT(code == white_pawn);
	  break;
	case 4:
	  CPPUNIT_ASSERT(code == black_rook);
	  break;
	case 5:
	  CPPUNIT_ASSERT(code == white_rook);
	  break;
	case 6:
	  CPPUNIT_ASSERT(code == black_knight);
	  break;
	case 7:
	  CPPUNIT_ASSERT(code == white_knight);
	  break;
	case 8:
	  CPPUNIT_ASSERT(code == black_bishop);
	  break;
	case 9:
	  CPPUNIT_ASSERT(code == white_bishop);
	  break;
	case 10:
	  CPPUNIT_ASSERT(code == black_queen);
	  break;
	case 11:
	  CPPUNIT_ASSERT(code == white_queen);
	  break;
	case 12:
	  CPPUNIT_ASSERT(code == black_king);
	  break;
	case 13:
	  CPPUNIT_ASSERT(code == white_king);
	  break;
      }
    }
  }
}

class CodeWithAccess : public Code {
  public:
    CodeData const& code_data(void) const { return *static_cast<CodeData const*>(this); }
};

void CodeTest::testEquality()
{
  for (int color_index1 = 0; color_index1 <= 1; ++color_index1)
  {
    for (int type_index1 = 1; type_index1 < 7; ++type_index1)
    {
      Code code1(colors[color_index1], types[type_index1]);
      CodeData const& data1(static_cast<CodeWithAccess*>(&code1)->code_data());
      for (int color_index2 = 0; color_index2 <= 1; ++color_index2)
      {
	for (int type_index2 = 1; type_index2 < 7; ++type_index2)
	{
	  Code code2(colors[color_index2], types[type_index2]);
	  CodeData const& data2(static_cast<CodeWithAccess*>(&code2)->code_data());
	  CPPUNIT_ASSERT((code1 == code2) == (color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((code1 != code2) == !(color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((code1 == data2) == (color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((code1 != data2) == !(color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((data1 == code2) == (color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((data1 != code2) == !(color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((data1 == data2) == (color_index1 == color_index2 && type_index1 == type_index2)); 
	  CPPUNIT_ASSERT((data1 != data2) == !(color_index1 == color_index2 && type_index1 == type_index2)); 
	}
      }
      CPPUNIT_ASSERT(code1 != M_default_constructed);
      CPPUNIT_ASSERT(!(code1 == M_default_constructed));
      CPPUNIT_ASSERT(data1 != M_default_constructed);
      CPPUNIT_ASSERT(!(data1 == M_default_constructed));
    }
  }
}

void CodeTest::testAssignment()
{
  Code code;
  for (int color_index1 = 0; color_index1 <= 1; ++color_index1)
  {
    for (int type_index1 = 1; type_index1 < 7; ++type_index1)
    {
      Code code1(colors[color_index1], types[type_index1]);
      CodeData const& data1(static_cast<CodeWithAccess*>(&code1)->code_data());
      code = code1;
      CPPUNIT_ASSERT(code == code1);
      code = data1;
      CPPUNIT_ASSERT(code == data1);
    }
  }
  code = M_default_constructed;
  CPPUNIT_ASSERT(code == M_default_constructed);
}

void CodeTest::testInspection()
{
  for (int color_index1 = 0; color_index1 <= 1; ++color_index1)
  {
    for (int type_index1 = 1; type_index1 < 7; ++type_index1)
    {
      Code code1(colors[color_index1], types[type_index1]);
      CPPUNIT_ASSERT(code1.color() == colors[color_index1]);
      CPPUNIT_ASSERT(code1.type() == types[type_index1]);
    }
  }
}

void CodeTest::testManipulation()
{
  for (int color_index1 = 0; color_index1 <= 1; ++color_index1)
  {
    for (int type_index1 = 1; type_index1 < 7; ++type_index1)
    {
      Code code1(colors[color_index1], types[type_index1]);
      code1.toggle_color();
      CPPUNIT_ASSERT(code1.color() != colors[color_index1]);
      CPPUNIT_ASSERT(code1.type() == types[type_index1]);
      code1 = colors[color_index1];
      CPPUNIT_ASSERT(code1.color() == colors[color_index1]);
      CPPUNIT_ASSERT(code1.type() == types[type_index1]);
      code1 = colors[1 - color_index1];
      CPPUNIT_ASSERT(code1.color() != colors[color_index1]);
      CPPUNIT_ASSERT(code1.type() == types[type_index1]);
      for (int type_index2 = 1; type_index2 < 7; ++type_index2)
      {
        code1 = types[type_index2];
	CPPUNIT_ASSERT(code1.color() == colors[1 - color_index1]);
	CPPUNIT_ASSERT(code1.type() == types[type_index2]);
      }
      CPPUNIT_ASSERT(!code1.is_nothing());
      code1.clear();
      CPPUNIT_ASSERT(code1.is_nothing());
    }
  }
}

class ColorWithAccess : public Color {
  public:
    ColorData const& color_data(void) const { return *static_cast<ColorData const*>(this); }
};

class TypeWithAccess : public Type {
  public:
    TypeData const& type_data(void) const { return *static_cast<TypeData const*>(this); }
};

void CodeTest::testTests()
{
  for (int color_index1 = 0; color_index1 <= 1; ++color_index1)
  {
    ColorData const& color_data1(static_cast<ColorWithAccess*>(&colors[color_index1])->color_data());
    for (int type_index1 = 1; type_index1 < 7; ++type_index1)
    {
      TypeData const& type_data1(static_cast<TypeWithAccess*>(&types[type_index1])->type_data());
      Code code1(colors[color_index1], types[type_index1]);
      CPPUNIT_ASSERT(code1.is_a_slider() == code1.type().is_a_slider());
      CPPUNIT_ASSERT(code1.is_a_rookmover() == code1.type().is_a_rookmover());
      CPPUNIT_ASSERT(code1.is_a_bishopmover() == code1.type().is_a_bishopmover());
      CPPUNIT_ASSERT(code1.is_a(types[type_index1]));
      CPPUNIT_ASSERT(code1.is_a(type_data1));
      for (int type_index2 = 1; type_index2 < 7; ++type_index2)
      {
        if (type_index2 == type_index1)
	  continue;
	TypeData const& type_data2(static_cast<TypeWithAccess*>(&types[type_index2])->type_data());
	CPPUNIT_ASSERT(!code1.is_a(types[type_index2]));
	CPPUNIT_ASSERT(!code1.is_a(type_data2));
      }
      CPPUNIT_ASSERT(code1.is(colors[color_index1]));
      CPPUNIT_ASSERT(code1.is(color_data1));
      for (int color_index2 = 0; color_index2 <= 1; ++color_index2)
      {
        if (color_index2 == color_index1)
	  continue;
	ColorData const& color_data2(static_cast<ColorWithAccess*>(&colors[color_index2])->color_data());
	CPPUNIT_ASSERT(!code1.is(colors[color_index2]));
	CPPUNIT_ASSERT(!code1.is(color_data2));
      }
    }
  }
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
