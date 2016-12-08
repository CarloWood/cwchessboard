// cwchessboard -- A C++ chessboard tool set
//
//! @file ChessPositionTest.h Testsuite header for class ChessPosition.
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

#ifndef CHESSPOSITION_TEST_H
#define CHESSPOSITION_TEST_H

#include <cppunit/extensions/HelperMacros.h>
#include "ChessPosition.h"

namespace testsuite {

using namespace cwchess;

class ChessPositionTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ChessPositionTest);

  CPPUNIT_TEST(testInitialPosition);
  CPPUNIT_TEST(testCopyconstructor);
  CPPUNIT_TEST(testFEN);
  CPPUNIT_TEST(testClear);
  CPPUNIT_TEST(testPlaceCastleFlags);
  CPPUNIT_TEST(testPlaceEnPassant);
  CPPUNIT_TEST(testPlacePinning);

  CPPUNIT_TEST_SUITE_END();

  private:
    ChessPosition M_default_constructed;		// Default constructed.

  public:
    ChessPositionTest() { }

    void setUp();
    void tearDown();

    void testInitialPosition();
    void testCopyconstructor();
    void testFEN();
    void testClear();
    void testPlaceCastleFlags();
    void testPlaceEnPassant();
    void testPlacePinning();

  private:
    void test_initial_position(ChessPosition const& chess_position);
    void test_clear(ChessPosition const& chess_position);
};

} // namespace testsuite

#endif // CHESSPOSITION_TEST_H

#ifdef TESTSUITE_IMPLEMENTATION

