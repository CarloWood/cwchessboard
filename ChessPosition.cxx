// cwchessboard -- A C++ chessboard tool set
//
//! @file ChessPosition.cxx This file contains the implementation of class ChessPosition.
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
#include "ChessPosition.h"
#include "Direction.h"
#include "ChessNotation.h"
#include "debug.h"
#include <sstream>
#include <cassert>

namespace cwchess {

#ifndef DOXYGEN
std::string EnPassant::FEN4() const
{
  if (!exists())
    return "-";
  std::ostringstream fen;
  Index passed(this->index());
  fen << (char)('a' + passed.col()) << (passed.row() + 1);
  return fen.str();
}
#endif

void ChessPosition::clear_en_passant()
{
  Index index = M_en_passant.pawn_index();
  if (index > ih1 && piece_at(index - 1) == pawn)
    M_pieces[index - 1].reset_can_take_king_side();
  if (index < ia8 && piece_at(index + 1) == pawn)
    M_pieces[index + 1].reset_can_take_queen_side();
  M_en_passant.clear();
}

void ChessPosition::clear()
{
  DoutEntering(dc::notice, "ChessPosition::clear() [" << (void*)this << "]");
  for (Index index = index_begin; index != index_end; ++index)
    M_pieces[index].set_type(nothing);
  M_en_passant.clear();
  M_castle_flags.clear();
  M_half_move_clock = 0;
  M_full_move_number = 1;
  for (int i = 0; i < 16; ++i)
  {
    CodeData data = { static_cast<uint8_t>(i) };
    M_bitboards[data].reset();
  }
  M_attackers[black].reset();
  M_attackers[white].reset();
  M_pinning[black].reset();
  M_pinning[white].reset();
  M_defended[black].reset();
  M_defended[white].reset();
  M_king_battery_attack_count[black] = 0;
  M_king_battery_attack_count[white] = 0;
  M_double_check = false;
}

void ChessPosition::initial_position()
{
  DoutEntering(dc::notice, "ChessPosition::initial_position()");
  clear();
  // It's white's turn.
  M_to_move = white;
  // White pawns.
  for (Index index = ia2; index <= ih2; ++index)
    place(white_pawn, index);
  // Black pawns.
  for (Index index = ia7; index <= ih7; ++index)
    place(black_pawn, index);
  // The other pieces.
  Color color = white;
  Index index = index_pre_begin;
  for (int ci = 0; ci < 2; ++ci)
  {
    place(Code(color, rook), ++index);
    place(Code(color, knight), ++index);
    place(Code(color, bishop), ++index);
    place(Code(color, queen), ++index);
    place(Code(color, king), ++index);
    place(Code(color, bishop), ++index);
    place(Code(color, knight), ++index);
    place(Code(color, rook), ++index);
    color = black;
    index += 48;	// Skip all pawns and empty squares.
  }
}

bool ChessPosition::increment_counters(bool pawn_advance_or_capture)
{
  if (pawn_advance_or_capture)
    M_half_move_clock = 0;
  else
    ++M_half_move_clock;
  if (M_to_move == white)
    ++M_full_move_number;
  return M_half_move_clock == 100;
}

bool ChessPosition::skip_move()
{
  reset_en_passant();
  M_to_move.toggle();
  M_double_check = M_castle_flags.in_check(M_to_move) ? double_check(M_to_move) : false;
  return increment_counters(false);
}

void ChessPosition::to_move(Color const& color)
{
  M_to_move = color;
  M_double_check = M_castle_flags.in_check(M_to_move) ? double_check(M_to_move) : false;
}

void ChessPosition::swap_colors()
{
  ChessPosition new_chess_position;
  new_chess_position.clear();
  new_chess_position.M_to_move = M_to_move.opposite();
  // First place the pawn that can be taken en passant, if any.
  Index en_passant_index = index_end;
  if (M_en_passant.exists())
  {
    Index index = M_en_passant.pawn_index();
    Piece& piece(M_pieces[index]);
    en_passant_index = Index(index.col(), 7 - index.row());
    new_chess_position.place(Code(piece.color().opposite(), pawn), en_passant_index);
    new_chess_position.set_en_passant(Index(index.col(), 7 - M_en_passant.index().row()));
  }
  // Then place all other pawns (and pieces).
  PieceIterator const end;
  for (PieceIterator iter(this, M_bitboards[black] | M_bitboards[white]); iter != end; ++iter)
  {
    Index index(iter.index().col(), 7 - iter.index().row());
    if (index == en_passant_index)
      continue;
    new_chess_position.place(Code(iter->color().opposite(), iter->type()), index);
  }
  new_chess_position.M_full_move_number = 1;	// The history of the game was changed in an unknown way: it is not allowed that black started the game.
  *this = new_chess_position;
}

// Called when a piece of color \a color has been removed from (col, row) (possibly replaced
// by a piece of the opposite color). This function updates the pawn flags of influenced
// pawns on the board.
void ChessPosition::update_removed(uint8_t col, uint8_t row, Color const& color)
{
  // A piece was removed from (col, row).
  // We have to update possible pawns on (col - 1, row +/- 1) and (col + 1, row +/- 1).
  bool ok;
  Code other_pawn;
  if (color == white)
  {
    ++row;
    ok = row <= 6;	// 6, not 7, because there can't be black pawns on row 7.
    other_pawn = black_pawn;
  }
  else
  {
    --row;
    ok = (int8_t)row >= 1;	// 1, not 0, because there can't be white pawns on row 0.
    other_pawn = white_pawn;
  }
  if (ok)
  {
    if (col > 0)
    {
      Index index(col - 1, row);
      if (M_pieces[index] == other_pawn)
	M_pieces[index].reset_can_take_king_side();
    }
    if (col < 7)
    {
      Index index(col + 1, row);
      if (M_pieces[index] == other_pawn)
	M_pieces[index].reset_can_take_queen_side();
    }
  }
}

// Called when a piece of color \a color has been placed at (col, row) (possibly replacing
// a piece of the opposite color). This function updates the pawn flags of influenced
// pawns on the board.
void ChessPosition::update_placed(uint8_t col, uint8_t row, Color const& color)
{
  // A piece was placed at (col, row).
  // We have to update possible pawns on (col - 1, row +/- 1) and (col + 1, row +/- 1).
  bool ok;
  Code other_pawn;
  if (color == white)
  {
    ++row;
    ok = row <= 6;	// 6, not 7, because there can't be black pawns on row 7.
    other_pawn = black_pawn;
  }
  else
  {
    --row;
    ok = (int8_t)row >= 1;	// 1, not 0, because there can't be white pawns on row 0.
    other_pawn = white_pawn;
  }
  if (ok)
  {
    if (col > 0)
    {
      Index index(col - 1, row);
      if (M_pieces[index] == other_pawn)
	M_pieces[index].set_can_take_king_side();
    }
    if (col < 7)
    {
      Index index(col + 1, row);
      if (M_pieces[index] == other_pawn)
	M_pieces[index].set_can_take_queen_side();
    }
  }
}

// This function recalculates M_pinning for the color of code (a black_king or white_king).
//
// @param code : The code of the king.
// @param index : The index of the king.
// @param mask : The position of the king.
// @param direction : The direction from the king towards the attacker.
// @param relevant_pieces : All pieces along that line (possibly cut off at the last attacker).
//
void ChessPosition::update_pinning(Code const& code, Index const& index, mask_t mask, Direction const& direction, BitBoard const& relevant_pieces)
{
  bool king_side_is_msb = (relevant_pieces() < mask);	// This means that the most significant bit is on the side of the king.
  // Run over all pieces, starting at the side of the king.
  PieceIterator piece_iter = king_side_is_msb ? PieceIterator(this, relevant_pieces, 0) : PieceIterator(this, relevant_pieces);
  PieceIterator end = king_side_is_msb ? PieceIterator(0) : PieceIterator();
  Code first_piece_code;
  Index first_piece_index;
  bool found_first_piece = false;
  bool taking_en_passant_not_allowed = false;
  while (piece_iter != end)
  {
    if (found_first_piece)
    {
      Code second_piece_code = piece_iter->code();
      if (!second_piece_code.has_opposite_color_of(code))
	break;
      else
      {
	if (second_piece_code.moves_along(direction))
	{
	  if (taking_en_passant_not_allowed)
	    M_en_passant.pinned_set();		// Disallow taking en passant.
	  else
	    M_pinning[code.color()] |= squares_from_to(piece_iter.index(), index);
	  break;
	}
	else if (M_en_passant.exists() && M_en_passant.pawn_index() == piece_iter.index() && first_piece_code.is_a(pawn) &&
	    direction.is_horizontal() && M_en_passant.pawn_index() - direction == first_piece_index &&
	    first_piece_code.has_opposite_color_of(second_piece_code))
	  taking_en_passant_not_allowed = true;
	else
	  break;
      }
    }
    else
    {
      first_piece_code = piece_iter->code();
      first_piece_index = piece_iter.index();
      if (first_piece_code.has_opposite_color_of(code))
      {
	if (!M_en_passant.exists() || M_en_passant.pawn_index() != first_piece_index || !direction.is_horizontal())
	  break;	// Nothing is pinned (though we might be in check).
	else
	{
	  // This handles a very special case.
	  taking_en_passant_not_allowed = true;
	}
      }
      else if (taking_en_passant_not_allowed &&
	  (!first_piece_code.is_a(pawn) || first_piece_index - direction != M_en_passant.pawn_index()))
	break;	// Also special case does not apply.
      else
	found_first_piece = true;
    }
    if (king_side_is_msb)
    {
      if (piece_iter.index() == ia1)	// We are not allowed to call --piece_iter if we're already at square a1.
        break;
      --piece_iter;
    }
    else
      ++piece_iter;
  }
}

// This function updates M_pieces and M_bitboards.
bool ChessPosition::place(Code const& code, Index const& index)
{
  DoutEntering(dc::place, "ChessPosition::place(" << code << ", " << index << ")");

  // Refuse to place pawns on row 1 or 8.
  if (code.is_a(pawn))
  {
    int row = index.row();
    if (row == 0 || row == 7)
      return false;
  }
  // Refuse to place two kings of the same color on the board.
  if (code.is_a(king) && M_bitboards[code].test())
    return false;

  Code const old_code = M_pieces[index].code();		// The current piece that is on this square, if any.

  // Do nothing if we replace a piece with the same piece.
  // This test is actually only effective if we're replacing a pawn that can be taken
  // en passant with a pawn of the same color; in which case this causes the en passant
  // state to be preserved.
  if (old_code == code)
    return true;

  mask_t const mask(index2mask(index));			// Calculate the bitboard mask.
  int index_row = index.row();				// Cache the row of the square involved.

  if (!old_code.is_nothing())
  {
    // Update administration regarding removal of a piece.
    M_bitboards[old_code.color()].reset(mask);
    M_bitboards[old_code].reset(mask);

    // Update castling flags.
    M_castle_flags.update_removed(old_code, index);

    // Update can_take_king/queen_side flags.
    if (code.is_nothing() || code.color() != old_code.color())
      update_removed(index.col(), index_row, old_code.color());

    // Update can_take_king/queen_side flags for pawns 'taking' the en_passant pawn, if any.
    if (
#if DEBUG_ENPASSANT_EXISTS
        M_en_passant.exists() &&		// This check is normally not needed because if M_en_passant.M_bits == 64,
						// then M_en_passant.pawn_index() returns 72, so the test will fail anyway.
#endif
        M_en_passant.pawn_index() == index)
      clear_en_passant();	// The pawn that could be taken en passant was removed.

    // Update is_blocked flag for pawns.
    if (code.is_nothing())
    {
      if (index_row > 1 && M_pieces[index - 8] == white_pawn)
      {
	M_pieces[index - 8].set_is_not_blocked();
        if (index_row == 2 && M_pieces[index + 8] == nothing)
	    M_pieces[index - 8].set_can_move_two_squares();
      }
      else if (index_row == 3 && M_pieces[index - 16] == white_pawn)
	M_pieces[index - 16].set_can_move_two_squares_if_not_blocked();
      if (index_row < 6 && M_pieces[index + 8] == black_pawn)
      {
	M_pieces[index + 8].set_is_not_blocked();
	if (index_row == 5 && M_pieces[index - 8] == nothing)
	    M_pieces[index + 8].set_can_move_two_squares();
      }
      else if (index_row == 4 && M_pieces[index + 16] == black_pawn)
	M_pieces[index + 16].set_can_move_two_squares_if_not_blocked();
    }

    // Reset all pinning flags if piece being removed is a king.
    if (old_code.is_a(king))
    {
      M_attackers[old_code].reset();
      M_pinning[old_code].reset();
      M_en_passant.pinned_reset();
      M_king_battery_attack_count[old_code.color().opposite()] = 0;
    }

    // Update the M_defended CountBoard.
    bool battery = false;
    M_defended[old_code.color()].sub(defendables(old_code, index, battery));
    if (battery)
      --M_king_battery_attack_count[old_code.color()];
    update_blocked_defendables(old_code, index, true);
  }

  if (!code.is_nothing())
  {
    // Update administration regarding adding of a piece.
    M_bitboards[code.color()].set(mask);
    M_bitboards[code].set(mask);

    // Update castling flags.
    M_castle_flags.update_placed(code, index);

    // Clear the right of taking en passant if a piece is placed behind the en passant pawn.
    if (M_en_passant.exists() && (index == M_en_passant.index() || index == M_en_passant.from_index()))
      clear_en_passant();

    // Update can_take_king/queen_side flags.
    if (old_code.is_nothing() || code.color() != old_code.color())
      update_placed(index.col(), index_row, code.color());

    // Update is_blocked flag for pawns.
    if (old_code.is_nothing())
    {
      if (index_row > 1 && M_pieces[index - 8] == white_pawn)
	M_pieces[index - 8].reset_is_not_blocked();
      else if (index_row == 3 && M_pieces[index - 16] == white_pawn)
	M_pieces[index - 16].reset_can_move_two_squares();
      if (index_row < 6 && M_pieces[index + 8] == black_pawn)
	M_pieces[index + 8].reset_is_not_blocked();
      else if (index_row == 4 && M_pieces[index + 16] == black_pawn)
	M_pieces[index + 16].reset_can_move_two_squares();
    }

    // Update pinning flags if a king is being placed on the board.
    if (code.is_a(king))
    {
      // Set bishop_code, rook_code and queen_code to the code of respective pieces of the opposite color.
      Color color(code.color());
      color.toggle();
      Code bishop_code(color, bishop);
      Code rook_code(color, rook);
      Code queen_code(color, queen);
      // Find all rook movers on the same line as this king.
      BitBoard rook_attackers(candidates_table[candidates_table_offset(rook) + index()]);
      rook_attackers &= M_bitboards[rook_code] | M_bitboards[queen_code];
      // Find all bishop movers on the same line as this king.
      BitBoard bishop_attackers(candidates_table[candidates_table_offset(bishop) + index()]);
      bishop_attackers &= M_bitboards[bishop_code] | M_bitboards[queen_code];
      // Finally fill M_attackers with all squares between king and attacking pieces (inclusive).
      BitBoard attackers(CW_MASK_T_CONST(0));
      for (PieceIterator piece_iter(this, rook_attackers); piece_iter != piece_end(); ++piece_iter)
	attackers |= squares_from_to(piece_iter.index(), index);
      for (PieceIterator piece_iter(this, bishop_attackers); piece_iter != piece_end(); ++piece_iter)
	attackers |= squares_from_to(piece_iter.index(), index);
      M_attackers[code] = attackers;
      BitBoard const all_pieces(M_bitboards[white] | M_bitboards[black]);
      BitBoard possible_pinning_directions(candidates_table[candidates_table_offset(king) + index()]);	// All squares around the king.
      possible_pinning_directions &= attackers;				// Only those squares in the direction of attackers.
      for (PieceIterator direction_iter(this, possible_pinning_directions); direction_iter != piece_end(); ++direction_iter)
      {
        Direction direction(direction_from_to(index, direction_iter.index()));	// Run over all directions in which there are attackers.
	BitBoard relevant_pieces(all_pieces);					// Only the pieces.
	relevant_pieces &= direction.from(index);				// On the line towards the attacker.
	update_pinning(code, index, mask, direction, relevant_pieces);
      }
    }

    // Update the M_defended CountBoard.
    bool battery = false;
    M_defended[code.color()].add(defendables(code, index, battery));
    if (battery)
      ++M_king_battery_attack_count[code.color()];
    update_blocked_defendables(code, index, false);
  }

  Flags flags(fl_none);

  if (code.is_a(pawn))
  {
    // Initialize the pawn flags for this pawn.
    // Calculate forward1: the square right in front of the pawn,
    //           forward2: the square two squares in front of the pawn, or 0 if that's off the board.
    //           kingside: the square that the pawn attacks towards the h-file (or 0 when the pawn is on the h-file).
    //           queenside: the square that the pawn attacks towards the a-file (or 0 when the pawn is on the a-file).
    //           other_pieces: All pieces of a different color.
    //           all_pieces: All pieces.
    BitBoardData forward1 = { mask };
    BitBoardData forward2 = { mask };
    BitBoard other_pieces, all_pieces;
    uint8_t initial_row;
    if (code.color() == white)
    {
      other_pieces = M_bitboards[black];
      forward1.M_bitmask <<= 8;
      forward2.M_bitmask <<= 16;
      all_pieces = other_pieces | M_bitboards[white];
      initial_row = 1;
    }
    else
    {
      other_pieces = M_bitboards[white];
      forward1.M_bitmask >>= 8;
      forward2.M_bitmask >>= 16;
      all_pieces = other_pieces | M_bitboards[black];
      initial_row = 6;
    }
    BitBoardData kingside(forward1), queenside(forward1);
    kingside.M_bitmask <<= 1;
    queenside.M_bitmask >>= 1;
    kingside.M_bitmask &= ~file_a.M_bitmask;
    queenside.M_bitmask &= ~file_h.M_bitmask;
    if (!(all_pieces & forward1))
    {
      flags |= fl_pawn_is_not_blocked;
      if (!(all_pieces & forward2) && initial_row == index_row)
        flags |= fl_pawn_can_move_two_squares;
    }
    if (M_en_passant.exists() && M_en_passant.from_index().row() != initial_row)
      other_pieces |= M_en_passant.index();
    if ((other_pieces & queenside))
      flags |= fl_pawn_can_take_queen_side;
    if ((other_pieces & kingside))
      flags |= fl_pawn_can_take_king_side;
  }

  // Replace or put the piece on the board.
  M_pieces[index] = Piece(code, flags);

  // Update pinning flags.
  //
  // We have two pinning masks (for each color of the king, so four in total).
  // The first mask (M_attackers) contains bits for every square between every attacker and the king (not including the king).
  // The second mask (M_pinning) contains the same, but only for those attackers that actually pin a piece.
  //
  // For the examples, we consider only rooks as attackers, and only over a horizontal line.
  //
  // We have the following cases:
  //
  // 1) The new piece is on a line with the enemy king : we need to check more.
  //
  //     x ? ? ? ? K
  //
  // 2) The new piece is not on a line with the enemy king : we do not need to update anything.
  //
  //     x ? ? ? ? ?
  //     ? ? ? ? ? K
  //
  // In case 1), we have the possibilities:
  //
  // 1a) The new piece is an attacker (a rook or queen, in the case of a horizontal line) of the enemy king.
  //
  //     r ? ? ? ? K
  //
  // 1b) The new piece is not an attacker (or empty)
  //
  //     n ? ? ? ? K
  //
  // This is sufficient information to update mask 1:
  // In the case of 1b we need to do nothing unless we're removing a piece (case 1br).
  // In the case 1a we need set bits only if the bit under the piece is not set already.
  //
  // For example,
  //            __ no attackers
  //        ___/
  //        ? ? r ? ? ? ? K
  // mask1: 0 0 1 1 1 1 1 0
  // place:         r		--> mask doesn't change.
  // place: r                   --> mask is extended.
  //
  // In case 1br, there are two cases:
  //
  // 1br1) We are removing an attacker.
  // 1br2) We are not removing an attacker.
  //
  // In case 1br1 it is necessary to recalculate mask 1. For example,
  //
  //        ? ? r - r - N K
  // mask1: 0 0 1 1 1 1 1 0
  // remove:    ^
  // mask1: 0 0 0 0 1 1 1 0
  //
  // At this point mask 1 is updated.
  //
  // In case 1br1 we only need to update mask 2 if the attacker being removed
  // was the pinning attacker, if the corresponding bit in mask 2 is set as well,
  // or if there wasn't any pinning along this line.
  //
  // For example,
  //
  //        ? ? r - r - N K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 0 0 1 1 1 0
  // remove:    ^		Not update of mask2 is needed.
  // remove:        ^
  // mask2: 0 0 1 1 1 1 1 0
  //
  // or
  //
  //        ? ? r N r - - K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 0 0 0 0 0 0
  // remove:        ^
  // mask2: 0 0 1 1 1 1 1 0
  //
  // In case 1br2 we need to reset mask 2, and there is only a need to recalculate
  // it if the piece being removed wasn't actually pinned. For example,
  //
  //        ? ? r - N - N K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 0 0 0 0 0 0
  // remove:            ^
  // mask2: 0 0 1 1 1 1 1 0
  //
  // or
  //
  //        ? ? r - N - - K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 1 1 1 1 1 0
  // remove:        ^
  // mask2: 0 0 0 0 0 0 0 0	Just reset: the removed piece was pinned (we're in check now).
  //
  // If the piece is an attacker (==> the corresponding bit in mask 1 is set), we need
  // to reset the bits for this direction in mask 2 and call ChessPosition::update_pinning.
  //
  //        ? ? r - - - N K	The knight is pinned.
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 1 1 1 1 1 0
  // place:         r
  // mask2: 0 0 0 0 1 1 1 0	The knight is pinned by the new piece.
  //
  // If the piece is not an attacker and the corresponding bit in mask 1 is not set,
  // then we are done.
  //
  //        ? ? r - - - N K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 1 1 1 1 1 0
  // place:   b
  // mask2: 0 0 1 1 1 1 1 0	The knight is still pinnded by the old piece (r).
  //
  // If the piece is not an attacker and the corresponding bit in mask 1 is set,
  // We have two possibilities: the corresponding bit in mask 2 is set, or not set:
  //
  // 1ba) The new piece is not an attacker and the corresponding bit in mask 1 and mask 2 are set.
  // 1bb) The new piece is not an attacker and the corresponding bit in mask 1 is set, and in mask 2 is not set.
  //
  // In case 1ba, the bits for this direction in mask 2 must be reset, and if old_code
  // is not empty, ChessPosition::update_pinning must be called.
  //
  //        ? ? r - - - N K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 1 1 1 1 1 0
  // place:     b
  // mask2: 0 0 0 0 0 0 0 0
  // place:         b
  // mask2: 0 0 0 0 0 0 0 0
  // place:             B
  // mask2: 0 0 1 1 1 1 1 0
  //
  // In case 1bb, if any bit for this direction in mask 2 is set, or if the color
  // of the piece that is placed is different from the king, we are done.
  // Otherwise we need to call ChessPosition::update_pinning.
  //
  //        ? ? r - r - N K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 0 0 1 1 1 0
  // place:       b		We are done.
  //
  //        ? ? r - - - - K
  // mask1: 0 0 1 1 1 1 1 0
  // mask2: 0 0 0 0 0 0 0 0
  // place:         B
  // mask2: 0 0 1 1 1 1 1 0
  //
  int color_count = 0;
  // Run over all colors.
  for (Color color = black; color_count < 2; ++color_count, color = white)
  {
    Code const king_code(color, king);				// The king of this color.
    Index const king_index(mask2index(M_bitboards[king_code]()));	// Where that king is.
    if (king_index == index_end)
      continue;							// If there isn't a king of that color, nothing can be pinned.
    BitBoard const line(squares_from_to(index, king_index));		// All squares from the new piece to the that king.
    if (!line.test())							// Are the piece and this king on one line at all?
      continue;							// case 2: If not, then this piece cannot influence pinning.

    // case 1:
    Direction const direction(direction_from_to(king_index, index));			// The direction from king to this piece.
    bool const attacker = (code.color() != color) && direction.matches(code.type());	// The new piece attacks the king if it is of opposite color
										      // and can move along the given direction.
    bool need_reset = false;		// Set if M_pinning might have bits set that need to be unset.
    bool need_update = false;		// Recalculate M_pinning.

    bool const corresponding_bit_in_M_attackers_is_set = M_attackers[color]() & mask;
    if (attacker)
    {
      // case 1a.
      if (!corresponding_bit_in_M_attackers_is_set)
      {
	// We have a new attacker that is even further away from the king.
	M_attackers[color] |= line;		// Extend M_attackers to include the new attacker.
      }
      need_reset = need_update = true;
    }
    else if (corresponding_bit_in_M_attackers_is_set)
    {
      // Case 1b.
      if (code.is_nothing())
      {
	// Case 1br.
	if (direction.matches(old_code.type()))
	{
	  // An attacker was removed. Check if M_attackers needs to be changed.
	  TypeData mover;
	  mover.M_bits = direction.flags & type_mask;
	  Code queen_code(color.opposite(), queen);
	  Code mover_code(color.opposite(), mover);
	  BitBoard mover_attackers(M_bitboards[mover_code] | M_bitboards[queen_code]);
	  BitBoard line(direction.from(king_index));
	  mover_attackers &= line;
	  BitBoard attackers(CW_MASK_T_CONST(0));
	  for (PieceIterator piece_iter(this, mover_attackers); piece_iter != piece_end(); ++piece_iter)
	    attackers |= squares_from_to(piece_iter.index(), king_index);
	  M_attackers[color].reset(line);
	  M_attackers[color].set(attackers);

	  // We only need an update if the corresponding bit in M_pinning is set as well
	  // (then this was the pinning attacker), or otherwise if M_pinning has no
	  // bits set at all, in which case this piece could have been blocking the pin.
	  need_reset = need_update = M_pinning[color].test(mask) || !(M_pinning[color] & line).test();
	}
	else
	{
	  // A non-attacker was removed.
	  need_reset = true;
	  // If the corresponding bit in M_pinning isn't set, then this wasn't a pinned piece, so we need to update M_pinning.
	  need_update = !(M_pinning[color]() & mask);
	}
      }
      else if ((M_pinning[color]() & mask))	// Is the corresponding bit in M_pinning set too?
      {
	// Case 1ba.
	need_reset = true;
	need_update = !old_code.is_nothing();
      }
      else
      {
	// Case 1bb.
	BitBoard line(direction.from(king_index));
	need_reset = need_update =
	    (code.color() == color || (M_en_passant.exists() && direction.is_horizontal())) && !M_pinning[color].test(line);
      }
    }
    if (need_reset)
    {
      BitBoard line(direction.from(king_index));
      M_pinning[color].reset(line);
      if (need_update)
      {
        if (__builtin_expect(M_en_passant.exists(), false))
	{
	  Index pawn_index(M_en_passant.pawn_index());
	  if (M_pieces[pawn_index].color() != color && line.test(pawn_index))
	    M_en_passant.pinned_reset();
	}
	update_pinning(king_code, king_index, M_bitboards[king_code](), direction, (M_bitboards[black] | M_bitboards[white]) & line);
      }
    }
  }

  // Update caching of in_check and M_double_check.
  bool in_check = check();
  M_castle_flags.set_check(M_to_move, in_check);
  M_castle_flags.set_check(M_to_move.opposite(), check(M_to_move.opposite()));
  M_double_check = in_check ? double_check(M_to_move) : false;

  // Success.
  return true;
}

bool ChessPosition::load_FEN(std::string const& FEN)
{
  DoutEntering(dc::notice, "ChessPosition::load_FEN(\"" << FEN << "\")");

  // Clear the position.
  clear();
  std::string::const_iterator iter = FEN.begin();
  char c;
  // Eat preceding spaces.
  while (*iter == ' ')
    ++iter;
  if (iter == FEN.end())
    return false;
  // Field 1: Piece placement.
  Color color;
  int col = 0, row = 7;
  while ((c = *iter++) != ' ')
  {
    color = white;
    switch (c)
    {
      case '/':
        if (col != 8 || row <= 0)
	  return false;
        col = 0;
        --row;
        break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        col += (int)(c - '0');
	break;
      case 'p':
      case 'r':
      case 'n':
      case 'b':
      case 'q':
      case 'k':
        color = black;
        /* FALL-THROUGH */
      case 'P':
      case 'R':
      case 'N':
      case 'B':
      case 'Q':
      case 'K':
      {
        c = std::toupper(c);
	Type type;
	if (c == 'P')
	  type = pawn;
	else if (c == 'R')
	  type = rook;
	else if (c == 'N')
	  type = knight;
	else if (c == 'B')
	  type = bishop;
	else if (c == 'Q')
	  type = queen;
	else // if (c == 'K')
	  type = king;
	place(Code(color, type), Index(col, row));
        ++col;
        break;
      }
      default:
        return false;
    }
    if (iter == FEN.end())
      return false;
  }
  if (col != 8)
    return false;
  if (iter == FEN.end())
    return false;
  // Eat extra spaces.
  while (*iter == ' ')
    if (++iter == FEN.end())
      return false;
  // Field 2: Active color.
  c = *iter++;
  if (c != 'w' && c != 'b')
    return false;
  M_to_move = (c == 'w') ? white : black;
  bool in_check = check();
  M_double_check = in_check ? double_check(M_to_move) : false;
  if (iter == FEN.end() || *iter != ' ')
    return false;
  // Eat space and possibly extra spaces.
  while (*iter == ' ')
    if (++iter == FEN.end())
      return false;
  // Field 3: Castling availability.
  uint8_t white_castle_flags = white_rook_queen_side_moved | white_king_moved | white_rook_king_side_moved;
  uint8_t black_castle_flags = black_rook_queen_side_moved | black_king_moved | black_rook_king_side_moved;
  while ((c = *iter++) != ' ')
  {
    if (c == '-')
      break;
    switch (c)
    {
      case 'K':
        white_castle_flags &= ~(white_king_moved | white_rook_king_side_moved);
	break;
      case 'Q':
        white_castle_flags &= ~(white_king_moved | white_rook_queen_side_moved);
	break;
      case 'k':
        black_castle_flags &= ~(black_king_moved | black_rook_king_side_moved);
	break;
      case 'q':
        black_castle_flags &= ~(black_king_moved | black_rook_queen_side_moved);
	break;
      default:
        return false;
    }
    if (iter == FEN.end())
      return false;
  }
  M_castle_flags = white_castle_flags | black_castle_flags;
  M_castle_flags.set_check(M_to_move, in_check);
  if (iter == FEN.end())
    return false;
  // Eat extra spaces.
  while (*iter == ' ')
    if (++iter == FEN.end())
      return false;
  // Field 4: En passant target square in algebraic notation.
  if ((c = *iter++) != '-')
  {
    if (c < 'a' || c > 'h' || iter == FEN.end())
      return false;
    int col = c - 'a';
    c = *iter++;
    if (c < '1' || c > '8' || iter == FEN.end())
      return false;
    int row = c - '1';
    Code other_pawn;
    if (M_to_move == white)
    {
      other_pawn = white_pawn;
      if (row != 5 || M_pieces[Index(col, row - 1)] != black_pawn || M_pieces[Index(col, row)] != nothing)
        return false;
    }
    else
    {
      other_pawn = black_pawn;
      if (row != 2 || M_pieces[Index(col, row + 1)] != white_pawn || M_pieces[Index(col, row)] != nothing)
        return false;
    }
    set_en_passant(Index(col, row));
  }
  if (iter == FEN.end() || *iter != ' ')
    return false;
  // Eat space and possibly extra spaces.
  while (*iter == ' ')
    if (++iter == FEN.end())
      return false;
  // Field 5: Halfmove clock: This is the number of halfmoves since the last pawn advance or capture.
  M_half_move_clock = 0;
  while ((c = *iter++) != ' ')
  {
    if (!std::isdigit(c))
      return false;
    M_half_move_clock = 10 * M_half_move_clock + c - '0';
    if (iter == FEN.end())
      return false;
  }
  // Eat extra spaces.
  while (*iter == ' ')
    if (++iter == FEN.end())
      return false;
  // Field 6: Fullmove number: The number of the full move.
  M_full_move_number = 0;
  while (iter != FEN.end() && *iter != ' ')
  {
    if (!std::isdigit(*iter))
      return false;
    M_full_move_number = 10 * M_full_move_number + *iter++ - '0';
  }
  if (M_full_move_number == 0)
    return false;
  // Success.
  return true;
}

std::string ChessPosition::FEN() const
{
  std::ostringstream fen;
  for (int row = 7; row >= 0; --row)
  {
    int empty_count = 0;
    for (int col = 0; col <= 7; ++col)
    {
      Code code = M_pieces[Index(col, row)].code();
      if (code.is_nothing())
        ++empty_count;
      else if (empty_count > 0)
      {
        fen << (char)('0' + empty_count);
	empty_count = 0;
      }
      switch (code())
      {
	case (black_bits|pawn_bits):
	  fen << 'p';
	  break;
	case (black_bits|rook_bits):
	  fen << 'r';
	  break;
	case (black_bits|knight_bits):
	  fen << 'n';
	  break;
	case (black_bits|bishop_bits):
	  fen << 'b';
	  break;
	case (black_bits|queen_bits):
	  fen << 'q';
	  break;
	case (black_bits|king_bits):
	  fen << 'k';
	  break;
	case (white_bits|pawn_bits):
	  fen << 'P';
	  break;
	case (white_bits|rook_bits):
	  fen << 'R';
	  break;
	case (white_bits|knight_bits):
	  fen << 'N';
	  break;
	case (white_bits|bishop_bits):
	  fen << 'B';
	  break;
	case (white_bits|queen_bits):
	  fen << 'Q';
	  break;
	case (white_bits|king_bits):
	  fen << 'K';
	  break;
      }
    }
    if (empty_count > 0)
      fen << (char)('0' + empty_count);
    if (row != 0)
      fen << '/';
  }
  if (M_to_move == white)
    fen << " w ";
  else
    fen << " b ";
  unsigned int flags = 0;
  if (M_castle_flags.can_castle_short(white))
    flags |= 1;
  if (M_castle_flags.can_castle_long(white))
    flags |= 2;
  if (M_castle_flags.can_castle_short(black))
    flags |= 4;
  if (M_castle_flags.can_castle_long(black))
    flags |= 8;
  if (flags == 0)
    fen << '-';
  else
  {
    for (unsigned int mask = 1; mask <= 8; mask <<= 1)
    {
      if ((flags & mask))
      {
	switch (mask)
	{
	  case 1:
	    fen << 'K';
	    break;
	  case 2:
	    fen << 'Q';
	    break;
	  case 4:
	    fen << 'k';
	    break;
	  case 8:
	    fen << 'q';
	    break;
	}
      }
    }
  }
  fen << ' ' << M_en_passant.FEN4() << ' ' << (int)M_half_move_clock << ' ' << (int)M_full_move_number;
  return fen.str();
}

bool ChessPosition::set_en_passant(Index const& index)
{
  int offset;
  Code code;
  if (index.row() == 2)
  {
    code = black_pawn;
    offset = 8;
    to_move(black);
  }
  else
  {
    code = white_pawn;
    offset = -8;
    to_move(white);
  }
  M_en_passant = EnPassant(index);
  Dout(dc::notice, "M_en_passant is set to " << (int)M_en_passant.M_bits);
  Index index_of_only_neighboring_pawn = index_end;
  bool possible = false;
  if (index.col() > 0 && M_pieces[index + offset - 1] == code)
  {
    possible = true;
    index_of_only_neighboring_pawn = index + offset - 1;
    M_pieces[index_of_only_neighboring_pawn].set_can_take_king_side();
  }
  if (index.col() < 7 && M_pieces[index + offset + 1] == code)
  {
    possible = true;
    Index right_pawn_index = index + offset + 1;
    M_pieces[right_pawn_index].set_can_take_queen_side();
    if (index_of_only_neighboring_pawn != index_end)
      index_of_only_neighboring_pawn = index_end;
    else
      index_of_only_neighboring_pawn = right_pawn_index;
  }
  if (index_of_only_neighboring_pawn != index_end)
  {
    mask_t mask = index2mask(index_of_only_neighboring_pawn);
    if (M_attackers[M_to_move].test(mask))
    {
      Code king_code(M_to_move, king);
      mask = M_bitboards[king_code]();
      Index king_index(mask2index(mask));
      Direction direction(direction_from_to(king_index, index_of_only_neighboring_pawn));
      if (direction.is_horizontal())
      {
	BitBoard line(direction.from(king_index));
	M_pinning[M_to_move].reset(line);
	update_pinning(king_code, king_index, mask, direction, (M_bitboards[black] | M_bitboards[white]) & line);
	M_en_passant.pinned_set();
      }
    }
  }
  return possible;
}

enum up_and_right {
  right = 1,
  up_left = 7,
  up = 8,
  up_right = 9,
};

enum down_and_left {
  left = 1,
  down_right = 7,
  down = 8,
  down_left = 9
};

template <typename T>
inline void move(mask_t& m, T);

template <>
inline void move<up_and_right>(mask_t& m, up_and_right offset)
{
  m <<= offset;
}

template <>
inline void move<down_and_left>(mask_t& m, down_and_left offset)
{
  m >>= offset;
}

BitBoard ChessPosition::all_pieces_minus_bishop_movers(Color const& color, Index const& index) const
{
  BitBoard result(M_bitboards[white] | M_bitboards[black]);	// A bitboard with bits set on every square where there is any piece.
  if (color.is_white())
  {
    result.reset(M_bitboards[white_queen]);	// Remove the queens and bishop, because we can defend through them.
    result.reset(M_bitboards[white_bishop]);

    BitBoard north_west_pawn(M_bitboards[white_pawn]);
    north_west_pawn &= Direction(north_west).from(index);
    if (north_west_pawn.test())
    {
      Index res(index_pre_begin);
      res.next_bit_in(north_west_pawn());
      result.reset(res);
      if (res.col() != 0)
      {
	res += north_west.offset;
	result.set(res);
      }
    }
    BitBoard north_east_pawn(M_bitboards[white_pawn]);
    north_east_pawn &= Direction(north_east).from(index);
    if (north_east_pawn.test())
    {
      Index res(index_pre_begin);
      res.next_bit_in(north_east_pawn());
      result.reset(res);
      if (res.col() != 7)
      {
	res += north_east.offset;
	result.set(res);
      }
    }
  }
  else
  {
    result.reset(M_bitboards[black_queen]);	// Remove the queens and bishop, because we can defend through them.
    result.reset(M_bitboards[black_bishop]);

    BitBoard south_west_pawn(M_bitboards[black_pawn]);
    south_west_pawn &= Direction(south_west).from(index);
    if (south_west_pawn.test())
    {
      Index res(index_end);
      res.prev_bit_in(south_west_pawn());
      result.reset(res);
      if (res.col() != 0)
      {
	res += south_west.offset;
	result.set(res);
      }
    }
    BitBoard south_east_pawn(M_bitboards[black_pawn]);
    south_east_pawn &= Direction(south_east).from(index);
    if (south_east_pawn.test())
    {
      Index res(index_end);
      res.prev_bit_in(south_east_pawn());
      result.reset(res);
      if (res.col() != 7)
      {
	res += south_east.offset;
	result.set(res);
      }
    }
  }
  return result;
}

BitBoard ChessPosition::defendables(Code const& code, Index const& index, bool& battery) const
{
  Color color = code.color();						// The color of the piece.
  mask_t pos = index2mask(index);					// The current position of the piece.
  switch(code.type()())
  {
    case nothing_bits:		// This should never be called, but return an empty bitboard anyway.
    {
      BitBoard result;
      result.reset();
      return result;
    }
    case pawn_bits:
    {
      int col = index.col();
      BitBoard result((color == white) ? (pos << 7) | (pos << 9) : (pos >> 9) | (pos >> 7));
      if (__builtin_expect(col == 0, false))
	result.reset(file_h);
      else if (__builtin_expect(col == 7, false))
	result.reset(file_a);
      return result;
    }
    case knight_bits:
    {
      BitBoard result(candidates_table[candidates_table_offset(knight) + index()]);
      return result;
    }
    case king_bits:
    {
      BitBoard result(candidates_table[candidates_table_offset(king) + index()]);
      return result;
    }
    case rook_bits:
    {
      BitBoard all_pieces_minus_rook_movers(M_bitboards[white] | M_bitboards[black]);
      Code queen_code(color, queen);
      Code rook_code(color, rook);
      BitBoard other_attackers(M_bitboards[queen_code] | M_bitboards[rook_code]);
      all_pieces_minus_rook_movers.reset(other_attackers);	// Remove the queens and rooks, because we can defend through them.

      // Find reachable squares above the rook.
      mask_t block = all_pieces_minus_rook_movers();
      mask_t probe = pos;
      mask_t reachables = 0;
      do
      {
	move(probe, up);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares below the rook.
      probe = pos;
      do
      {
        move(probe, down);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Result so far.
      BitBoard result(reachables);

      // Find reachable squares left of the rook.
      block |= file_h.M_bitmask;
      probe = pos;
      do
      {
        move(probe, left);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_h.M_bitmask;

      // Update result.
      result |= reachables;

      // Find reachable squares right of the rook.
      block = all_pieces_minus_rook_movers() | file_a.M_bitmask;
      probe = pos;
      do
      {
        move(probe, right);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_a.M_bitmask;

      result |= reachables;

      // Do we need to update M_king_battery_attack_count?
      BitBoard opposite_king_pos(M_bitboards[Code(color.opposite(), king)]);	// The position of the opposite king.
      if (__builtin_expect(
	  result.test(opposite_king_pos) &&		// Do we give check?
	  result.test(other_attackers),			// and block or look through another attacker?
	  false))
      {
	// This other attacker could be on a different line than the one to the king.
	// So, check if it is really on the line with the king or not.
	Index king_index(mask2index(opposite_king_pos()));
	Direction direction(direction_from_to(king_index, index));
	BitBoard line(direction.from(king_index));	// All squares from checked king in the direction of the new attacker.
	line &= result;					// ... but only until the blocker on the other side.
	line &= other_attackers;			// ... only possible other attackers.
	battery = line.test();				// Battery exists if line is non-empty (note that 'result' has 'index' never set).
      }

      return result;
    }
    case bishop_bits:
    {
      BitBoard all_pieces_minus_bishop_movers(this->all_pieces_minus_bishop_movers(color, index));

      // Find reachable squares left below the bishop.
      mask_t block = all_pieces_minus_bishop_movers() | file_h.M_bitmask;
      mask_t probe = pos;
      mask_t reachables = 0;
      do
      {
	move(probe, down_left);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares left above the bishop.
      probe = pos;
      do
      {
        move(probe, up_left);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_h.M_bitmask;

      // Result so far.
      BitBoard result(reachables);

      // Find reachable squares right above the bishop;
      block = all_pieces_minus_bishop_movers() | file_a.M_bitmask;
      probe = pos;
      do
      {
        move(probe, up_right);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares right below the bishop;
      probe = pos;
      do
      {
        move(probe, down_right);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_a.M_bitmask;

      result |= reachables;

      // Do we need to update M_king_battery_attack_count?
      BitBoard opposite_king_pos(M_bitboards[Code(color.opposite(), king)]);	// The position of the opposite king.
      if (__builtin_expect(result.test(opposite_king_pos), false))	// Do we give check?
      {
	Code queen_code(color, queen);
	Code bishop_code(color, bishop);
	BitBoard other_attackers(M_bitboards[queen_code] | M_bitboards[bishop_code]);
	if (result.test(other_attackers))				// and block or look through another attacker?
	{
	  // This other attacker could be on a different line than the one to the king.
	  // So, check if it is really on the line with the king or not.
	  Index king_index(mask2index(opposite_king_pos()));
	  Direction direction(direction_from_to(king_index, index));
	  BitBoard line(direction.from(king_index));	// All squares from checked king in the direction of the new attacker.
	  line &= result;					// ... but only until the blocker on the other side.
	  line &= other_attackers;			// ... only possible other attackers.
	  battery = line.test();				// Battery exists if line is non-empty (note that 'result' has 'index' never set).
	}
      }

      return result;
    }
    case queen_bits:
    {
      BitBoard all_pieces_minus_bishop_movers(this->all_pieces_minus_bishop_movers(color, index));
      BitBoard all_pieces_minus_rook_movers(M_bitboards[white] | M_bitboards[black]);	// A bitboard with bits set on every square where there is any piece.
      Code queen_code(color, queen);
      Code rook_code(color, rook);
      BitBoard other_rook_movers(M_bitboards[queen_code] | M_bitboards[rook_code]);
      all_pieces_minus_rook_movers.reset(other_rook_movers);	// Remove the queens and rooks, because we can defend through them.

      // Find reachable squares above the queen.
      mask_t block = all_pieces_minus_rook_movers();
      mask_t probe = pos;
      mask_t reachables = 0;
      do
      {
	move(probe, up);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares below the queen.
      probe = pos;
      do
      {
        move(probe, down);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Result so far.
      BitBoard result(reachables);

      // Find reachable squares left of the queen.
      block |= file_h.M_bitmask;
      probe = pos;
      do
      {
        move(probe, left);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // And left, but diagonal.
      block = all_pieces_minus_bishop_movers() | file_h.M_bitmask;

      // Find reachable squares left below the queen.
      probe = pos;
      do
      {
	move(probe, down_left);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares left above the bishop.
      probe = pos;
      do
      {
        move(probe, up_left);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_h.M_bitmask;

      // Update result.
      result |= reachables;

      // Find reachable squares right of the queen.
      block = all_pieces_minus_rook_movers() | file_a.M_bitmask;
      probe = pos;
      do
      {
        move(probe, right);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // And right, but diagonal.
      block = all_pieces_minus_bishop_movers() | file_a.M_bitmask;

      // Find reachable squares right above the queen;
      probe = pos;
      do
      {
        move(probe, up_right);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares right below the queen;
      probe = pos;
      do
      {
        move(probe, down_right);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_a.M_bitmask;

      result |= reachables;

      // Do we need to update M_king_battery_attack_count?
      BitBoard opposite_king_pos(M_bitboards[Code(color.opposite(), king)]);	// The position of the opposite king.
      if (__builtin_expect(result.test(opposite_king_pos), false))	// Do we give check?
      {
	Code queen_code(color, queen);
	Code bishop_code(color, bishop);
	BitBoard other_attackers(other_rook_movers | M_bitboards[queen_code] | M_bitboards[bishop_code]);
	if (result.test(other_attackers))				// and block or look through another attacker?
	{
	  // This other attacker could be on a different line than the one to the king.
	  // So, check if it is really on the line with the king or not.
	  Index king_index(mask2index(opposite_king_pos()));
	  Direction direction(direction_from_to(king_index, index));
	  BitBoard line(direction.from(king_index));	// All squares from checked king in the direction of the new attacker.
	  line &= result;					// ... but only until the blocker on the other side.
	  line &= other_attackers;			// ... only possible other attackers.
	  battery = line.test();				// Battery exists if line is non-empty (note that 'result' has 'index' never set).
	}
      }

      return result;
    }
  }
  return BitBoard();	// Never reached.
}

// This function updates M_defended by adding or subtracting BitBoard's
// that represent blocked squares by placing 'code' on square 'index'.
// It also updates M_king_battery_attack_count.
void ChessPosition::update_blocked_defendables(Code const& code, Index const& index, bool add)
{
  // A bitboard with bits set on every square where there is any piece.
  BitBoard const all_pieces(M_bitboards[white] | M_bitboards[black]);
  // Calculate a bitboard with bits set for every rook and queen.
  BitBoard blocked_rookmovers(M_bitboards[black_rook] | M_bitboards[black_queen] | M_bitboards[white_rook] | M_bitboards[white_queen]);
  // Calculate a bitboard with bits set for every bishop and queen.
  BitBoard blocked_bishopmovers(M_bitboards[black_bishop] | M_bitboards[black_queen] | M_bitboards[white_bishop] | M_bitboards[white_queen]);
  // However, remove those pieces from these bitboards that can defend THROUGH the considered piece and are of the same color.
  // For example, if 'code' is a white rook-- then we remove white rooks and white queens from 'blocked_rookmovers'
  // because they won't be blocked by 'code'.
  if (code.is_a_slider())
  {
    Color color(code.color());
    Code queen_code(color, queen);
    // Queen, rook or bishop.
    if (code.is_a_rookmover())
    {
      // Queen or rook.
      Code rook_code(color, rook);
      blocked_rookmovers.reset(M_bitboards[rook_code] | M_bitboards[queen_code]);
    }
    if (code.is_a_bishopmover())
    {
      // Queen or bishop.
      Code bishop_code(color, bishop);
      blocked_bishopmovers.reset(M_bitboards[bishop_code] | M_bitboards[queen_code]);
    }
  }

  // A single piece can block eight different pieces (for all eight directions). Those pieces
  // do not necessarily need to be of the same color of course. In order to update the two
  // M_defended[] variables (squares defended by white and squares defended by black), we
  // need to keep distinguish between squares that need updating because a white piece
  // is (un)blocked, and squares that need updating because a black piece is (un)blocked.

  ArrayColor<BitBoard> result;
  result[black].reset();
  result[white].reset();

  // Find the first piece above the piece at 'index'.
  //
  // This first direction is heavily commented.
  // Consider the following case:
  //
  // <-- North, South -->
  //   Q R . r . R k .
  //         ^
  //       index
  // Where code = r = black_rook, Q = white_queen, R = white_rook and k = black_king.
  //
  // This case could for example happen if the right-most white rook just took a black piece
  // and now black picks up his rook to take back.
  //
  // Note that blocked_rookmovers is:
  //   Q R . r . R k .
  //   1 1 0 0 0 1 0 0 <-- blocked_rookmovers, all rook movers or opposite color of 'r'.

  // All squares north of index.
  //   Q R . r . R k .
  //   1 1 1 0 0 0 0 0 <-- line
  BitBoard line(north.from(index));
  // All pieces north of index.
  //   Q R . r . R k .
  //   1 1 0 0 0 0 0 0 <-- line
  line &= all_pieces;

  //   Q R . r . R k .
  //                   ^
  //              blocked_piece
  Index blocked_piece(index_pre_begin);
  // The first piece north of index, if any.
  //   Q R . r . R k .
  //     ^
  // blocked_piece
  blocked_piece.next_bit_in(line());
  // Does this piece exist, and is it blocked by the piece on index?
  // If another white rook or queen was placed (or picked up) instead
  // of the black rook (r), then 'blocked_piece' would not really be
  // blocked (it can see through it). In that case the 'test' would
  // fail.
  if (__builtin_expect(blocked_piece != index_end &&
      blocked_rookmovers.test(blocked_piece), false))
  {
    // The piece is really blocked.
    //
    // Find all rooks and queens on the board of the same color
    // as this blocked piece (because they look through eachother).
    //   Q R . r . R k .
    //   1 1 0 0 0 1 0 0 <-- rookmovers_of_same_color
    // In our example this is the same as blocked_rookmovers because
    // the removed piece 'r' is a rook mover too, of opposite color.

    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code rook_code(blocked_piece_color, rook);
    Code queen_code(blocked_piece_color, queen);
    BitBoard rookmovers_of_same_color(M_bitboards[rook_code] | M_bitboards[queen_code]);

    // Find the current blocker.

    BitBoard all_blockers(all_pieces);
    // All pieces that would block 'blocked_piece'.
    //   Q R . r . R k .
    //   0 0 0 ? 0 0 1 0 <-- all_blockers
    // The question mark reflects the fact that we don't really
    // know if that rook is being removed or placed at this point.
    all_blockers.reset(rookmovers_of_same_color);
    // All squares below index.
    //   Q R . r . R k .
    //   0 0 0 0 1 1 1 1 <-- opposite_line
    BitBoard opposite_line(south.from(index));
    // All pieces below index that would block 'blocked_piece' if index is empty.
    //   Q R . r . R k .
    //   0 0 0 0 0 0 1 0 <-- all_blockers
    all_blockers &= opposite_line;
    // The first piece south of 'blocked_piece' that would block 'blocked_piece' if index is empty.
    //   Q R . r . R k .
    //               ^
    //         current_blocker
    Index current_blocker(index_end);
    current_blocker.prev_bit_in(all_blockers());

    // Calculate all squares that become/are hidden from 'blocked_piece' by the piece in index.

    // All squares south of index.
    BitBoard blocked_squares(opposite_line);
    // Minus all squares south of current blocker.
    //   Q R . r . R k .
    //   0 0 0 0 1 1 1 0 <-- blocked_squares
    if (current_blocker != index_pre_begin)
      blocked_squares.reset(south.from(current_blocker));

    // Is there anything to update?
    if (blocked_squares)
    {
      // The update of M_defended is delayed. Instead, collect all results (seperated by color).
      result[blocked_piece_color] |= blocked_squares;

      // If the current_blocker is a king of opposite color of blocked_piece,
      // then we are dealing with an attack on the king and M_king_battery_attack_count
      // might need to be updated too.
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(rookmovers_of_same_color), false))
      {
        if (add)
	  ++M_king_battery_attack_count[blocked_piece_color];
	else
	  --M_king_battery_attack_count[blocked_piece_color];
      }

      // Call call_back_battery for each additional piece that is blocked:
      // that is north of 'blocked_piece' but 'looks through' the piece on index.
      //
      // At this point we have:
      //   Q R . r . R k .
      //   1 1 0 0 0 0 0 0 <-- line
      //     ^
      // blocked_piece
      //
      // So, it will find the 'Q' next and call call_back_battery once.

      for (blocked_piece.next_bit_in(line());
           blocked_piece != index_end && rookmovers_of_same_color.test(blocked_piece);
	   blocked_piece.next_bit_in(line()))
      {
	if (add)
	  M_defended[blocked_piece_color].add(blocked_squares);
	else
	  M_defended[blocked_piece_color].sub(blocked_squares);
	if (king_attack)
	{
	  if (add)
	    ++M_king_battery_attack_count[blocked_piece_color];
	  else
	    --M_king_battery_attack_count[blocked_piece_color];
        }
      }
    }
  }

  // Find the first piece below this piece.
  line = south.from(index);
  line &= all_pieces;
  blocked_piece = index_end;
  blocked_piece.prev_bit_in(line());
  if (__builtin_expect(blocked_piece != index_pre_begin && blocked_rookmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code rook_code(blocked_piece_color, rook);
    Code queen_code(blocked_piece_color, queen);
    BitBoard rookmovers_of_same_color(M_bitboards[rook_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(rookmovers_of_same_color);
    BitBoard opposite_line(north.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_pre_begin);
    current_blocker.next_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_end)
      blocked_squares.reset(north.from(current_blocker));

    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(rookmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      if (blocked_piece != index_begin)
      {
	for (blocked_piece.prev_bit_in(line()); blocked_piece != index_pre_begin && rookmovers_of_same_color.test(blocked_piece); blocked_piece.prev_bit_in(line()))
	{
	  if (add)
	    M_defended[blocked_piece_color].add(blocked_squares);
	  else
	    M_defended[blocked_piece_color].sub(blocked_squares);
	  if (king_attack)
	  {
	    if (add)
	      ++M_king_battery_attack_count[blocked_piece_color];
	    else
	      --M_king_battery_attack_count[blocked_piece_color];
	  }
	  if (blocked_piece == index_begin)
	    break;
	}
      }
    }
  }

  // Find the first piece right of this piece.
  line = east.from(index);
  line &= all_pieces;
  blocked_piece = index_pre_begin;
  blocked_piece.next_bit_in(line());
  if (__builtin_expect(blocked_piece != index_end && blocked_rookmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code rook_code(blocked_piece_color, rook);
    Code queen_code(blocked_piece_color, queen);
    BitBoard rookmovers_of_same_color(M_bitboards[rook_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(rookmovers_of_same_color);
    BitBoard opposite_line(west.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_end);
    current_blocker.prev_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_pre_begin)
      blocked_squares.reset(west.from(current_blocker));
    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(rookmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      for (blocked_piece.next_bit_in(line()); blocked_piece != index_end && rookmovers_of_same_color.test(blocked_piece); blocked_piece.next_bit_in(line()))
      {
	if (add)
	  M_defended[blocked_piece_color].add(blocked_squares);
	else
	  M_defended[blocked_piece_color].sub(blocked_squares);
	if (king_attack)
	{
	  if (add)
	    ++M_king_battery_attack_count[blocked_piece_color];
	  else
	    --M_king_battery_attack_count[blocked_piece_color];
	}
      }
    }
  }

  // Find the first piece left of this piece.
  line = west.from(index);
  line &= all_pieces;
  blocked_piece = index_end;
  blocked_piece.prev_bit_in(line());
  if (__builtin_expect(blocked_piece != index_pre_begin && blocked_rookmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code rook_code(blocked_piece_color, rook);
    Code queen_code(blocked_piece_color, queen);
    BitBoard rookmovers_of_same_color(M_bitboards[rook_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(rookmovers_of_same_color);
    BitBoard opposite_line(east.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_pre_begin);
    current_blocker.next_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_end)
      blocked_squares.reset(east.from(current_blocker));
    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(rookmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      if (blocked_piece != index_begin)
      {
	for (blocked_piece.prev_bit_in(line()); blocked_piece != index_pre_begin && rookmovers_of_same_color.test(blocked_piece); blocked_piece.prev_bit_in(line()))
	{
	  if (add)
	    M_defended[blocked_piece_color].add(blocked_squares);
	  else
	    M_defended[blocked_piece_color].sub(blocked_squares);
	  if (king_attack)
	  {
	    if (add)
	      ++M_king_battery_attack_count[blocked_piece_color];
	    else
	      --M_king_battery_attack_count[blocked_piece_color];
	  }
	  if (blocked_piece == index_begin)
	    break;
	}
      }
    }
  }

  // Find the first piece north-west of this piece.
  line = north_west.from(index);
  line &= all_pieces;
  blocked_piece = index_pre_begin;
  blocked_piece.next_bit_in(line());
  if (__builtin_expect(blocked_piece != index_end && blocked_bishopmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code bishop_code(blocked_piece_color, bishop);
    Code queen_code(blocked_piece_color, queen);
    BitBoard bishopmovers_of_same_color(M_bitboards[bishop_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(bishopmovers_of_same_color);
    BitBoard opposite_line(south_east.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_end);
    current_blocker.prev_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_pre_begin)
      blocked_squares.reset(south_east.from(current_blocker));
    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      if (__builtin_expect(code == black_pawn, false) && blocked_piece_color == black)
	blocked_squares.reset(index + south_east.offset);
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(bishopmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      for (blocked_piece.next_bit_in(line()); blocked_piece != index_end && bishopmovers_of_same_color.test(blocked_piece); blocked_piece.next_bit_in(line()))
      {
	if (add)
	  M_defended[blocked_piece_color].add(blocked_squares);
	else
	  M_defended[blocked_piece_color].sub(blocked_squares);
	if (king_attack)
	{
	  if (add)
	    ++M_king_battery_attack_count[blocked_piece_color];
	  else
	    --M_king_battery_attack_count[blocked_piece_color];
	}
      }
    }
  }

  // Find the first piece south-east this piece.
  line = south_east.from(index);
  line &= all_pieces;
  blocked_piece = index_end;
  blocked_piece.prev_bit_in(line());
  if (__builtin_expect(blocked_piece != index_pre_begin && blocked_bishopmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code bishop_code(blocked_piece_color, bishop);
    Code queen_code(blocked_piece_color, queen);
    BitBoard bishopmovers_of_same_color(M_bitboards[bishop_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(bishopmovers_of_same_color);
    BitBoard opposite_line(north_west.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_pre_begin);
    current_blocker.next_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_end)
      blocked_squares.reset(north_west.from(current_blocker));
    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      if (__builtin_expect(code == white_pawn, false) && blocked_piece_color == white)
	blocked_squares.reset(index + north_west.offset);
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(bishopmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      if (blocked_piece != index_begin)
      {
	for (blocked_piece.prev_bit_in(line()); blocked_piece != index_pre_begin && bishopmovers_of_same_color.test(blocked_piece); blocked_piece.prev_bit_in(line()))
	{
	  if (add)
	    M_defended[blocked_piece_color].add(blocked_squares);
	  else
	    M_defended[blocked_piece_color].sub(blocked_squares);
	  if (king_attack)
	  {
	    if (add)
	      ++M_king_battery_attack_count[blocked_piece_color];
	    else
	      --M_king_battery_attack_count[blocked_piece_color];
	  }
	  if (blocked_piece == index_begin)
	    break;
	}
      }
    }
  }

  // Find the first piece north-east of this piece.
  line = north_east.from(index);
  line &= all_pieces;
  blocked_piece = index_pre_begin;
  blocked_piece.next_bit_in(line());
  if (__builtin_expect(blocked_piece != index_end && blocked_bishopmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code bishop_code(blocked_piece_color, bishop);
    Code queen_code(blocked_piece_color, queen);
    BitBoard bishopmovers_of_same_color(M_bitboards[bishop_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(bishopmovers_of_same_color);
    BitBoard opposite_line(south_west.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_end);
    current_blocker.prev_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_pre_begin)
      blocked_squares.reset(south_west.from(current_blocker));
    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      if (__builtin_expect(code == black_pawn, false) && blocked_piece_color == black)
	blocked_squares.reset(index + south_west.offset);
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(bishopmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      for (blocked_piece.next_bit_in(line()); blocked_piece != index_end && bishopmovers_of_same_color.test(blocked_piece); blocked_piece.next_bit_in(line()))
      {
	if (add)
	  M_defended[blocked_piece_color].add(blocked_squares);
	else
	  M_defended[blocked_piece_color].sub(blocked_squares);
	if (king_attack)
	{
	  if (add)
	    ++M_king_battery_attack_count[blocked_piece_color];
	  else
	    --M_king_battery_attack_count[blocked_piece_color];
	}
      }
    }
  }

  // Find the first piece south-west of this piece.
  line = south_west.from(index);
  line &= all_pieces;
  blocked_piece = index_end;
  blocked_piece.prev_bit_in(line());
  if (__builtin_expect(blocked_piece != index_pre_begin && blocked_bishopmovers.test(blocked_piece), false))
  {
    // The piece is blocked.
    Color blocked_piece_color(M_pieces[blocked_piece].color());
    Code bishop_code(blocked_piece_color, bishop);
    Code queen_code(blocked_piece_color, queen);
    BitBoard bishopmovers_of_same_color(M_bitboards[bishop_code] | M_bitboards[queen_code]);
    // Find the current blocker.
    BitBoard all_blockers(all_pieces);
    all_blockers.reset(bishopmovers_of_same_color);
    BitBoard opposite_line(north_east.from(index));
    all_blockers &= opposite_line;
    Index current_blocker(index_pre_begin);
    current_blocker.next_bit_in(all_blockers());
    BitBoard blocked_squares(opposite_line);
    if (current_blocker != index_end)
      blocked_squares.reset(north_east.from(current_blocker));
    // Call it multiple times if there is a battery.
    if (blocked_squares)
    {
      if (__builtin_expect(code == white_pawn, false) && blocked_piece_color == white)
	blocked_squares.reset(index + north_east.offset);
      result[blocked_piece_color] |= blocked_squares;
      Code king_code(blocked_piece_color.opposite(), king);
      bool king_attack = M_bitboards[king_code].test(blocked_squares);
      if (__builtin_expect(king_attack && blocked_squares.test(bishopmovers_of_same_color), false))
      {
        if (add)
          ++M_king_battery_attack_count[blocked_piece_color];
        else
          --M_king_battery_attack_count[blocked_piece_color];
      }
      if (blocked_piece != index_begin)
      {
	for (blocked_piece.prev_bit_in(line()); blocked_piece != index_pre_begin && bishopmovers_of_same_color.test(blocked_piece); blocked_piece.prev_bit_in(line()))
	{
	  if (add)
	    M_defended[blocked_piece_color].add(blocked_squares);
	  else
	    M_defended[blocked_piece_color].sub(blocked_squares);
	  if (king_attack)
	  {
	    if (add)
	      ++M_king_battery_attack_count[blocked_piece_color];
	    else
	      --M_king_battery_attack_count[blocked_piece_color];
	  }
	  if (blocked_piece == index_begin)
	    break;
	}
      }
    }
  }

  // Update M_defended with the collected results.
  if (BitBoard black_result = result[black])
  {
    if (add)
      M_defended[black].add(black_result);
    else
      M_defended[black].sub(black_result);
  }
  if (BitBoard white_result = result[white])
  {
    if (add)
      M_defended[white].add(white_result);
    else
      M_defended[white].sub(white_result);
  }
}

BitBoard ChessPosition::reachables(Index const& index, bool attacked_squares) const
{
  BitBoard all_pieces(M_bitboards[white] | M_bitboards[black]);		// A bitboard with bits set on every square where there is any piece.
  Piece piece(M_pieces[index]);						// The piece that we want to find the reachables for.
  Color color = piece.color();						// The color of the piece.
  mask_t pos = index2mask(index);					// The current position of the piece.
  switch(piece.type()())
  {
    case nothing_bits:		// This should never be called, but return an empty bitboard anyway.
    {
      BitBoard result;
      result.reset();
      return result;
    }
    case pawn_bits:
    {
      if (attacked_squares)	// Show attacked squares instead of move candidates?
      {
	BitBoard result((color == white) ? (pos << 7) | (pos << 9) : (pos >> 9) | (pos >> 7));
	result.reset(M_bitboards[color]);
	int col = index.col();
	if (__builtin_expect(col == 0, false))
	  result.reset(file_h);
        else if (__builtin_expect(col == 7, false))
	  result.reset(file_a);
	return result;
      }
      return candidates(index);	// The candidates are the reachables in the case of pawns.
    }
    case rook_bits:
    {
      // Find reachable squares above the rook.
      mask_t block = all_pieces();
      mask_t probe = pos;
      mask_t reachables = 0;
      do
      {
	move(probe, up);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares below the rook.
      probe = pos;
      do
      {
        move(probe, down);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Result so far.
      BitBoard result(reachables);

      // Find reachable squares left of the rook.
      block |= file_h.M_bitmask;
      probe = pos;
      do
      {
        move(probe, left);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_h.M_bitmask;

      // Update result.
      result |= reachables;

      // Find reachable squares right of the rook.
      block = all_pieces() | file_a.M_bitmask;
      probe = pos;
      do
      {
        move(probe, right);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_a.M_bitmask;

      result |= reachables;
      result.reset(M_bitboards[color]);				// We cannot take our own pieces.
      return result;
    }
    case knight_bits:
    {
      BitBoard result(candidates_table[candidates_table_offset(knight) + index()]);
      result.reset(M_bitboards[color]);
      return result;
    }
    case bishop_bits:
    {
      // Find reachable squares left below the bishop.
      mask_t block = all_pieces() | file_h.M_bitmask;
      mask_t probe = pos;
      mask_t reachables = 0;
      do
      {
	move(probe, down_left);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares left above the bishop.
      probe = pos;
      do
      {
        move(probe, up_left);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_h.M_bitmask;

      // Result so far.
      BitBoard result(reachables);

      // Find reachable squares right above the bishop;
      block = all_pieces() | file_a.M_bitmask;
      probe = pos;
      do
      {
        move(probe, up_right);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares right below the bishop;
      probe = pos;
      do
      {
        move(probe, down_right);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_a.M_bitmask;

      result |= reachables;
      result.reset(M_bitboards[color]);
      return result;
    }
    case queen_bits:
    {
      // Find reachable squares above the queen.
      mask_t block = all_pieces();
      mask_t probe = pos;
      mask_t reachables = 0;
      do
      {
	move(probe, up);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares below the queen.
      probe = pos;
      do
      {
        move(probe, down);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Result so far.
      BitBoard result(reachables);

      // Find reachable squares left of the queen.
      block |= file_h.M_bitmask;
      probe = pos;
      do
      {
        move(probe, left);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares left below the queen.
      probe = pos;
      do
      {
	move(probe, down_left);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares left above the bishop.
      probe = pos;
      do
      {
        move(probe, up_left);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_h.M_bitmask;

      // Update result.
      result |= reachables;

      // Find reachable squares right of the queen.
      block = all_pieces() | file_a.M_bitmask;
      probe = pos;
      do
      {
        move(probe, right);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares right above the queen;
      probe = pos;
      do
      {
        move(probe, up_right);
	reachables |= probe;
      }
      while((block & probe) != probe);

      // Find reachable squares right below the queen;
      probe = pos;
      do
      {
        move(probe, down_right);
	reachables |= probe;
      }
      while((block & probe) != probe);
      reachables &= ~file_a.M_bitmask;

      result |= reachables;
      result.reset(M_bitboards[color]);
      return result;
    }
    case king_bits:
    {
      BitBoard result(candidates_table[candidates_table_offset(king) + index()]);
      result.reset(M_bitboards[color]);
      if (attacked_squares || !M_castle_flags.can_castle(color))
        return result;
      BitBoard long_castle_squares(0), short_castle_squares(0);
      bool can_castle_long = false, can_castle_short = false;
      if (M_castle_flags.can_castle_short(color))
      {
        short_castle_squares = BitBoard((color == white) ? f1|g1 : f8|g8);
	if (!(all_pieces & short_castle_squares))
	{
	  can_castle_short = true;
	  short_castle_squares.reset(f1|f8);
	}
      }
      if (M_castle_flags.can_castle_long(color))
      {
        long_castle_squares = BitBoard((color == white) ? b1|c1|d1 : b8|c8|d8);
	if (!(all_pieces & long_castle_squares))
	{
	  can_castle_long = true;
	  long_castle_squares.reset(b1|b8|d1|d8);
	}
      }
      if (can_castle_short || can_castle_long)
      {
        // Check if the king or any of the squares involved in the castling is in check.
        BitBoard attacked_squares;
	attacked_squares.reset();
        for (PieceIterator iter = piece_begin(color.opposite()); iter != piece_end(); ++iter)
	  attacked_squares |= reachables(iter.index(), true);
        if (can_castle_short && !(attacked_squares & ((color == white) ? (e1|f1|g1) : (e8|f8|g8))))
	  result |= short_castle_squares;
        if (can_castle_long && !(attacked_squares & ((color == white) ? (c1|d1|e1) : (c8|d8|e8))))
	  result |= long_castle_squares;
      }
      return result;
    }
  }
  return BitBoard();	// Never reached.
}

BitBoard ChessPosition::moves(Index const& index) const
{
  Code code(M_pieces[index].code());
  Color color(code.color());

  BitBoard reachables(this->reachables(index));

  // If it is NOT this colors turn then it is not in check (that would be impossible in a legal position).
  // Are we in check?
  if (__builtin_expect(color == M_to_move && M_castle_flags.in_check(M_to_move), false))
  {
    bool is_king = code.is_a(king);
    // If we are in double check, then we can only move with the king.
    if (__builtin_expect(M_double_check && !is_king, false))
      return BitBoard((mask_t)0);
    // We can only move a non-king if we can take the piece that
    // gives check, or place something in front of it.
    BitBoard attacker_squares;
    attacker_squares.reset();
    Code king_code(color, king);
    BitBoard king_pos(M_bitboards[king_code]);
    Index king_index(mask2index(king_pos()));
    if (__builtin_expect(!M_double_check && !is_king, true))
    {
      if (color == black)
      {
	BitBoard queenside_pawn(king_pos() >> 9);
	BitBoard kingside_pawn(king_pos() >> 7);
	queenside_pawn.reset(file_h);
	kingside_pawn.reset(file_a);
	attacker_squares = (queenside_pawn | kingside_pawn) & M_bitboards[white_pawn];
	attacker_squares |= candidates_table[candidates_table_offset(knight) + king_index()] & M_bitboards[white_knight];
      }
      else
      {
	BitBoard queenside_pawn(king_pos() << 7);
	BitBoard kingside_pawn(king_pos() << 9);
	queenside_pawn.reset(file_h);
	kingside_pawn.reset(file_a);
	attacker_squares = (queenside_pawn | kingside_pawn) & M_bitboards[black_pawn];
	attacker_squares |= candidates_table[candidates_table_offset(knight) + king_index()] & M_bitboards[black_knight];
      }
    }
    // If attacker_squares were found then we're not in double check and we already found a check by a pawn or knight,
    // so there is no need to look for checks by sliding pieces.
    if (!attacker_squares)
    {
      BitBoard const all_pieces(M_bitboards[white] | M_bitboards[black]);
      Color opposite_color(color.opposite());
      Code rook_code(opposite_color, rook);
      Code queen_code(opposite_color, queen);
      BitBoard rookmovers(candidates_table[candidates_table_offset(rook) + king_index()] & (M_bitboards[rook_code] | M_bitboards[queen_code]));
      for (PieceIterator piece_iter(this, rookmovers); piece_iter != piece_end(); ++piece_iter)
      {
        Direction const& direction(direction_from_to(king_index, piece_iter.index()));
        BitBoard line(squares_from_to(piece_iter.index(), king_index));
	if ((line & all_pieces) == BitBoard(piece_iter.index()))
	{
	  // We found the rookmover that gives check.
	  if (is_king)
	  {
	    // If this wraps around from a-file to h-file or visa versa then that is not a problem: it will be far away from the king.
	    Index one_step_away_from_attacker(king_index - direction);
	    // However, we can't rely on reset() to work for indexes outside the board.
	    if (one_step_away_from_attacker() < 64)
	      reachables.reset(one_step_away_from_attacker);
	  }
	  else
	    attacker_squares |= line;
	  break;
	}
      }
      if (M_double_check || !attacker_squares)
      {
	Code bishop_code(opposite_color, bishop);
	BitBoard bishopmovers(candidates_table[candidates_table_offset(bishop) + king_index()] & (M_bitboards[bishop_code] | M_bitboards[queen_code]));
	for (PieceIterator piece_iter(this, bishopmovers); piece_iter != piece_end(); ++piece_iter)
	{
	  Direction const& direction(direction_from_to(king_index, piece_iter.index()));
	  BitBoard line(squares_from_to(piece_iter.index(), king_index));
	  if ((line & all_pieces) == piece_iter.index())
	  {
	    // We found the bishopmover that gives check.
	    if (is_king)
	    {
	      Index one_step_away_from_attacker(king_index - direction);
	      if (one_step_away_from_attacker() < 64)
		reachables.reset(one_step_away_from_attacker);
	    }
	    else
	      attacker_squares |= line;
	    break;
	  }
	}
      }
    }
    if (!is_king)
    {
      // The only possible move is taking the attacker, or placing something in front of it.
      reachables &= attacker_squares;
    }
  }
  BitBoard pinning(M_pinning[color]);
  if (__builtin_expect(pinning.test(index), false))
  {
    // Remove squares that would result in a check.
    if (code.is_a(pawn))
      reachables &= pinning;
    else
    {
      Index king_index(index_of_king(color));
      Direction direction(direction_from_to(king_index, index));
      BitBoard line(direction.from(king_index));
      reachables &= line;
    }
  }
  if (__builtin_expect(M_en_passant.exists() && M_en_passant.pinned() && code.is_a(pawn), false))
      reachables.reset(M_en_passant.index());		// Taking en passant is prohibitted.
  if (__builtin_expect(code.is_a(king), false))
    reachables.reset(M_defended[color.opposite()].any());	// The king can't go a square where it puts itself in check.

  return reachables;
}

bool ChessPosition::legal(Move const& move) const
{
  Index from(move.from());
  Index to(move.to());
  // Lets play on the board.
  if (to() > 63 || from() > 63)
    return false;
  BitBoard from_pos(from);
  BitBoard to_pos(to);
  Piece const& piece(M_pieces[from]);
  // Only touch your own pieces.
  if (piece.code().is_nothing() || piece.color() != M_to_move)
    return false;
  bool is_pawn_promotion = piece.code().is_a(pawn) && (to.row() == 0 || to.row() == 7);
  // Only allow promotion of pawns.
  if (is_pawn_promotion)
  {
    Type promotion_type(move.promotion_type());
    if (promotion_type != rook && promotion_type != knight && promotion_type != bishop && promotion_type != queen)
      return false;
  }
  else if (move.promotion_type() != nothing)
    return false;
  // Get all legal squares to move to for this piece and test if we try to go one of them.
  return moves(from).test(to_pos);
}

bool ChessPosition::execute(Move const& move)
{
  BitBoard from_pos(move.from());
  BitBoard to_pos(move.to());
  BitBoard all_pieces(M_bitboards[black] | M_bitboards[white]);
  bool pawn_move = (from_pos & M_bitboards[Code(M_to_move, pawn)]);
  bool pawn_advance_or_capture = pawn_move | !!(to_pos & all_pieces);
  // Handle en passant.
  if (__builtin_expect(M_en_passant.exists(), false))
  {
    if (pawn_move && M_en_passant.index() == move.to())
    {
      // A pawn was taken en passant, remove it.
      // This resets M_en_passant.
      place(Code(), M_en_passant.pawn_index());
    }
    else
    {
      // Clear the right to take en passant.
      clear_en_passant();
    }
  }
  // Set en passant flags, if applicable.
  if (pawn_move)
  {
    uint8_t offset = move.to()() - move.from()();	// -16, -9, -8, -7, 7, 8, 9 or 16.
    bool pawn_advanced_two_squares = !(offset & 0xf);	// Only -16 and 16 have the last four bits clear.
    if (pawn_advanced_two_squares)
    {
      // Mark that we can take this pawn en passant.
      // Toggling the third bit finds the passed square: row 3 becomes row 2, and row 4 becomes row 5.
      IndexData passed_square = { static_cast<uint8_t>(move.to()() ^ 8) };
      set_en_passant(passed_square);
      // set_en_passant toggles the move, toggle it back because we'll toggle it again below.
      M_to_move.toggle();
    }
  }

  // FIXME. For now, use place() to execute the move.
  CastleFlags castle_flags(M_castle_flags);
  Piece piece(M_pieces[move.from()]);
  place(Code(), move.from());			// 160 ns.
  if (move.is_promotion())
    place(Code(M_to_move, move.promotion_type()), move.to());
  else
  {
    place(piece.code(), move.to());		// 110 ns.
    // Is this a castling?
    uint8_t col_diff = move.to().col() - move.from().col();
    if (__builtin_expect(piece.code().is_a(king), false) && __builtin_expect(col_diff && !(col_diff & 1), false))
    {
      IndexData rook_from = { static_cast<uint8_t>(move.from()() - 4 + 7 * (2 + move.to()() - move.from()()) / 4) };
      IndexData rook_to = { static_cast<uint8_t>(move.from()() + (move.to()() - move.from()()) / 2) };
      place(Code(), rook_from);
      place(Code(M_to_move, rook), rook_to);
    }
  }

  // Change whose turn it is.
  M_to_move.toggle();

  // FIXME. Correct M_castle_flags again.
  bool in_check = M_castle_flags.in_check(M_to_move);
  M_castle_flags = castle_flags;
  M_castle_flags.set_check(M_to_move, in_check);
  M_castle_flags.piece_moved_from(piece, move.from());

  // Cache whether or not we gave a double check.
  M_double_check = M_castle_flags.in_check(M_to_move) ? double_check(M_to_move) : false;
  // Increment the counters and return whether or not it's a draw by the 50 move rule.
  return increment_counters(pawn_advance_or_capture);
}

BitBoardData ChessPosition::candidates_table[5 * 64] = {
  // Knight
  { CW_MASK_T_CONST(0x0000000000020400)}, { CW_MASK_T_CONST(0x0000000000050800)}, { CW_MASK_T_CONST(0x00000000000a1100)}, { CW_MASK_T_CONST(0x0000000000142200)},
  { CW_MASK_T_CONST(0x0000000000284400)}, { CW_MASK_T_CONST(0x0000000000508800)}, { CW_MASK_T_CONST(0x0000000000a01000)}, { CW_MASK_T_CONST(0x0000000000402000)},
  { CW_MASK_T_CONST(0x0000000002040004)}, { CW_MASK_T_CONST(0x0000000005080008)}, { CW_MASK_T_CONST(0x000000000a110011)}, { CW_MASK_T_CONST(0x0000000014220022)},
  { CW_MASK_T_CONST(0x0000000028440044)}, { CW_MASK_T_CONST(0x0000000050880088)}, { CW_MASK_T_CONST(0x00000000a0100010)}, { CW_MASK_T_CONST(0x0000000040200020)},
  { CW_MASK_T_CONST(0x0000000204000402)}, { CW_MASK_T_CONST(0x0000000508000805)}, { CW_MASK_T_CONST(0x0000000a1100110a)}, { CW_MASK_T_CONST(0x0000001422002214)},
  { CW_MASK_T_CONST(0x0000002844004428)}, { CW_MASK_T_CONST(0x0000005088008850)}, { CW_MASK_T_CONST(0x000000a0100010a0)}, { CW_MASK_T_CONST(0x0000004020002040)},
  { CW_MASK_T_CONST(0x0000020400040200)}, { CW_MASK_T_CONST(0x0000050800080500)}, { CW_MASK_T_CONST(0x00000a1100110a00)}, { CW_MASK_T_CONST(0x0000142200221400)},
  { CW_MASK_T_CONST(0x0000284400442800)}, { CW_MASK_T_CONST(0x0000508800885000)}, { CW_MASK_T_CONST(0x0000a0100010a000)}, { CW_MASK_T_CONST(0x0000402000204000)},
  { CW_MASK_T_CONST(0x0002040004020000)}, { CW_MASK_T_CONST(0x0005080008050000)}, { CW_MASK_T_CONST(0x000a1100110a0000)}, { CW_MASK_T_CONST(0x0014220022140000)},
  { CW_MASK_T_CONST(0x0028440044280000)}, { CW_MASK_T_CONST(0x0050880088500000)}, { CW_MASK_T_CONST(0x00a0100010a00000)}, { CW_MASK_T_CONST(0x0040200020400000)},
  { CW_MASK_T_CONST(0x0204000402000000)}, { CW_MASK_T_CONST(0x0508000805000000)}, { CW_MASK_T_CONST(0x0a1100110a000000)}, { CW_MASK_T_CONST(0x1422002214000000)},
  { CW_MASK_T_CONST(0x2844004428000000)}, { CW_MASK_T_CONST(0x5088008850000000)}, { CW_MASK_T_CONST(0xa0100010a0000000)}, { CW_MASK_T_CONST(0x4020002040000000)},
  { CW_MASK_T_CONST(0x0400040200000000)}, { CW_MASK_T_CONST(0x0800080500000000)}, { CW_MASK_T_CONST(0x1100110a00000000)}, { CW_MASK_T_CONST(0x2200221400000000)},
  { CW_MASK_T_CONST(0x4400442800000000)}, { CW_MASK_T_CONST(0x8800885000000000)}, { CW_MASK_T_CONST(0x100010a000000000)}, { CW_MASK_T_CONST(0x2000204000000000)},
  { CW_MASK_T_CONST(0x0004020000000000)}, { CW_MASK_T_CONST(0x0008050000000000)}, { CW_MASK_T_CONST(0x00110a0000000000)}, { CW_MASK_T_CONST(0x0022140000000000)},
  { CW_MASK_T_CONST(0x0044280000000000)}, { CW_MASK_T_CONST(0x0088500000000000)}, { CW_MASK_T_CONST(0x0010a00000000000)}, { CW_MASK_T_CONST(0x0020400000000000)},
  // King
  { CW_MASK_T_CONST(0x0000000000000302)}, { CW_MASK_T_CONST(0x0000000000000705)}, { CW_MASK_T_CONST(0x0000000000000e0a)}, { CW_MASK_T_CONST(0x0000000000001c14)},
  { CW_MASK_T_CONST(0x0000000000003828)}, { CW_MASK_T_CONST(0x0000000000007050)}, { CW_MASK_T_CONST(0x000000000000e0a0)}, { CW_MASK_T_CONST(0x000000000000c040)},
  { CW_MASK_T_CONST(0x0000000000030203)}, { CW_MASK_T_CONST(0x0000000000070507)}, { CW_MASK_T_CONST(0x00000000000e0a0e)}, { CW_MASK_T_CONST(0x00000000001c141c)},
  { CW_MASK_T_CONST(0x0000000000382838)}, { CW_MASK_T_CONST(0x0000000000705070)}, { CW_MASK_T_CONST(0x0000000000e0a0e0)}, { CW_MASK_T_CONST(0x0000000000c040c0)},
  { CW_MASK_T_CONST(0x0000000003020300)}, { CW_MASK_T_CONST(0x0000000007050700)}, { CW_MASK_T_CONST(0x000000000e0a0e00)}, { CW_MASK_T_CONST(0x000000001c141c00)},
  { CW_MASK_T_CONST(0x0000000038283800)}, { CW_MASK_T_CONST(0x0000000070507000)}, { CW_MASK_T_CONST(0x00000000e0a0e000)}, { CW_MASK_T_CONST(0x00000000c040c000)},
  { CW_MASK_T_CONST(0x0000000302030000)}, { CW_MASK_T_CONST(0x0000000705070000)}, { CW_MASK_T_CONST(0x0000000e0a0e0000)}, { CW_MASK_T_CONST(0x0000001c141c0000)},
  { CW_MASK_T_CONST(0x0000003828380000)}, { CW_MASK_T_CONST(0x0000007050700000)}, { CW_MASK_T_CONST(0x000000e0a0e00000)}, { CW_MASK_T_CONST(0x000000c040c00000)},
  { CW_MASK_T_CONST(0x0000030203000000)}, { CW_MASK_T_CONST(0x0000070507000000)}, { CW_MASK_T_CONST(0x00000e0a0e000000)}, { CW_MASK_T_CONST(0x00001c141c000000)},
  { CW_MASK_T_CONST(0x0000382838000000)}, { CW_MASK_T_CONST(0x0000705070000000)}, { CW_MASK_T_CONST(0x0000e0a0e0000000)}, { CW_MASK_T_CONST(0x0000c040c0000000)},
  { CW_MASK_T_CONST(0x0003020300000000)}, { CW_MASK_T_CONST(0x0007050700000000)}, { CW_MASK_T_CONST(0x000e0a0e00000000)}, { CW_MASK_T_CONST(0x001c141c00000000)},
  { CW_MASK_T_CONST(0x0038283800000000)}, { CW_MASK_T_CONST(0x0070507000000000)}, { CW_MASK_T_CONST(0x00e0a0e000000000)}, { CW_MASK_T_CONST(0x00c040c000000000)},
  { CW_MASK_T_CONST(0x0302030000000000)}, { CW_MASK_T_CONST(0x0705070000000000)}, { CW_MASK_T_CONST(0x0e0a0e0000000000)}, { CW_MASK_T_CONST(0x1c141c0000000000)},
  { CW_MASK_T_CONST(0x3828380000000000)}, { CW_MASK_T_CONST(0x7050700000000000)}, { CW_MASK_T_CONST(0xe0a0e00000000000)}, { CW_MASK_T_CONST(0xc040c00000000000)},
  { CW_MASK_T_CONST(0x0203000000000000)}, { CW_MASK_T_CONST(0x0507000000000000)}, { CW_MASK_T_CONST(0x0a0e000000000000)}, { CW_MASK_T_CONST(0x141c000000000000)},
  { CW_MASK_T_CONST(0x2838000000000000)}, { CW_MASK_T_CONST(0x5070000000000000)}, { CW_MASK_T_CONST(0xa0e0000000000000)}, { CW_MASK_T_CONST(0x40c0000000000000)},
  // Bishop
  { CW_MASK_T_CONST(0x8040201008040200)}, { CW_MASK_T_CONST(0x0080402010080500)}, { CW_MASK_T_CONST(0x0000804020110a00)}, { CW_MASK_T_CONST(0x0000008041221400)},
  { CW_MASK_T_CONST(0x0000000182442800)}, { CW_MASK_T_CONST(0x0000010204885000)}, { CW_MASK_T_CONST(0x000102040810a000)}, { CW_MASK_T_CONST(0x0102040810204000)},
  { CW_MASK_T_CONST(0x4020100804020002)}, { CW_MASK_T_CONST(0x8040201008050005)}, { CW_MASK_T_CONST(0x00804020110a000a)}, { CW_MASK_T_CONST(0x0000804122140014)},
  { CW_MASK_T_CONST(0x0000018244280028)}, { CW_MASK_T_CONST(0x0001020488500050)}, { CW_MASK_T_CONST(0x0102040810a000a0)}, { CW_MASK_T_CONST(0x0204081020400040)},
  { CW_MASK_T_CONST(0x2010080402000204)}, { CW_MASK_T_CONST(0x4020100805000508)}, { CW_MASK_T_CONST(0x804020110a000a11)}, { CW_MASK_T_CONST(0x0080412214001422)},
  { CW_MASK_T_CONST(0x0001824428002844)}, { CW_MASK_T_CONST(0x0102048850005088)}, { CW_MASK_T_CONST(0x02040810a000a010)}, { CW_MASK_T_CONST(0x0408102040004020)},
  { CW_MASK_T_CONST(0x1008040200020408)}, { CW_MASK_T_CONST(0x2010080500050810)}, { CW_MASK_T_CONST(0x4020110a000a1120)}, { CW_MASK_T_CONST(0x8041221400142241)},
  { CW_MASK_T_CONST(0x0182442800284482)}, { CW_MASK_T_CONST(0x0204885000508804)}, { CW_MASK_T_CONST(0x040810a000a01008)}, { CW_MASK_T_CONST(0x0810204000402010)},
  { CW_MASK_T_CONST(0x0804020002040810)}, { CW_MASK_T_CONST(0x1008050005081020)}, { CW_MASK_T_CONST(0x20110a000a112040)}, { CW_MASK_T_CONST(0x4122140014224180)},
  { CW_MASK_T_CONST(0x8244280028448201)}, { CW_MASK_T_CONST(0x0488500050880402)}, { CW_MASK_T_CONST(0x0810a000a0100804)}, { CW_MASK_T_CONST(0x1020400040201008)},
  { CW_MASK_T_CONST(0x0402000204081020)}, { CW_MASK_T_CONST(0x0805000508102040)}, { CW_MASK_T_CONST(0x110a000a11204080)}, { CW_MASK_T_CONST(0x2214001422418000)},
  { CW_MASK_T_CONST(0x4428002844820100)}, { CW_MASK_T_CONST(0x8850005088040201)}, { CW_MASK_T_CONST(0x10a000a010080402)}, { CW_MASK_T_CONST(0x2040004020100804)},
  { CW_MASK_T_CONST(0x0200020408102040)}, { CW_MASK_T_CONST(0x0500050810204080)}, { CW_MASK_T_CONST(0x0a000a1120408000)}, { CW_MASK_T_CONST(0x1400142241800000)},
  { CW_MASK_T_CONST(0x2800284482010000)}, { CW_MASK_T_CONST(0x5000508804020100)}, { CW_MASK_T_CONST(0xa000a01008040201)}, { CW_MASK_T_CONST(0x4000402010080402)},
  { CW_MASK_T_CONST(0x0002040810204080)}, { CW_MASK_T_CONST(0x0005081020408000)}, { CW_MASK_T_CONST(0x000a112040800000)}, { CW_MASK_T_CONST(0x0014224180000000)},
  { CW_MASK_T_CONST(0x0028448201000000)}, { CW_MASK_T_CONST(0x0050880402010000)}, { CW_MASK_T_CONST(0x00a0100804020100)}, { CW_MASK_T_CONST(0x0040201008040201)},
  // Rook
  { CW_MASK_T_CONST(0x01010101010101fe)}, { CW_MASK_T_CONST(0x02020202020202fd)}, { CW_MASK_T_CONST(0x04040404040404fb)}, { CW_MASK_T_CONST(0x08080808080808f7)},
  { CW_MASK_T_CONST(0x10101010101010ef)}, { CW_MASK_T_CONST(0x20202020202020df)}, { CW_MASK_T_CONST(0x40404040404040bf)}, { CW_MASK_T_CONST(0x808080808080807f)},
  { CW_MASK_T_CONST(0x010101010101fe01)}, { CW_MASK_T_CONST(0x020202020202fd02)}, { CW_MASK_T_CONST(0x040404040404fb04)}, { CW_MASK_T_CONST(0x080808080808f708)},
  { CW_MASK_T_CONST(0x101010101010ef10)}, { CW_MASK_T_CONST(0x202020202020df20)}, { CW_MASK_T_CONST(0x404040404040bf40)}, { CW_MASK_T_CONST(0x8080808080807f80)},
  { CW_MASK_T_CONST(0x0101010101fe0101)}, { CW_MASK_T_CONST(0x0202020202fd0202)}, { CW_MASK_T_CONST(0x0404040404fb0404)}, { CW_MASK_T_CONST(0x0808080808f70808)},
  { CW_MASK_T_CONST(0x1010101010ef1010)}, { CW_MASK_T_CONST(0x2020202020df2020)}, { CW_MASK_T_CONST(0x4040404040bf4040)}, { CW_MASK_T_CONST(0x80808080807f8080)},
  { CW_MASK_T_CONST(0x01010101fe010101)}, { CW_MASK_T_CONST(0x02020202fd020202)}, { CW_MASK_T_CONST(0x04040404fb040404)}, { CW_MASK_T_CONST(0x08080808f7080808)},
  { CW_MASK_T_CONST(0x10101010ef101010)}, { CW_MASK_T_CONST(0x20202020df202020)}, { CW_MASK_T_CONST(0x40404040bf404040)}, { CW_MASK_T_CONST(0x808080807f808080)},
  { CW_MASK_T_CONST(0x010101fe01010101)}, { CW_MASK_T_CONST(0x020202fd02020202)}, { CW_MASK_T_CONST(0x040404fb04040404)}, { CW_MASK_T_CONST(0x080808f708080808)},
  { CW_MASK_T_CONST(0x101010ef10101010)}, { CW_MASK_T_CONST(0x202020df20202020)}, { CW_MASK_T_CONST(0x404040bf40404040)}, { CW_MASK_T_CONST(0x8080807f80808080)},
  { CW_MASK_T_CONST(0x0101fe0101010101)}, { CW_MASK_T_CONST(0x0202fd0202020202)}, { CW_MASK_T_CONST(0x0404fb0404040404)}, { CW_MASK_T_CONST(0x0808f70808080808)},
  { CW_MASK_T_CONST(0x1010ef1010101010)}, { CW_MASK_T_CONST(0x2020df2020202020)}, { CW_MASK_T_CONST(0x4040bf4040404040)}, { CW_MASK_T_CONST(0x80807f8080808080)},
  { CW_MASK_T_CONST(0x01fe010101010101)}, { CW_MASK_T_CONST(0x02fd020202020202)}, { CW_MASK_T_CONST(0x04fb040404040404)}, { CW_MASK_T_CONST(0x08f7080808080808)},
  { CW_MASK_T_CONST(0x10ef101010101010)}, { CW_MASK_T_CONST(0x20df202020202020)}, { CW_MASK_T_CONST(0x40bf404040404040)}, { CW_MASK_T_CONST(0x807f808080808080)},
  { CW_MASK_T_CONST(0xfe01010101010101)}, { CW_MASK_T_CONST(0xfd02020202020202)}, { CW_MASK_T_CONST(0xfb04040404040404)}, { CW_MASK_T_CONST(0xf708080808080808)},
  { CW_MASK_T_CONST(0xef10101010101010)}, { CW_MASK_T_CONST(0xdf20202020202020)}, { CW_MASK_T_CONST(0xbf40404040404040)}, { CW_MASK_T_CONST(0x7f80808080808080)},
  // Queen
  { CW_MASK_T_CONST(0x81412111090503fe)}, { CW_MASK_T_CONST(0x02824222120a07fd)}, { CW_MASK_T_CONST(0x0404844424150efb)}, { CW_MASK_T_CONST(0x08080888492a1cf7)},
  { CW_MASK_T_CONST(0x10101011925438ef)}, { CW_MASK_T_CONST(0x2020212224a870df)}, { CW_MASK_T_CONST(0x404142444850e0bf)}, { CW_MASK_T_CONST(0x8182848890a0c07f)},
  { CW_MASK_T_CONST(0x412111090503fe03)}, { CW_MASK_T_CONST(0x824222120a07fd07)}, { CW_MASK_T_CONST(0x04844424150efb0e)}, { CW_MASK_T_CONST(0x080888492a1cf71c)},
  { CW_MASK_T_CONST(0x101011925438ef38)}, { CW_MASK_T_CONST(0x20212224a870df70)}, { CW_MASK_T_CONST(0x4142444850e0bfe0)}, { CW_MASK_T_CONST(0x82848890a0c07fc0)},
  { CW_MASK_T_CONST(0x2111090503fe0305)}, { CW_MASK_T_CONST(0x4222120a07fd070a)}, { CW_MASK_T_CONST(0x844424150efb0e15)}, { CW_MASK_T_CONST(0x0888492a1cf71c2a)},
  { CW_MASK_T_CONST(0x1011925438ef3854)}, { CW_MASK_T_CONST(0x212224a870df70a8)}, { CW_MASK_T_CONST(0x42444850e0bfe050)}, { CW_MASK_T_CONST(0x848890a0c07fc0a0)},
  { CW_MASK_T_CONST(0x11090503fe030509)}, { CW_MASK_T_CONST(0x22120a07fd070a12)}, { CW_MASK_T_CONST(0x4424150efb0e1524)}, { CW_MASK_T_CONST(0x88492a1cf71c2a49)},
  { CW_MASK_T_CONST(0x11925438ef385492)}, { CW_MASK_T_CONST(0x2224a870df70a824)}, { CW_MASK_T_CONST(0x444850e0bfe05048)}, { CW_MASK_T_CONST(0x8890a0c07fc0a090)},
  { CW_MASK_T_CONST(0x090503fe03050911)}, { CW_MASK_T_CONST(0x120a07fd070a1222)}, { CW_MASK_T_CONST(0x24150efb0e152444)}, { CW_MASK_T_CONST(0x492a1cf71c2a4988)},
  { CW_MASK_T_CONST(0x925438ef38549211)}, { CW_MASK_T_CONST(0x24a870df70a82422)}, { CW_MASK_T_CONST(0x4850e0bfe0504844)}, { CW_MASK_T_CONST(0x90a0c07fc0a09088)},
  { CW_MASK_T_CONST(0x0503fe0305091121)}, { CW_MASK_T_CONST(0x0a07fd070a122242)}, { CW_MASK_T_CONST(0x150efb0e15244484)}, { CW_MASK_T_CONST(0x2a1cf71c2a498808)},
  { CW_MASK_T_CONST(0x5438ef3854921110)}, { CW_MASK_T_CONST(0xa870df70a8242221)}, { CW_MASK_T_CONST(0x50e0bfe050484442)}, { CW_MASK_T_CONST(0xa0c07fc0a0908884)},
  { CW_MASK_T_CONST(0x03fe030509112141)}, { CW_MASK_T_CONST(0x07fd070a12224282)}, { CW_MASK_T_CONST(0x0efb0e1524448404)}, { CW_MASK_T_CONST(0x1cf71c2a49880808)},
  { CW_MASK_T_CONST(0x38ef385492111010)}, { CW_MASK_T_CONST(0x70df70a824222120)}, { CW_MASK_T_CONST(0xe0bfe05048444241)}, { CW_MASK_T_CONST(0xc07fc0a090888482)},
  { CW_MASK_T_CONST(0xfe03050911214181)}, { CW_MASK_T_CONST(0xfd070a1222428202)}, { CW_MASK_T_CONST(0xfb0e152444840404)}, { CW_MASK_T_CONST(0xf71c2a4988080808)},
  { CW_MASK_T_CONST(0xef38549211101010)}, { CW_MASK_T_CONST(0xdf70a82422212020)}, { CW_MASK_T_CONST(0xbfe0504844424140)}, { CW_MASK_T_CONST(0x7fc0a09088848281)},
};

} // namespace cwchess
