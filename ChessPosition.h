// cwchessboard -- A C++ chessboard tool set
//
//! @file ChessPosition.h This file contains the declaration of class ChessPosition.
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

#ifndef CHESSPOSITION_H
#define CHESSPOSITION_H

#ifndef USE_PCH
#endif

#include "BitBoard.h"
#include "Piece.h"
#include "PieceIterator.h"
#include "MoveIterator.h"
#include "Array.h"
#include "CastleFlags.h"
#include "EnPassant.h"
#include "CountBoard.h"

namespace cwchess {

/** @brief A chess position.
 *
 * This class represents a chess position.
 * It has all information regarding a single position,
 * including the full move count (because that is needed
 * for the FEN code), the half move clock (to determine
 * whether a derived position becomes a draw due to the
 * 50 moves rule), en passant and castling information.
 */
class ChessPosition {
  private:
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

  public:

  /** @name Constructor */
  //@{

    //! @brief Construct an uninitialized position.
    //
    // Because ChessPosition::clear doesn't set M_to_move, the only
    // member variable initialized here is M_to_move because it needs
    // to be set to something before we can call ChessPosition::place,
    // or uninitialized memory would be used.
    ChessPosition() : M_to_move(white) { }

  //@}

  /** @name Position setup */
  //@{

    /** @brief Clear the board.
     *
     * This does not change whose move it is.
     * The half move clock and full move number are reset (to zero and one respectively).
     */
    void clear();

    /** @brief Set up the initial position. */
    void initial_position();

    /** @brief Skip a move.
     *
     * This resets the right to capture en passant.
     * Skipping a move is counted as move.
     *
     * @returns TRUE if the result is a draw due to the 50 moves rule.
     */
    bool skip_move();

    /** @brief Explicitly set whose turn it is.
     *
     * This does not change the half move clock or full move number, nor does it change the en passant information.
     * The reason for that is that this call is probably done as part of a position setup and calls that explicitely
     * set en passant information and the move counters might already have been called. However, calling this
     * an odd number of times after en passant information was set therefore leaves the position is an erroneous state.
     *
     * If you just want to change who is to move from a given position, then use %skip_move.
     *
     * @param color : The color that has to move next from this position.
     */
    void to_move(Color const& color);

    /** @brief Explicitly set the number of half moves since the last pawn move or capture.
     *
     * This does not change the full move number, en passant information or whose turn it is.
     */
    void set_half_move_clock(int count) { M_half_move_clock = count; }

    /** @brief Explicitly set the move number.
     *
     * This does not change the half move clock, en passant information or whose turn it is.
     */
    void set_full_move_number(int move) { M_full_move_number = move; }

    /** @brief Explicity set the en passant information.
     *
     * This also sets whose turn it is. No error checking is done on whether there is
     * actually a pawn that could have just moved two squares.
     *
     * @param index : The square that a pawn just passed by advancing two square.
     *
     * @returns TRUE if there are indeed one or two pawns that can take en passant.
     */
    bool set_en_passant(Index const& index);

    /** @brief Swap colors.
     *
     * This changes the color of each piece,
     * toggles whose turn it is and mirrors the position vertically.
     * The resulting position is technically identical to the original position,
     * but with different colors for each piece.
     *
     * In order to make the position technically equivalent, the half
     * move clock is unchanged; however, the full move number is reset
     * (to one) because it would impossible to really have the same
     * history: it is not possible that black started the game.
     */
    void swap_colors();

    /** @brief Place a piece on the board.
     *
     * Place a \a piece on the board.
     * If there is already a piece there, it is replaced.
     * It is also possible to place 'nothing', in which case any piece that was there is removed.
     *
     * If the king or a rook is placed, then the castling flags are updated to
     * allow castling by default. If this is not what you want, then call %moved
     * to signal that castling is not allowed <em>after</em> placing both, the king
     * and the rook(s).
     *
     * If a pawn is marked as En Passant then that is persistent: it will stay
     * that way (placing a piece is not the same as making a move). However, if
     * the pawn that can be taken en passant is moved, replaced (by again a pawn)
     * or when a piece is put behind it, so that it's impossible that it was
     * the last move (no logic will check other reasons, like being in check),
     * then its En Passant status is reset. For example, if there is a white
     * pawn on f4 that can be taken en passant by a black pawn on g4, and the
     * pawn on g4 is removed or replaced then that doesn't change the status
     * of the pawn on f4. If (later) a black pawn in placed on g4, then that
     * pawn will be able to take the white pawn on f4, provided that nothing
     * has been placed, even temporarily, on f3 or f2, and the pawn on f4 was
     * never replaced.
     *
     * At the moment the only placement that is refused is placing pawns on rank 1 or 8.
     *
     * @param code : The piece to be placed on the board.
     * @param index : Where to place the piece.
     *
     * @returns TRUE if the piece was placed, FALSE if it was refused.
     */
    bool place(Code const& code, Index const& index);