namespace testsuite {

CPPUNIT_TEST_SUITE_REGISTRATION(ChessPositionTest);

void ChessPositionTest::setUp()
{
}

void ChessPositionTest::tearDown()
{
}

void ChessPositionTest::test_initial_position(ChessPosition const& chess_position)
{
  CPPUNIT_ASSERT(chess_position.piece_at(ia1) == white_rook);
  CPPUNIT_ASSERT(chess_position.piece_at(ib1) == white_knight);
  CPPUNIT_ASSERT(chess_position.piece_at(ic1) == white_bishop);
  CPPUNIT_ASSERT(chess_position.piece_at(id1) == white_queen);
  CPPUNIT_ASSERT(chess_position.piece_at(ie1) == white_king);
  CPPUNIT_ASSERT(chess_position.piece_at(if1) == white_bishop);
  CPPUNIT_ASSERT(chess_position.piece_at(ig1) == white_knight);
  CPPUNIT_ASSERT(chess_position.piece_at(ih1) == white_rook);
  CPPUNIT_ASSERT(chess_position.piece_at(ia2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ib2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ic2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(id2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ie2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(if2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ig2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ih2) == white_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ia3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ib3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ic3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(id3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ie3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(if3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ig3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ih3) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ia4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ib4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ic4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(id4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ie4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(if4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ig4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ih4) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ia5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ib5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ic5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(id5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ie5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(if5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ig5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ih5) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ia6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ib6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ic6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(id6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ie6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(if6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ig6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ih6) == nothing);
  CPPUNIT_ASSERT(chess_position.piece_at(ia7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ib7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ic7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(id7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ie7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(if7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ig7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ih7) == black_pawn);
  CPPUNIT_ASSERT(chess_position.piece_at(ia8) == black_rook);
  CPPUNIT_ASSERT(chess_position.piece_at(ib8) == black_knight);
  CPPUNIT_ASSERT(chess_position.piece_at(ic8) == black_bishop);
  CPPUNIT_ASSERT(chess_position.piece_at(id8) == black_queen);
  CPPUNIT_ASSERT(chess_position.piece_at(ie8) == black_king);
  CPPUNIT_ASSERT(chess_position.piece_at(if8) == black_bishop);
  CPPUNIT_ASSERT(chess_position.piece_at(ig8) == black_knight);
  CPPUNIT_ASSERT(chess_position.piece_at(ih8) == black_rook);
  CPPUNIT_ASSERT(chess_position.to_move() == white);
  CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(white));
  CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(black));
  CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(white));
  CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(black));
  CPPUNIT_ASSERT(!chess_position.en_passant().exists());
  CPPUNIT_ASSERT(chess_position.half_move_clock() == 0);
  CPPUNIT_ASSERT(chess_position.full_move_number() == 1);
  CPPUNIT_ASSERT(chess_position.FEN() == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void ChessPositionTest::test_clear(ChessPosition const& chess_position)
{
  for (Index index = index_begin; index != index_end; ++index)
    CPPUNIT_ASSERT(chess_position.piece_at(index) == nothing);
  CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(white));
  CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(black));
  CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(white));
  CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(black));
  CPPUNIT_ASSERT(!chess_position.en_passant().exists());
  CPPUNIT_ASSERT(chess_position.half_move_clock() == 0);
  CPPUNIT_ASSERT(chess_position.full_move_number() == 1);
  CPPUNIT_ASSERT(chess_position.FEN() == "8/8/8/8/8/8/8/8 " + std::string((chess_position.to_move() == white) ? "w" : "b") + " - - 0 1");
}

void ChessPositionTest::testInitialPosition()
{
  ChessPosition chess_position;
  chess_position.initial_position();
  test_initial_position(chess_position);
}

void ChessPositionTest::testCopyconstructor()
{
  ChessPosition chess_position1;
  chess_position1.initial_position();
  ChessPosition chess_position2(chess_position1);
  test_initial_position(chess_position2);
}

void ChessPositionTest::testFEN()
{
  ChessPosition chess_position;
  chess_position.load_FEN("rq2k2r/p1pbn1p1/2n1p3/1p3pB1/PbP1pPp1/N2P4/1P1NB2P/1R1Q1RK1 b kq f3 0 14");
  CPPUNIT_ASSERT(chess_position.FEN() == "rq2k2r/p1pbn1p1/2n1p3/1p3pB1/PbP1pPp1/N2P4/1P1NB2P/1R1Q1RK1 b kq f3 0 14");
  chess_position.load_FEN("r3k1nr/1pppnppp/1b1bpqN1/pP6/Q4B2/3P3R/1PP1PPPP/R3KBN1 w Qk a6 0 69");
  CPPUNIT_ASSERT(chess_position.FEN() == "r3k1nr/1pppnppp/1b1bpqN1/pP6/Q4B2/3P3R/1PP1PPPP/R3KBN1 w Qk a6 0 69");
  chess_position.load_FEN("r3k1nr/1pppnppp/1b1bpqN1/pP6/Q4B2/3P3R/1PP1PPPP/R3KBN1 w kq a6 0 69");
  CPPUNIT_ASSERT(chess_position.FEN() == "r3k1nr/1pppnppp/1b1bpqN1/pP6/Q4B2/3P3R/1PP1PPPP/R3KBN1 w kq a6 0 69");
  chess_position.load_FEN("r3k2r/8/1p6/pPp1p1p1/R1PpPpPp/1P1P1P1P/2KB2R1/3B4 w - - 49 2");
  CPPUNIT_ASSERT(chess_position.FEN() == "r3k2r/8/1p6/pPp1p1p1/R1PpPpPp/1P1P1P1P/2KB2R1/3B4 w - - 49 2");
  chess_position.load_FEN("7r/r6p/1p2p3/pPp1Pp1k/R1Pp1PpP/1P1P2P1/2KBB1R1/8 b - h3 0 2");
  CPPUNIT_ASSERT(chess_position.FEN() == "7r/r6p/1p2p3/pPp1Pp1k/R1Pp1PpP/1P1P2P1/2KBB1R1/8 b - h3 0 2");
  chess_position.load_FEN("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
  CPPUNIT_ASSERT(chess_position.FEN() == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}

void ChessPositionTest::testClear()
{
  ChessPosition chess_position;
  chess_position.load_FEN("rq2k2r/p1pbn1p1/2n1p3/1p3pB1/PbP1pPp1/N2P4/1P1NB2P/1R1Q1RK1 b kq f3 0 14");
  chess_position.clear();
  test_clear(chess_position);
  CPPUNIT_ASSERT(chess_position.to_move() == black);
  chess_position.load_FEN("r3k2r/8/1p6/pPp1p1p1/R1PpPpPp/1P1P1P1P/2KB2R1/3B4 w - - 49 2");
  chess_position.clear();
  test_clear(chess_position);
  CPPUNIT_ASSERT(chess_position.to_move() == white);
}

void ChessPositionTest::testPlaceCastleFlags()
{
  ChessPosition chess_position;
  ChessPosition default_position;
  Color color(black), opposite_color(white);
  for (int colorc = 0; colorc <= 1; ++colorc, color.toggle(), opposite_color.toggle())
  {
    int row = 7 - 7 * colorc;
    bool value = false;
    chess_position.clear();
    default_position.clear();
    for (int i = 0; i < 2; ++i)
    {
      chess_position = default_position;
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, king), Index(4, row));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, rook), Index(0, row));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, rook), Index(7, row));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position = default_position;
      chess_position.place(Code(color, king), Index(4, row));
      chess_position.place(Code(color, rook), Index(7, row));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, rook), Index(0, row));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position = default_position;
      chess_position.place(Code(color, rook), Index(7, row));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, rook), Index(0, row));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, king), Index(4, row));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(), Index(4, row));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, king), Index(4, row));
      chess_position.place(Code(), Index(0, row));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      chess_position.place(Code(color, rook), Index(0, row));
      chess_position.place(Code(), Index(7, row));
      CPPUNIT_ASSERT(!chess_position.castle_flags().can_castle_short(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(color));
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_short(opposite_color) == value);
      CPPUNIT_ASSERT(chess_position.castle_flags().can_castle_long(opposite_color) == value);
      default_position.place(Code(opposite_color, king), Index(4, 7 - row));
      default_position.place(Code(opposite_color, rook), Index(0, 7 - row));
      default_position.place(Code(opposite_color, rook), Index(7, 7 - row));
      value = !value;
    }
  }
}

