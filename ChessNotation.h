// cwchessboard -- A C++ chessboard tool set
//
//! @file ChessNotation.h This file contains the declaration of class ChessNotation.
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

#ifndef CHESSNOTATION_H
#define CHESSNOTATION_H

#ifndef USE_PCH
#include <iosfwd>
#endif

#include "ChessPosition.h"
#include "Type.h"
#include "Piece.h"
#include "Index.h"
#include "Move.h"

namespace cwchess {

/** @brief A helper class to write other objects to an ostream.
 *
 * This class allows objects of several types to be written to an ostream in a human readable form.
 *
 * The constructor always takes the chess_position that the
 * object refers to as first argument.
 *
 * Usage example:
 * \code
 * std::cout << ChessNotation(chess_position, move);
 * \endcode
 */
class ChessNotation {
  private:
    ChessPosition const& M_chess_position;
    Type const* M_type;
    Piece const* M_piece;
    Index const* M_index;
    Move const* M_move;

  public:
  
  /** @name Constructors */
  //@{

    ChessNotation(ChessPosition const& chess_position, Piece const& piece) :
        M_chess_position(chess_position), M_type(NULL), M_piece(&piece), M_index(NULL), M_move(NULL) { }
    ChessNotation(ChessPosition const& chess_position, Index const& index) :
        M_chess_position(chess_position), M_type(NULL), M_piece(NULL), M_index(&index), M_move(NULL) { }
    ChessNotation(ChessPosition const& chess_position, Move const& move) :
        M_chess_position(chess_position), M_type(NULL), M_piece(NULL), M_index(NULL), M_move(&move) { }
    ChessNotation(ChessPosition const& chess_position, Type const& type) :
        M_chess_position(chess_position), M_type(&type), M_piece(NULL), M_index(NULL), M_move(NULL) { }

  //@}

  public_notdocumented:
    void print_on(std::ostream& os, Type const& type) const;
    void print_on(std::ostream& os, Piece const& piece) const;
    void print_on(std::ostream& os, Index const& index) const;
    void print_on(std::ostream& os, Move const& move) const;

    friend std::ostream& operator<<(std::ostream& os, ChessNotation const& chess_notation);
};

} // namespace ChessNotation

#endif	// CHESSNOTATION_H
