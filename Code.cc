// cwchessboard -- A C++ chessboard tool set
//
//! @file Code.cc This file contains the implementation of class Code.
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
#endif

#include "Code.h"
#include "CwChessboardCodes.h"

namespace cwchess {

/*
 * Code is encoded as CTTT, where C is the color and TTT the type.
 * CwChessboardCode is encoded as TTTC.
 *
 * In both cases, 0 is black and 1 is white, however TTT is encoded differently.
 *
 * Code uses:
 *     nothing = 0,
 *     black_pawn = 1,
 *     black_knight = 2,
 *     black_king = 3,
 *     black_bishop = 5,
 *     black_rook = 6,
 *     black_queen = 7,
 *     white_pawn = 9,
 *     white_knight = 10,
 *     white_king = 11,
 *     white_bishop = 13,
 *     white_rook = 14,
 *     white_queen = 15.
 *
 * CwChessboardCode uses:
 *     empty_square = 0,
 *     black_pawn = 2,
 *     white_pawn = 3,
 *     black_rook = 4,
 *     white_rook = 5,
 *     black_knight = 6,
 *     white_knight = 7,
 *     black_bishop = 8,
 *     white_bishop = 9,
 *     black_queen = 10,
 *     white_queen = 11,
 *     black_king = 12,
 *     white_king = 13.
 */
CwChessboardCode Code::Code_to_CwChessboardCode[16] = {
  ::empty_square, /* nothing = 0 */
  ::black_pawn,   /* black_pawn = 1 */
  ::black_knight, /* black_knight = 2 */
  ::black_king,   /* black_king = 3 */
  0,    	  /* unused (4) */
  ::black_bishop, /* black_bishop = 5 */
  ::black_rook,   /* black_rook = 6 */
  ::black_queen,  /* black_queen = 7 */
  0,              /* unused (8) */
  ::white_pawn,   /* white_pawn = 9 */
  ::white_knight, /* white_knight = 10 */
  ::white_king,   /* white_king = 11 */
  0,              /* unused (12) */
  ::white_bishop, /* white_bishop = 13 */
  ::white_rook,   /* white_rook = 14 */
  ::white_queen   /* white_queen = 15 */
};

CodeData Code::CwChessboardCode_to_Code[14] = {
  { 0 },        /* empty_square = 0 */
  { 0 },        /* unused (1) */
  black_pawn,   /* black_pawn = 2 */
  white_pawn,   /* white_pawn = 3 */
  black_rook,   /* black_rook = 4 */
  white_rook,   /* white_rook = 5 */
  black_knight, /* black_knight = 6 */
  white_knight, /* white_knight = 7 */
  black_bishop, /* black_bishop = 8 */
  white_bishop, /* white_bishop = 9 */
  black_queen,  /* black_queen = 10 */
  white_queen,  /* white_queen = 11 */
  black_king,   /* black_king = 12 */
  white_king    /* white_king = 13 */
};

} // namespace cwchess
