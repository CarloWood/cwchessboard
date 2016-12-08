// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file tstbenchmark.cc A program to measure the move generation speed.
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

#ifndef USE_PCH
#include "sys.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <sys/time.h>
#include "debug.h"
#endif

#include "ChessPosition.h"
#include "PieceIterator.h"
#include "MoveIterator.h"
#include "ChessNotation.h"

using namespace cwchess;

void print_move(ChessPosition const& chess_position, Move const& move)
{
  if (chess_position.to_move() == white)
    std::cout << chess_position.full_move_number() << ". ";
  std::ostringstream str;
  str << ChessNotation(chess_position, move);
  std::cout << std::setfill(' ') << std::setw(10) << std::left << str.str();
  if (chess_position.to_move() == black)
    std::cout << '\n';
}

int main()
{
  Debug(libcw_do.off());
  Debug(debug::init());

  time_t seed = 1220638382; // Use fixed seed for reproducibility. time(NULL);
  //std::cout << "seed = " << seed << std::endl;
  std::srand(seed);
 
  ChessPosition chess_position;
  std::vector<Move> game;
  Move moves[2048];
  Move* move_ptr;
  static int random_numbers[5000000];

  // Pre-calculate random numbers.
  for (int i = 0; i < 5000000; ++i)
    random_numbers[i] = std::rand();

  int games = 0;
  int total_moves = 0;
  int Move_count = 0;

  struct timeval before, after;
  gettimeofday(&before, NULL);

  do
  {
    chess_position.initial_position();
    //game.clear();
    for(;;)
    {
      move_ptr = moves;
      for (PieceIterator piece_iter(chess_position.piece_begin(chess_position.to_move()));	// >
	   piece_iter != chess_position.piece_end();						// >- 20 ns
	   ++piece_iter)									// >
      {
	MoveIterator const move_end;
#if 0
	Move* move_save = move_ptr;
        for (int i = 0; i < 100; ++i)
	{
	  move_ptr = move_save;
#endif
	  for (MoveIterator move_iter(chess_position.move_begin(piece_iter.index()));		// 216 ns (per executed move)
	       move_iter != move_end;								// >- 161 ns (per executed move)
	       ++move_iter)									// >
	  {
	    *move_ptr++ = *move_iter;
	  }
#if 0
	}
#endif
      }
      int number_of_moves = move_ptr - moves;
      Move_count += number_of_moves;
      if (number_of_moves == 0)
	break;
      int mn = random_numbers[total_moves] % number_of_moves;	// 12 ns.
      ++total_moves;
      //game.push_back(move);
      if (chess_position.execute(moves[mn]))			// 300 ns.
	break;
    }
    ++games;
  }
  while (games < 10000);

  gettimeofday(&after, NULL);

#if 0
  chess_position.initial_position();
  for (std::vector<Move>::iterator iter = game.begin(); iter != game.end(); ++iter)
  {
    //print_move(chess_position, *iter);
    Move const& move(*iter);
    print_move(chess_position, move);
    chess_position.execute(move);
  }
  if (chess_position.to_move() == black)
    std::cout << std::endl;
#endif

  std::cout << "Number of games played: " << games << "\nTotal number of moves played: " << total_moves << std::endl;

  timersub(&after, &before, &after);
  uint64_t t = after.tv_sec;
  t *= 1000000;
  t += after.tv_usec;
  std::cout << "Generated Move objects: " << (unsigned long)(Move_count / (t / 1000000.0) + 0.5) << " Moves/second." << std::endl;
  std::cout << "Executed ply: " << (unsigned long)(total_moves / (t / 1000000.0) + 0.5) << " ply/second." << std::endl;
  std::cout << "Computing time: " << ((double)t / total_moves) << " microseconds per executed move (ply)." << std::endl;
}