void ChessPositionTest::testPlaceEnPassant()
{
  ChessPosition chess_position;
  for (int col = 0; col < 8; col += 4 - col / 4)	// col = 0, 4, 7
  {
    std::string FEN_code;
    for (int reversed = 0; reversed < 2; ++reversed)
    {
      Index d3_, d4_, e2_, e3_, e4_, f3_, f4_;
      Color white_ = (reversed == 0) ? white : black;
      Color black_ = (reversed == 0) ? black : white;
      int r2 = (reversed == 0) ? 1 : 6;
      int r3 = (reversed == 0) ? 2 : 5;
      int r4 = (reversed == 0) ? 3 : 4;
      if (col > 0)
      {
	d3_ = Index(col - 1, r3);
	d4_ = Index(col - 1, r4);
      }
      e2_ = Index(col, r2);
      e3_ = Index(col, r3);
      e4_ = Index(col, r4);
      if (col < 7)
      {
        f3_ = Index(col + 1, r3);
	f4_ = Index(col + 1, r4);
      }
      if (reversed == 0)
      {
	if (col == 0)
	  FEN_code = "8/8/8/8/P7/8/8/8 b - a3 0 1";
	else if (col == 4)
	  FEN_code = "8/8/8/8/4P3/8/8/8 b - e3 0 1";
	else if (col == 7)
	  FEN_code = "8/8/8/8/7P/8/8/8 b - h3 0 1";
      }
      else
      {
	if (col == 0)
	  FEN_code = "8/8/8/p7/8/8/8/8 w - a6 0 1";
	else if (col == 4)
	  FEN_code = "8/8/8/4p3/8/8/8/8 w - e6 0 1";
	else if (col == 7)
	  FEN_code = "8/8/8/7p/8/8/8/8 w - h6 0 1";
      }
      chess_position.clear();
      // Place a white pawn on e4.
      chess_position.place(Code(white_, pawn), e4_);
      CPPUNIT_ASSERT(chess_position.piece_at(e4_).flags() == fl_pawn_is_not_blocked);
      CPPUNIT_ASSERT(!chess_position.en_passant().exists());
      if (col > 0)
      {
	// Place a black pawn on d4.
	chess_position.place(Code(black_, pawn), d4_);
	// The pawn on d4 cannot take the pawn on e4.
	CPPUNIT_ASSERT(!chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == fl_pawn_is_not_blocked);
      }
      // Load a position with a white pawn on e4 that was just moved there from e2.
      chess_position.load_FEN(FEN_code);
      CPPUNIT_ASSERT(chess_position.en_passant().exists());
      CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
      if (col > 0)
      {
	// Now place a black pawn on d4.
	chess_position.place(Code(black_, pawn), d4_);
	// Now this pawn can take the pawn on e4.
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == (fl_pawn_can_take_king_side|fl_pawn_is_not_blocked));
      }
      if (col < 7)
      {
	// Place a black pawn on f4.
	chess_position.place(Code(black_, pawn), f4_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	if (col > 0)
	  CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == (fl_pawn_can_take_king_side|fl_pawn_is_not_blocked));
	CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == (fl_pawn_can_take_queen_side|fl_pawn_is_not_blocked));
      }
      if (col > 0)
      {
	// Remove the pawn on d4.
	chess_position.place(Code(), d4_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	if (col < 7)
	  CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == (fl_pawn_can_take_queen_side|fl_pawn_is_not_blocked));
      }
      if (col < 7)
      {
	// Remove the pawn on f4.
	chess_position.place(Code(), f4_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	// Place a white pawn on f3.
	chess_position.place(Code(white_, pawn), f3_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	CPPUNIT_ASSERT(chess_position.piece_at(f3_).flags() == fl_pawn_is_not_blocked);
	// Place again a black pawn on f4.
	chess_position.place(Code(black_, pawn), f4_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == fl_pawn_can_take_queen_side);
	CPPUNIT_ASSERT(chess_position.piece_at(f3_).flags() == fl_none);
      }
      if (col > 0)
      {
	// Place again a black pawn on d4.
	chess_position.place(Code(black_, pawn), d4_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == (fl_pawn_can_take_king_side|fl_pawn_is_not_blocked));
	if (col < 7)
	{
	  CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == fl_pawn_can_take_queen_side);
	  CPPUNIT_ASSERT(chess_position.piece_at(f3_).flags() == fl_none);
	}
	// Place a black pawn on d3.
	chess_position.place(Code(black_, pawn), d3_);
	CPPUNIT_ASSERT(chess_position.en_passant().exists());
	CPPUNIT_ASSERT(chess_position.en_passant().index() == e3_);
	CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == fl_pawn_can_take_king_side);
	CPPUNIT_ASSERT(chess_position.piece_at(d3_).flags() == fl_pawn_is_not_blocked);
	if (col < 7)
	{
	  CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == fl_pawn_can_take_queen_side);
	  CPPUNIT_ASSERT(chess_position.piece_at(f3_).flags() == fl_none);
	}
      }
      // Place a white piece on e3.
      chess_position.place(Code(white_, knight), e3_);
      CPPUNIT_ASSERT(!chess_position.en_passant().exists());
      CPPUNIT_ASSERT(chess_position.piece_at(e4_).flags() == fl_pawn_is_not_blocked);
      if (col > 0)
      {
	CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == fl_pawn_can_take_king_side);
	CPPUNIT_ASSERT(chess_position.piece_at(d3_).flags() == fl_pawn_is_not_blocked);
      }
      if (col < 7)
      {
	CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == fl_pawn_can_take_queen_side);
	CPPUNIT_ASSERT(chess_position.piece_at(f3_).flags() == fl_none);
      }
      // Remove the piece again.
      chess_position.place(Code(), e3_);
      CPPUNIT_ASSERT(!chess_position.en_passant().exists());
      CPPUNIT_ASSERT(chess_position.piece_at(e4_).flags() == fl_pawn_is_not_blocked);
      if (col > 0)
      {
	CPPUNIT_ASSERT(chess_position.piece_at(d4_).flags() == fl_none);
	CPPUNIT_ASSERT(chess_position.piece_at(d3_).flags() == fl_pawn_is_not_blocked);
      }
      if (col < 7)
      {
	CPPUNIT_ASSERT(chess_position.piece_at(f4_).flags() == fl_none);
	CPPUNIT_ASSERT(chess_position.piece_at(f3_).flags() == fl_none);
      }
      // Restore position with e4.
      chess_position.load_FEN(FEN_code);
      // Place a piece on e2.
      chess_position.place(Code(black_, knight), e2_);
      CPPUNIT_ASSERT(!chess_position.en_passant().exists());
    }
  }
}

