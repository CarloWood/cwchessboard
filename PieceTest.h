// cwchessboard -- A C++ chessboard tool set
//
//! @file PieceTest.h Testsuite header for class Piece.
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

#ifndef PIECE_TEST_H
#define PIECE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

namespace testsuite {
  class PieceTest;
}

#include "Piece.h"

namespace testsuite {

using namespace cwchess;

class PieceTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PieceTest);

  CPPUNIT_TEST(testConstruction);
  CPPUNIT_TEST(testCopyconstructor);
  CPPUNIT_TEST(testAssignment);
  CPPUNIT_TEST(testEquality);
  CPPUNIT_TEST(testSet);

  CPPUNIT_TEST_SUITE_END();

  private:
    Piece M_empty_piece;		// Default constructed.
    Piece M_white_pawn_piece;
    Piece M_black_queen_piece;
    Piece M_white_knight_piece;
    Piece M_black_rook_piece;

  public:
    PieceTest() : M_white_pawn_piece(white, pawn), M_black_queen_piece(black, queen, fl_pawn_can_take_king_side),
        M_white_knight_piece(white_knight, fl_pawn_is_not_blocked), M_black_rook_piece(black_rook, fl_pawn_can_take_queen_side) { }

    void setUp();
    void tearDown();

    void testConstruction();
    void testCopyconstructor();
    void testAssignment();
    void testEquality();
    void testSet();
};

} // namespace testsuite

#endif // PIECE_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

CPPUNIT_TEST_SUITE_REGISTRATION(PieceTest);

void PieceTest::setUp()
{
}

void PieceTest::tearDown()
{
}

