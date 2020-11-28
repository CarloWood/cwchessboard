// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file ChessboardWidget.cxx This file contains the implementation of the gtkmm class ChessboardWidget.
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
#include "ChessboardWidget.h"
#include "CwChessboardCodes.h"
#include "debug.h"
#include <gdkmm/seat.h>
#include <gtkmm.h>
#ifdef CWDEBUG
#include "debug_ostream_operators_gtkmm.h"
#endif

// Fine tuning.
#define CW_CHESSBOARD_EXPOSE_DEBUG 0

namespace cwmm {

#ifdef CWDEBUG
using ::operator<<;
#endif

//
// The widget is devided into 108 segments (rectangles).
//
// Let the largest rectangle below be the 'Allocation' of the ChessboardWidget.
//
// The origin is in the top-left corner.
// A chessboard square is sside x sside pixels.
// The chessboard square encodings are given by convert_colrow2index:
//  (col, row) = (0, 0) is the bottom-left most corner square (A1 when white is at the bottom).
//
//  O = (0, 0)                                __ background
//   \                                       /
//    O-------------------------------------/-------.
//    |  |                                 /   |    |
//    |  |               __ border        x    |    |
//    |  |              /                      |    |
//    |--E-------------/-----------------------+----|   E = (m_edge_x, m_edge_y) = { top_left_edge_x(), top_left_edge_x() }
//    |  |  |   |   | x |   |   |   |   |   |  |    |
//    |  |--B-------------------------------.--|    |   B = E + (m_border_width, m_border_width) = { top_left_board_x(), top_left_board_y() }
//    |  |8 |0,7|           |            7,7|  |    |
//    |  |--|---D           |  x            |--|    |   D = B + (m_sside, m_sside)
//    |  |7 |               |   \           |  |    |
//    |  |--|  64 squares   |    \__ chess  |--|    |
//    |  |6 |               |        board  |  |    |
//    |  |--|               |               |--|    |
//    |  |5 |               |               |  |    |
//    |  |--|---------------+---------------|--|    |
//    |  |4 |               |               |  |    |
//    |  |--|               |               |--|    |
//    |  |3 |               |               |  |    |
//    |  |--|               |               |--|    |
//    |  |2 |               |               |  |    |
//    |  |--|               |               |--|    |
//    |  |1 |0,0            |            7,0|  |    |
//    |  |--'-------------------------------C--|    |    C = B + (squares * m_sside, squares * m_sside) = { bottom_right_board_x(), bottom_right_board_y() }
//    |  |  | A | B | C | D | E | F | G | H |  |    |
//    |--'-------------------------------------F----|    F = E + (squares * m_sside + 2 * m_border_width, squares * m_sside + 2 * m_border_width) =
//    |  |                                     |    |        { bottom_right_edge_x(), bottom_right_edge_y() }
//    |  |                                     |    |
//    |  |                                     |    |
//    '---------------------------------------------'
//    |  |  |   |   |   |   |   |   |   |   |  |    |
//                                                  \__ (allocation.width, allocation.height)
//
// Represented by a bitmask, (col, row) = (0, 0) corresponds to the least significant bit (bit 0),
// and so on up till (col, row) = (7, 7) corresponding to bit 63.
//
// To encode the variabe coordinates into a bit mask,
// it seem best to use a grid of 5x5 for the following
// (pseudo) segments:
//
//    O---------------------------------------------.
//py= |  |  |                               |  |    |
//  0 |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |--E-------------------------------------+----|
//  1 |  |  |                               |  |    |
//    |--|--B-------------------------------.--|----|
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//  2 |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    |--|--'-------------------------------C--|----|
//  3 |  |  |                               |  |    |
//    |--'-------------------------------------F----|
//    |  |  |                               |  |    |
//  4 |  |  |                               |  |    |
//    |  |  |                               |  |    |
//    '---------------------------------------------A
// px= 0  1                  2               3   4
//
// Because x and y are symmetrical (until we deal with
// the chessboard itself, because that is unfortunately
// encoded with the y-axis being mirrored) it seems a
// good idea to encode the horizontal and vertical
// directories as an enum.

enum Extent {
  ext_top_left,
  ext_bottom_right
};

// Two of those can be converted into a Pseudo Segment Coordinates
// object,

struct PseudoSegmentCoordinates
{
  Extent m_extent;
  std::array<uint16_t, 2> m_pseudo_coordinate;  // 0, 1, 2, 3 or 4, x and y coordinate.
  std::array<uint8_t, 2> m_border;              // A value in the range [0, 8], 1...8 representing the horizontal
                                                // columns 'A' through 'H' or the vertical rows '8' through '1',
                                                // and 0 representing "entirely to the left of / below the board".
                                                // 8 means "on or to the right / above the board".

  PseudoSegmentCoordinates(double x, double y, Extent extent, ChessboardWidget const& widget);
};

PseudoSegmentCoordinates::PseudoSegmentCoordinates(double x, double y, Extent extent, ChessboardWidget const& widget) : m_extent(extent)
{
  std::array<gint, 2> coordinate = {{ static_cast<gint>(x), static_cast<gint>(y) }};
  for (int xy = 0; xy < 2; ++xy)
  {
    gint bottom_right_board = xy ? widget.bottom_right_board_y() : widget.bottom_right_board_x();
    if (coordinate[xy] < bottom_right_board)
    {
      gint top_left_board = xy ? widget.top_left_board_y() : widget.top_left_board_x();
      if (coordinate[xy] >= top_left_board)
      {
        m_pseudo_coordinate[xy] = 2;
        m_border[xy] = 1 + (coordinate[xy] - top_left_board) / widget.sside();
      }
      else
      {
        gint top_left_edge = xy ? widget.top_left_edge_y() : widget.top_left_edge_x();
        m_pseudo_coordinate[xy] = (coordinate[xy] < top_left_edge) ? 0 : 1;
        m_border[xy] = 0;
      }
    }
    else
    {
      gint bottom_right_edge = xy ? widget.bottom_right_edge_y() : widget.bottom_right_edge_x();
      m_pseudo_coordinate[xy] = (coordinate[xy] < bottom_right_edge) ? 3 : 4;
      m_border[xy] = 9;
    }
  }
}

// A PseudoSegmentCoordinates object can be converted to a PseudoSegmentsMask.
PseudoSegmentsMask psc2psm(PseudoSegmentCoordinates psc)
{
  int const index = psc.m_pseudo_coordinate[0] + psc.m_pseudo_coordinate[1] * 5;    // 0..24
  uint32_t pseudo_segments;
  static uint32_t const top_left[] = {
    0x0000001, 0x0000003, 0x0000007, 0x000000f, 0x000001f,
    0x0000021, 0x0000063, 0x00000e7, 0x00001ef, 0x00003ff,
    0x0000421, 0x0000c63, 0x0001ce7, 0x0003def, 0x0007fff,
    0x0008421, 0x0018c63, 0x0039ce7, 0x007bdef, 0x00fffff,
    0x0108421, 0x0318c63, 0x0739ce7, 0x0f7bdef, 0x1ffffff
  };
  static uint32_t const bottom_right[] = {
    0x1ffffff, 0x1ef7bde, 0x1ce739c, 0x18c6318, 0x1084210,
    0x1ffffe0, 0x1ef7bc0, 0x1ce7380, 0x18c6300, 0x1084200,
    0x1fffc00, 0x1ef7800, 0x1ce7000, 0x18c6000, 0x1084000,
    0x1ff8000, 0x1ef0000, 0x1ce0000, 0x18c0000, 0x1080000,
    0x1f00000, 0x1e00000, 0x1c00000, 0x1800000, 0x1000000
  };
  if (psc.m_extent == ext_top_left)
    pseudo_segments = top_left[index];
  else
    pseudo_segments = bottom_right[index];
  std::array<uint8_t, 2> border_mask;
  for (int xy = 0; xy < 2; ++xy)
  {
    // With white at the bottom, the least significant bit of m_squares_mask corresponds with A1, while
    // the first eight (least significant) bits correspond with A1 through H1.
    // This must map 1-on-1 into m_border_mask[0].
    // The (x,y) coordinates have their origin in the top-left and to make this code simplest, we
    // want symmetry in the y = x axis. Therefore m_border_mask[1] must have its least significant
    // bit at the top row (row 8 of the chess board, with white on the bottom):
    //
    //        x
    //  o------------>
    //  |
    //  |  E-------------------------------------+
    //  |  |  |   |   |   |   |   |   |   |   |  |
    // y|  |--B-------------------------------.--|  .---.
    //  |  |8 |0,7                         7,7|  |  |LSB| <-- m_border_mask[1]
    //  |  |--|                               |--|  |---|
    //  v  |7 |                               |  |  |   |
    //     |--|                               |--|  |---|
    //     |6 |                               |  |  |   |
    //     |--|                               |--|  |---|
    //     |5 |                               |  |  |   |
    //     |--|          64 squares           |--|  |---|
    //     |4 |                               |  |  |   |
    //     |--|                               |--|  |---|
    //     |3 |                               |  |  |   |
    //     |--|                               |--|  |---|
    //     |2 |                               |  |  |   |
    //     |--|                               |--|  |---|
    //     |1 |0,0                         7,0|  |  |MSB|
    //     |--'-------------------------------C--|  '---'
    //     |  | A | B | C | D | E | F | G | H |  |
    //     '-------------------------------------F
    //        .-------------------------------.
    //        |LSB|   |   |   |   |   |   |MSB| <-- m_border_mask[0]
    //        '-------------------------------'
    //   .--0   1   2   3   4   5   6   7   8   9
    //   |
    //   |    extension is top_left (left here; rotate 90 degrees clockwise for vertical orientation).
    //   v
    //   0:     0   0   0   0   0   0   0   0
    //   1:     1   0   0   0   0   0   0   0
    //   2:     1   1   0   0   0   0   0   0
    //   .
    //   7:     1   1   1   1   1   1   1   0
    //   8:     1   1   1   1   1   1   1   1
    //   9:     1   1   1   1   1   1   1   1
    //
    //        extension is bottom_right (right here; rotate 90 degrees clockwise for vertical orientation).
    //
    //   0:     1   1   1   1   1   1   1   1
    //   1:     1   1   1   1   1   1   1   1
    //   2:     0   1   1   1   1   1   1   1
    //   .
    //   7:     0   0   0   0   0   0   1   1
    //   8:     0   0   0   0   0   0   0   1
    //   9:     0   0   0   0   0   0   0   0

    int pc = psc.m_border[xy];  // In the range [0, 9].
    border_mask[xy] = (psc.m_extent == ext_top_left) ? ~(0xff << pc) : ~(0xff >> (9 - pc));
  }
  return { pseudo_segments, border_mask };
}

WidgetSegments::WidgetSegments(PseudoSegmentsMask psm) :
  m_squares_mask(0), m_pseudo_segments_mask(psm.m_pseudo_segments),
  m_border{{ (psm.m_pseudo_segments & 0x80) ? psm.m_border_mask[0] : uint8_t{0},
             (psm.m_pseudo_segments & 0x2000) ? psm.m_border_mask[1] : uint8_t{0},
             (psm.m_pseudo_segments & 0x20000) ? psm.m_border_mask[0] : uint8_t{0},
             (psm.m_pseudo_segments & 0x800) ? psm.m_border_mask[1] : uint8_t{0} }}
{
  uint64_t row = psm.m_border_mask[0];
  for (uint8_t row_mask = 0x80; row_mask; row_mask >>= 1, row <<= 8)
    if ((psm.m_border_mask[1] & row_mask))
      m_squares_mask |= row;
}

WidgetSegments ChessboardWidget::rect_to_segments(Cairo::Rectangle const& rect) const
{
  PseudoSegmentsMask bottom_right_segment = psc2psm({rect.x, rect.y, ext_bottom_right, *this});
  PseudoSegmentsMask top_left_segment = psc2psm({rect.x + rect.width, rect.y + rect.height, ext_top_left, *this});

  PseudoSegmentsMask segment = bottom_right_segment & top_left_segment;

  // Now convert this to a WidgetSegments, which then can be easily combined to store multiple rectangles.
  return segment;
}

#ifdef CWDEBUG
std::ostream& operator<<(std::ostream& os, WidgetSegments const& ws)
{
  os << std::hex << ws.m_squares_mask << ", " << ws.m_pseudo_segments_mask;
  for (int b = 0; b < 4; ++b)
    os << ", " << (int)ws.m_border[b];
  return os << std::dec;
}
#endif

//------------------------------------------------------------------------------
// Piece graphics functions.

static double const black_line_width = 0.026;
static double const white_line_width = 1.5 * black_line_width;

static double snap_bottom(double y, double translate, double scale, double line_width)
{
  if (scale < 27)
    return y;
  return (round((y + 0.5 * line_width) * scale - translate) + translate) / scale - 0.5 * line_width;
}

static double snap_top(double y, double translate, double scale, double line_width)
{
  if (scale < 27)
    return y;
  return (round((y - 0.5 * line_width) * scale - translate) + translate) / scale + 0.5 * line_width;
}

static double snap_line_width(double line_width, double scale)
{
  if (line_width * scale < 1.0)
    return line_width;
  return trunc(line_width * scale + 0.3) / scale;
}

// The fill color of the pieces.
void ChessboardWidget::set_fill_color(cairo_t* cr, gboolean white)
{
  if (white)
    cairo_set_source_rgb(cr, m_white_piece_fill_color.red,
                             m_white_piece_fill_color.green,
			     m_white_piece_fill_color.blue);
  else
    cairo_set_source_rgb(cr, m_black_piece_fill_color.red,
                             m_black_piece_fill_color.green,
			     m_black_piece_fill_color.blue);
}

// The line color of the pieces.
void ChessboardWidget::set_line_color(cairo_t* cr, gboolean white)
{
  if (white)
    cairo_set_source_rgb(cr, m_white_piece_line_color.red,
                             m_white_piece_line_color.green,
			     m_white_piece_line_color.blue);
  else
    cairo_set_source_rgb(cr, m_black_piece_line_color.red,
                             m_black_piece_line_color.green,
			     m_black_piece_line_color.blue);
}

void ChessboardWidget::recreate_hud_layers(Cairo::RefPtr<Cairo::Context> const& cr)
{
  DoutEntering(dc::widget, "ChessboardWidget::recreate_hud_layers()");

  for (guint hud = 0; hud < number_of_hud_layers; ++hud)
  {
    // (Re)create the HUD layer.
    m_hud_layer_surface[hud] = Cairo::Surface::create(cr->get_target(), Cairo::CONTENT_COLOR_ALPHA, squares * m_sside, squares * m_sside);

    m_hud_has_content[hud] = 0;
    m_hud_need_redraw[hud] = (guint64)-1;
  }
  // Because we need to expose all squares.
  invalidate_board();
}

// This function is called after realize, or after a resize.
// Allocate and redraw the piece caches.
void ChessboardWidget::redraw_pixmap(Cairo::RefPtr<Cairo::Context> const& cr)
{
  DoutEntering(dc::widget, "ChessboardWidget::redraw_pixmap(cr)");

  int const old_total_size = 2 * m_border_width + squares * m_sside;    // m_total_size was already changed in on_size_allocate.
  int const old_sside = m_sside;

  // Calculate the size of the chess board widget.
  Gtk::Allocation allocation = get_allocation();
  Dout(dc::notice, "allocation = " << allocation);

#if CW_CHESSBOARD_EXPOSE_DEBUG
  m_buffer = Cairo::Surface::create(cr->get_target(), Cairo::CONTENT_COLOR_ALPHA, allocation.get_width(), allocation.get_height());
#endif

  m_border_width = 0;
  // Since `m_total_size = squares * sside + 2 * ChessboardWidget::default_calc_board_border_width(sside)`, we can determine sside from m_total_size.
  // Without a border the sside is
  m_sside = m_total_size / squares;
  if (m_draw_border)
  {
    // Otherwise the above is an upper limit. Find the largest value that still fits.
    for (;;)
    {
      m_border_width = ChessboardWidget::default_calc_board_border_width(m_sside);
      if (squares * m_sside + 2 * m_border_width <= m_total_size)
        break;
      --m_sside;
    }
  }

  m_edge_x = (allocation.get_width() - m_total_size) / 2;
  m_edge_y = (allocation.get_height() - m_total_size) / 2;

  Dout(dc::widget, "Size including border is " << m_total_size);
  Dout(dc::widget, "Border width is " << m_border_width << "; " << squares << 'x' << squares << " squares with side " << m_sside);

  // Invalidate everything.
  m_need_redraw_invalidated = (guint64)-1;

  // Cache the rectangular region where the chessboard resides as a Cairo::RefPtr<Cairo::Region>.
  Cairo::RectangleInt rect;
  rect.x = m_edge_x + m_border_width;
  rect.y = m_edge_y + m_border_width;
  rect.width = squares * m_sside;
  rect.height = squares * m_sside;
  m_chessboard_region = Cairo::Region::create(rect);

  m_top_or_left_background_rect.x = 0;
  m_top_or_left_background_rect.y = 0;
  if (m_edge_x == 0)
  {
    m_top_or_left_background_rect.width = m_total_size;
    m_top_or_left_background_rect.height = top_left_edge_y();
    Dout(dc::notice, "m_top_or_left_background_rect.height was set to top_left_edge_y() = " << m_top_or_left_background_rect.height <<
        "; m_border_width = " << m_border_width << "; m_edge_y = " << m_edge_y);
    m_bottom_or_right_background_rect.width = m_total_size;
    m_bottom_or_right_background_rect.height = allocation.get_height() - bottom_right_edge_y();
  }
  else
  {
    m_top_or_left_background_rect.width = top_left_edge_x();
    m_top_or_left_background_rect.height = m_total_size;
    Dout(dc::notice, "m_top_or_left_background_rect.height was set to m_total_size = " << m_top_or_left_background_rect.height);
    m_bottom_or_right_background_rect.width = allocation.get_width() - bottom_right_edge_x();
    m_bottom_or_right_background_rect.height = m_total_size;
  }
  m_bottom_or_right_background_rect.x = allocation.get_width() - m_bottom_or_right_background_rect.width;
  m_bottom_or_right_background_rect.y = allocation.get_height() - m_bottom_or_right_background_rect.height;

  // Also update m_marker_thickness_px and m_cursor_thickness_px.
  set_marker_thickness(m_marker_thickness);
  set_cursor_thickness(m_cursor_thickness);

  if (old_total_size == m_total_size)
    return;

  // If the resize even changed the side of the squares then we have to redraw
  // the cached pieces and the HUD layer cache. Initially m_sside is set to
  // -1 and the expression will always evalute to true.
  if (m_sside != old_sside)
  {
    // Destroy hatching cache for redraw later.
    m_hatching_surface.surface.clear();

    // Create new cairo surfaces for the piece cache and (re)draw the pieces.
    redraw_pieces(cr->get_target());

    // Calculate the marker thickness.
    m_marker_thickness_px = std::max(1, std::min((gint)round(m_marker_thickness * m_sside), m_sside / 2));
    Dout(dc::widget, "Marker thickness set to " << m_marker_thickness_px);

    // Calculate the cursor thickness.
    m_cursor_thickness_px = std::max(1, std::min((gint)round(m_cursor_thickness * m_sside), m_sside / 2));
    Dout(dc::widget, "Cursor thickness set to " << m_cursor_thickness_px);

    // (Re)create alpha layer.
    Dout(dc::widget|continued_cf, "(Re)creating HUD layers... ");
    recreate_hud_layers(cr);
    Dout(dc::finish, "done");
  }

  invalidate_border();
}

// This function should only be called from on_draw.
// Call invalidate_square to redraw a square from elsewhere.
void ChessboardWidget::redraw_square(Cairo::RefPtr<Cairo::Context> const& cr, gint index)
{
  CwChessboardCode code = m_board_codes[index];
  CwChessboardColorHandle bghandle = convert_code2bghandle(code);
  CwChessboardColorHandle mahandle = convert_code2mahandle(code);
  gint col = convert_index2column(index);
  gint row = convert_index2row(index);
  gint sx = top_left_board_x() + (m_flip_board ? 7 - col : col) * m_sside;
  gint sy = top_left_board_y() + (squares - 1 - (m_flip_board ? 7 - row : row)) * m_sside;

//  DoutEntering(dc::widget, "ChessboardWidget::redraw_square(cr, " << index << ")" << " with Board Code: " << (int)code);

  // Draw background color.
  cr->rectangle(sx, sy, m_sside, m_sside);
  if (bghandle != 0)
    cr->set_source_rgb(
        m_color_palet[bghandle - 1].red,
        m_color_palet[bghandle - 1].green,
        m_color_palet[bghandle - 1].blue);
  else if (((col + row) & 1))
    cr->set_source_rgb(
        m_light_square_color.red,
        m_light_square_color.green,
        m_light_square_color.blue);
  else
    cr->set_source_rgb(
        m_dark_square_color.red,
        m_dark_square_color.green,
        m_dark_square_color.blue);
  if (!mahandle || !m_marker_below)
    cr->fill();
  else
  {
    cr->fill_preserve();
    // Draw marker.
    cr->rectangle(sx + m_marker_thickness_px, sy + m_marker_thickness_px,
        m_sside - 2 * m_marker_thickness_px, m_sside - 2 * m_marker_thickness_px);
    Cairo::FillRule prev_fill_rule = cr->get_fill_rule();
    cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
    cr->set_source_rgb(
        m_color_palet[mahandle - 1].red,
        m_color_palet[mahandle - 1].green,
        m_color_palet[mahandle - 1].blue);
    cr->fill();
    cr->set_fill_rule(prev_fill_rule);
  }

  guint64 bit = 1;
  bit <<= index;

  // Draw bottom HUD layer, if any.
  if ((m_hud_has_content[0] & bit))
  {
    cr->set_source(m_hud_layer_surface[0], top_left_board_x(), top_left_board_y());
    cr->rectangle(sx, sy, m_sside, m_sside);
    cr->fill();
  }

  // Draw marker, if any and still needed.
  if (mahandle && !m_marker_below)
  {
    cr->rectangle(sx, sy, m_sside, m_sside);
    cr->rectangle(sx + m_marker_thickness_px, sy + m_marker_thickness_px,
        m_sside - 2 * m_marker_thickness_px, m_sside - 2 * m_marker_thickness_px);
    Cairo::FillRule prev_fill_rule = cr->get_fill_rule();
    cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
    cr->set_source_rgb(
        m_color_palet[mahandle - 1].red,
        m_color_palet[mahandle - 1].green,
        m_color_palet[mahandle - 1].blue);
    cr->fill();
    cr->set_fill_rule(prev_fill_rule);
  }

  if (m_show_cursor && m_cursor_col == col && m_cursor_row == row)
  {
    cr->rectangle(sx, sy, m_sside, m_sside);
    cr->rectangle(sx + m_cursor_thickness_px, sy + m_cursor_thickness_px,
        m_sside - 2 * m_cursor_thickness_px, m_sside - 2 * m_cursor_thickness_px);
    Cairo::FillRule prev_fill_rule = cr->get_fill_rule();
    cr->set_fill_rule(Cairo::FILL_RULE_EVEN_ODD);
    cr->set_source_rgb(
        m_cursor_color.red,
        m_cursor_color.green,
        m_cursor_color.blue);
    cr->fill();
    cr->set_fill_rule(prev_fill_rule);
  }

  // Draw piece, if any.
  if (!is_empty_square(code))
  {
    cr->set_source(m_piece_surface[convert_code2piece_index(code)].surface, sx, sy);
    cr->paint();
  }

  // Draw top HUD layer, if any.
  if ((m_hud_has_content[1] & bit))
  {
    cr->set_source(m_hud_layer_surface[1], top_left_board_x(), top_left_board_y());
    cr->rectangle(sx, sy, m_sside, m_sside);
    cr->fill();
  }
}

void ChessboardWidget::redraw_pieces(Cairo::RefPtr<Cairo::Surface> const& surface)
{
  DoutEntering(dc::widget, "ChessboardWidget::redraw_pieces(cr)");

  for (int i = 0; i < 12; ++i)
  {
    m_piece_surface[i].surface = Cairo::Surface::create(surface, Cairo::CONTENT_COLOR_ALPHA, m_sside, m_sside);
    Dout(dc::widget|continued_cf, "(Re)drawing piece cache " << i << "... ");
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_piece_surface[i].surface);
    unsigned char code = static_cast<unsigned char>(i + 2);
    cr->rectangle(0, 0, m_sside, m_sside);
    cr->clip();
    switch (convert_code2piece(code))
    {
      case pawn:
        draw_pawn(cr->cobj(), 0.5 * m_sside, 0.5 * m_sside, m_sside, is_white_piece(code));
        break;
      case rook:
        draw_rook(cr->cobj(), 0.5 * m_sside, 0.5 * m_sside, m_sside, is_white_piece(code));
        break;
      case knight:
        draw_knight(cr->cobj(), 0.5 * m_sside, 0.5 * m_sside, m_sside, is_white_piece(code));
        break;
      case bishop:
        draw_bishop(cr->cobj(), 0.5 * m_sside, 0.5 * m_sside, m_sside, is_white_piece(code));
        break;
      case queen:
        draw_queen(cr->cobj(), 0.5 * m_sside, 0.5 * m_sside, m_sside, is_white_piece(code));
        break;
      case king:
        draw_king(cr->cobj(), 0.5 * m_sside, 0.5 * m_sside, m_sside, is_white_piece(code));
        break;
    }
    Dout(dc::finish, "done");
  }
  invalidate_board();
}