    /** @brief Specifically specify that a king or rook has already moved.
     *
     * Calling this function is only necessary if king and rook are both
     * on the initial squares. Placing the rook or king after calling
     * this function will reset the castling flags (to allow castling);
     * therefore first place the pieces and then, if necessary, call
     * this function.
     *
     * @param index : The (initial position of the) rook or king.
     */
    void set_has_moved(Index const& index) { M_castle_flags.piece_moved_from(piece_at(index), index); }

    /** @brief Specifically specify that a king or rook didn't move yet.
     *
     * This function only has effect if the king or rook is on its initial square.
     *
     * @param index : The (initial position of the) rook or king.
     */
    void clear_has_moved(Index const& index) { M_castle_flags.update_placed(piece_at(index).code(), index); }

    /** @brief Read a FEN code.
     *
     * @param FEN : A Forsyth-Edwards Notation.
     *
     * If the FEN code is non-parsable, the function return FALSE and
     * the position is in an undefined state. Therefore you might
     * want to use the function as follows:
     *
     * <pre>
     * ChessPosition tmp(current_chess_position);
     * if (tmp.load_FEN(fen))
     *   current_chess_position = tmp;
     * else
     * {
     *   // report error
     * }
     * </pre>
     *
     * @returns TRUE if loading was successful.
     */
    bool load_FEN(std::string const& FEN);

  //@}

#ifndef DOXYGEN
    // Needs access to M_pieces.
    friend class PieceIterator;
#endif

  /** @name Accessors */
  //@{

    /** @brief Return the Piece on the square \a index. */
    Piece piece_at(Index const& index) const { return M_pieces[index]; }

    /** @brief Return the Piece on the square \a col, \a row. */
    Piece piece_at(int col, int row) const { return M_pieces[Index(col, row)]; }

    /** @brief Return whose turn it is. */
    Color to_move() const { return M_to_move; }

    /** @brief Return the half move clock. */
    unsigned int half_move_clock() const { return M_half_move_clock; }

    /** @brief Return the full move number. */
    unsigned int full_move_number() const { return M_full_move_number; }

    /** @brief Return the castle flag object. */
    CastleFlags const& castle_flags() const { return M_castle_flags; }

    /** @brief Return the en passant object. */
    EnPassant const& en_passant() const { return M_en_passant; }

    /** @brief Return a BitBoard with bits set for all \a code, where \a code may not be 'nothing'. */
    BitBoard const& all(Code const& code) const { return M_bitboards[code]; }

    /** @brief Return a BitBoard with bits set for all pieces of color \a color. */
    BitBoard const& all(Color const& color) const { return M_bitboards[color]; }

  //@}

public_notdocumented:
    // Added for debugging purposes.
    ArrayColor<CountBoard> const& get_defended() const { return M_defended; }
    BitBoard attackers(Color const& color) const { return M_attackers[color]; }
    BitBoard pinned(Color const& color) const { return M_pinning[color]; }

public:
  /** @name Visitors */
  //@{

    /** @brief Return the FEN code for this position. */
    std::string FEN() const;

    /** @brief Return the offset into the candidates_table for type \a type.
     *
     * The type may not be a pawn (there is no candidates_table entry for a pawn).
     */
    int candidates_table_offset(Type const& type) const
    {
      int n = type();
      n -= (n > 4) ? 3 : 2;
      n <<= 6;
      return n;
    }

    /** @brief Return a BitBoard with bits set for all squares that are candidates to move to.
     *
     * In the case of the pawn, the returned BitBoard is exact.
     * In the case of the king, the castling squares are not included.
     * In all other cases the returned squares assume an empty board.
     *
     * This function may not be called for an empty square.
     */
    BitBoard candidates(Index const& index) const
    {
      Piece const& piece(M_pieces[index]);
      if (piece == black_pawn)
      {
	BitBoardData data;
	mask_t flags = piece.flags()();
	data.M_bitmask = (flags << 50) | (flags << 40);
	data.M_bitmask &= CW_MASK_T_CONST(0xe0400000000000);
	data.M_bitmask >>= 62 - index();
        return BitBoard(data);
      }
      else if (piece == white_pawn)
      {
	BitBoardData data;
	mask_t flags = piece.flags()();
	data.M_bitmask = flags | (flags << 6);
	data.M_bitmask &= 0x1038;
	data.M_bitmask <<= index() + 4;
        return BitBoard(data);
      }
      return BitBoard(candidates_table[candidates_table_offset(piece.type()) + index()]);
    }

    /** @brief Return a BitBoard with bits set for each square that a piece can reach in one move.
     *
     * The parameter \a attacked_squares only influences pawns and the king.
     * If set, the squares returned for a pawn are always the two attacked
     * squares (independent of other pieces); and the king does not return
     * any castling squares. The definition would be: squares that the
     * opponent may not move to with his king.
     *
     * @param index : The position of the piece under investigation.
     * @param attacked_squares : If true, rather return if squares are attacked than whether the piece can move there.
     */
    BitBoard reachables(Index const& index, bool attacked_squares = false) const;

