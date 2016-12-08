// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file tstchessposition.cc A test application that prints all legal moves of a given position.
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

#include "sys.h"
#include <iostream>
#include <vector>
#include <cassert>
#include "ChessPosition.h"
#include "MoveIterator.h"
#include "ChessNotation.h"
#include "debug.h"

namespace test {
  using cwchess::ArrayCode;
  using cwchess::BitBoard;
  using cwchess::ArrayIndex;
  using cwchess::Piece;
  using cwchess::ArrayColor;
  using cwchess::CountBoard;
  using cwchess::CastleFlags;
  using cwchess::Color;
  using cwchess::EnPassant;

  struct ChessPosition {
    ArrayCode<BitBoard> M_bitboards;			//!< Bitboards reflecting the current position.
    ArrayIndex<Piece> M_pieces;				//!< A piece per square. The index of the array is an Index.
    ArrayColor<BitBoard> M_attackers;			//!< Bitboards for squares of enemy pieces on the same line as the king and all squares in between.
    ArrayColor<BitBoard> M_pinning;			//!< Squares between attacker and king for actually pinned pieces (including attacker).
    ArrayColor<CountBoard> M_defended;			//!< The number times a square is defended.
    ArrayColor<uint8_t> M_king_battery_attack_count;	//!< The number of times that a king is 'attacked' by pieces behind another attacker.
    uint16_t M_full_move_number;			//!< The number of the full move. It starts at 1, and is incremented after Black's move.
    uint8_t M_half_move_clock;				//!< Number of half moves since the last pawn advance or capture.
    CastleFlags M_castle_flags;				//!< Whether black and white may castle long or short.
    Color M_to_move;					//!< The active color.
    EnPassant M_en_passant;				//!< A pawn that can be taken en passant, or zeroed if none such pawn exists.
    bool M_double_check;				//!< Cached value of wether or not M_to_move is in double check.
  };
}

int main(int argc, char* argv[])
{
  Debug(debug::init());

  using namespace cwchess;

  //-------------------------------------------------------------------------
  // Print the sizes of classes.
  std::cout << "sizeof(Color) = " << sizeof(Color) << '\n';
  assert(sizeof(Color) == 1);
  std::cout << "sizeof(Index) = " << sizeof(Index) << '\n';
  assert(sizeof(Index) == 1);
  std::cout << "sizeof(Code) = " << sizeof(Code) << '\n';
  assert(sizeof(Code) == 1);
  std::cout << "sizeof(Flags) = " << sizeof(Flags) << '\n';
  assert(sizeof(Flags) == 1);
  std::cout << "sizeof(Piece) = " << sizeof(Piece) << '\n';
  assert(sizeof(Piece) == sizeof(Code) + sizeof(Flags));
  std::cout << "sizeof(BitBoard) = " << sizeof(BitBoard) << '\n';
  assert(sizeof(BitBoard) == 8);
  std::cout << "sizeof(CountBoard) = " << sizeof(CountBoard) << '\n';
  assert(sizeof(CountBoard) == 5 * sizeof(BitBoard));
  std::cout << "sizeof(CastleFlags) = " << sizeof(CastleFlags) << '\n';
  assert(sizeof(CastleFlags) == 1);
  std::cout << "sizeof(EnPassant) = " << sizeof(EnPassant) << '\n';
  assert(sizeof(EnPassant) == 1);
  std::cout << "sizeof(ArrayCode<BitBoard>) = " << sizeof(ArrayCode<BitBoard>) << '\n';
  assert(sizeof(ArrayCode<BitBoard>) == 16 * sizeof(BitBoard));
  std::cout << "sizeof(ArrayIndex<Piece>) = " << sizeof(ArrayIndex<Piece>) << '\n';
  assert(sizeof(ArrayIndex<Piece>) == 64 * sizeof(Piece));
  std::cout << "sizeof(ArrayColor<BitBoard>) = " << sizeof(ArrayColor<BitBoard>) << '\n';
  assert(sizeof(ArrayColor<BitBoard>) == 2 * sizeof(BitBoard));
  std::cout << "sizeof(ArrayColor<CountBoard>) = " << sizeof(ArrayColor<CountBoard>) << '\n';
  assert(sizeof(ArrayColor<CountBoard>) == 2 * sizeof(CountBoard));
  std::cout << "sizeof(ArrayColor<uint8_t>) = " << sizeof(ArrayColor<uint8_t>) << '\n';
  assert(sizeof(ArrayColor<uint8_t>) == 2);
  size_t sum = sizeof(ArrayCode<BitBoard>) + sizeof(ArrayIndex<Piece>) +
      sizeof(ArrayColor<BitBoard>) + sizeof(ArrayColor<BitBoard>) + sizeof(ArrayColor<CountBoard>) + sizeof(ArrayColor<uint8_t>) +
      sizeof(uint16_t) + sizeof(uint8_t) + sizeof(CastleFlags) + sizeof(Color) + sizeof(EnPassant) + sizeof(bool);
  std::cout << "Sum is " << sum << '\n';
  std::cout << "sizeof(test::ChessPosition) = " << sizeof(test::ChessPosition) << '\n';
  std::cout << "sizeof(ChessPosition) = " << sizeof(ChessPosition) << '\n';
#ifdef CWDEBUG
  if (sizeof(ChessPosition) != sum)
    Dout(dc::warning, "ChessPosition is not compact!?!");
#endif

  //-------------------------------------------------------------------------
  // Load FEN code from command line and print all legal moves.
  //

  ChessPosition chess_position;

  if (argc > 1)
  {
    std::string str(argv[1]);
    std::cout << "Loading \"" << str << "\".\n";

    if (chess_position.load_FEN(str))
      std::cout << "Loading successful\n";
    else
      std::cout << "Loading failed!\n";
  }
  else
    chess_position.initial_position();

  std::cout << "FEN code is: \"" << chess_position.FEN() << "\".\n";

  std::vector<Move> moves;
  Color color(chess_position.to_move());
  PieceIterator piece_end(chess_position.piece_end());
  for (PieceIterator piece_iter = chess_position.piece_begin(color); piece_iter != piece_end; ++piece_iter)
  {
    MoveIterator move_end(chess_position.move_end());
    for (MoveIterator iter = chess_position.move_begin(piece_iter.index()); iter != move_end; ++iter)
      moves.push_back(*iter);
  }
  std::cout << "There are " << moves.size() << " moves: ";
  bool first = true;
  for (std::vector<Move>::iterator iter = moves.begin(); iter != moves.end(); ++iter)
  {
    if (first)
      first = false;
    else
      std::cout << ", ";
    std::cout << ChessNotation(chess_position, *iter);
  }
  std::cout << std::endl;
}