void ChessPositionTest::testPlacePinning()
{
  ChessPosition chess_position;
  PieceIterator const piece_end(chess_position.piece_end());
  char const* FEN_codes[] = {
    "8/7Q/7r/5p2/1R1pP2k/7r/8/K7 b - e3 0 1",
    "5r2/7R/3k2qR/4pP2/1r1PK3/B7/8/8 w - e3 0 1",
    "5r2/7R/3k2qR/4pP2/1r2n3/Br1P1K2/8/8 w - e3 0 1",
    "8/2R5/6b1/R1rkp3/4q3/3P1B2/2KN4/8 w - - 0 1",
    "6K1/8/4N3/8/3q4/8/4p3/R4k2 b - - 0 1",
    "k7/8/2P5/b7/8/8/1P5q/R3K3 b Q - 0 1"
  };
  static mask_t const next_position = CW_MASK_T_CONST(0xffffffffffffffff);
  static mask_t moves[] = {
    CW_MASK_T_CONST(0x7f8080), CW_MASK_T_CONST(0x80000), CW_MASK_T_CONST(0xc040400000),
    CW_MASK_T_CONST(0x30000000), CW_MASK_T_CONST(0x80008000000000), CW_MASK_T_CONST(0x302),
    CW_MASK_T_CONST(0x20202020d020202), CW_MASK_T_CONST(0x3000000000), CW_MASK_T_CONST(0xc07fc02000000000),
    next_position,
    CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x8000000), CW_MASK_T_CONST(0x40000000000),
    CW_MASK_T_CONST(0xb00000000000), CW_MASK_T_CONST(0xdf20202000000000),
    CW_MASK_T_CONST(0x2000204), CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x380000),
    CW_MASK_T_CONST(0x400000000000), CW_MASK_T_CONST(0x408080808080), CW_MASK_T_CONST(0x807f000000000000),
    next_position,
    CW_MASK_T_CONST(0xd0202), CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x204400442800), CW_MASK_T_CONST(0x0),
    CW_MASK_T_CONST(0x40c00000000), CW_MASK_T_CONST(0xb00000000000), CW_MASK_T_CONST(0xdf20202000000000),
    CW_MASK_T_CONST(0x2000204), CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x101000),
    CW_MASK_T_CONST(0x200000000000), CW_MASK_T_CONST(0x408080808080), CW_MASK_T_CONST(0x807f000000000000),
    next_position,
    CW_MASK_T_CONST(0x200000), CW_MASK_T_CONST(0x300000000), CW_MASK_T_CONST(0x180008000000),
    CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x10a000a000000000), CW_MASK_T_CONST(0x2020a),
    CW_MASK_T_CONST(0x4000000), CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x400000000),
    CW_MASK_T_CONST(0x400000000),
    next_position,
    CW_MASK_T_CONST(0x6000), CW_MASK_T_CONST(0x10), CW_MASK_T_CONST(0x9),
    CW_MASK_T_CONST(0x10101010101013e), CW_MASK_T_CONST(0x2844004428000000),
    CW_MASK_T_CONST(0x20a0000000000000),
    next_position,
    CW_MASK_T_CONST(0x82848890a0c07ec0), CW_MASK_T_CONST(0x0), CW_MASK_T_CONST(0x201000000000000),
    CW_MASK_T_CONST(0x10101010e), CW_MASK_T_CONST(0x28), CW_MASK_T_CONST(0x2020000), CW_MASK_T_CONST(0x4000000000000)
  };
  int number_of_FEN_codes = sizeof(FEN_codes) / sizeof(*FEN_codes);
  int moves_count = 0;
  for (int FEN_code_index = 0; FEN_code_index < number_of_FEN_codes; ++FEN_code_index)
  {
    chess_position.load_FEN(FEN_codes[FEN_code_index]);
    int color_count = 0;
    for (Color color(black); color_count < 2; ++color_count, color = white)
    {
      for (PieceIterator piece_iter = chess_position.piece_begin(color); piece_iter != piece_end; ++piece_iter, ++moves_count)
	CPPUNIT_ASSERT(chess_position.moves(piece_iter.index())() == moves[moves_count]);
    }
    CPPUNIT_ASSERT(FEN_code_index == number_of_FEN_codes - 1 || moves[moves_count] == next_position);
    ++moves_count;
  }
}

} // namespace testsuite

#endif // TESTSUITE_IMPLEMENTATION