    /** @brief Return a BitBoard with bits set for each square that a piece defends, or would defend if an exchange was going on there.
     *
     * This function returns the squares that the enemy king is not allowed to go to
     * as a result of this piece. However, the attacks are considered to go through
     * slider pieces of the same type. For example, if you ask about the defendables
     * of a white rook on e1, and there is already a white rook (or queen) on e2,
     * then the square e3 (and beyond) is returned nevertheless.
     *
     * It is not necessary for the piece to actually stand on the board yet.
     *
     * @param code : The piece.
     * @param index : The place that the piece is considered to be standing on.
     * @param battery : Internal output variable (ignore it).
     */
    BitBoard defendables(Code const& code, Index const& index, bool& battery) const;

    /** @brief Return the index of the king with color \a color. */
    Index index_of_king(Color const& color) const { CodeData data = { static_cast<uint8_t>(king_bits | color()) }; return mask2index(M_bitboards[data]()); }

    /** @brief Return true if the king is in check. */
    bool check() const { return M_bitboards[Code(M_to_move, king)].test(M_defended[M_to_move.opposite()].any()); }

    /** @brief Return true if the king of color \a color is in check. */
    bool check(Color const& color) const { return M_bitboards[Code(color, king)].test(M_defended[color.opposite()].any()); }

    /** @brief Return true if the king of color \a color is in double check. */
    bool double_check(Color const& color) const
        { Color opposite_color(color.opposite());
	  CodeData data = { static_cast<uint8_t>(king_bits | color()) };
	  return M_defended[opposite_color].count(M_bitboards[data]) - M_king_battery_attack_count[opposite_color] > 1; }

    /** @brief Return true if the king or rook on \a index has moved or not.
     *
     * Calling this function for another piece than a king or rook returns false.
     */
    bool has_moved(Index const& index) { return M_castle_flags.has_moved(M_pieces[index].code(), index); }

    /** @brief Return a BitBoard with bits set for each square the piece at index can legally go to.
     *
     * @param index : The square that the piece stands on that is to be considered.
     */
    BitBoard moves(Index const& index) const;

    /** @brief Return true if the move is a legal move. */
    bool legal(Move const& move) const;

  //@}

  /** @name Iterators */
  //@{

  /** @brief Return an iterator to the first piece of color \a color.
   *
   * The iterator will iterate over all pieces with color \a color.
   */
  PieceIterator piece_begin(Color const& color) const { return PieceIterator(this, M_bitboards[color]); }

  /** @brief Return an iterator one beyond the last piece. */
  PieceIterator piece_end() const { return PieceIterator(); }

  /** @brief Return an iterator to the first piece with code \a code.
   *
   * The iterator will iterate over all pieces with code \a code; \a code may not be empty.
   */
  PieceIterator piece_begin(Code const& code) const { return PieceIterator(this, M_bitboards[code]); }

  /** @brief Return an iterator to the first move of the piece at index \a index. */
  MoveIterator move_begin(Index const& index) const { return MoveIterator(this, index); }

  /** @brief Return an iterator one beyond the last move. */
  MoveIterator move_end() const { return MoveIterator(); }

  //@}

  /** @name Game play */
  //@{

  /** @brief Execute move \a move. */
  bool execute(Move const& move);

  //@}

  protected:
    void reset_en_passant() { if (M_en_passant.exists()) clear_en_passant(); }

  private:
    // Reset the right to take en passant.
    void clear_en_passant();

    // Increment M_half_move_clock (or reset if \a pawn_advance_or_capture is true) and M_full_move_number if appropriate.
    bool increment_counters(bool pawn_advance_or_capture);

    // Update the fl_pawn_can_take_* pawn flags for a piece of color \a color that was removed from (col, row).
    void update_removed(uint8_t col, uint8_t row, Color const& color);

    // Update the fl_pawn_can_take_* pawn flags for a piece of color \a color that was placed at (col, row).
    void update_placed(uint8_t col, uint8_t row, Color const& color);

    // Update pinning flags.
    void update_pinning(Code const& code, Index const& index, mask_t mask, Direction const& direction, BitBoard const& line);

    // Calculate a bitboard with all pieces minus all bishop movers and first hit pawn, plus blocking bits after those first pawns.
    BitBoard all_pieces_minus_bishop_movers(Color const& color, Index const& index) const;

    // Calculate the squares that would be blocked by a piece with code \a code at \a index and than add or subtract them from M_defended.
    // Also update M_king_battery_attack_count.
    void update_blocked_defendables(Code const& code, Index const& index, bool add);

  private:
    static BitBoardData candidates_table[5 * 64];
};

} // namespace cwchess

#ifndef DOXYGEN		// Hide this from doxygen in order to make some graph less wide.
#include "PieceIterator.inl"
#include "MoveIterator.inl"
#endif

#endif	// CHESSPOSITION_H
