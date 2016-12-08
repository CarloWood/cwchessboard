// cwchessboard -- A C++ chessboard tool set
//
//! @file PgnDatabase.h This file contains the declaration of class pgn::Database.
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

#ifndef PGNDATABASE_H
#define PGNDATABASE_H

#ifndef USE_PCH
#include <string>
#include <glibmm/refptr.h>
#include <glibmm/dispatcher.h>
#include <giomm/file.h>
#endif

#include "Referenceable.h"
#include "MemoryBlockList.h"

namespace cwchess {
namespace pgn {

using util::MemoryBlockList;
using util::MemoryBlockNode;

class Database : public util::Referenceable {

  enum state_type {
    white_space,
    string_token,
    comment_token
  };

  private:
    bool M_saw_carriage_return;
    size_t M_line_wrapped;
    int M_number_of_lines;
    size_t M_number_of_characters;
    state_type M_state;
    unsigned char const* M_search_table;
    unsigned char const* M_search_table_storeR;
    static unsigned char* S_state_tables[11];

  protected:
    MemoryBlockList* M_buffer;				//!< Linked list of blocks with valid data.
    Glib::RefPtr<MemoryBlockNode> M_new_block;		//!< Temporary storage for new block that is being read and not linked yet.
    //! Constructor.
    Database(void) : M_buffer(NULL), M_saw_carriage_return(false), M_line_wrapped(0),
        M_number_of_lines(0), M_number_of_characters(0), M_state(white_space) { }

    /** @brief Process next data block.
     *
     * This function is called for all subsequent blocks of data during the initialization of the Database object.
     */
    void process_next_data_block(char const* data, size_t size);
  public:
    //! @brief Return the path name of the database.
    virtual std::string get_path(void) const = 0;

    int number_of_lines(void) const { return M_number_of_lines; }
    size_t number_of_characters(void) const { return M_number_of_characters; }
};

class DatabaseSeekable : public Database {
  public:
    typedef sigc::slot<void, size_t> SlotOpenFinished;
    // This is the minimum blocksize needed to reach a speed of 130 MB/s.
    // The 64 is to take MemoryBlockNode (32 bytes) and a possible malloc overhead into account.
    // That means we're not reading an integral number of disk blocks at a time, but that
    // turns out to make no difference (on my machine).
    static size_t const S_buffer_size = 6 * 4096 - 64;
  private:
    Glib::RefPtr<Gio::File> M_file;
    Glib::RefPtr<Gio::Cancellable> M_cancellable;
    gsize M_bytes_read;
    Glib::RefPtr<Gio::FileInputStream> M_file_input_stream;
    SlotOpenFinished M_slot_open_finished;
    Glib::Thread* M_read_thread;
    Glib::Dispatcher M_processing_finished;
  public:
    static Glib::RefPtr<Database> open(std::string const& path, SlotOpenFinished const& slot)
        { return Glib::RefPtr<Database>(new DatabaseSeekable(path, slot)); }
  protected:
    DatabaseSeekable(std::string const& path, SlotOpenFinished const& slot_open_finished) :
        M_file(Gio::File::create_for_path(path)), M_cancellable(Gio::Cancellable::create()),
	M_bytes_read(0), M_slot_open_finished(slot_open_finished) { load(); }
    virtual ~DatabaseSeekable();
  private:
    void load(void);
    void read_async_open_ready(Glib::RefPtr<Gio::AsyncResult>& result);
    static void read_async_ready(GObject* source_object, GAsyncResult* async_res, gpointer user_data);
    void read_async_ready(GObject* source_object, GAsyncResult* async_res);
    void need_more_data(void);
    void processing_finished(void);

    //! @brief Return the path name of the database.
    virtual std::string get_path(void) const { M_file->get_path(); }

  private:
    void read_thread(void);
};

} // namespace pgn
} // namespace cwchess

#endif	// PGNDATABASE_H