void ChessboardWidget::invalidate_border()
{
  if (get_realized())
  {
    // Don't call this function when we're not drawing a border.
    ASSERT(m_border_width > 0);
    cairo_rectangle_int_t rect;
    rect.x = top_left_edge_x();
    rect.y = top_left_edge_y();
    rect.width = bottom_right_edge_x() - top_left_edge_x();
    rect.height = bottom_right_edge_y() - top_left_edge_y();
    Cairo::RefPtr<Cairo::Region> border_region = Cairo::Region::create(rect);
    border_region->subtract(m_chessboard_region);
    Dout(dc::notice, "ChessboardWidget::invalidate_border(): calling queue_draw_region(" << border_region << ")");
    queue_draw_region(border_region);
  }
}

void ChessboardWidget::invalidate_turn_indicators()
{
  if (get_realized())
  {
    gint const border_width = m_border_width;
    gint const border_shadow_width = 2;
    gint const edge_width = border_width - border_shadow_width - 1;
    gint const side = squares * m_sside;

    double const factor = 0.085786;       // (1/sqrt(2) − 0.5)/(1 + sqrt(2)).
    int const dx = (int)std::ceil((edge_width + 1) * factor);

    Cairo::RectangleInt top_indicator_rect, bottom_indicator_rect;
    top_indicator_rect.x = bottom_right_board_x() + 1 - dx;
    top_indicator_rect.y = top_left_board_y() - 1 - edge_width;
    top_indicator_rect.width = edge_width + dx;
    top_indicator_rect.height = edge_width;
    Cairo::RefPtr<Cairo::Region> indicator_region = Cairo::Region::create(top_indicator_rect);
    bottom_indicator_rect.x = top_indicator_rect.x;
    bottom_indicator_rect.y = bottom_right_board_y() + 1;
    bottom_indicator_rect.width = edge_width + dx;
    bottom_indicator_rect.height = edge_width;
    indicator_region->do_union(bottom_indicator_rect);
    top_indicator_rect.x += dx;
    top_indicator_rect.y += edge_width;
    top_indicator_rect.width = edge_width;
    top_indicator_rect.height = dx;
    indicator_region->do_union(top_indicator_rect);
    bottom_indicator_rect.x += dx;
    bottom_indicator_rect.y -= dx;
    bottom_indicator_rect.width = edge_width;
    bottom_indicator_rect.height = dx;
    indicator_region->do_union(bottom_indicator_rect);
    Dout(dc::notice, "ChessboardWidget::invalidate_turn_indicators(): calling queue_draw_region(" << indicator_region << ")");
    queue_draw_region(indicator_region);
  }
}

void ChessboardWidget::invalidate_square(gint col, gint row)
{
  guint64 redraw_mask = 1;
  redraw_mask <<= convert_colrow2index(col, row);
  // No need to call queue_draw_area again when need_redraw_invalidated is already set.
  if (!(m_need_redraw_invalidated & redraw_mask))
  {
    if (get_realized())
    {
      int x, y;
      colrow2xy(col, row, x, y);
      Dout(dc::notice, "ChessboardWidget::invalidate_square(" << col << ", " << row << "): calling queue_draw_area(" << x << ", " << y << ", " << m_sside << ", " << m_sside << ")");
      queue_draw_area(x, y, m_sside, m_sside);
      m_need_redraw_invalidated |= redraw_mask;
    }
  }
}

void ChessboardWidget::invalidate_board()
{
  if (get_realized())
  {
    Dout(dc::notice, "ChessboardWidget::invalidate_board(): calling queue_draw_region(" << m_chessboard_region << ")");
    queue_draw_region(m_chessboard_region);
    m_need_redraw_invalidated = (guint64)-1;
  }
}

void ChessboardWidget::invalidate_markers()
{
  for (gint row = 0; row < squares; ++row)
    for (gint col = 0; col < squares; ++col)
      if (convert_code2mahandle(m_board_codes[convert_colrow2index(col, row)]) != 0)
        invalidate_square(col, row);
}

void ChessboardWidget::invalidate_cursor()
{
  if (m_show_cursor && is_inside_board(m_cursor_col, m_cursor_row))
    invalidate_square(m_cursor_col, m_cursor_row);
}

guint64 ChessboardWidget::invalidate_arrow(gint col1, gint row1, gint col2, gint row2)
{
  DoutEntering(dc::widget|continued_cf, "ChessboardWidget::invalidate_arrow(" << col1 << ", " << row1 << ", " << col2 << ", " << row2 << ") = ");

  guint64 result = 0;
  if (col1 == col2)                     // Vertical arrow?
  {
    if (row1 > row2)
    {
      gint tmp = row1;
      row1 = row2;
      row2 = tmp;
    }
    guint64 bit = (guint64)1 << convert_colrow2index(col1, row1);
    for (gint row = row1; row <= row2; ++row, bit <<= 8)
    {
      result |= bit;
      invalidate_square(col1, row);
    }
    Dout(dc::finish, std::hex << result);
    return result;
  }
  else if (row1 == row2)                // Horizontal arrow?
  {
    if (col1 > col2)
    {
      gint tmp = col1;
      col1 = col2;
      col2 = tmp;
    }
    guint64 bit = (guint64)1 << convert_colrow2index(col1, row1);
    for (gint col = col1; col <= col2; ++col, bit <<= 1)
    {
      result |= bit;
      invalidate_square(col, row1);
    }
    Dout(dc::finish, std::hex << result);
    return result;
  }
  if (row1 > row2)
  {
    // Swap point 1 and 2.
    gint tmp;
    tmp = col1; col1 = col2; col2 = tmp;
    tmp = row1; row1 = row2; row2 = tmp;
  }
  double const arrow_width = 0.125;     // FIXME: must be half the real arrow thickness.
  double delta = arrow_width *
      sqrt((row2 - row1) * (row2 - row1) + (col2 - col1) * (col2 - col1)) / (row2 - row1);
  gint col_start = col1;
  gint row = row1;
  result = 0;
  guint64 sign = 1;
  if (col1 > col2)
  {
    sign = 2;
    delta = -delta;
  }
  for (double r = row1 + 0.5; r < row2; r += 1.0)
  {
    double c = col1 + (r - row1) * (col2 - col1) / (row2 - row1);
    gint col_end = (gint)round(c + delta);
    guint64 mask1 = (sign << convert_colrow2index(col_start, row)) - 1;
    guint64 mask2 = ((3 - sign) << convert_colrow2index(col_end, row)) - 1;
    result |= mask1 ^ mask2;
    col_start = (gint)round(c - delta);
    ++row;
  }
  guint64 mask1 = (sign << convert_colrow2index(col_start, row)) - 1;
  guint64 mask2 = ((3 - sign) << convert_colrow2index(col2, row)) - 1;
  result |= mask1 ^ mask2;
  Dout(dc::finish, std::hex << result);
  guint64 bit = 1;
  for (gint row = 0; row < squares; ++row)
    for (gint col = 0; col < squares; ++col, bit <<= 1)
      if ((result & bit))
        invalidate_square(col, row);
  return result;
}

void ChessboardWidget::update_cursor_position(gdouble x, gdouble y, gboolean forced)
{
  gint col = x2col(x);
  gint row = y2row(y);
  gboolean cursor_moved = (col != m_cursor_col || row != m_cursor_row);
  gboolean was_inside_board = is_inside_board(m_cursor_col, m_cursor_row);
  gboolean inside_board = is_inside_board(col, row);
  if (cursor_moved || forced)
  {
    if (inside_board)
    {
      if (was_inside_board)
        on_cursor_entered_square(m_cursor_col, m_cursor_row, col, row);
      else
        on_cursor_entered_square(-1, -1, col, row);
      if (m_show_cursor)
        invalidate_square(col, row);
    }
    else if (was_inside_board)
      on_cursor_left_chessboard(m_cursor_col, m_cursor_row);
    if (m_show_cursor && was_inside_board)
      invalidate_square(m_cursor_col, m_cursor_row);
    m_cursor_col = col;
    m_cursor_row = row;
  }
  if (m_show_cursor && !m_need_redraw_invalidated)
  {
    // Make sure we'll get more motion events.
    Dout(dc::motion_event, "FIXME: NOT Calling gdk_window_get_device_position()");
#if 0 // Is this still needed?
    GdkDisplay* display = gdk_display_get_default();
    GdkSeat* seat = gdk_display_get_default_seat(display);
    GdkDevice* pointer = gdk_seat_get_pointer(seat);
    gdk_window_get_device_position(gtk_widget_get_window(GTK_WIDGET(chessboard)), pointer, NULL, NULL, NULL);
#endif
  }
}

