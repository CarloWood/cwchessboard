// cwchessboard -- A C++ chessboard tool set
//
//! @file tstpgn.cc A test application to test reading PGN files.
//
// Copyright (C) 2010, by
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
#include <giomm/init.h>
#include "ChessGame.h"
#include "PgnDatabase.h"
#include "debug.h"

//char const* filename="/home/carlo/projects/cwchessboard/chessgames/ftp.cis.uab.edu/crafty/enormous.pgn";
//char const* filename="test.pgn";
//char const* filename="/home/carlo/chess/jin.log.pgn";
//char const* filename="/opt/large/backups/usr-src-20080707.tar";
char const* filename="/home/carlo/projects/cwchessboard/cwchessboard/torture.pgn";

using namespace cwchess;

Glib::RefPtr<Glib::MainLoop> main_loop;

void open_finished(size_t len)
{
  std::cout << "Total size read: " << len << '\n';
  main_loop->quit();
}

int main(int argc, char* argv[])
{
  Debug(debug::init());
  if (!Glib::thread_supported())
    Glib::thread_init();
  Gio::init();

  char const* infile = filename;
  if (argc > 1)
    infile = argv[1];
  Glib::RefPtr<pgn::Database> pgn_data_base = pgn::DatabaseSeekable::open(infile, sigc::ptr_fun(&open_finished));
  main_loop = Glib::MainLoop::create(false);
  main_loop->run();
}
