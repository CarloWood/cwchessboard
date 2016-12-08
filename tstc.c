// cwchessboard -- A GTK+ chessboard widget
//
//! @file tstc.c An example of how to use the GTK+ widget with C code.
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
// CwChessboard usage example in C code.

#include <gtk/gtk.h>
#include "CwChessboard.h"

static void destroy_event(gchar* response)
{
  gtk_main_quit();
}

static gint handle = -1;
static gint arrow_begin_col;
static gint arrow_begin_row;

static inline gboolean is_inside_board(gint col, gint row)
{
   return !((col | row) & ~0x7);
}

gboolean button_press_event(GtkWidget* widget, GdkEventButton* event)
{
  CwChessboard* chessboard = CW_CHESSBOARD(widget);

  if (event->button == 1)
  {
    handle = -1;
    // Find the square under the mouse, if any.
    gint col = cw_chessboard_x2col(chessboard, event->x);
    gint row = cw_chessboard_y2row(chessboard, event->y);
    if (is_inside_board(col, row))
    {
      CwChessboardCode code = cw_chessboard_get_square(chessboard,col, row);
      if (code > 1)	// The least significant bit reflects the color of the piece.
		      // A code <= 1 means an empty square.
      {
	// Make the square empty.
	cw_chessboard_set_square(chessboard, col, row, empty_square);
	// Put the piece under the mouse pointer.
	double hsside = 0.5 * chessboard->sside;
	double fraction = hsside - (gint)hsside;
	handle = cw_chessboard_add_floating_piece(chessboard, code,
	    event->x - fraction, event->y - fraction, TRUE);
      }
    }
  }
  else if (event->button == 2)
  {
    gint col = cw_chessboard_x2col(chessboard, event->x);
    gint row = cw_chessboard_y2row(chessboard, event->y);
    if (is_inside_board(col, row))
    {
      cw_chessboard_show_cursor(chessboard);
      cw_chessboard_set_marker_color(chessboard, col, row, 1);
      arrow_begin_col = col;
      arrow_begin_row = row;
    }
  }
  else if (event->button == 3)
    cw_chessboard_show_cursor(chessboard);

  return (handle != -1);
}

gboolean button_release_event(GtkWidget* widget, GdkEventButton* event)
{
  CwChessboard* chessboard = CW_CHESSBOARD(widget);
  if (event->button == 1)
  {
    if (handle != -1)
    {
      // Find the square under the mouse, if any.
      gint col = cw_chessboard_x2col(chessboard, event->x);
      gint row = cw_chessboard_y2row(chessboard, event->y);
      if (is_inside_board(col, row))
      {
	// Put the piece on the new square.
	cw_chessboard_set_square(chessboard, col, row, cw_chessboard_get_floating_piece(chessboard, handle));
      }
      // Remove the piece under the mouse pointer.
      cw_chessboard_remove_floating_piece(chessboard, handle);
      handle = -1;
      return TRUE;
    }
  }
  else if (event->button == 2)
  {
    GdkColor color = { 0, 0, 0, 65535 };
    gint col = cw_chessboard_x2col(chessboard, event->x);
    gint row = cw_chessboard_y2row(chessboard, event->y);
    cw_chessboard_hide_cursor(chessboard);
    cw_chessboard_set_marker_color(chessboard, arrow_begin_col, arrow_begin_row, 0);
    if ((arrow_begin_col != col || arrow_begin_row != row) &&
        is_inside_board(arrow_begin_col, arrow_begin_row) &&
	is_inside_board(col, row))
      cw_chessboard_add_arrow(chessboard, arrow_begin_col, arrow_begin_row, col, row, &color);
  }
  else if (event->button == 3)
    cw_chessboard_hide_cursor(chessboard);
  return FALSE;
}

static void setup_menu(GtkWidget* vbox)
{
  // Create File menu.
  GtkWidget* file_menu = gtk_menu_new();

  // File menu items.
  GtkWidget* quit_item = gtk_menu_item_new_with_label("Quit");

  // Add them to the menu.
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);

  // We can attach the Quit menu item to our exit function.
  g_signal_connect_swapped(G_OBJECT(quit_item), "activate", G_CALLBACK(destroy_event), (gpointer) "file.quit");

  // We do need to show menu items.
  gtk_widget_show(quit_item);

  GtkWidget* menu_bar = gtk_menu_bar_new();
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 2);
  gtk_widget_show(menu_bar);

  GtkWidget* file_item = gtk_menu_item_new_with_label("File");
  gtk_widget_show(file_item);

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
}