// Draw pawn.
void ChessboardWidget::draw_pawn(cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  static double const base_outside_diameter_cm = 3.265;
  static double const width_pawn_cm = 5.31;
  static double const base_radius = 0.5 * (base_outside_diameter_cm / width_pawn_cm - black_line_width);
  static double const mid_outside_diameter_cm = 1.98;
  static double const mid_radius = 0.5 * (mid_outside_diameter_cm / width_pawn_cm - black_line_width);
  static double const head_outside_diameter_cm = 1.12;
  static double const head_radius = 0.5 * (head_outside_diameter_cm / width_pawn_cm - black_line_width);
  static double const height_pawn_cm = 5.43;
  static double const bottom_pawn_cm = 0.58;
  static double const foot_height = 0.0387;
  static double const base_y = 0.5 - bottom_pawn_cm / height_pawn_cm - 0.5 * black_line_width;
  static double const base_scale = 0.931;
  static double const mid_y = -0.0545;
  static double const top_offset_cm = 0.62;
  static double const head_y = -0.5 + top_offset_cm / height_pawn_cm + 0.5 * black_line_width + head_radius;

  static double const base_angle = 1.148;
  static double const mid_angle1 = 0.992;
  static double const inner_neck_width_cm = 0.41;
  static double const neck_right = 0.5 * (inner_neck_width_cm / width_pawn_cm + black_line_width);
  static double const head_angle = asin(neck_right / head_radius);
  static double const mid_scale = (mid_y - (head_y + head_radius * cos(head_angle)) -
      0.1 * black_line_width) / sqrt(mid_radius * mid_radius - neck_right * neck_right);
  static double const mid_angle2 = asin(head_radius * sin(head_angle) / mid_radius);

  double const base_y_sn = snap_bottom(base_y, y, scale, black_line_width);

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw the left-side of the base.
  cairo_move_to(cr, -base_radius, base_y_sn);
  cairo_save(cr);
  cairo_translate(cr, 0.0, base_y_sn - foot_height);
  cairo_scale(cr, 1.0, base_scale);
  cairo_arc(cr, 0.0, 0.0, base_radius, -M_PI, -M_PI + base_angle);
  cairo_restore(cr);

  // Draw the left-side of the mid-section.
  cairo_save(cr);
  cairo_translate(cr, 0.0, mid_y);
  cairo_scale(cr, 1.0, mid_scale);
  cairo_arc(cr, 0.0, 0.0, mid_radius, -M_PI - mid_angle1, -0.5 * M_PI - mid_angle2);
  cairo_restore(cr);

  // Draw the head of the pawn.
  cairo_arc(cr, 0.0, head_y, head_radius, -1.5 * M_PI + head_angle, 0.5 * M_PI - head_angle);

  // Draw the right-side of the mid-section.
  cairo_save(cr);
  cairo_translate(cr, 0.0, mid_y);
  cairo_scale(cr, 1.0, mid_scale);
  cairo_arc(cr, 0.0, 0.0, mid_radius,  -0.5 * M_PI + mid_angle2, mid_angle1);
  cairo_restore(cr);

  // Draw the right-side of the base.
  cairo_save(cr);
  cairo_translate(cr, 0.0, base_y_sn - foot_height);
  cairo_scale(cr, 1.0, base_scale);
  cairo_arc(cr, 0.0, 0.0, base_radius, -base_angle, 0.0);
  cairo_restore(cr);
  cairo_line_to(cr, base_radius, base_y_sn);

  // Draw the base line of the pawn, right to left.
  cairo_close_path(cr);

  set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void ChessboardWidget::draw_king(cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const blob_left_cm = 1.22;
  static double const band_edge_left_cm = 2.55;
  static double const band_left_cm = 2.67;
  static double const inside_left_cm = 3.06;
  static double const center_blob_left_cm = 4.525;
  static double const cross_left_cm = 4.71;
  static double const width_king_cm = 10.67;
  static double const bottom_king_cm = 1.155;
  static double const band_line_top_cm = 2.95;
  static double const band_top_king_cm = 4.04;
  static double const center_y_cm = 5.02;
  static double const blob_top_cm = 7.4; // 7.28
  static double const center_blob_top_cm = 8.18;
  static double const cross_y_king_cm = 9.17;
  static double const cross_top_cm = 9.86;
  static double const height_king_cm = 10.86;
  // Derived values.
  static double const mid_x_king_cm = width_king_cm / 2;
  static double const mid_y_king_cm = height_king_cm / 2;

  // Same, in coordinates.
  static double const blob_left = (blob_left_cm - mid_x_king_cm) / width_king_cm;
  static double const band_edge_left = (band_edge_left_cm - mid_x_king_cm) / width_king_cm;
  static double const band_left = (band_left_cm - mid_x_king_cm) / width_king_cm;
  static double const inside_left = (inside_left_cm - mid_x_king_cm) / width_king_cm;
  static double const center_blob_left = (center_blob_left_cm - mid_x_king_cm) / width_king_cm;
  static double const cross_left = (cross_left_cm - mid_x_king_cm) / width_king_cm;
  static double const bottom_king = (mid_y_king_cm - bottom_king_cm) / height_king_cm;
  static double const band_line_top = (mid_y_king_cm - band_line_top_cm) / height_king_cm;
  static double const band_top_king = (mid_y_king_cm - band_top_king_cm) / height_king_cm;
  static double const center_y = (mid_y_king_cm - center_y_cm) / height_king_cm;
  static double const blob_top = (mid_y_king_cm - blob_top_cm) / height_king_cm;
  static double const center_blob_top = (mid_y_king_cm - center_blob_top_cm) / height_king_cm;
  static double const cross_y_king = (mid_y_king_cm - cross_y_king_cm) / height_king_cm;
  static double const cross_top = (mid_y_king_cm - cross_top_cm) / height_king_cm;

  // Derived values.
  static double const inside_radius_king = -inside_left;
  static double const inside_scale_king = 0.180132; // Same value as used for the queen.
  static double const band_top_radius = -band_edge_left;
  static double const band_top_scale = inside_scale_king;
  static double const band_top_y = band_top_king + band_top_radius * band_top_scale;
  static double const cos_alpha = band_left / band_edge_left;
  static double const alpha = acos(cos_alpha);
  static double const band_bottom_scale = inside_scale_king;
  static double const band_bottom_radius = band_top_radius;
  static double const band_bottom_y = bottom_king - band_bottom_radius * band_bottom_scale;
  static double const dx = band_top_radius * (1.0 - cos_alpha);
  static double const band_line_scale = band_top_scale;
  static double const band_line_radius = band_top_radius - dx;
  static double const band_line_y = band_line_top + band_line_radius * band_line_scale;
  static double const blob_radius = 0.7071067 * (blob_left + band_top_y - band_left - blob_top);
  static double const blob_x = blob_left + blob_radius;
  static double const blob_y = blob_top + blob_radius;
  static double const center_blob_radius = -center_blob_left;
  static double const center_blob_y = center_blob_top + center_blob_radius;
  // Manual adjustment... looks better.
  static double const adjusted_center_blob_radius = center_blob_radius + 0.01;
  static double const beta_king = asin(adjusted_center_blob_radius / (center_y - center_blob_y));
  static double const center2_y = blob_y - blob_x - 1.4142136 * blob_radius;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw left blob.
  cairo_move_to(cr, band_left, band_top_y);
  cairo_arc(cr, blob_x, blob_y, blob_radius, 0.75 * M_PI, 1.75 * M_PI);
  cairo_line_to(cr, 0.0, center2_y);

  // Draw right blob.
  cairo_arc(cr, -blob_x, blob_y, blob_radius, -0.75 * M_PI, 0.25 * M_PI);
  cairo_line_to(cr, -band_left, band_top_y);

  set_fill_color(cr, white);
  cairo_fill_preserve(cr);

  // Draw vertical line in the middle.
  cairo_move_to(cr, 0.0, band_top_y);
  cairo_line_to(cr, 0.0, center_y);

  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  // Draw center blob.
  cairo_move_to(cr, 0.0, center_y);
  cairo_arc(cr, 0.0, center_blob_y, adjusted_center_blob_radius, M_PI - beta_king, beta_king);
  cairo_close_path(cr);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  // Draw cross.
  cairo_move_to(cr, 0.0, center_blob_y - adjusted_center_blob_radius);
  cairo_line_to(cr, 0.0, cross_top);
  cairo_move_to(cr, cross_left, cross_y_king);
  cairo_line_to(cr, -cross_left, cross_y_king);
  cairo_stroke(cr);

  // Draw half ellipse just below the blobs.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_top_y);
  cairo_scale(cr, 1.0, band_top_scale);
  cairo_arc(cr, 0.0, 0.0, band_top_radius, M_PI - alpha, 2 * M_PI + alpha);
  cairo_restore(cr);

  // Draw right side of the upper band.
  cairo_line_to(cr, -band_left, band_line_y);

  // Draw right side of lower band and bottom.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_bottom_y);
  cairo_scale(cr, 1.0, band_bottom_scale);
  cairo_arc(cr, 0.0, 0.0, band_bottom_radius, 0.0, M_PI);
  cairo_restore(cr);

  // Draw left side of lower band.
  cairo_line_to(cr, band_left, band_line_y);

  // Draw left side of upper band.
  cairo_close_path(cr);

  cairo_path_t* path = cairo_copy_path(cr);

  if (white)
    set_fill_color(cr, white);
  cairo_fill(cr);

  // Draw lower half ellipse of upper band.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_line_y);
  cairo_scale(cr, 1.0, band_line_scale);
  cairo_arc(cr, 0.0, 0.0, band_line_radius, -M_PI, 0.0);
  cairo_restore(cr);

  cairo_new_sub_path(cr);

  // Draw opening at bottom, align it with the real bottom.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band_bottom_y + band_bottom_radius * band_bottom_scale - inside_radius_king * inside_scale_king);
  cairo_scale(cr, 1.0, inside_scale_king);
  if (white)
    cairo_arc(cr, 0.0, 0.0, inside_radius_king, -M_PI, M_PI);
  else
    cairo_arc(cr, 0.0, 0.0, inside_radius_king, -M_PI - alpha, alpha);
  cairo_restore(cr);

  set_line_color(cr, white);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }

  cairo_append_path(cr, path);
  if (!white)
    set_fill_color(cr, white);
  cairo_stroke(cr);

  cairo_path_destroy(path);

  if (!white)
  {
    // Draw the white lines inside the blobs.

    static double const av_line_width = 0.5 * (black_line_width + white_line_width);
    static double const da = av_line_width / band_top_radius;
    static double const dy = av_line_width * tan(0.5 * beta_king);

    cairo_save(cr);
    cairo_translate(cr, 0.0, band_top_y);
    cairo_scale(cr, 1.0, band_top_scale);
    cairo_arc_negative(cr, 0.0, 0.0, band_top_radius, -0.5 * M_PI - da, M_PI + alpha + da);
    cairo_restore(cr);

    cairo_arc(cr, blob_x, blob_y, blob_radius - av_line_width, 0.75 * M_PI, 1.75 * M_PI);

    static double const center2b_y = center2_y + av_line_width * 1.4142136;
    static double const sin_beta = adjusted_center_blob_radius / (center_y - center_blob_y);
    static double const x_king = sin_beta * (center_y + av_line_width / sin_beta - center2b_y) / sin(0.25 * M_PI - beta_king);
    static double const y_king = center2b_y - x_king;

    cairo_line_to(cr, -x_king, y_king);
    cairo_line_to(cr, -av_line_width, center_y + dy);

    cairo_close_path(cr);

    cairo_new_sub_path(cr);

    cairo_save(cr);
    cairo_translate(cr, 0.0, band_top_y);
    cairo_scale(cr, 1.0, band_top_scale);
    cairo_arc_negative(cr, 0.0, 0.0, band_top_radius, -alpha - da, -0.5 * M_PI + da);
    cairo_restore(cr);

    cairo_line_to(cr, av_line_width, center_y + dy);
    cairo_line_to(cr, x_king, y_king);

    cairo_arc(cr, -blob_x, blob_y, blob_radius - av_line_width, -0.75 * M_PI, 0.25 * M_PI);

    cairo_close_path(cr);

    cairo_new_sub_path(cr);

    cairo_move_to(cr, 0.0, center_y - av_line_width / sin_beta);
    cairo_arc(cr, 0.0, center_blob_y, adjusted_center_blob_radius - av_line_width, M_PI - beta_king, beta_king);

    cairo_close_path(cr);

    set_line_color(cr, white);
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
  }

  cairo_restore(cr);
}

