// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file debug_ostream_operators.h Debug ostream inserter function declarations.
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

#ifndef DEBUG_OSTREAM_OPERATORS_H
#define DEBUG_OSTREAM_OPERATORS_H

#ifdef CWDEBUG

namespace cwchess {

class Color;
class Index;
class Code;
class Type;
class BitBoard;

extern std::ostream& operator<<(std::ostream& os, Color const& color);	//!< Print debug info for Color instance \a color.
extern std::ostream& operator<<(std::ostream& os, Index const& index);	//!< Print debug info for Index instance \a index.
extern std::ostream& operator<<(std::ostream& os, Code const& code);	//!< Print debug info for Code instance \a code.
extern std::ostream& operator<<(std::ostream& os, Type const& type);	//!< Print debug info for Type instance \a type.
extern std::ostream& operator<<(std::ostream& os, BitBoard const& bit_board);	//!< Print debug info for BitBoard instance \a bit_board.

} // namespace cwchess

#endif // CWDEBUG
#endif // DEBUG_OSTREAM_OPERATORS_H

