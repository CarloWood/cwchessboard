// cwchessboard -- A GTK+ chessboard widget
//
// Copyright (C) 2008 Carlo Wood
//
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

//! @file CwChessboardCodes.h
//! @brief This file contains the definitions of the CwChessboardCode constants.

#ifndef CWCHESSBOARDCODES_H
#define CWCHESSBOARDCODES_H

#include <stdint.h>

/** @typedef CwChessboardCode
 *  @brief A code to specify a chess piece.
 *
 * One of the following constants:
 * #empty_square, #black_pawn, #white_pawn, #black_rook, #white_rook, #black_knight, #white_knight,
 * #black_bishop, #white_bishop, #black_queen, #white_queen, #black_king or #white_king.
 *
 * @sa cw_chessboard_set_square, cw_chessboard_add_floating_piece, cw_chessboard_get_floating_piece
 */
typedef uint16_t CwChessboardCode;

// Doxygen refuses to document static variables, so fool it into thinking these are non-static.
// The comments for each constant are needed to get doxygen to show them at all.
#ifndef DOXYGEN_STATIC
#define DOXYGEN_STATIC static
#endif

/** An empty square. The value <code>1</code> will also result in an empty square. */
DOXYGEN_STATIC CwChessboardCode const empty_square = 0;

/** A black pawn. */
DOXYGEN_STATIC CwChessboardCode const black_pawn = 2;

/** A white pawn. */
DOXYGEN_STATIC CwChessboardCode const white_pawn = 3;

/** A black rook. */
DOXYGEN_STATIC CwChessboardCode const black_rook = 4;

/** A white rook. */
DOXYGEN_STATIC CwChessboardCode const white_rook = 5;

/** A black knight. */
DOXYGEN_STATIC CwChessboardCode const black_knight = 6;

/** A white knight. */
DOXYGEN_STATIC CwChessboardCode const white_knight = 7;

/** A black bishop. */
DOXYGEN_STATIC CwChessboardCode const black_bishop = 8;

/** A white bishop. */
DOXYGEN_STATIC CwChessboardCode const white_bishop = 9;

/** A black queen. */
DOXYGEN_STATIC CwChessboardCode const black_queen = 10;

/** A white queen. */
DOXYGEN_STATIC CwChessboardCode const white_queen = 11;

/**
 * A black king.
 * Since the CwChessboard widget does not do any checking,
 * it is possible to display more than one king.
 */
DOXYGEN_STATIC CwChessboardCode const black_king = 12;

/**
 * A white king.
 * Since the CwChessboard widget does not do any checking,
 * it is possible to display more than one king.
 */
DOXYGEN_STATIC CwChessboardCode const white_king = 13;

#endif // CWCHESSBOARDCODES_H