void ChessboardWidget::draw_queen(cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const width_queen_cm = 5.34;
  static double const inside_width_cm = 2.97;
  static double const band1_width_cm = 2.59;
  static double const crown_bottom_width_cm = 3.31;
  static double const height_queen_cm = 5.39;
  static double const bottom_queen_cm = 0.5;
  static double const inside_height_cm = 0.54;
  static double const band1_height_cm = 0.47;
  static double const band2_height_cm = 0.43;
  static double const tooth_outside_cm = 1.83;
  static double const tooth_inside_cm = 2.20;
  static double const tooth_inside2_cm = 2.36;
  static double const ball_outside_diameter_cm = 0.6;
  static double const ball_top1_cm = 4.31;
  static double const ball_right1_cm = 0.90;
  static double const ball_top2_cm = 4.80;
  static double const ball_right2_cm = 1.88;
  static double const tooth3_x_cm = 2.25;
  // Derived values.
  static double const mid_x_queen_cm = width_queen_cm / 2;
  static double const mid_y_queen_cm = height_queen_cm / 2;

  // Same, in coordinates.
  static double const inside_width = inside_width_cm / width_queen_cm;
  static double const band1_width = band1_width_cm / width_queen_cm;
  static double const crown_bottom_width = crown_bottom_width_cm / width_queen_cm;
  static double const bottom_queen = (mid_y_queen_cm - bottom_queen_cm) / height_queen_cm;
  static double const inside_height = inside_height_cm / height_queen_cm;
  static double const band1_height = band1_height_cm / height_queen_cm;
  static double const band2_height = band2_height_cm / height_queen_cm;
  static double const tooth_outside = (mid_y_queen_cm - tooth_outside_cm) / height_queen_cm;
  static double const tooth_inside = (mid_y_queen_cm - tooth_inside_cm) / height_queen_cm;
  static double const tooth_inside2 = (mid_y_queen_cm - tooth_inside2_cm) / height_queen_cm;
  static double const ball_outside_diameter = ball_outside_diameter_cm / height_queen_cm;
  static double const ball_top1 = (mid_y_queen_cm - ball_top1_cm) / height_queen_cm;
  static double const ball_right1 = (ball_right1_cm - mid_x_queen_cm) / width_queen_cm;
  static double const ball_top2 = (mid_y_queen_cm - ball_top2_cm) / height_queen_cm;
  static double const ball_right2 = (ball_right2_cm - mid_x_queen_cm) / width_queen_cm;
  static double const tooth3_x = (tooth3_x_cm - mid_x_queen_cm) / width_queen_cm;

  // Derived values.
  static double const inside_radius_queen = inside_width / 2;
  static double const inside_scale_queen = inside_height / inside_width;
  static double const inside_y_queen = bottom_queen - inside_radius_queen * inside_scale_queen;
  static double const band1_radius = band1_width / 2;
  static double const band1_scale = inside_scale_queen;
  static double const band1_y = bottom_queen - inside_height - band1_height + band1_radius * band1_scale;
  static double const crown_bottom_left = -crown_bottom_width / 2;
  static double const band2_radius = band1_radius + (-band1_radius - crown_bottom_left) * band2_height / (band1_y - tooth_outside);
  static double const band2_scale = band1_scale;
  static double const band2_y = bottom_queen - inside_height - band1_height - band2_height + band2_radius * band2_scale;
  static double const ball1_x = ball_right1 - ball_outside_diameter / 2;
  static double const ball2_x = ball_right2 - ball_outside_diameter / 2;
  static double const ball1_y = ball_top1 + ball_outside_diameter / 2;
  static double const ball2_y = ball_top2 + ball_outside_diameter / 2;
  static double const ball_radius_queen = (ball_outside_diameter - black_line_width) / 2;
  // We calculate ball3_y, so it lays on a perfect circle with the other balls.
  // The distance from ballN to a point (0, ball_center_y) is:
  // sqrt(ballN_x^2 + (ballN_y - ball_center_y)^2), and we find
  // ball_center_y by setting this distance equal for ball1 and 2:
  // ball1_x^2 + ball1_y^2 - 2 ball1_y ball_center_y = ball2_x^2 + ball2_y^2 - 2 ball2_y ball_center_y -->
  static double const ball_center_y = 0.5 * (ball2_x * ball2_x + ball2_y * ball2_y - ball1_x * ball1_x - ball1_y * ball1_y) / (ball2_y - ball1_y);
  static double const ball3_y = ball_center_y - sqrt(ball1_x * ball1_x + (ball1_y - ball_center_y) * (ball1_y - ball_center_y));
  // The tooth points are derived (which turns out better than measuring them).
  static double const ball1_angle = atan((0.5 * (crown_bottom_left + ball2_x) - ball1_x) / (tooth_outside - ball1_y));
  static double const tooth1_x = ball1_x + ball_radius_queen * sin(ball1_angle);
  static double const tooth2_x = ball2_x;
  static double const tooth1_top = ball1_y + ball_radius_queen * cos(ball1_angle);
  static double const tooth2_top = ball2_y + ball_radius_queen;
  static double const tooth3_top = ball3_y + ball_radius_queen;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw the outline of the queen.
  // First fill, then stroke.
  for (int stroke = 0; stroke < 2; ++stroke)
  {
    // Draw the right side.
    cairo_move_to(cr, -tooth1_x, tooth1_top);
    cairo_line_to(cr, -crown_bottom_left, tooth_outside);
    cairo_line_to(cr, band1_radius, band1_y);
    // The call to arc() draws the last line part, to (inside_radius_queen, inside_y_queen).

    // Draw the half ellipse that makes out the bottom.
    cairo_save(cr);
    cairo_translate(cr, 0.0, inside_y_queen);
    cairo_scale(cr, 1.0, inside_scale_queen);
    cairo_arc(cr, 0.0, 0.0, inside_radius_queen, 0.0, M_PI);
    cairo_restore(cr);

    // Draw the left side.
    cairo_line_to(cr, -band1_radius, band1_y);
    cairo_line_to(cr, crown_bottom_left, tooth_outside);
    cairo_line_to(cr, tooth1_x, tooth1_top);

    // The lines of the teeth should not be 'connected' when we are stroking.
    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, tooth1_x, tooth1_top);
    }

    // Draw right-side of left-most tooth.
    cairo_line_to(cr, tooth2_x, tooth_inside);

    // Draw left-side of second tooth.
    cairo_line_to(cr, tooth2_x, tooth2_top);

    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, tooth2_x, tooth2_top);
    }

    // Draw right-side of second tooth.
    cairo_line_to(cr, tooth3_x, tooth_inside2);

    // Draw left-side of middle tooth.
    cairo_line_to(cr, 0.0, tooth3_top);

    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, 0.0, tooth3_top);
    }

    // Draw right-side of middle tooth.
    cairo_line_to(cr, -tooth3_x, tooth_inside2);

    // Draw left-side of fourth tooth.
    cairo_line_to(cr, -tooth2_x, tooth2_top);

    if (stroke)
    {
      cairo_new_sub_path(cr);
      cairo_move_to(cr, -tooth2_x, tooth2_top);
    }

    // Draw right-side of fourth tooth.
    cairo_line_to(cr, -tooth2_x, tooth_inside);

    // Draw left-side of last tooth.
    cairo_line_to(cr, -tooth1_x, tooth1_top);

    if (stroke)
    {
      if (white)
        set_line_color(cr, white);
      else
        set_fill_color(cr, white);
      cairo_stroke(cr);
    }
    else
    {
      set_fill_color(cr, white);
      cairo_fill(cr);

      // Draw the upper side of the bottom ellipse.
      cairo_save(cr);
      cairo_translate(cr, 0.0, inside_y_queen);
      cairo_scale(cr, 1.0, inside_scale_queen);
      cairo_arc(cr, 0.0, 0.0, inside_radius_queen, -M_PI, 0.0);
      cairo_restore(cr);

      cairo_new_sub_path(cr);

      // Draw the half ellipse of band1.
      cairo_save(cr);
      cairo_translate(cr, 0.0, band1_y);
      cairo_scale(cr, 1.0, band1_scale);
      cairo_arc(cr, 0.0, 0.0, band1_radius, -M_PI, 0.0);
      cairo_restore(cr);

      set_line_color(cr, white);
      if (white)
	cairo_stroke(cr);
      else
      {
	cairo_set_line_width(cr, white_line_width);
	cairo_stroke(cr);
	cairo_set_line_width(cr, black_line_width);
      }
    }
  }

  // Draw the five balls.

  cairo_arc(cr, ball1_x, ball1_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_arc(cr, ball2_x, ball2_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_arc(cr, 0.0, ball3_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_arc(cr, -ball2_x, ball2_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_arc(cr, -ball1_x, ball1_y, ball_radius_queen, -M_PI, M_PI);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  if (white)
  {
    // Draw the splines at the bottom of the teeth.
    // The top left y-coordinate.
    static double const y0_queen = 0.0952;
    // The y-coordinate of the middle point.
    static double const ym = 0.0331;
    // The top left y-coordinate lays on the left side of the first tooth.
    // Calculate the x-coordinate:
    static double const x0_queen = tooth1_x + (y0_queen - tooth1_top) * (crown_bottom_left - tooth1_x) / (tooth_outside - tooth1_top);
    // The (apparent) tilting angle.
    static double const tilt_angle = atan((ym - y0_queen) / x0_queen);

    // The angle that the control lines make with the y-axis, before
    // mapping them onto a cylinder and before tilting the cylinder.
    static double const beta_queen = 1.202;
    // The length of the control lines.
    static double const len = 0.1728;
    // The y-value of the control points before tilting (relative to y0_queen).
    static double const py = len * cos(beta_queen);
    static double const y0_plus_py_cos_tilt_angle = y0_queen + py * cos(tilt_angle);
    static double const sin_tilt_angle = sin(tilt_angle);
    // The x-offset of the control points (this is an angle).
    static double px_offset = len * sin(beta_queen);

    cairo_move_to(cr, crown_bottom_left, tooth_outside);
    cairo_line_to(cr, x0_queen, y0_queen);

    // We have to draw N splines.
    int const N = 4;
    for (int i = 0; i < N; ++i)
    {
      double const alpha = i * M_PI / N;
      // The spline points before tilting.
      double px2 = x0_queen * cos(alpha + px_offset);
      double pz2 = - x0_queen * sin(alpha + px_offset);
      double px3 = x0_queen * cos(alpha + M_PI / N - px_offset);
      double pz3 = - x0_queen * sin(alpha + M_PI / N - px_offset);
      double px4 = x0_queen * cos(alpha + M_PI / N);
      double pz4 = - x0_queen * sin(alpha + M_PI / N);
      // Calculate the tilted values. This only has influence on the y value
      // (we rotate around the x-axis, and the resulting z-value doesn't interest us).
      double tpy2 = y0_plus_py_cos_tilt_angle - pz2 * sin_tilt_angle;
      double tpy3 = y0_plus_py_cos_tilt_angle - pz3 * sin_tilt_angle;
      double tpy4 = y0_queen - pz4 * sin_tilt_angle;
      cairo_curve_to(cr, px2, tpy2, px3, tpy3, px4, tpy4);
    }

    cairo_line_to(cr, -crown_bottom_left, tooth_outside);
  }

  // Draw the half ellipse of band2.
  cairo_save(cr);
  cairo_translate(cr, 0.0, band2_y);
  cairo_scale(cr, 1.0, band2_scale);
  cairo_arc_negative(cr, 0.0, 0.0, band2_radius, -0.15, -M_PI + 0.15);
  cairo_restore(cr);

  if (white)
  {
    cairo_close_path(cr);
    set_fill_color(cr, white);
    cairo_fill_preserve(cr);
  }
  else
    cairo_set_line_width(cr, white_line_width);
  set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void ChessboardWidget::draw_rook(cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const width_rook_cm = 5.33;
  static double const foot_left_cm = 0.90;
  static double const base_left_cm = 1.26;
  static double const tower_left_cm = 1.64;
  static double const opening_left_cm = 1.795;
  static double const opening_right_cm = 2.315;
  static double const height_rook_cm = 5.30;
  static double const bottom_rook_cm = 0.58;
  static double const foot_top_cm = 0.95;
  static double const base_top_cm = 1.41;
  static double const tower_bottom_cm = 1.76;
  static double const tower_top_cm = 3.43;
  static double const top_bottom_cm = 3.81;
  static double const opening_bottom_cm = 4.25;
  // static double const top_top_cm = 4.61;

  // In coordinates.
  static double const foot_left = -0.5 + foot_left_cm / width_rook_cm + 0.5 * black_line_width;
  static double const base_left = -0.5 + base_left_cm / width_rook_cm + 0.5 * black_line_width;
  static double const tower_left = -0.5 + tower_left_cm / width_rook_cm + 0.5 * black_line_width;
  static double const opening_left = -0.5 + opening_left_cm / width_rook_cm + 0.5 * black_line_width;
  static double const opening_right = -0.5 + opening_right_cm / width_rook_cm + 0.5 * black_line_width;
  static double const bottom_rook = 0.5 - bottom_rook_cm / height_rook_cm - 0.5 * black_line_width;
  static double const foot_top = 0.5 - foot_top_cm / height_rook_cm - 0.5 * black_line_width;
  static double const base_top = 0.5 - base_top_cm / height_rook_cm - 0.5 * black_line_width;
  static double const tower_bottom = 0.5 - tower_bottom_cm / height_rook_cm - 0.5 * black_line_width;
  static double const tower_top = 0.5 - tower_top_cm / height_rook_cm - 0.5 * black_line_width;
  static double const top_bottom = 0.5 - top_bottom_cm / height_rook_cm - 0.5 * black_line_width;
  static double const opening_bottom = 0.5 - opening_bottom_cm / height_rook_cm - 0.5 * black_line_width;
  // static double const top_top = 0.5 - top_top_cm / height_rook_cm - 0.5 * black_line_width;
  // For alignment purposes, it's better to have the rook *precisely* centered.
  static double const top_top = -bottom_rook;

  // Snapped coordinates.
  double const inner_line_width = white ? black_line_width : snap_line_width(white_line_width, scale);
  double const bottom_sn = snap_bottom(bottom_rook, y, scale, black_line_width);
  double const foot_top_sn = snap_bottom(foot_top, y, scale, inner_line_width);
  double const base_top_sn = snap_bottom(base_top, y, scale, inner_line_width);
  double const tower_bottom_sn = snap_bottom(tower_bottom, y, scale, inner_line_width);
  double const tower_top_sn = snap_top(tower_top, y, scale, inner_line_width);
  double const top_bottom_sn = snap_top(top_bottom, y, scale, inner_line_width);
  double const opening_bottom_sn = snap_top(opening_bottom, y, scale, black_line_width);
  double const top_top_sn = snap_top(top_top, y, scale, black_line_width);

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw left side.
  cairo_move_to(cr, foot_left, bottom_sn);
  cairo_line_to(cr, foot_left, foot_top_sn);
  cairo_line_to(cr, base_left, foot_top_sn);
  cairo_line_to(cr, base_left, base_top_sn);
  cairo_line_to(cr, tower_left, tower_bottom_sn);
  cairo_line_to(cr, tower_left, tower_top_sn);
  cairo_line_to(cr, base_left, top_bottom_sn);
  cairo_line_to(cr, base_left, top_top_sn);

  // Draw top side.
  cairo_line_to(cr, opening_left, top_top_sn);
  cairo_line_to(cr, opening_left, opening_bottom_sn);
  cairo_line_to(cr, opening_right, opening_bottom_sn);
  cairo_line_to(cr, opening_right, top_top_sn);
  cairo_line_to(cr, -opening_right, top_top_sn);
  cairo_line_to(cr, -opening_right, opening_bottom_sn);
  cairo_line_to(cr, -opening_left, opening_bottom_sn);
  cairo_line_to(cr, -opening_left, top_top_sn);
  cairo_line_to(cr, -base_left, top_top_sn);

  // Draw right side.
  cairo_line_to(cr, -base_left, top_bottom_sn);
  cairo_line_to(cr, -tower_left, tower_top_sn);
  cairo_line_to(cr, -tower_left, tower_bottom_sn);
  cairo_line_to(cr, -base_left, base_top_sn);
  cairo_line_to(cr, -base_left, foot_top_sn);
  cairo_line_to(cr, -foot_left, foot_top_sn);
  cairo_line_to(cr, -foot_left, bottom_sn);

  // Draw bottom line.
  cairo_close_path(cr);
  cairo_path_t* path = cairo_copy_path(cr);

  set_fill_color(cr, white);
  cairo_fill(cr);

  // Draw inner horizontal lines.
  cairo_move_to(cr, base_left + 0.5 * black_line_width, foot_top_sn);
  cairo_line_to(cr, -base_left - 0.5 * black_line_width, foot_top_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, base_left, base_top_sn);
  cairo_line_to(cr, -base_left, base_top_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, tower_left + (white ? 0.0 : (0.5 * black_line_width)), tower_bottom_sn);
  cairo_line_to(cr, -tower_left - (white ? 0.0 : (0.5 * black_line_width)), tower_bottom_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, tower_left + (white ? 0.0 : (0.5 * black_line_width)), tower_top_sn);
  cairo_line_to(cr, -tower_left - (white ? 0.0 : (0.5 * black_line_width)), tower_top_sn);
  cairo_new_sub_path(cr);
  cairo_move_to(cr, base_left + black_line_width * 0.5, top_bottom_sn);
  cairo_line_to(cr, -base_left - black_line_width * 0.5, top_bottom_sn);

  set_line_color(cr, white);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, inner_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }

  cairo_append_path(cr, path);
  if (!white)
    set_fill_color(cr, white);
  cairo_stroke(cr);

  cairo_path_destroy(path);

  cairo_restore(cr);
}

void ChessboardWidget::draw_bishop(cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements, in cm.
  static double const width_bishop_cm = 5.34;
  static double const ribbon_width_cm = 0.49;
  static double const ribbon_bottom_left_cm = 0.72;
  static double const ribbon_top_left_cm = 2.28;
  static double const inside_outer_diameter_cm = 2.0;
  static double const circle_diameter_cm = 2.44;
  static double const cross_width_cm = 0.93;
  static double const ball_outer_diameter_cm = 0.81;
  static double const ball_inner_diameter_cm = 0.41;
  static double const circle_start_angle = 0.767;
  static double const ribbon_end_angle = 1.097;
  static double const height_bishop_cm = 5.44;
  static double const ribbon_bottom_y1_cm = 0.52;
  static double const ribbon_bottom_y2_cm = 0.76;
  static double const ribbon_bottom_y3_cm = 0.55;
  static double const ribbon_top_y1_cm = 0.99;
  static double const ribbon_top_y2_cm = 1.25;
  static double const ribbon_inside_y_cm = 0.93;
  static double const inside_bottom_cm = 1.34;
  static double const inside_top_cm = 1.86;
  static double const band_top_bishop_cm = 2.34;
  static double const circle_y_cm = 3.11;
  static double const cross_y_bishop_cm = 3.24;
  static double const point_y_cm = 4.47;
  static double const ball_y_cm = 4.675;
  static double const sp1_x_cm = 2.1;
  static double const sp1_y_cm = 3.95;
  static double const ribbon_bottom_x1_cm = 3.34;
  static double const ribbon_bottom_x2_cm = 4.1;
  static double const ribbon_top_x1_cm = 3.54;
  static double const ribbon_top_x2_cm = 4.24;

  // Translate to coordinates.
  static double const ribbon_width = ribbon_width_cm / height_bishop_cm;
  static double const ribbon_bottom_left = -0.5 + ribbon_bottom_left_cm / width_bishop_cm;
  static double const ribbon_bottom_x1 = -0.5 + ribbon_bottom_x1_cm / width_bishop_cm;
  static double const ribbon_bottom_x2 = -0.5 + ribbon_bottom_x2_cm / width_bishop_cm;
  static double const ribbon_top_x1 = -0.5 + ribbon_top_x1_cm / width_bishop_cm;
  static double const ribbon_top_x2 = -0.5 + ribbon_top_x2_cm / width_bishop_cm;
  static double const ribbon_top_left = -0.5 + ribbon_top_left_cm / width_bishop_cm;
  static double const inside_radius_bishop = 0.5 * (inside_outer_diameter_cm / width_bishop_cm - black_line_width);
  static double const circle_radius = 0.5 * circle_diameter_cm / width_bishop_cm;
  static double const cross_leg = 0.5 * cross_width_cm / width_bishop_cm;
  static double const ball_radius_bishop  = 0.25 * (ball_outer_diameter_cm + ball_inner_diameter_cm) / width_bishop_cm;
  static double const ball_line_width = black_line_width; // 0.5 * (ball_outer_diameter_cm - ball_inner_diameter_cm) / width_bishop_cm
  static double const ribbon_bottom_y1 = 0.5 - ribbon_bottom_y1_cm / height_bishop_cm - 0.5 * black_line_width;
  static double const ribbon_bottom_y2 = 0.5 - ribbon_bottom_y2_cm / height_bishop_cm + 0.5 * black_line_width;
  static double const ribbon_bottom_y3 = 0.5 - ribbon_bottom_y3_cm / height_bishop_cm;
  static double const ribbon_inside_y = 0.5 - ribbon_inside_y_cm / height_bishop_cm;
  static double const ribbon_top_y1 = 0.5 - ribbon_top_y1_cm / height_bishop_cm - 0.5 * black_line_width;
  static double const ribbon_top_y2 = 0.5 - ribbon_top_y2_cm / height_bishop_cm + 0.5 * black_line_width;
  static double const inside_scale_bishop = ((inside_top_cm - inside_bottom_cm) / height_bishop_cm - black_line_width) / (2 * inside_radius_bishop);
  static double const inside_y_bishop = 0.5 - 0.5 * (inside_top_cm + inside_bottom_cm) / height_bishop_cm;
  static double const inside_bottom = 0.5 - inside_bottom_cm / height_bishop_cm - 0.5 * black_line_width;
  static double const band_top_bishop = 0.5 - band_top_bishop_cm / height_bishop_cm + 0.5 * black_line_width;
  static double const circle_y = 0.5 - circle_y_cm / height_bishop_cm;
  static double const cross_y_bishop = 0.5 - cross_y_bishop_cm / height_bishop_cm;
  static double const point_y = 0.5 - point_y_cm / height_bishop_cm;
  static double const ball_y = 0.5 - ball_y_cm / height_bishop_cm;
  static double const inside_angle = acos(-ribbon_top_left / inside_radius_bishop);
  static double const sp1_x = -0.5 + sp1_x_cm / width_bishop_cm;
  static double const sp1_y = 0.5 - sp1_y_cm / height_bishop_cm;

  // Precalculations for the ribbon.
  static double const spline_magic = 0.551784;
  static double const cp2_x = ribbon_bottom_y1 - ribbon_inside_y;
  static double const sp2_x = spline_magic * cp2_x;
  static double const sp2_y = ribbon_inside_y + spline_magic * (ribbon_bottom_y1 - ribbon_inside_y);
  static double const sp3_x = ribbon_bottom_x1 - spline_magic * (ribbon_bottom_x1 - cp2_x);
  static double const sp3_y = ribbon_bottom_y1;
  static double const sp4_x = ribbon_bottom_x1 + spline_magic * (ribbon_bottom_x2 - ribbon_bottom_x1);
  static double const sp4_y = ribbon_bottom_y1;
  static double const sp5_x = ribbon_bottom_x2 - spline_magic * (ribbon_bottom_x2 - ribbon_bottom_x1);
  static double const sp5_y = ribbon_bottom_y2;
  static double const cp6_x = -ribbon_bottom_left - (ribbon_bottom_y3 - ribbon_bottom_y2) * tan(ribbon_end_angle);
  static double const sp6_x = ribbon_bottom_x2 + spline_magic * (cp6_x - ribbon_bottom_x2);
  static double const sp6_y = ribbon_bottom_y2;
  static double const sp7_x = -ribbon_bottom_left - spline_magic * (-ribbon_bottom_left - cp6_x);
  static double const sp7_y = ribbon_bottom_y3 - spline_magic * (ribbon_bottom_y3 - ribbon_bottom_y2);
  static double const ribbon_end_top_x = -ribbon_bottom_left + ribbon_width * cos(ribbon_end_angle);
  static double const ribbon_end_top_y = ribbon_bottom_y3 - ribbon_width * sin(ribbon_end_angle);
  static double const cp8_x = ribbon_end_top_x - (ribbon_end_top_y - ribbon_top_y2) * tan(ribbon_end_angle);
  static double const sp8_x = ribbon_end_top_x - spline_magic * (ribbon_end_top_x - cp8_x);
  static double const sp8_y = ribbon_end_top_y - spline_magic * (ribbon_end_top_y - ribbon_top_y2);
  static double const sp9_x = ribbon_top_x2 + spline_magic * (cp8_x - ribbon_top_x2);
  static double const sp9_y = ribbon_top_y2;
  static double const sp10_x = ribbon_top_x2 - spline_magic * (ribbon_top_x2 - ribbon_top_x1);
  static double const sp10_y = ribbon_top_y2;
  static double const sp11_x = ribbon_top_x1 + spline_magic * (ribbon_top_x2 - ribbon_top_x1);
  static double const sp11_y = ribbon_top_y1;
  static double const ribbon_top_y3 = 0.2695;
  static double const sp12_x = ribbon_top_x1 - spline_magic * (ribbon_top_x1 + ribbon_top_left);
  static double const sp12_y = ribbon_top_y1;
  static double const sp13_x = -ribbon_top_left;
  static double const sp13_y = ribbon_top_y3 + 0.509 * spline_magic * (ribbon_top_y1 - ribbon_top_y3);

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);
  cairo_set_line_width(cr, black_line_width);

  // Draw the ribbon.

  // Part 14.
  cairo_move_to(cr, -ribbon_top_x1, ribbon_top_y1);
  cairo_curve_to(cr, -sp11_x, sp11_y, -sp10_x, sp10_y, -ribbon_top_x2, ribbon_top_y2);

  // Part 13.
  cairo_curve_to(cr, -sp9_x, sp9_y, -sp8_x, sp8_y, -ribbon_end_top_x, ribbon_end_top_y);

  // Part 12.
  cairo_line_to(cr, ribbon_bottom_left, ribbon_bottom_y3);

  // Part 11.
  cairo_curve_to(cr, -sp7_x, sp7_y, -sp6_x, sp6_y, -ribbon_bottom_x2, ribbon_bottom_y2);

  // Part 10.
  cairo_curve_to(cr, -sp5_x, sp5_y, -sp4_x, sp4_y, -ribbon_bottom_x1, ribbon_bottom_y1);

  // Part 9.
  cairo_curve_to(cr, -sp3_x, sp3_y, -sp2_x, sp2_y, 0.0, ribbon_inside_y);

  // Part 1.
  cairo_curve_to(cr, sp2_x, sp2_y, sp3_x, sp3_y, ribbon_bottom_x1, ribbon_bottom_y1);

  // Part 2.
  cairo_curve_to(cr, sp4_x, sp4_y, sp5_x, sp5_y, ribbon_bottom_x2, ribbon_bottom_y2);

  // Part 3.
  cairo_curve_to(cr, sp6_x, sp6_y, sp7_x, sp7_y, -ribbon_bottom_left, ribbon_bottom_y3);

  // Part 4.
  cairo_line_to(cr, ribbon_end_top_x, ribbon_end_top_y);

  // Part 5.
  cairo_curve_to(cr, sp8_x, sp8_y, sp9_x, sp9_y, ribbon_top_x2, ribbon_top_y2);

  // Part 6.
  cairo_curve_to(cr, sp10_x, sp10_y, sp11_x, sp11_y, ribbon_top_x1, ribbon_top_y1);

  if (!white)
  {
    set_fill_color(cr, white);
    cairo_fill_preserve(cr);
    cairo_stroke(cr);
    cairo_move_to(cr, ribbon_top_x1, ribbon_top_y1);
  }

  // Part 7.
  cairo_curve_to(cr, sp12_x, sp12_y, sp13_x, sp13_y, -ribbon_top_left, ribbon_top_y3);

  // Part 8 and 17.
  cairo_save(cr);
  cairo_translate(cr, 0.0, inside_y_bishop);
  cairo_scale(cr, 1.0, inside_scale_bishop);
  cairo_arc(cr, 0.0, 0.0, inside_radius_bishop, inside_angle, M_PI - inside_angle);
  cairo_restore(cr);

  // Part 16.
  cairo_line_to(cr, ribbon_top_left, ribbon_top_y3);

  // Part 15.
  cairo_curve_to(cr, -sp13_x, sp13_y, -sp12_x, sp12_y, -ribbon_top_x1 + 0.01 * black_line_width, ribbon_top_y1);
  cairo_close_path(cr);

  if (white)
    set_fill_color(cr, white);
  else
    set_line_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  else
    set_fill_color(cr, white);
  cairo_stroke(cr);

  // Draw vertical line between left and right ribbon.
  cairo_move_to(cr, 0.0, inside_bottom);
  cairo_line_to(cr, 0.0, ribbon_inside_y);
  cairo_stroke(cr);

  // Draw the outline of the bishop.

  cairo_save(cr);
  cairo_translate(cr, 0.0, inside_y_bishop);
  cairo_scale(cr, 1.0, inside_scale_bishop);
  cairo_arc(cr, 0.0, 0.0, inside_radius_bishop, 0.0, -M_PI);
  cairo_restore(cr);

  cairo_arc(cr, 0.0, circle_y, circle_radius, -M_PI - circle_start_angle, -M_PI);

  cairo_curve_to(cr, -circle_radius, circle_y - 0.0848, sp1_x - 0.02657, sp1_y + 0.01722, sp1_x, sp1_y);
  cairo_curve_to(cr, sp1_x + 0.08845, sp1_y - 0.05733, -0.000333, point_y + 0.000265, 0.0, point_y);
  cairo_curve_to(cr, 0.000333, point_y + 0.000265, -sp1_x - 0.08845, sp1_y - 0.05733, -sp1_x, sp1_y);
  cairo_curve_to(cr, -sp1_x + 0.02657, sp1_y + 0.01722, circle_radius, circle_y - 0.0848, circle_radius, circle_y);

  cairo_arc(cr, 0.0, circle_y, circle_radius, 0.0, circle_start_angle);

  cairo_close_path(cr);

  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  // Draw inside lines.
  if (!white)
    set_line_color(cr, white);
  cairo_save(cr);
  if (!white)
  {
    static double const x2_bishop = -circle_radius * cos(circle_start_angle);
    static double const y2_bishop = (circle_y + circle_radius * sin(circle_start_angle));
    cairo_move_to(cr, -inside_radius_bishop, inside_y_bishop);
    cairo_line_to(cr, x2_bishop, y2_bishop);
    cairo_line_to(cr, -x2_bishop, y2_bishop);
    cairo_line_to(cr, inside_radius_bishop, inside_y_bishop);
    cairo_close_path(cr);
    cairo_clip(cr);
  }
  cairo_save(cr);
  cairo_translate(cr, 0.0, inside_y_bishop);
  cairo_scale(cr, 1.0, inside_scale_bishop);
  cairo_arc(cr, 0.0, 0.0, inside_radius_bishop, -M_PI, 0.0);
  cairo_restore(cr);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }
  cairo_restore(cr);

  // Ok, some real math needed here.
  // First, we scale the y-axis - so that the problem changes to a circle instead of an ellipse.
  // Also, flip the y-axis, so that the top of the screen becomes the positive y-axis.
  // This means that instead of using the normal 'y' values, we now should use y / -inside_scale_bishop.
  // Next, calculate the line through (-inside_radius_bishop, inside_y_bishop) and the
  // start of the circle with vector parametric formula: V = X0 + U t1,
  // where U is the unit vector (u1, u2), t1 is the parameter and X0 = (x0, 0), the
  // point where the line crosses the x-axis.
  static double const x1 = -inside_radius_bishop;
  static double const y1 = inside_y_bishop / -inside_scale_bishop;
  static double const x2 = -circle_radius * cos(circle_start_angle);
  static double const y2 = (circle_y + circle_radius * sin(circle_start_angle)) / -inside_scale_bishop;
  static double const d = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
  static double const u1 = (x2 - x1) / d;
  static double const u2 = (y2 - y1) / d;
  static double const x0 = x1 + (x2 - x1) * (0 - y1) / (y2 - y1);
  // A line through the center of the circle, perpendicular to this line is:
  // V = Y0 + W t2, where Y0 = (0, y0) and W = (u2, -u1).
  // Those two lines cross therefore where,
  // x: x0 + u1 t1 = u2 t2
  // y: u2 t1 = y0 - u1 t2
  // Since the t2 parameter gives the distance from the center in terms of a unit vector,
  // the distance from the center to the crossing point is |t2|, which we find by cancelling t1:
  // x0 u2 + u1 (y0 - u1 t2) = u2^2 t2 --> t2 = x0 u2 + u1 y0, since u1^2 + u2^2 = 1.
  // In the crossing point, this value is negative (u2 is positive, and the cross point
  // lays left of the center). Hence we find that the distance is -t2 =
  // - x0 u2 - u1 y0 = band_top_bishop / -inside_scale_bishop - y0.
  static double const y0 = (band_top_bishop / -inside_scale_bishop + x0 * u2) / (1 - u1);
  static double const band_radius = band_top_bishop / -inside_scale_bishop - y0;
  static double const angle = atan(u1 / u2);
  cairo_save(cr);
  cairo_scale(cr, 1.0, -inside_scale_bishop);
  if (!white)
  {
    static double const t2 = x0 * u2 + u1 * y0;
    static double const t1 = (y0 - u1 * t2) / u2;
    static double const x = x0 + u1 * t1;
    cairo_move_to(cr, x, y0);
    cairo_line_to(cr, x + d * u1, y0 + d * u2);
    cairo_line_to(cr, -x - d * u1, y0 + d * u2);
    cairo_line_to(cr, -x, y0);
    cairo_close_path(cr);
    cairo_clip(cr);
  }
  cairo_arc(cr, 0.0, y0, band_radius, angle, M_PI - angle);
  // Reverse the scale before stroking, without restoring the clipping area.
  cairo_scale(cr, 1.0, -1.0 / inside_scale_bishop);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }
  cairo_restore(cr);

  // Draw the cross.
  cairo_move_to(cr, -cross_leg, cross_y_bishop);
  cairo_line_to(cr, cross_leg, cross_y_bishop);
  cairo_move_to(cr, 0.0, cross_y_bishop - cross_leg);
  cairo_line_to(cr, 0.0, cross_y_bishop + cross_leg);
  if (white)
    cairo_stroke(cr);
  else
  {
    cairo_set_line_width(cr, white_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, black_line_width);
  }

  if (!white)
  {
    cairo_move_to(cr, -inside_radius_bishop, inside_y_bishop);
    cairo_arc(cr, 0.0, circle_y, circle_radius, -M_PI - circle_start_angle, -M_PI);
    cairo_move_to(cr, inside_radius_bishop, inside_y_bishop);
    cairo_arc_negative(cr, 0.0, circle_y, circle_radius, circle_start_angle, 0.0);
    set_fill_color(cr, white);
    cairo_stroke(cr);
  }

  // Draw the little ball on the top.
  cairo_set_line_width(cr, ball_line_width);
  cairo_arc(cr, 0.0, ball_y, ball_radius_bishop, -M_PI, M_PI);
  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);

  cairo_restore(cr);
}

