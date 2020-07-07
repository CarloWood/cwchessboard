// cwchessboard -- A C++ chessboard tool set
//
//! @file chattr.h Character attribute definitions and arrays.
//
// Copyright (C) 1998, Andrea Cocito.
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

#ifndef CHATTR_H
#define CHATTR_H

#ifndef USE_PCH
#include <stdint.h>
#include <limits.h>
#endif

typedef uint16_t attr_t;

//
// Character attribute macros
//
attr_t const pgn_blank		      = 0x0001;			//!< ' ' | '\\t' | '\\v' | '\\f'. Note that '\\f' is normally not legal in a PGN.
attr_t const pgn_eol		      = 0x0002;			//!< '\\r' | '\\n'
attr_t const pgn_white_space	      = pgn_blank | pgn_eol;	//!< (pgn_blank | pgn_eol)
attr_t const pgn_file		      = 0x0004;			//!< abcdefgh
attr_t const pgn_rank		      = 0x0008;			//!< 12345678
attr_t const pgn_piece		      = 0x0010;			//!< RNBQK
attr_t const pgn_check		      = 0x0020;			//!< +#
attr_t const pgn_punctuation_junk     = 0x0040;			//!< ,;
attr_t const pgn_digit		      = 0x0080;			//!< 0123456789
attr_t const pgn_alpha		      = 0x0100;			//!< abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ
attr_t const pgn_alnum		      = pgn_alpha | pgn_digit;	//!< (pgn_alpha | pgn_digit)
attr_t const pgn_tagname_begin	      = pgn_alnum;		//!< pgn_alnum
attr_t const pgn_tagname_continuation = 0x0200;			//!< (pgn_alnum | '_')
attr_t const pgn_tag_separator_junk   = 0x0400;			//!< :=
attr_t const pgn_printable_string     = 0x0800;			//!< ascii range(-96, -1) | ascii range(35, 91) | ' ' | ascii range(93, 126) | '!'
attr_t const pgn_quote_or_eol         = 0x1000;			//!< (ntl_eol | '"')
attr_t const pgn_comment_start        = 0x2000;			//!< {;
attr_t const pgn_printable_comment    = 0x4000;			//!< ascii range(-96, -1) | ascii range(32, 124) | '~' | pgn_blank | pgn_eol
attr_t const pgn_printable            = 0x8000;			//!< ascii range(-96, -1) | ascii range(32, 126)

extern char const ToLowerTab_8859_1[];
extern char const ToUpperTab_8859_1[];
extern attr_t const PGN_CharAttrTab[];

//! @brief Convert a character to its lower-case equivalent.
inline char to_lower(char c) { return ToLowerTab_8859_1[c - CHAR_MIN]; }
//! @brief Convert a character to its upper-case equivalent.
inline char to_upper(char c) { return ToUpperTab_8859_1[c - CHAR_MIN]; }

//
// Character classification functions.
// NOTE: The is_upper and is_lower macros do not apply to the complete
// ISO 8859-1 character set, unlike the to_upper and to_lower macros above.
// is_upper and is_lower only apply for comparisons of the US ASCII subset.
//

//! @brief Test whether a character is a blank.
inline attr_t is_blank(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_blank; }
//! @brief Test whether a character is an end-of-line character.
inline attr_t is_eol(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_eol; }
//! @brief Test whether a character is white space.
inline attr_t is_white_space(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_white_space; }
//! @brief Test whether a character is a file symbol.
inline attr_t is_file(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_file; }
//! @brief Test whether a character is a rank symbol.
inline attr_t is_rank(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_rank; }
//! @brief Test whether a character is a chess piece symbol.
inline attr_t is_piece(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_piece; }
//! @brief Test whether a character is a check or mate symbol.
inline attr_t is_check(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_check; }
//! @brief Test whether a character is punctuation junk.
inline attr_t is_punctuation_junk(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_punctuation_junk; }
//! @brief Test whether a character is a digit.
inline attr_t is_digit(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_digit; }
//! @brief Test whether a character is alphabetic.
inline attr_t is_alpha(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_alpha; }
//! @brief Test whether a character is alphanumeric.
inline attr_t is_alnum(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_alnum; }
//! @brief Test whether a character could be the first character of a tagname.
inline attr_t is_tagname_begin(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_tagname_begin; }
//! @brief Test whether a character could be part of a tagname.
inline attr_t is_tagname_continuation(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_tagname_continuation; }
//! @brief Test whether a character could tag separator junk.
inline attr_t is_tag_separator_junk(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_tag_separator_junk; }
//! @brief Test whether a character is allowed in a string.
inline attr_t is_printable_string(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_printable_string; }
//! @brief Test whether a character is a quote or EOL.
inline attr_t is_quote_or_eol(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_quote_or_eol; }
//! @brief Test whether a character is a comment start.
inline attr_t is_comment_start(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_comment_start; }
//! @brief Test whether a character is allowed in a brace comment.
inline attr_t is_printable_comment(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_printable_comment; }
//! @brief Test whether a character is allowed in a semi-colon comment.
inline attr_t is_printable(char c) { return PGN_CharAttrTab[c - CHAR_MIN] & pgn_printable; }

#endif // CHATTR_H