void PieceTest::testConstruction()
{
  CPPUNIT_ASSERT(M_empty_piece.code().is_nothing());
  CPPUNIT_ASSERT(M_empty_piece.flags() == fl_none);
  CPPUNIT_ASSERT(M_white_pawn_piece.code() == white_pawn);
  CPPUNIT_ASSERT(M_white_pawn_piece.color() == white);
  CPPUNIT_ASSERT(M_white_pawn_piece.type() == pawn);
  CPPUNIT_ASSERT(M_white_pawn_piece.flags() == fl_none);
  CPPUNIT_ASSERT(M_black_queen_piece.code() == black_queen);
  CPPUNIT_ASSERT(M_black_queen_piece.color() == black);
  CPPUNIT_ASSERT(M_black_queen_piece.type() == queen);
  CPPUNIT_ASSERT(M_black_queen_piece.flags() == fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(M_white_knight_piece.code() == white_knight);
  CPPUNIT_ASSERT(M_white_knight_piece.color() == white);
  CPPUNIT_ASSERT(M_white_knight_piece.type() == knight);
  CPPUNIT_ASSERT(M_white_knight_piece.flags() == fl_pawn_is_not_blocked);
  CPPUNIT_ASSERT(M_black_rook_piece.code() == black_rook);
  CPPUNIT_ASSERT(M_black_rook_piece.color() == black);
  CPPUNIT_ASSERT(M_black_rook_piece.type() == rook);
  CPPUNIT_ASSERT(M_black_rook_piece.flags() == fl_pawn_can_take_queen_side);
}

void PieceTest::testCopyconstructor()
{
  Piece empty_piece(M_empty_piece);
  CPPUNIT_ASSERT(empty_piece.code().is_nothing());
  CPPUNIT_ASSERT(empty_piece.flags() == fl_none);
  Piece white_pawn_piece(M_white_pawn_piece);
  CPPUNIT_ASSERT(white_pawn_piece.code() == white_pawn);
  CPPUNIT_ASSERT(white_pawn_piece.color() == white);
  CPPUNIT_ASSERT(white_pawn_piece.type() == pawn);
  CPPUNIT_ASSERT(white_pawn_piece.flags() == fl_none);
  Piece black_queen_piece(M_black_queen_piece);
  CPPUNIT_ASSERT(black_queen_piece.code() == black_queen);
  CPPUNIT_ASSERT(black_queen_piece.color() == black);
  CPPUNIT_ASSERT(black_queen_piece.type() == queen);
  CPPUNIT_ASSERT(black_queen_piece.flags() == fl_pawn_can_take_king_side);
  Piece white_knight_piece(M_white_knight_piece);
  CPPUNIT_ASSERT(white_knight_piece.code() == white_knight);
  CPPUNIT_ASSERT(white_knight_piece.color() == white);
  CPPUNIT_ASSERT(white_knight_piece.type() == knight);
  CPPUNIT_ASSERT(white_knight_piece.flags() == fl_pawn_is_not_blocked);
  Piece black_rook_piece(M_black_rook_piece);
  CPPUNIT_ASSERT(black_rook_piece.code() == black_rook);
  CPPUNIT_ASSERT(black_rook_piece.color() == black);
  CPPUNIT_ASSERT(black_rook_piece.type() == rook);
  CPPUNIT_ASSERT(black_rook_piece.flags() == fl_pawn_can_take_queen_side);
}

void PieceTest::testAssignment()
{
  Piece empty_piece(white, queen, fl_pawn_can_move_two_squares);
  empty_piece = M_empty_piece;
  CPPUNIT_ASSERT(empty_piece.code().is_nothing());
  CPPUNIT_ASSERT(empty_piece.flags() == fl_none);
  Piece white_pawn_piece;
  white_pawn_piece = M_white_pawn_piece;
  CPPUNIT_ASSERT(white_pawn_piece.code() == white_pawn);
  CPPUNIT_ASSERT(white_pawn_piece.color() == white);
  CPPUNIT_ASSERT(white_pawn_piece.type() == pawn);
  CPPUNIT_ASSERT(white_pawn_piece.flags() == fl_none);
  Piece black_queen_piece;
  black_queen_piece = M_black_queen_piece;
  CPPUNIT_ASSERT(black_queen_piece.code() == black_queen);
  CPPUNIT_ASSERT(black_queen_piece.color() == black);
  CPPUNIT_ASSERT(black_queen_piece.type() == queen);
  CPPUNIT_ASSERT(black_queen_piece.flags() == fl_pawn_can_take_king_side);
  Piece white_knight_piece;
  white_knight_piece = M_white_knight_piece;
  CPPUNIT_ASSERT(white_knight_piece.code() == white_knight);
  CPPUNIT_ASSERT(white_knight_piece.color() == white);
  CPPUNIT_ASSERT(white_knight_piece.type() == knight);
  CPPUNIT_ASSERT(white_knight_piece.flags() == fl_pawn_is_not_blocked);
  Piece black_rook_piece;
  black_rook_piece = M_black_rook_piece;
  CPPUNIT_ASSERT(black_rook_piece.code() == black_rook);
  CPPUNIT_ASSERT(black_rook_piece.color() == black);
  CPPUNIT_ASSERT(black_rook_piece.type() == rook);
  CPPUNIT_ASSERT(black_rook_piece.flags() == fl_pawn_can_take_queen_side);
}

void PieceTest::testEquality()
{
  Piece piece1(white, rook);
  CPPUNIT_ASSERT(piece1 != nothing && nothing != piece1);
  CPPUNIT_ASSERT(piece1 != black && black != piece1);
  CPPUNIT_ASSERT(piece1 == white && white == piece1);
  CPPUNIT_ASSERT(piece1 == rook && rook == piece1);
  CPPUNIT_ASSERT(piece1 != pawn && pawn != piece1);
  CPPUNIT_ASSERT(piece1 == white_rook && white_rook == piece1);
  CPPUNIT_ASSERT(piece1 != black_rook && black_rook != piece1);
  Piece piece2(white, pawn);
  CPPUNIT_ASSERT(piece2 == piece2 && piece1 != piece2);
  CPPUNIT_ASSERT(piece2 != nothing && nothing != piece2);
  CPPUNIT_ASSERT(piece2 != black && black != piece2);
  CPPUNIT_ASSERT(piece2 == white && white == piece2);
  CPPUNIT_ASSERT(piece2 != rook && rook != piece2);
  CPPUNIT_ASSERT(piece2 == pawn && pawn == piece2);
  CPPUNIT_ASSERT(piece2 == white_pawn && white_pawn == piece2);
  CPPUNIT_ASSERT(piece2 != black_pawn && black_pawn != piece2);
  Piece piece3(black, king, fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(piece3 == piece3 && piece3 != piece1 && piece3 != piece2);
  CPPUNIT_ASSERT(piece3 != nothing && nothing != piece3);
  CPPUNIT_ASSERT(piece3 == black && black == piece3);
  CPPUNIT_ASSERT(piece3 != white && white != piece3);
  CPPUNIT_ASSERT(piece3 != rook && rook != piece3);
  CPPUNIT_ASSERT(piece3 == king && king == piece3);
  CPPUNIT_ASSERT(piece3 == black_king && black_king == piece3);
  CPPUNIT_ASSERT(piece3 != black_pawn && black_pawn != piece3);
  Piece piece4(black, king, fl_pawn_can_take_king_side);
  CPPUNIT_ASSERT(piece3 == piece4);
  CPPUNIT_ASSERT(!(piece3 != piece4));
}

void PieceTest::testSet()
{
  Piece piece(black, king, fl_pawn_can_move_two_squares);
  CPPUNIT_ASSERT(piece.flags() == fl_pawn_can_move_two_squares);
  piece.set_flags(fl_none);
  CPPUNIT_ASSERT(piece.flags() == fl_none);
  piece.set_flags(fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(piece.flags() == fl_pawn_can_take_queen_side);
  piece.set_type(pawn);
  CPPUNIT_ASSERT(piece.flags() == fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(piece.code() == black_pawn);
  piece = white_king;
  piece.set_type(pawn);
  CPPUNIT_ASSERT(piece.flags() == fl_pawn_can_take_queen_side);
  CPPUNIT_ASSERT(piece.code() == white_pawn);
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