void ChessboardWidget::draw_knight(cairo_t* cr, gdouble x, gdouble y, gdouble scale, gboolean white)
{
  // Measurements.
  static double const height_knight_cm = 21.9;
  static double const pixels_per_cm = 32.467;
  static double const min_nose_x_px = 8.0;
  static double const right_ear_y_px = 15.0;		// See 'Draw right ear'.
  static double const bottom_right_x_px = 582.82;	// See 'back' curve.
  static double const bottom_right_y_px = 580.82;
  static double const bottom_left_x_px = 190.00;	// See 'front' curve.
  // Derived.
  static double const pixel_scale = 1.0 / (pixels_per_cm * height_knight_cm);
  static double const knight_black_line_width = 0.95 * black_line_width / pixel_scale;
  static double const knight_white_line_width = 1.3 * knight_black_line_width;
  static double const knight_white_glyp_line_width = knight_white_line_width - knight_black_line_width;

  // The outline of the knight in coordinates, without translation.
  static double const max_y = bottom_right_y_px * pixel_scale;
  static double const min_y = right_ear_y_px * pixel_scale;
  static double const max_x = bottom_right_x_px * pixel_scale;
  static double const min_x = min_nose_x_px * pixel_scale;

  // Calculate coordinate offsets, needed to center the knight.
  static double const pixel_translate_x = -(max_x + min_x) / 2;
  static double const pixel_translate_y = -(max_y + min_y) / 2;

  cairo_save(cr);
  cairo_translate(cr, x, y);
  cairo_scale(cr, scale, scale);

  // At this point, the coordinates run from -0.5 till 0.5.
  // However, we use pixels as coordinates (as measured in gimp)
  // Translate the image so that the pixel-coordinate center falls on (0, 0).
  cairo_translate(cr, pixel_translate_x, pixel_translate_y);
  // Scale, so we can use "pixel-coordinates".
  cairo_scale(cr, pixel_scale, pixel_scale);

  // Fill body.
  cairo_move_to(cr, 319.00, 299.00);
  cairo_curve_to(cr, 322.00, 449.00, 165.00, 445.00, 192.00, 570.00);
  cairo_curve_to(cr, 192.00, 570.00, 568.50, 571.00, 568.50, 571.00);
  cairo_curve_to(cr, 577.00, 426.00, 533.00, 99.00, 340.50, 88.50);
  cairo_curve_to(cr, 245.50, 87.50, 206.00, 86.00, 195.00, 102.00);
  set_fill_color(cr, white);
  cairo_fill(cr);

  // Draw shadow.
  cairo_move_to(cr, 315.00, 300.33);
  cairo_curve_to(cr, 301.43, 300.80, 291.75, 314.52, 282.00, 325.00);
  cairo_curve_to(cr, 298.67, 317.33, 316.33, 325.00, 317.33, 344.33);
  cairo_curve_to(cr, 321.33, 337.33, 326.00, 326.00, 315.00, 300.33);
  if (white)
    set_line_color(cr, white);
  cairo_fill(cr);

  // Draw back.
  // Change the thickness of the top of the back to a line width:
  static double const back_top_offset = (93.00 - knight_black_line_width) - 82.00;
  cairo_move_to(cr, 582.82, 580.82);
  cairo_curve_to(cr, 589.00, 359.00, 530.00,85.00, 332.00, 82.00 + back_top_offset);
  cairo_curve_to(cr, 320.87, 82.04 + back_top_offset, 314.25, 82.12 + back_top_offset, 302.50, 82.38 + back_top_offset);
  cairo_curve_to(cr, 302.75, 95.38, 296.22, 93.73, 319.50, 94.00);
  cairo_curve_to(cr, 510.50, 93.00, 556.12, 359.00, 556.12, 563.00);
  cairo_close_path(cr);
  cairo_fill(cr);

  // Draw front.
  cairo_move_to(cr, 190.00, 570.00);
  cairo_curve_to(cr, 190.00, 550.75, 190.00, 549.00, 190.00, 540.00);
  cairo_curve_to(cr, 190.00, 493.25, 210.50, 482.50, 285.00, 409.50);
  cairo_curve_to(cr, 298.25, 391.75, 313.00, 357.50, 317.75, 344.75);
  cairo_curve_to(cr, 320.25, 340.00, 320.25, 330.00, 320.00, 280.00);
  cairo_set_line_width(cr, knight_black_line_width);
  cairo_stroke(cr);

  // Draw head.
  cairo_move_to(cr, 144.00, 31.50);
  cairo_curve_to(cr, 122.50, 67.00, 147.50, 57.50, 146.00, 105.00);
  cairo_curve_to(cr, 112.00, 125.50, 123.00, 140.50, 102.50, 170.00);
  cairo_curve_to(cr, 84.00, 199.50, 128.00, 181.50, 33.50, 313.50);
  cairo_curve_to(cr, -23.00, 414.00, 81.50, 468.00, 130.00, 447.50);
  cairo_curve_to(cr, 182.50, 398.00, 142.50, 427.00, 179.50, 390.00);
  cairo_curve_to(cr, 194.50, 376.50, 212.50, 349.50, 237.50, 347.00);
  cairo_curve_to(cr, 268.00, 344.00, 283.50, 323.50, 306.00, 301.00);
  cairo_curve_to(cr, 327.50, 276.50, 330.00, 264.50, 330.00, 228.50);
  if (white)
    set_fill_color(cr, white);
  cairo_fill_preserve(cr);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  if (white)
    set_line_color(cr, white);
  cairo_stroke(cr);
  cairo_move_to(cr, 201.00, 94.50);
  cairo_curve_to(cr, 184.50, 54.50, 152.00, 43.50, 144.00, 31.50);
  cairo_stroke(cr);

  // Draw between ears.
  cairo_move_to(cr, 170.50, 136.50);
  cairo_curve_to(cr, 170.00, 129.50, 175.50, 125.00, 183.50, 116.00);
  cairo_curve_to(cr, 204.50, 91.00, 216.00, 94.00, 238.00, 91.00);
  cairo_stroke(cr);

  if (!white)
  {
    // Draw white hair.
    cairo_move_to(cr, 529.00, 570.00);
    cairo_curve_to(cr, 530.50, 352.00, 476.50, 128.50, 334.00, 121.00);
    cairo_curve_to(cr, 310.50, 118.50, 310.00, 117.50, 296.50, 117.50);
    cairo_curve_to(cr, 291.50, 100.00, 252.50, 95.50, 242.20, 119.35);
    cairo_curve_to(cr, 227.55, 120.95, 212.22, 124.23, 198.50, 130.50);
    cairo_curve_to(cr, 178.00, 137.50, 158.50, 147.50, 154.00, 137.00);
    cairo_curve_to(cr, 149.50, 127.00, 145.50, 121.00, 204.00, 100.00);
    cairo_curve_to(cr, 226.50, 90.00, 276.50, 92.00, 319.50, 94.00);
    cairo_curve_to(cr, 510.50, 93.00, 556.00, 354.00, 556.00, 570.00);
    cairo_curve_to(cr, 548.06, 571.00, 537.73, 569.45, 529.00, 570.00);
    set_line_color(cr, white);
    cairo_fill(cr);
  }

  // Draw bottom.
  double dummy = bottom_left_x_px;
  double bottom_right_y_px_sn = bottom_right_y_px;
  if (scale >= 27)
  {
    // Snap bottom to grid.
    cairo_user_to_device(cr, &dummy, &bottom_right_y_px_sn);
    bottom_right_y_px_sn = round(bottom_right_y_px_sn);
    cairo_device_to_user(cr, &dummy, &bottom_right_y_px_sn);
  }
  cairo_rectangle(cr, bottom_left_x_px - 0.5 * knight_black_line_width,
                      bottom_right_y_px_sn - knight_black_line_width,
                      bottom_right_x_px - (bottom_left_x_px - 0.5 * knight_black_line_width),
		      knight_black_line_width);
  if (!white)
    set_fill_color(cr, white);
  cairo_fill(cr);

  // Draw mouth.
  cairo_move_to(cr, 113.67, 389.33);
  cairo_curve_to(cr, 121.00, 388.00, 129.33, 406.67, 120.67, 414.67);
  cairo_curve_to(cr, 114.33, 419.33, 104.33, 431.00, 112.67, 444.00);
  //cairo_curve_to(cr, 133.53, 466.89, 104.22, 478.73, 93.00, 446.67);
  cairo_line_to(cr, 93.00, 446.67);
  cairo_curve_to(cr, 89.00, 418.67, 94.67, 417.33, 100.00, 412.67);
  cairo_curve_to(cr, 112.67, 402.00, 100.67, 394.67, 113.67, 389.33);
  if (!white)
    set_line_color(cr, white);
  if (white)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, knight_white_glyp_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, knight_black_line_width);
  }

  // Redraw a part of the head.
  cairo_move_to(cr, 33.50, 313.50);
  cairo_curve_to(cr, -23.00, 414.00, 81.50, 468.00, 130.00, 447.50);
  if (!white)
    set_fill_color(cr, white);
  cairo_stroke(cr);

  if (!white)
  {
    // Draw jaw.
    cairo_move_to(cr, 312.32, 293.46);
    cairo_curve_to(cr, 328.01, 273.63, 330.00, 260.62, 330.00, 228.50);
    cairo_set_line_width(cr, knight_white_line_width);
    set_line_color(cr, white);
    cairo_stroke(cr);
    cairo_set_line_width(cr, knight_black_line_width);
  }

  // Draw right ear.
  for (int stroke = 0; stroke < 2; ++stroke)
  {
    cairo_move_to(cr, 242.00, 114.00);
    cairo_curve_to(cr, 235.00, 76.00, 235.50, 92.50, 267.00, 15.00);	// 15.00 corresponds to min_y
    if (stroke)
      cairo_move_to(cr, 267.00, 15.00);
    cairo_curve_to(cr, 309.50, 85.50, 312.00, 88.00, 295.00, 117.00);
    if (stroke)
    {
      if (white)
        set_line_color(cr, white);
      cairo_stroke(cr);
    }
    else
    {
      if (white)
	set_fill_color(cr, white);
      else
        set_fill_color(cr, white);
      cairo_fill(cr);
    }
  }

  if (!white)
    set_line_color(cr, white);

  // Draw nose.
  cairo_move_to(cr, 76.00, 363.00);
  cairo_curve_to(cr, 66.00, 372.33, 78.33, 379.00, 66.00, 384.00);
  cairo_curve_to(cr, 21.00, 399.00, 61.67, 331.00, 79.67, 341.67);
  cairo_curve_to(cr, 81.00, 342.00, 84.67, 353.33, 76.00, 363.00);
  if (white)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, knight_white_glyp_line_width);
    cairo_stroke(cr);
    cairo_set_line_width(cr, knight_black_line_width);
  }

  // Draw eye.
  cairo_move_to(cr, 173.33, 208.00);
  cairo_curve_to(cr, 180.67, 207.00, 182.00, 197.67, 182.00, 197.67);
  cairo_curve_to(cr, 184.59, 176.98, 182.28, 177.30, 190.67, 173.00);
  cairo_curve_to(cr, 201.00, 169.33, 198.33, 146.00, 173.33, 161.67);
  cairo_curve_to(cr, 146.00, 181.33, 130.67, 192.00, 128.33, 202.67);
  cairo_curve_to(cr, 124.00, 233.33, 131.00, 227.33, 144.67, 207.00);
  cairo_curve_to(cr, 150.67, 201.00, 158.67, 193.67, 162.33, 203.33);
  cairo_curve_to(cr, 164.67, 206.00, 165.63, 209.29, 173.33, 208.00);
  if (white)
    cairo_fill(cr);
  else
  {
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, knight_white_glyp_line_width);
    cairo_stroke(cr);
  }

  cairo_restore(cr);
}

