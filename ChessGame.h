// cwchessboard -- A C++ chessboard tool set
//
//! @file ChessGame.h This file contains the declaration of class ChessGame.
//
// Copyright (C) 2008 - 2010, by
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

#ifndef CHESSGAME_H
#define CHESSGAME_H

#ifndef USE_PCH
#include <vector>
#endif

#include "MetaData.h"
#include "GameNode.h"
#include "PgnGame.h"

namespace cwchess {

class ChessGame {
  private:
    MetaData M_meta_data;
    std::vector<GameNode> M_nodes;

  public:
    //! @brief Construct an uninitialized ChessGame object.
    ChessGame(void) { }

    //! @brief Access the meta data of the game.
    MetaData const& meta_data(void) const { return M_meta_data; }

    //! @brief Access the game nodes of the game.
    std::vector<GameNode> const& nodes(void) const { M_nodes; }

    //! @brief Read a game from a PGN file.
    void read(pgn::Game const& pgn_game);
};

} // namespace cwchess

#endif	// CHESSGAME_H
