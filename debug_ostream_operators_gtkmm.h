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

#pragma once

#ifdef CWDEBUG

//---------------------------------------------------------------------------
// gtkmm debug ostream operators.

#include <iosfwd>			// std::ostream&
#include <gtkmm.h>
#include <gdk/gdk.h>		        // GdkEventType, GdkEvent
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>		        // GtkWidget, GtkAllocation, GAction
#pragma GCC diagnostic pop
#include <glib.h>			// GIOCondition

std::ostream& operator<<(std::ostream& os, GdkEventType event_type);
std::ostream& operator<<(std::ostream& os, GdkEvent const* event);
std::ostream& operator<<(std::ostream& os, GtkWidget const* widget);
std::ostream& operator<<(std::ostream& os, GtkAllocation const* widget);
std::ostream& operator<<(std::ostream& os, GdkEventButton const* eb);
std::ostream& operator<<(std::ostream& os, GdkModifierType modifier_type);
std::ostream& operator<<(std::ostream& os, GdkDevice const* device);
std::ostream& operator<<(std::ostream& os, GdkWindow const* window);
std::ostream& operator<<(std::ostream& os, GdkEventMotion const* em);
std::ostream& operator<<(std::ostream& os, GdkEventConfigure const* ec);
std::ostream& operator<<(std::ostream& os, cairo_rectangle_t const& rect);
std::ostream& operator<<(std::ostream& os, cairo_rectangle_int_t const& rect);
std::ostream& operator<<(std::ostream& os, GdkEventExpose const* ee);
std::ostream& operator<<(std::ostream& os, cairo_region_t const* region);
std::ostream& operator<<(std::ostream& os, GAction const* action);
std::ostream& operator<<(std::ostream& os, GdkColor const& color);
std::ostream& operator<<(std::ostream& os, Gtk::Allocation const& allocation);
std::ostream& operator<<(std::ostream& os, Cairo::RefPtr<Cairo::Region> const& region);

#endif // CWDEBUG