ChessboardWidget::ChessboardWidget() :
  // Set default values for the colors used when drawing the pieces.
  m_white_piece_fill_color{ 1.0, 1.0, 1.0 },
  m_white_piece_line_color{ 0.0, 0.0, 0.0 },
  m_black_piece_fill_color{ 0.0, 0.0, 0.0 },
  m_black_piece_line_color{ 1.0, 1.0, 1.0 },
  m_allocated_colors_mask(0),
  m_cursor_color{ 1.0, 0.0, 0.0 },
  m_draw_border(true),
  m_flip_board(false),
  m_has_hud_layer{},
  m_draw_turn_indicators(true),
  m_active_turn_indicator(true),
  m_marker_thickness(0.08),
  m_marker_below(false),
  m_cursor_thickness(0.04),
  m_show_cursor(false),
  m_edge_x(0),
  m_edge_y(0),
  m_sside(-1),
  m_border_width(0),
  m_cursor_col(-1),
  m_cursor_row(-1),
  m_piece_surface{},
  m_hud_has_content{},
  m_hud_need_redraw{},
  m_hatching_surface{},
  m_board_codes{},
  m_need_redraw_invalidated(0),
  m_number_of_floating_pieces(0),
  m_floating_piece{},
  m_floating_piece_handle(-1),
  m_redraw_background(true),
  m_resized(false),
#ifdef CWDEBUG
  m_inside_on_draw(false),
  m_show_buffer(false),
#endif
  m_total_size(0),
  m_redraw_pixmap(false),
  m_redraw_pieces(false)
{
  DoutEntering(dc::notice, "ChessboardWidget::ChessboardWidget()");

  // The default values for the dark square color (a1).
  gushort const dark_square_red = 45875;
  gushort const dark_square_green = 58981;
  gushort const dark_square_blue = 45875;

  // The default values for the light square color (h1).
  gushort const light_square_red = 65535;
  gushort const light_square_green = 65535;
  gushort const light_square_blue = 58981;

  // Make sure that these colors are different with the current color map.
  // If the they are not different then, shift the light squares to white
  // and the dark squares to black until they ARE different.

  GdkColor dark_square_color;
  GdkColor light_square_color;

  for (double x = 0.0; x <= 1.0; x += 0.01)
  {
    dark_square_color.red = (gushort)((1.0 - x) * dark_square_red);
    dark_square_color.green = (gushort)((1.0 - x) * dark_square_green);
    dark_square_color.blue = (gushort)((1.0 - x) * dark_square_blue);
#if 0 // No longer needed?
    if (!gdk_colormap_alloc_color(colormap, &dark_square_color, FALSE, TRUE))
      DoutFatal(dc::fatal, "gdk_colormap_alloc_color failed to allocate dark_square_color (" <<
          dark_square_color.red << ", " <<
          dark_square_color.green << ", " <<
          dark_square_color.blue << ")");
#endif
    light_square_color.red = light_square_red + (gushort)(x * (65535 - light_square_red));
    light_square_color.green = light_square_green + (gushort)(x * (65535 - light_square_green));
    light_square_color.blue = light_square_blue + (gushort)(x * (65535 - light_square_blue));
#if 0 // No longer needed?
    if (!gdk_colormap_alloc_color(colormap, &light_square_color, FALSE, TRUE))
      DoutFatal(dc::fatal, "gdk_colormap_alloc_color failed to allocate light_square_color (" <<
          light_square_color.red << ", " <<
          light_square_color.green << ", " <<
          light_square_color.blue << ")");
#endif

    if (dark_square_color.red != light_square_color.red ||
        dark_square_color.green != light_square_color.green ||
        dark_square_color.blue != light_square_color.blue)
      break;
  }

  // Store the actual colors.
  set_dark_square_color(dark_square_color);
  set_light_square_color(light_square_color);

  // Set the default widget's background color equal to the color of the light squares.
  m_widget_background_color = light_square_color;

  // Set the default border color.
  uint32_t border_color_html = 0x597359;
  GdkColor brown = { 0, static_cast<guint16>((border_color_html & 0xff0000) >> 8), static_cast<guint16>(border_color_html & 0xff00), static_cast<guint16>((border_color_html & 0xff) << 8) };
  set_border_color(brown);

  m_board_codes[0] = white_rook;
  m_board_codes[1] = white_knight;
  m_board_codes[2] = white_bishop;
  m_board_codes[3] = white_queen;
  m_board_codes[4] = white_king;
  m_board_codes[5] = white_bishop;
  m_board_codes[6] = white_knight;
  m_board_codes[7] = white_rook;
  m_board_codes[8] = white_pawn;
  m_board_codes[9] = white_pawn;
  m_board_codes[10] = white_pawn;
  m_board_codes[11] = white_pawn;
  m_board_codes[12] = white_pawn;
  m_board_codes[13] = white_pawn;
  m_board_codes[14] = white_pawn;
  m_board_codes[15] = white_pawn;
  m_board_codes[63] = black_rook;
  m_board_codes[62] = black_knight;
  m_board_codes[61] = black_bishop;
  m_board_codes[60] = black_king;
  m_board_codes[59] = black_queen;
  m_board_codes[58] = black_bishop;
  m_board_codes[57] = black_knight;
  m_board_codes[56] = black_rook;
  m_board_codes[55] = black_pawn;
  m_board_codes[54] = black_pawn;
  m_board_codes[53] = black_pawn;
  m_board_codes[52] = black_pawn;
  m_board_codes[51] = black_pawn;
  m_board_codes[50] = black_pawn;
  m_board_codes[49] = black_pawn;
  m_board_codes[48] = black_pawn;

  //*static_cast<Gtk::DrawingArea*>(this) = std::move(*Glib::wrap(&M_chessboard->parent));

  // Set the mask for signals that we want to receive.
  set_events(Gdk::EXPOSURE_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

  // Connect the events to our virtual functions.
#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  // This code was not tested (GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED is defined on my machine).
  signal_button_press_event().connect(sigc::mem_fun(*this, &ChessboardWidget::on_button_press_event));
  signal_button_release_event().connect(sigc::mem_fun(*this, &ChessboardWidget::on_button_release_event));
#endif
}

ChessboardWidget::~ChessboardWidget()
{
  DoutEntering(dc::notice, "ChessboardWidget::~ChessboardWidget()");
}

static Cairo::RefPtr<Cairo::Region> convert_mask2region(guint64 mask, gint x, gint y, gint sside, gboolean flip_board)
{
  Cairo::RefPtr<Cairo::Region> region = Cairo::Region::create();
  guint64 row_mask = (guint64)0xff << 56;
  for (gint row = 7; row >= 0; --row, row_mask >>= 8)
  {
    if ((mask & row_mask))
    {
      guint64 col_mask = (guint64)1 << 8 * row;
      gint col_start = 0;
      gint col_end;
      do
      {
        while (col_start != 8 && !(mask & col_mask))
        {
          ++col_start;
          col_mask <<= 1;
        }
        if (col_start == 8)
          break;        // No new start found;
        col_end = col_start;
        while (col_end != 8 && (mask & col_mask))
        {
          ++col_end;
          col_mask <<= 1;
        }
        Cairo::RectangleInt rect;
        rect.x = x + (flip_board ? 8 - col_end : col_start) * sside;
        rect.y = y + (flip_board ? row : 7 - row) * sside;
        rect.width = (col_end - col_start) * sside;
        rect.height = sside;
        region->do_union(rect);
        col_start = col_end;
      }
      while(col_end !=8);
    }
  }
  return region;
}

void ChessboardWidget::redraw_hud_layer(guint hud)
{
  DoutEntering(dc::widget, "Calling ChessboardWidget::redraw_hud_layer(" << hud << ")");

  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_hud_layer_surface[hud]);
  cr->set_operator(Cairo::OPERATOR_CLEAR);
  guint64 need_clear = m_hud_has_content[hud] & m_hud_need_redraw[hud];
  for (gint row = 0; row < 8; ++row)
    if ((need_clear & ((guint64)0xff << (8 * row))))
    {
      guint64 bit = (guint64)1 << (8 * row);
      for (gint col = 0; col < 8; ++col, bit <<= 1)
        if ((need_clear & bit))
        {
          cr->rectangle((m_flip_board ? 7 - col : col) * m_sside, (m_flip_board ? row : 7 - row) * m_sside, m_sside, m_sside);
//          invalidate_square(col, row);
        }
    }
  cr->fill();
  cr->set_operator(Cairo::OPERATOR_OVER);
  m_hud_has_content[hud] &= ~m_hud_need_redraw[hud];

  if (m_has_hud_layer[hud])
  {
    draw_hud_layer(cr, m_sside, hud);
  }
  Cairo::RefPtr<Cairo::Region> need_redraw_region = convert_mask2region(m_hud_need_redraw[hud], 0, 0, m_sside, m_flip_board);
  Gdk::Cairo::add_region_to_path(cr, need_redraw_region);
  cr->clip();

  for (Arrow* arrow : m_arrows)
  {
    if ((m_hud_need_redraw[hud] & arrow->has_content[hud]))
    {
      guint64 other_content = 0;
      for (guint h = 0; h < number_of_hud_layers; ++h)
        if (h != hud)
          other_content |= arrow->has_content[h];
      gboolean has_colliding_content = (m_hud_need_redraw[hud] & other_content) != 0;
      if (has_colliding_content)
      {
        cr->save();
        Cairo::RefPtr<Cairo::Region> clip_region = convert_mask2region(arrow->has_content[hud], 0, 0, m_sside, m_flip_board);
        Gdk::Cairo::add_region_to_path(cr, clip_region);
        cr->clip();
      }
      gdouble length = std::sqrt((arrow->end_col - arrow->begin_col) * (arrow->end_col - arrow->begin_col) +
          (arrow->end_row - arrow->begin_row) * (arrow->end_row - arrow->begin_row));
      gdouble begin_x = (0.5 + (m_flip_board ? 7 - arrow->begin_col : arrow->begin_col)) * m_sside;
      gdouble begin_y = (0.5 + (m_flip_board ? arrow->begin_row : 7 - arrow->begin_row)) * m_sside;
      // Unit vector in direction of arrow.
      gdouble vx = m_sside * (arrow->end_col - arrow->begin_col) / length;
      gdouble vy = m_sside * (arrow->begin_row - arrow->end_row) / length;
      if (m_flip_board)
      {
        vx = -vx;
        vy = -vy;
      }
      // Unit vector, rotated 90 degrees counter-clockwise.
      gdouble tx = -vy;
      gdouble ty = vx;
      // Draw arrow.
      cr->move_to(begin_x + 0.125 * tx, begin_y + 0.125 * ty);
      cr->rel_line_to((length - 0.25) * vx, (length - 0.25) * vy);
      cr->rel_line_to(0.125 * tx, 0.125 * ty);
      cr->line_to(begin_x + length * vx, begin_y + length * vy);
      cr->line_to(begin_x + (length - 0.25) * vx - 0.25 * tx, begin_y + (length - 0.25) * vy - 0.25 * ty);
      cr->rel_line_to(0.125 * tx, 0.125 * ty);
      cr->rel_line_to((0.25 - length) * vx, (0.25 - length) * vy);
      cr->close_path();
      cr->set_source_rgba(arrow->color.red, arrow->color.green, arrow->color.blue, 0.5);
      cr->fill();
      if (has_colliding_content)
        cr->restore();
      m_hud_has_content[hud] |= arrow->has_content[hud];
    }
  }

  m_hud_need_redraw[hud] = 0;
}

bool ChessboardWidget::on_draw(Cairo::RefPtr<Cairo::Context> const& crmm)
{
#if !CW_CHESSBOARD_EXPOSE_DEBUG
  DoutEntering(dc::widget, "ChessboardWidget::on_draw(cr)");
#else
  DoutEntering(dc::widget, "ChessboardWidget::on_draw(cr) [m_show_buffer = " << std::boolalpha << m_show_buffer << "]");
#endif
  Debug(m_inside_on_draw = true);

  Gtk::Allocation allocation = get_allocation();
  Dout(dc::clip, "allocation = " << allocation);

#if CW_CHESSBOARD_EXPOSE_DEBUG
  if (m_show_buffer)
  {
    crmm->save();
    crmm->set_source(m_buffer, 0.0, 0.0);
    crmm->paint();
    crmm->restore();
    m_show_buffer = false;
    return true;
  }
#endif

#if 0
  if (m_resized)
  {
    Dout(dc::notice, "Invalidating everything because m_resized = true.");
    // Everything was invalidated (probably due to a spurious call to on_size_allocate).
    m_need_redraw_invalidated = (guint64)-1;
    m_resized = false;
  }
#endif

  if (m_redraw_pixmap)
  {
    redraw_pixmap(crmm);
    m_redraw_pixmap = false;
  }

  Dout(dc::notice, "Clip rectangle list:");
  std::vector<Cairo::Rectangle> rectangles;
  crmm->copy_clip_rectangle_list(rectangles);
  WidgetSegments need_redraw;
  for (auto&& rect : rectangles)
  {
    Dout(dc::notice, "  rect = " << rect);
    need_redraw |= rect_to_segments(rect);
  }
  Dout(dc::notice, "need_redraw = " << need_redraw);

#if CW_CHESSBOARD_EXPOSE_DEBUG
  auto cr = Cairo::Context::create(m_buffer);
#else
  auto& cr = crmm;
#endif

  // Last minute update of the HUD layers.
  for (guint hud = 0; hud < number_of_hud_layers; ++hud)
    if (m_hud_need_redraw[hud])
      redraw_hud_layer(hud);

  // Last minute update of pixmap.
  guint64 redraw_mask = 1;
  for (gint i = 0; i < 64; ++i, redraw_mask <<= 1)
    if ((need_redraw & redraw_mask))
      redraw_square(cr, i);             // This uses the HUD layer.
  m_need_redraw_invalidated = 0;

  // Redraw border when the border was invalidated.
  if (need_redraw.any_border())
    draw_border(cr, need_redraw);

// Draw a green line around updated areas.
#if CW_CHESSBOARD_EXPOSE_DEBUG
  for (auto&& rect : rectangles)
  {
    cr->save();
    cr->set_line_width(1.0);
    cr->rectangle(rect.x, rect.y, rect.width, rect.height);
    cr->set_source_rgb(0.0, 1.0, 0.0);
    cr->stroke();
    cr->restore();
  }
#endif

  // Draw any floating pieces.
  if (m_number_of_floating_pieces)
  {
    cr->save();
//    cr->rectangle(top_left_board_x(), top_left_board_y(), squares * m_sside, squares * m_sside);
//    cr->clip();
    for (gsize i = 0; i < m_number_of_floating_pieces; ++i)
      if (m_floating_piece[i].moved)
      {
        cr->set_source(
            m_piece_surface[convert_code2piece_index(m_floating_piece[i].code)].surface,
            m_floating_piece[i].x, m_floating_piece[i].y);
        cr->paint();
      }
    cr->restore();
  }

  // Either top_left_edge_x or top_left_edge_y equals 0 (see redraw_pixmap).
  // The type of configuration depends on 'vertical':
  //
  // vertical == true      vertical == false.
  // .-----------,         .--+----------+--,
  // |           |         |  |          |  |
  // +-----------+         |  |          |  |
  // |           |         |  | pixmap   |  |
  // | pixmap    |         |  |          |  |
  // |           |         |  |          |  |
  // |           |         |  |          |  |
  // +-----------+         `--+----------+--'
  // |           |
  // `-----------'
  // The less than zero happens when the window is so small that sside would have to be less than min_sside.
  gboolean vertical = (top_left_edge_x() == 0);

  // This default is false, which is the case when the board is only updated.
  gboolean region_extends_outside_pixmap = FALSE;
  // However, if we resized -- or another window moved over the widget -- it could be true.
  Gdk::Rectangle clipbox;
  Gdk::Cairo::get_clip_rectangle(cr, clipbox);
  Dout(dc::clip, "get_clip_rectangle returned " << clipbox);
  if (G_UNLIKELY(clipbox.get_y() < top_left_edge_y()))
    region_extends_outside_pixmap = vertical;
  if (G_UNLIKELY(clipbox.get_y() + clipbox.get_height() > bottom_right_board_y()))
    region_extends_outside_pixmap = vertical;
  if (G_UNLIKELY(clipbox.get_x() < top_left_edge_x()))
    region_extends_outside_pixmap = !vertical;
  if (G_UNLIKELY(clipbox.get_x() + clipbox.get_width() > bottom_right_board_x()))
    region_extends_outside_pixmap = !vertical;
  if (G_UNLIKELY(region_extends_outside_pixmap))
  {
    cairo_rectangle_int_t pixmap_rect;
    pixmap_rect.x = top_left_edge_x();
    pixmap_rect.y = top_left_edge_y();
    pixmap_rect.width = bottom_right_board_x() - top_left_edge_x();
    pixmap_rect.height = bottom_right_board_y() - top_left_edge_y();
    cairo_region_t* pixmap_region = cairo_region_create_rectangle(&pixmap_rect);
    cairo_rectangle_int_t region_rect;
    region_rect.x = clipbox.get_x();
    region_rect.y = clipbox.get_y();
    region_rect.width = clipbox.get_width();
    region_rect.height = clipbox.get_height();
    cairo_region_t* clip_region = cairo_region_create_rectangle(&region_rect);
    cairo_region_intersect(clip_region, pixmap_region);
    cairo_region_destroy(clip_region);
    cairo_region_destroy(pixmap_region);
  }

  Debug(m_inside_on_draw = false);

#if CW_CHESSBOARD_EXPOSE_DEBUG
  m_show_buffer = true;
  queue_draw();
#endif

  return true;
}