int main(int argc, char* argv[])
{
  if (!gtk_init_check(&argc, &argv))
  {
    printf("gtk_init_check failed\n");
    return 1;
  }

  // Create the chessboard widget.
  GtkWidget* chessboard_widget = cw_chessboard_new();
  CwChessboard* chessboard = CW_CHESSBOARD(chessboard_widget);

  // Allocate some colors.
  CwChessboardColorHandle light = cw_chessboard_allocate_color_handle_rgb(chessboard, 1.0, 1.0, 0.6);
  CwChessboardColorHandle dark = cw_chessboard_allocate_color_handle_rgb(chessboard, 0.8, 0.93, 0.47);
  CwChessboardColorHandle red = cw_chessboard_allocate_color_handle_rgb(chessboard, 1.0, 0.0, 0.0);
  CwChessboardColorHandle yellow = cw_chessboard_allocate_color_handle_rgb(chessboard, 1.0, 1.0, 0.0);

  // Highlight column 'e'.
  for (int row = 1; row < 7; ++row)
    cw_chessboard_set_background_color(chessboard, 4, row, (row & 1) ? light : dark);

  // Set up position.
  cw_chessboard_set_square(chessboard, 1, 0, empty_square);
  cw_chessboard_set_square(chessboard, 3, 0, empty_square);
  cw_chessboard_set_square(chessboard, 6, 0, empty_square);
  cw_chessboard_set_square(chessboard, 3, 1, empty_square);
  cw_chessboard_set_square(chessboard, 4, 1, empty_square);
  cw_chessboard_set_square(chessboard, 2, 2, white_knight);
  cw_chessboard_set_square(chessboard, 2, 3, white_knight);
  cw_chessboard_set_square(chessboard, 3, 3, white_pawn);
  cw_chessboard_set_square(chessboard, 5, 2, white_queen);
  cw_chessboard_set_square(chessboard, 2, 1, black_bishop);
  cw_chessboard_set_square(chessboard, 2, 5, black_pawn);
  cw_chessboard_set_square(chessboard, 5, 5, black_knight);
  cw_chessboard_set_square(chessboard, 3, 6, black_knight);
  cw_chessboard_set_square(chessboard, 1, 7, empty_square);
  cw_chessboard_set_square(chessboard, 2, 7, empty_square);
  cw_chessboard_set_square(chessboard, 2, 6, empty_square);
  cw_chessboard_set_square(chessboard, 6, 7, empty_square);

  // Set a few markers.
  cw_chessboard_set_marker_thickness(chessboard, 0.03);
  cw_chessboard_set_marker_color(chessboard, 4, 7, red);
  cw_chessboard_set_marker_color(chessboard, 2, 1, red);
  cw_chessboard_set_marker_color(chessboard, 4, 6, yellow);

  // Draw a few arrows.
  GdkColor green = { 0, 0, 32000, 0 };
  cw_chessboard_add_arrow(chessboard, 5, 2, 4, 1, &green);
  GdkColor red2 = { 0, 65535, 0, 0 };
  cw_chessboard_add_arrow(chessboard, 2, 3, 3, 5, &red2);

  // Create the main window.
  GtkWidget* main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  // Set initial size.
  gtk_window_set_default_size(GTK_WINDOW(main_window), 500, 532);

  // A vbox to put a menu and chessboard in.
  GtkWidget* vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(main_window), vbox);
  gtk_widget_show(vbox);

  // Set up the menu.
  setup_menu(vbox);

  // Add the chessboard to the vbox.
  gtk_container_add(GTK_CONTAINER(vbox), chessboard_widget);

  // Show the chessboard.
  gtk_widget_show(chessboard_widget);

  // Install handler for mouse movement.
  g_signal_connect(G_OBJECT(chessboard), "button-press-event", G_CALLBACK(button_press_event), NULL);
  g_signal_connect(G_OBJECT(chessboard), "button-release-event", G_CALLBACK(button_release_event), NULL);

  // Install handler for destroy event.
  g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(destroy_event), NULL);

  // Draw the main window.
  gtk_widget_show(main_window);

  // Start main loop.
  gtk_main();
}