bool ChessboardWidget::on_motion_notify_event(GdkEventMotion* motion_event)
{
  DoutEntering(dc::motion_event, "ChessboardWidget::on_motion_notify_event(" << motion_event << ")");

  if (m_floating_piece_handle != -1)
  {
    double hsside = 0.5 * m_sside;
    double fraction = hsside - (gint)hsside;
    move_floating_piece(m_floating_piece_handle, motion_event->x - fraction, motion_event->y - fraction);
    return true;
  }

  update_cursor_position(motion_event->x, motion_event->y, false);

  return false;
}

void ChessboardWidget::on_realize()
{
  DoutEntering(dc::widget, "ChessboardWidget::on_realize()");
  Gtk::Widget::on_realize();
  m_redraw_pixmap = true;
  //FIXME set background colors:
  //gtk_style_set_background(widget->style, gtk_widget_get_window(widget), GTK_STATE_NORMAL);
  //gdk_window_set_background(gtk_widget_get_window(widget), &chessboard->priv->widget_background_color);
}

void ChessboardWidget::on_unrealize()
{
  DoutEntering(dc::widget, "ChessboardWidget::on_unrealize()");
  for (int i = 0; i < 12; ++i)
    m_piece_surface[i].surface.clear();
  m_hatching_surface.surface.clear();
  m_chessboard_region.clear();
  for (guint hud = 0; hud < number_of_hud_layers; ++hud)
    m_hud_layer_surface[hud].clear();
  m_sside = -1;
  Gtk::Widget::on_unrealize();
}

void ChessboardWidget::on_size_allocate(Gtk::Allocation& allocation)
{
  DoutEntering(dc::widget, "ChessboardWidget::on_size_allocate(" << allocation << ")");
  // We were resized.
  int const total_size = std::min(allocation.get_width(), allocation.get_height());
#if 0
  // Calculate the size and place of the chessboard widget, including border
  // and use that as the adjusted allocation.
  // Calculate the adjusted allocation such that it becomes a centered square with side `total_size`.
  allocation.set_x(allocation.get_x() + (allocation.get_width() - total_size) / 2);
  allocation.set_y(allocation.get_y() + (allocation.get_height() - total_size) / 2);
  allocation.set_width(total_size);
  allocation.set_height(total_size);
//  Gtk::Allocation old_allocation = get_allocation();
#endif
  m_redraw_background = true;
  m_resized = true;
#if 0
  // on_size_allocate turns out to be called too often, also when nothing changed.
  if (allocation == old_allocation)
    return;
#endif
  m_total_size = total_size;
  set_allocation(allocation);
  // Redraw everything.
  if (get_realized())
  {
    //FIXME: is this needed now we don't change the allocation?
    get_window()->move_resize(allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height());
    m_redraw_pixmap = true;
  }
}

void ChessboardWidget::get_preferred_width_vfunc(int& minimum_width, int& natural_width) const
{
  DoutEntering(dc::widget|continued_cf, "ChessboardWidget::get_preferred_width_vfunc({");

  // Return the minimum size we really need.
  gint min_border_width = 0;
  if (m_draw_border)
    min_border_width = 2 * default_calc_board_border_width(min_sside);
  minimum_width = natural_width = squares * min_sside + min_border_width;

  Dout(dc::finish, minimum_width << "}, {" << natural_width << "})");
}

void ChessboardWidget::get_preferred_height_vfunc(int& minimum_height, int& natural_height) const
{
  DoutEntering(dc::widget|continued_cf, "ChessboardWidget::get_preferred_height_vfunc({");

  // Return the minimum size we really need.
  gint min_border_width = 0;
  if (m_draw_border)
    min_border_width = 2 * default_calc_board_border_width(min_sside);
  minimum_height = natural_height = squares * min_sside + min_border_width;

  Dout(dc::finish, minimum_height << "}, {" << natural_height << "})");
}

void ChessboardWidget::set_square(gint col, gint row, CwChessboardCode code)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_square(" << col << ", " << row << ", " << (int)code << ")");
  gint index = convert_colrow2index(col, row);
  CwChessboardCode old_code = m_board_codes[index];
  if (old_code != code)
  {
    m_board_codes[index] = (old_code & ~s_piece_color_mask) | (code & s_piece_color_mask);
    invalidate_square(col, row);
  }
}

CwChessboardCode ChessboardWidget::get_square(gint col, gint row) const
{
  DoutEntering(dc::widget, "ChessboardWidget::get_square(" << col << ", " << row << ")");
  gint index = convert_colrow2index(col, row);
  return m_board_codes[index] & s_piece_color_mask;
}

void ChessboardWidget::set_draw_border(gboolean draw)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_draw_border(" << std::boolalpha << draw << ")");
  if (m_draw_border != draw)
  {
    m_draw_border = draw;
    m_redraw_pixmap = true;
    invalidate_border();
  }
}

void ChessboardWidget::set_draw_turn_indicators(gboolean draw)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_draw_turn_indicators(" << std::boolalpha << draw << ")");
  if (m_draw_turn_indicators != draw)
  {
    m_draw_turn_indicators = draw;
    if (get_realized() && m_border_width)
      invalidate_turn_indicators();
  }
}

void ChessboardWidget::set_active_turn_indicator(gboolean white)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_active_turn_indicator(" << std::boolalpha << white << ")");
  if (m_active_turn_indicator != white)
  {
    m_active_turn_indicator = white;
    if (get_realized() && m_border_width && m_draw_turn_indicators)
      invalidate_turn_indicators();
  }
}

void ChessboardWidget::set_flip_board(gboolean flip)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_flip_board(" << std::boolalpha << flip << ")");
  if (m_flip_board != flip)
  {
    m_flip_board = flip;
    if (get_realized())
    {
      invalidate_border();
      for (guint hud = 0; hud < number_of_hud_layers; ++hud)
      {
        m_hud_need_redraw[hud] = (guint64)-1;
        m_hud_has_content[hud] = (guint64)-1;
      }
      invalidate_board();
    }
  }
}

CwChessboardColorHandle ChessboardWidget::allocate_color_handle_rgb(gdouble red, gdouble green, gdouble blue)
{
  DoutEntering(dc::widget, "ChessboardWidget::allocate_color_handle_rgb(" << red << ", " << green << ", " << blue << ")");
  guint32 bit = 1;
  guint color_index = 0;
  while ((m_allocated_colors_mask & bit))
  {
    bit <<= 1;
    ++color_index;
  }
  g_assert(color_index < G_N_ELEMENTS(m_color_palet));
  m_allocated_colors_mask |= bit;
  m_color_palet[color_index].red = red;
  m_color_palet[color_index].green = green;
  m_color_palet[color_index].blue = blue;
  return color_index + 1;
}

void ChessboardWidget::free_color_handle(CwChessboardColorHandle handle)
{
  g_assert(handle > 0);
  guint color_index = handle - 1;
  g_assert(color_index < G_N_ELEMENTS(m_color_palet));
  guint32 bit = 1 << color_index;
  g_assert((m_allocated_colors_mask & bit) != 0);
  m_allocated_colors_mask &= ~bit;
}

void ChessboardWidget::set_marker_color(gint col, gint row, CwChessboardColorHandle mahandle)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_marker_color(" << col << ", " << row << ", " << (int)mahandle << ")");
  BoardIndex index = convert_colrow2index(col, row);
  CwChessboardCode old_code = m_board_codes[index];
  m_board_codes[index] = convert_mahandle2code(mahandle) | (old_code & ~s_mahandle_mask);
  invalidate_square(col, row);
}

void ChessboardWidget::set_marker_thickness(gdouble thickness)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_marker_thickness(" << std::boolalpha << thickness << ")");
  m_marker_thickness = std::min(std::max(0.0, thickness), 0.5);
  m_marker_thickness_px = std::max(1, std::min((gint)std::round(m_marker_thickness * m_sside), m_sside / 2));
  invalidate_markers();
}

void ChessboardWidget::set_background_color(gint col, gint row, CwChessboardColorHandle bghandle)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_background_color(" << col << ", " << row << ", " << (int)bghandle << ")");
  BoardIndex index = convert_colrow2index(col, row);
  CwChessboardCode old_code = m_board_codes[index];
  m_board_codes[index] = convert_bghandle2code(bghandle) | (old_code & ~s_bghandle_mask);
  invalidate_square(col, row);
}

void ChessboardWidget::set_dark_square_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_dark_square_color(" << color << ")");
  m_dark_square_color.red = color.red / 65535.0;
  m_dark_square_color.green = color.green / 65535.0;
  m_dark_square_color.blue = color.blue / 65535.0;
  invalidate_board();
}

void ChessboardWidget::set_background_colors(ColorHandle const* handles)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_background_colors(" << handles << ")");
  // Read through the whole array, only calling set_background_color when the value changed.
  CwChessboardCode* board_codes = m_board_codes;
  for (int i = 0; i < 64; ++i)
    if (convert_code2bghandle(board_codes[i]) != handles[i])
      set_background_color(convert_index2column(i), convert_index2row(i), handles[i]);
}

void ChessboardWidget::get_dark_square_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_dark_square_color.red);
  color.green = (guint16)round(65535.0 * m_dark_square_color.green);
  color.blue = (guint16)round(65535.0 * m_dark_square_color.blue);
}

void ChessboardWidget::set_light_square_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_light_square_color(" << color << ")");
  m_light_square_color.red = color.red / 65535.0;
  m_light_square_color.green = color.green / 65535.0;
  m_light_square_color.blue = color.blue / 65535.0;
  invalidate_board();
}

void ChessboardWidget::get_light_square_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_light_square_color.red);
  color.green = (guint16)round(65535.0 * m_light_square_color.green);
  color.blue = (guint16)round(65535.0 * m_light_square_color.blue);
}

void ChessboardWidget::set_border_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_border_color(" << color << ")");
  m_board_border_color.red = color.red / 65535.0;
  m_board_border_color.green = color.green / 65535.0;
  m_board_border_color.blue = color.blue / 65535.0;
  invalidate_border();
}

void ChessboardWidget::get_border_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_board_border_color.red);
  color.green = (guint16)round(65535.0 * m_board_border_color.green);
  color.blue = (guint16)round(65535.0 * m_board_border_color.blue);
}

void ChessboardWidget::set_white_fill_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_white_fill_color(" << color << ")");
  m_white_piece_fill_color.red = color.red / 65535.0;
  m_white_piece_fill_color.green = color.green / 65535.0;
  m_white_piece_fill_color.blue = color.blue / 65535.0;
  m_redraw_pieces = true;
}

void ChessboardWidget::get_white_fill_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_white_piece_fill_color.red);
  color.green = (guint16)round(65535.0 * m_white_piece_fill_color.green);
  color.blue = (guint16)round(65535.0 * m_white_piece_fill_color.blue);
}

void ChessboardWidget::set_white_line_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_white_line_color(" << color << ")");
  m_white_piece_line_color.red = color.red / 65535.0;
  m_white_piece_line_color.green = color.green / 65535.0;
  m_white_piece_line_color.blue = color.blue / 65535.0;
  m_redraw_pieces = true;
}

void ChessboardWidget::get_white_line_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_white_piece_line_color.red);
  color.green = (guint16)round(65535.0 * m_white_piece_line_color.green);
  color.blue = (guint16)round(65535.0 * m_white_piece_line_color.blue);
}

void ChessboardWidget::set_black_fill_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_black_fill_color(" << color << ")");
  m_black_piece_fill_color.red = color.red / 65535.0;
  m_black_piece_fill_color.green = color.green / 65535.0;
  m_black_piece_fill_color.blue = color.blue / 65535.0;
  m_redraw_pieces = true;
}

void ChessboardWidget::get_black_fill_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_black_piece_fill_color.red);
  color.green = (guint16)round(65535.0 * m_black_piece_fill_color.green);
  color.blue = (guint16)round(65535.0 * m_black_piece_fill_color.blue);
}

void ChessboardWidget::set_black_line_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_black_line_color(" << color << ")");
  m_black_piece_line_color.red = color.red / 65535.0;
  m_black_piece_line_color.green = color.green / 65535.0;
  m_black_piece_line_color.blue = color.blue / 65535.0;
  m_redraw_pieces = true;
}

void ChessboardWidget::get_black_line_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_black_piece_line_color.red);
  color.green = (guint16)round(65535.0 * m_black_piece_line_color.green);
  color.blue = (guint16)round(65535.0 * m_black_piece_line_color.blue);
}

void ChessboardWidget::set_cursor_color(GdkColor const& color)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_cursor_color(" << color << ")");
  m_cursor_color.red = color.red / 65535.0;
  m_cursor_color.green = color.green / 65535.0;
  m_cursor_color.blue = color.blue / 65535.0;
  invalidate_cursor();
}

void ChessboardWidget::get_cursor_color(GdkColor& color) const
{
  color.red = (guint16)round(65535.0 * m_cursor_color.red);
  color.green = (guint16)round(65535.0 * m_cursor_color.green);
  color.blue = (guint16)round(65535.0 * m_cursor_color.blue);
}

void ChessboardWidget::set_cursor_thickness(gdouble thickness)
{
  DoutEntering(dc::widget, "ChessboardWidget::set_cursor_thickness" << thickness << ")");
  m_cursor_thickness = std::min(std::max(0.0, thickness), 0.5);
  m_cursor_thickness_px = std::max(1, std::min((gint)round(m_cursor_thickness * m_sside), m_sside / 2));
  invalidate_cursor();
}

void ChessboardWidget::show_cursor()
{
  if (m_show_cursor)
    return;
  m_show_cursor = true;
  Dout(dc::widget, "Calling gdk_window_get_pointer()");
  auto display = Gdk::Display::get_default();
  auto seat = display->get_default_seat();
  auto pointer = seat->get_pointer();
  gint x, y;
  [[maybe_unused]] Gdk::ModifierType mt;
  get_window()->get_device_position(pointer, x, y, mt);
  update_cursor_position(x, y, true);
}

void ChessboardWidget::hide_cursor()
{
  invalidate_cursor();
  m_show_cursor = FALSE;
}

void ChessboardWidget::move_floating_piece(gint handle, gdouble x, gdouble y)
{
  // Produces too much debug output.
  //DoutEntering(dc::widget, "ChessboardWidget::move_floating_piece(" << (int)handle << ", " << x << ", " << y << ")");

  g_assert(handle >= 0 && (gsize)handle < G_N_ELEMENTS(m_floating_piece));
  g_assert(!is_empty_square(m_floating_piece[handle].code));

  // If x (y) is non-integer, this should really be m_sside + 1, however
  // it is safe to use just sside at all times because pieces never extend
  // all the way to the border of a square: there is nothing drawn there,
  // so there is no reason to invalidate it.
  Gdk::Rectangle rect(m_floating_piece[handle].x, m_floating_piece[handle].y, m_sside, m_sside);
  queue_draw_area(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());

  Gtk::Allocation allocation = get_allocation();
  gboolean outside_window =
      rect.get_x() + rect.get_width() < 0 ||
      rect.get_x() > allocation.get_x() ||
      rect.get_y() + rect.get_height() < 0 ||
      rect.get_y() > allocation.get_y();
  // Redraw background of widget if the old place of the floating piece is outside the board.
  m_redraw_background = m_redraw_background ||
      rect.get_x() < top_left_edge_x() ||
      rect.get_x() + rect.get_width() > bottom_right_edge_x() ||
      rect.get_y() < top_left_edge_y() ||
      rect.get_y() + rect.get_height() > bottom_right_edge_y();
  rect.set_x((gint)trunc(x - 0.5 * m_sside));
  rect.set_y((gint)trunc(y - 0.5 * m_sside));

//  queue_draw_area(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());
  outside_window = outside_window &&
      (rect.get_x() + rect.get_width() < 0 ||
      rect.get_x() > allocation.get_x() ||
      rect.get_y() + rect.get_height() < 0 ||
      rect.get_y() > allocation.get_y());

  if (outside_window && m_floating_piece[handle].pointer_device)
  {
    Dout(dc::motion_event, "FIXME: NOT Calling gdk_window_get_pointer()");
#if 0 // Do still need this?
    gdk_window_get_pointer(gtk_widget_get_window(widget), NULL, NULL, NULL);
#endif
  }
  m_floating_piece[handle].x = rect.get_x();
  m_floating_piece[handle].y = rect.get_y();
  m_floating_piece[handle].moved = true;
}

gint ChessboardWidget::add_floating_piece(CwChessboardCode code, gdouble x, gdouble y, gboolean pointer_device)
{
  DoutEntering(dc::widget, "ChessboardWidget::add_floating_piece(code:" << code <<
      ", x:" << x << ", y:" << y << ", " << pointer_device << ")");

  // There can't be more than 32 floating pieces.
  g_assert(m_number_of_floating_pieces < G_N_ELEMENTS(m_floating_piece));

  m_number_of_floating_pieces++;
  gint handle = 0;
  while (m_floating_piece[handle].code != empty_square)
    ++handle;
  // See remark in ChessboardWidget::move_floating_piece.
  Gdk::Rectangle rect((gint)std::trunc(x - 0.5 * m_sside), (gint)std::trunc(y - 0.5 * m_sside), m_sside, m_sside);
  m_floating_piece[handle].code = code & s_piece_color_mask;
  m_floating_piece[handle].x = rect.get_x();
  m_floating_piece[handle].y = rect.get_y();
  m_floating_piece[handle].moved = TRUE;
  if (m_floating_piece_handle != -1)
    pointer_device = false;     // Refuse to add two pointer devices at the same time.
  m_floating_piece[handle].pointer_device = pointer_device;
  if (pointer_device)
    m_floating_piece_handle = handle;
  Dout(dc::notice, "ChessboardWidget::add_floating_piece(" << code << ", " << x << ", " << y << ", " << pointer_device << "): calling queue_draw_area(" << rect << ")");
  queue_draw_area(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());

  Dout(dc::widget, "number_of_floating_pieces = " << m_number_of_floating_pieces);
  Dout(dc::widget, "Allocated handle " << handle);

  return handle;
}

void ChessboardWidget::remove_floating_piece(gint handle)
{
  DoutEntering(dc::widget, "ChessboardWidget::remove_floating_piece(handle:" << handle << ")");

  g_assert(handle >= 0 && (gsize)handle < G_N_ELEMENTS(m_floating_piece));
  g_assert(!is_empty_square(m_floating_piece[handle].code));

  // See remark in ChessboardWidget::move_floating_piece.
  Gdk::Rectangle rect(m_floating_piece[handle].x, m_floating_piece[handle].y, m_sside, m_sside);

  Dout(dc::notice, "ChessboardWidget::remove_floating_piece(" << handle << "): calling queue_draw_area(" << rect << ")");
  queue_draw_area(rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height());

  // Redraw background of widget if the old place of the floating piece is outside the board.
  m_redraw_background = m_redraw_background ||
      rect.get_x() < top_left_edge_x() ||
      rect.get_x() + rect.get_width() > bottom_right_edge_x() ||
      rect.get_y() < top_left_edge_y() ||
      rect.get_y() + rect.get_height() > bottom_right_edge_y();
  if (m_floating_piece[handle].pointer_device)
    m_floating_piece_handle = -1;
  m_number_of_floating_pieces--;
  m_floating_piece[handle].code = empty_square;
  Dout(dc::widget, "number_of_floating_pieces = " << m_number_of_floating_pieces);
}

void ChessboardWidget::enable_hud_layer(guint hud)
{
  DoutEntering(dc::widget, "ChessboardWidget::enable_hud_layer(" << hud << ")");
  g_return_if_fail(hud < number_of_hud_layers);
  m_has_hud_layer[hud] = TRUE;
  m_hud_need_redraw[hud] = (guint64)-1;
}

gpointer ChessboardWidget::add_arrow(gint begin_col, gint begin_row, gint end_col, gint end_row, GdkColor const& color)
{
  DoutEntering(dc::widget|continued_cf, "ChessboardWidget::add_arrow(" <<
      begin_col << ", " << begin_row << ", " << end_col << ", " << end_row << ", " << color << ") = ");

  g_return_val_if_fail(begin_col != end_col || begin_row != end_row, NULL);
  g_return_val_if_fail(is_inside_board(begin_col, begin_row) && is_inside_board(end_col, end_row), NULL);

  Arrow* arrow = (Arrow*)g_malloc(sizeof(Arrow));
  m_arrows.push_back(arrow);
  arrow->begin_col = begin_col;
  arrow->begin_row = begin_row;
  arrow->end_col = end_col;
  arrow->end_row = end_row;
  arrow->color.red = color.red / 65535.0;
  arrow->color.green = color.green / 65535.0;
  arrow->color.blue = color.blue / 65535.0;
  guint64 content = invalidate_arrow(begin_col, begin_row, end_col, end_row);
  guint64 start_square = (guint64)1 << convert_colrow2index(begin_col, begin_row);
  m_hud_need_redraw[0] |= (arrow->has_content[0] = start_square);
  m_hud_need_redraw[1] |= (arrow->has_content[1] = content ^ start_square);
  Dout(dc::finish, arrow);
  return arrow;
}

void ChessboardWidget::remove_arrow(gpointer ptr)
{
  DoutEntering(dc::widget, "ChessboardWidget::remove_arrow(" << ptr << ")");
  Arrow* arrow = (Arrow*)ptr;
  // Only call remove_arrow for an arrow that was added before with add_arrow.
  ASSERT(std::find(m_arrows.begin(), m_arrows.end(), arrow) != m_arrows.end());
  m_arrows.erase(std::remove(m_arrows.begin(), m_arrows.end(), arrow), m_arrows.end());
  invalidate_arrow(arrow->begin_col, arrow->begin_row, arrow->end_col, arrow->end_row);
  m_hud_need_redraw[0] |= arrow->has_content[0];
  m_hud_need_redraw[1] |= arrow->has_content[1];
  g_free(ptr);
}

void ChessboardWidget::draw_hud_layer(Cairo::RefPtr<Cairo::Context> const& crmm, gint sside, guint hud)
{
  DoutEntering(dc::widget, "ChessboardWidget::draw_hud_layer(cr, " << sside << ", " << hud << ")");
  // Only call this function from on_draw().
  ASSERT(m_inside_on_draw);

  g_return_if_fail(hud < number_of_hud_layers);

  guint64 bit = 1;
  for (gint row = 0; row < 8; ++row)
    for (gint col = 0; col < 8; ++col, bit <<= 1)
    {
      if ((m_hud_need_redraw[hud] & bit))
      {
        crmm->save();
        // FIXME: also rotate the square 180 degrees when flipped.
        crmm->translate((m_flip_board ? 7 - col : col) * sside, (m_flip_board ? row : 7 - row) * sside);
        crmm->rectangle(0, 0, sside, sside);
        crmm->clip();
        if (draw_hud_square(crmm, col, row, sside, hud))
        {
          m_hud_has_content[hud] |= bit;
          invalidate_square(col, row);
        }
        crmm->restore();
      }
    }
}

gboolean ChessboardWidget::draw_hud_square(Cairo::RefPtr<Cairo::Context> const& crmm, gint col, gint row, gint sside, guint hud)
{
  DoutEntering(dc::widget, "ChessboardWidget::draw_hud_square(cr" << ", " << col << ", " << row << ", " << sside << ", " << hud << ")");
  // Only call this function from on_draw().
  ASSERT(m_inside_on_draw);

  g_return_val_if_fail(hud < number_of_hud_layers, FALSE);
  g_return_val_if_fail(is_inside_board(col, row), FALSE);

  int const number_of_lines = 21;
  double const line_width = 0.25;

  // No HUD layer for the white squares.
  if (hud == 1 || ((col + row) & 1) == 1)
    return false;

  if (!m_hatching_surface.surface)
  {
    m_hatching_surface.surface = Cairo::Surface::create(crmm->get_target(), Cairo::CONTENT_COLOR_ALPHA, sside, sside);
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(m_hatching_surface.surface);
    cr->set_line_width(line_width);
    cr->set_line_cap(Cairo::LINE_CAP_ROUND);
    cr->set_source_rgb(0, 0, 0);
    cr->scale((double)sside / number_of_lines, (double)sside / number_of_lines);
    for (int h = 0; h < number_of_lines; ++h)
    {
      double s = h + line_width; 
      cr->move_to(s - 0.5 * line_width, 0.5 * line_width);
      cr->line_to(0.5 * line_width, s - 0.5 * line_width);
      cr->move_to(s + 0.5 * line_width, number_of_lines - 0.5 * line_width);
      cr->line_to(number_of_lines - 0.5 * line_width, s + 0.5 * line_width);
    }
    cr->stroke();
  }

  crmm->set_source(m_hatching_surface.surface, 0.0, 0.0);
  crmm->paint();

  return true;
}

void ChessboardWidget::draw_border(Cairo::RefPtr<Cairo::Context> const& cr, WidgetSegments const& need_redraw)
{
  DoutEntering(dc::widget, "ChessboardWidget::draw_border(cr)");
  // Only call this function from on_draw().
  ASSERT(m_inside_on_draw);

  if (need_redraw.any_background())
  {
    // Fill the area around the board with the background color.
    Gtk::Allocation allocation = get_allocation();
    gint const side = squares * m_sside;

    if (need_redraw.left_background())
      cr->rectangle(0, 0, top_left_edge_x(), allocation.get_height());
    if (need_redraw.right_background())
      cr->rectangle(bottom_right_edge_x(), 0, allocation.get_width() - bottom_right_edge_x(), allocation.get_height());
    if (need_redraw.top_background())
      cr->rectangle(0, 0, allocation.get_width(), top_left_edge_y());
    if (need_redraw.bottom_background())
      cr->rectangle(0, bottom_right_edge_y(), allocation.get_width(), allocation.get_height() - bottom_right_edge_y());
    cr->set_source_rgb(m_widget_background_color.red / 65535.0,
        m_widget_background_color.green / 65535.0, m_widget_background_color.blue / 65535.0);
    cr->fill();
  }

  gint const border_shadow_width = 2;
  gint const border_width = m_border_width;
  gint const border_edge_width = border_width - border_shadow_width - 1;
  gint const side = squares * m_sside;

  cr->save();
  // We draw relative to the top-left of the border.
  cr->translate(top_left_board_x() - border_width, top_left_board_y() - border_width);

  gint const right_edge_x = border_width + side + border_width;

  // Draw an edge around that that will contain the chessboard coordinates.
  static int const border_mask_bits[] = { 0x40, 0x80, 0x100, 0x2000, 0x40000, 0x20000, 0x10000, 0x800, 0x40 };
  bool already_drawn = need_redraw.mask(0x800);
  for (int i = 0; i < 8; i += 2)
  {
    double const light_color = (i == 0 || i == 6) ? 1.0 : 0.0;
    bool const need_left_corner = need_redraw.mask(border_mask_bits[i]);
    bool const need_right_corner = need_redraw.mask(border_mask_bits[i + 2]);
    cr->set_source_rgb((light_color + m_board_border_color.red) * 0.5, (light_color + m_board_border_color.green) * 0.5, (light_color + m_board_border_color.blue) * 0.5);
    if (need_redraw.mask(border_mask_bits[i + 1]))
    {
      // Draw the shadow around the border edge.
      cr->move_to(need_left_corner ? 0 : border_width, 0);
      cr->line_to(right_edge_x - (need_right_corner ? 0 : border_width), 0);
      cr->line_to(right_edge_x - (need_right_corner ? border_shadow_width : border_width), border_shadow_width);
      cr->line_to((need_left_corner ? border_shadow_width : border_width), border_shadow_width);
      cr->close_path();
      cr->fill();
      // Draw the border edge.
      int x_offset1 = (!already_drawn && need_left_corner) ? 0 : border_edge_width + 1;
      int x_offset2 = need_right_corner ? 0 : border_edge_width + 1;
      cr->set_source_rgb(m_board_border_color.red, m_board_border_color.green, m_board_border_color.blue);
      cr->rectangle(border_shadow_width + x_offset1, border_shadow_width, side + 2 * (border_edge_width + 1) - x_offset1 - x_offset2, border_edge_width + 1);
      cr->fill();
    }
    else
    {
      if (need_left_corner)
      {
        // Draw the shadow triangle.
        cr->move_to(0, 0);
        cr->line_to(border_width, 0);
        cr->line_to(border_width, border_shadow_width);
        cr->line_to(border_shadow_width, border_shadow_width);
        cr->close_path();
        cr->fill();
        if (!already_drawn)
        {
          // Draw the border edge corner.
          cr->set_source_rgb(m_board_border_color.red, m_board_border_color.green, m_board_border_color.blue);
          cr->rectangle(border_shadow_width, border_shadow_width, border_edge_width + 1, border_edge_width + 1);
          cr->fill();
        }
      }
      if (need_right_corner)
      {
        // Draw the shadow triangle.
        cr->move_to(right_edge_x, 0);
        cr->line_to(right_edge_x - border_width, 0);
        cr->line_to(right_edge_x - border_width, border_shadow_width);
        cr->line_to(right_edge_x - border_shadow_width, border_shadow_width);
        cr->close_path();
        cr->fill();
        // Draw the border edge corner.
        cr->set_source_rgb(m_board_border_color.red, m_board_border_color.green, m_board_border_color.blue);
        cr->rectangle(border_shadow_width + border_edge_width + 1 + side, border_shadow_width, border_edge_width + 1, border_edge_width + 1);
        cr->fill();
      }
    }
    already_drawn = need_right_corner;
    int halfwidth = m_border_width + 4 * m_sside;
    cr->translate(halfwidth, halfwidth);
    cr->rotate(0.5 * M_PI);
    cr->translate(-halfwidth, -halfwidth);
  }

  // Draw a thin, dark line at the top and left side of the board.
  cr->set_line_width(1.0);
  bool const need_redraw_top_border = need_redraw.mask(0x1c0);
  bool const need_redraw_right_border = need_redraw.mask(0x10840);
  if (need_redraw_top_border || need_redraw_right_border)
  {
    cr->set_source_rgb(m_board_border_color.red * 0.5, m_board_border_color.green * 0.5, m_board_border_color.blue * 0.5);

    if (need_redraw_top_border)
      cr->move_to(side + border_width + 0.5, border_width - 0.5);
    else
      cr->move_to(border_width - 0.5, side + border_width + 0.5);
    cr->line_to(border_width - 0.5, border_width - 0.5);
    if (need_redraw_top_border && need_redraw_right_border)
      cr->line_to(border_width - 0.5, side + border_width + 0.5);

    cr->stroke();
  }

  // Draw a thin, light line at the bottom and right side of the board.
  bool const need_redraw_bottom_border = need_redraw.mask(0x70000);
  bool const need_redraw_left_border = need_redraw.mask(0x42100);
  if (need_redraw_bottom_border || need_redraw_left_border)
  {
    cr->set_source_rgb((1.0 + m_board_border_color.red) * 0.5, (1.0 + m_board_border_color.green) * 0.5, (1.0 + m_board_border_color.blue) * 0.5);

    if (need_redraw_bottom_border)
      cr->move_to(border_width - 0.5, side + border_width + 0.5);
    else
      cr->move_to(side + border_width + 0.5, border_width - 0.5);
    cr->line_to(side + border_width + 0.5, side + border_width + 0.5);
    if (need_redraw_bottom_border && need_redraw_left_border)
      cr->line_to(side + border_width + 0.5, border_width - 0.5);

    cr->stroke();
  }

  // Draw the coordinates.
  cr->set_source_rgb(1.0, 1.0, 1.0);
  if (need_redraw.mask(0x20800))
  {
    auto layout = Pango::Layout::create(cr);
    {
      Pango::FontDescription desc("Sans Bold 14");
      int font_pixels = (border_edge_width > 14) ? std::max(14.0, border_edge_width * 0.66) : border_edge_width;
      desc.set_absolute_size(std::max(font_pixels, 7) * PANGO_SCALE);
      layout->set_font_description(desc);
    }
    char c[2] = { 0, 0 };
    for (int col = 0; col < 8; ++col)
    {
      if (!need_redraw.symbol(2, col))
        continue;
      c[0] = 'A' + col;
      layout->set_text(c);
      int width, height;
      layout->get_size(width, height);
      cr->move_to(border_width + ((m_flip_board ? 7 - col : col) + 0.5) * m_sside - ((double)width / PANGO_SCALE) / 2,
          side + 1.5 * border_width - ((double)height / PANGO_SCALE) / 2);
      layout->show_in_cairo_context(cr);
    }
    for (int row = 0; row < 8; ++row)
    {
      if (!need_redraw.symbol(3, 7 - row))
        continue;
      c[0] = '1' + row;
      layout->set_text(c);
      int width, height;
      layout->get_size(width, height);
      cr->move_to(border_width / 2 - ((double)width / PANGO_SCALE) / 2,
          border_width + ((m_flip_board ? row : 7 - row) + 0.5) * m_sside - ((double)height / PANGO_SCALE) / 2);
      layout->show_in_cairo_context(cr);
    }
  }

  cr->restore();

  // Draw the turn indicator, if requested.
  if (m_draw_turn_indicators)
  {
    gint const border_width = m_border_width;
    gint const border_shadow_width = 2;
    gint const edge_width = border_width - border_shadow_width - 1;
    gint const side = squares * m_sside;
    bool const top = (m_active_turn_indicator == m_flip_board);
//    double const dir = top ? 1.0 : -1.0;
    double const factor = 0.085786;       // (1/sqrt(2) − 0.5)/(1 + sqrt(2)).

    cr->save();

    // We draw relative to the top-left of a square of edge_width X edge_width.
    cr->translate(
        top_left_board_x() + side + 1,
        top_left_board_y() - border_width + border_shadow_width + (top ? 0 : side + edge_width + 2));

#if 0
    cr->move_to(0, top ? edge_width : 0);
    cr->rel_line_to(0, dir * ((edge_width + 1) * factor + 1));
    cr->rel_line_to(edge_width, 0);
    cr->line_to(edge_width, top ? 0 : edge_width);
    cr->rel_line_to(-(edge_width + (edge_width + 1) * factor + 1), 0);
    cr->rel_line_to(0, dir * edge_width);
    cr->close_path();

    cr->set_source_rgb(m_board_border_color.red, m_board_border_color.green, m_board_border_color.blue);
    cr->fill();
#endif

    double val = m_active_turn_indicator ? 1.0 : 0.0;     // White or black color.
    cr->set_source_rgb(val, val, val);
    cr->arc(edge_width * 0.5 - std::max((edge_width + 1) * factor - 1, 0.0),
            edge_width * 0.5 - (edge_width + 1) * (top ? -factor : factor), edge_width * 0.5, 0, 2 * M_PI);
    cr->fill();

    cr->restore();
  }
}

// Definitions of static constexpr masks.
constexpr CwChessboardCode ChessboardWidget::s_color_mask;
constexpr CwChessboardCode ChessboardWidget::s_piece_mask;
constexpr CwChessboardCode ChessboardWidget::s_piece_color_mask;
constexpr CwChessboardCode ChessboardWidget::s_bghandle_mask;
constexpr CwChessboardCode ChessboardWidget::s_mahandle_mask;

} // namespace cwmm
