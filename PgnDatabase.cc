// cwchessboard -- A C++ chessboard tool set
//
//! @file PgnDatabase.cc This file contains the implementation of class pgn::Database.
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

#ifdef CWDEBUG
#define BOOST_SPIRIT_DEBUG
#endif

#ifndef USE_PCH
#include "sys.h"
#include <cstring>
#include <ctime>		// Needed for clock_gettime.
#include <iomanip>
#include "debug.h"
#include <glib.h>
#ifdef CWDEBUG
#include <libcwd/buf2str.h>
#endif
#endif

#include "PgnDatabase.h"
#include "PgnGrammar.h"
#include "chattr.h"
#include "Color.h"

namespace cwchess {
namespace pgn {

void Database::process_next_data_block(char const* data, size_t size)
{
}

void DatabaseSeekable::load(void)
{
  if (!Glib::thread_supported())
    DoutFatal(dc::fatal, "DatabaseSeekable::load: Threading not initialized. Call Glib::init_thread() at the start of main().");
  M_file->read_async(sigc::mem_fun(this, &DatabaseSeekable::read_async_open_ready), M_cancellable);
}

void DatabaseSeekable::read_async_open_ready(Glib::RefPtr<Gio::AsyncResult>& result)
{
  M_file_input_stream = M_file->read_finish(result);
  M_buffer = new MemoryBlockList(sigc::mem_fun(*this, &DatabaseSeekable::need_more_data));
  M_read_thread = Glib::Thread::create(sigc::mem_fun(*this, &DatabaseSeekable::read_thread), false);
  M_new_block = MemoryBlockNode::create(S_buffer_size);
  // We're using the glib API for the inner loop in order to avoid unnecessary calls to new/delete.
  GInputStream* stream = M_file_input_stream->InputStream::gobj();
  g_input_stream_read_async(stream, M_new_block->block_begin(), S_buffer_size,
      G_PRIORITY_DEFAULT, M_cancellable->gobj(), &DatabaseSeekable::read_async_ready, this);
  M_processing_finished.connect(sigc::mem_fun(*this, &DatabaseSeekable::processing_finished));
}

void DatabaseSeekable::need_more_data(void)
{
  M_new_block = MemoryBlockNode::create(S_buffer_size);
  g_input_stream_read_async(M_file_input_stream->InputStream::gobj(), M_new_block->block_begin(), S_buffer_size,
      G_PRIORITY_DEFAULT, M_cancellable->gobj(), &DatabaseSeekable::read_async_ready, this);
}

// The inner loop.
inline void DatabaseSeekable::read_async_ready(GObject* source_object, GAsyncResult* async_res)
{
  GInputStream* stream = M_file_input_stream->InputStream::gobj();
  GError* error = NULL;
  gssize len = g_input_stream_read_finish(stream, async_res, &error);
  if (len == -1)
    DoutFatal(dc::core, "read_finish() returned -1");
  if (len > 0)
  {
    M_bytes_read += len;
    Dout(dc::notice, "Appending a block with " << len << " bytes to the buffer.");
    // This is the only place where append is called, which is the only
    // function that increments MemoryBlockList::M_blocks.
    M_buffer->append(M_new_block, len);
  }
  else
  {
    Dout(dc::notice, "g_input_stream_read_finish() returned 0. Closing buffer.");
    // If we get here, we're completely done reading the file.
    // Hence, the read thread is the only thread accessing the buffer and
    // we can allow it to process the last block, after which it
    // will destroy the buffer.
    M_buffer->close();
  }
}

void DatabaseSeekable::read_async_ready(GObject* source_object, GAsyncResult* async_res, gpointer user_data)
{
  DatabaseSeekable* database_seekable = reinterpret_cast<DatabaseSeekable*>(user_data);
  database_seekable->read_async_ready(source_object, async_res);
}

DatabaseSeekable::~DatabaseSeekable()
{
  // Just in case. Normally this should already be freed after loading of the database finished.
  if (M_buffer)
    delete M_buffer;
}

namespace {

timespec& operator-=(timespec& t1, timespec const& t2)
{
  t1.tv_sec -= t2.tv_sec;
  t1.tv_nsec -= t2.tv_nsec;
  if (t1.tv_nsec < 0)
  {
    --t1.tv_sec;
    t1.tv_nsec += 1000000000L;
  }
  return t1;
}

timespec& operator+=(timespec& t1, timespec const& t2)
{
  t1.tv_sec += t2.tv_sec;
  t1.tv_nsec += t2.tv_nsec;
  if (t1.tv_nsec > 999999999L)
  {
    ++t1.tv_sec;
    t1.tv_nsec -= 1000000000L;
  }
  return t1;
}

std::ostream& operator<<(std::ostream& os, timespec const& t1)
{
  return os << t1.tv_sec << '.' << std::setfill('0') << std::setw(9) << t1.tv_nsec;
}

} // namespace

//
// The read thread.
//
// The code below belongs to a different thread. It reads and processes the buffer.
//

#define DEBUG_PARSER 0

//! @brief Variable data of a Scanner.
template<class ForwardIterator>
struct ScannerData {
  ForwardIterator* M_iter;		//!< The current position.
  unsigned int M_line;			//!< The current line number, starts at 1.
  unsigned int M_column;		//!< The current column, starts at 0.
  unsigned int M_number_of_characters;	//!< The number of characters before the current position.
  Color M_to_move;			//!< The color that is too move at this point in the game.
#if DEBUG_PARSER
  ForwardIterator M_line_start;		//!< Pointer to the start of the current line.

  ScannerData(ForwardIterator* iter) : M_iter(iter), M_line(1), M_column(0), M_number_of_characters(0), M_line_start(iter->buffer()) { }
#else
  void init(ForwardIterator* iter)
  {
    M_iter = iter;
    M_line = 1;
    M_column = 0;
    M_number_of_characters = 0;
  }
#endif
};

class EndOfFileReached : public std::exception {
};

class ParseError : public std::exception {
};

static EndOfFileReached const end_of_file_reached;

//! @brief A class used to read input from a PGN database.
template<class ForwardIterator>
class Scanner {
  private:
    ScannerData<ForwardIterator> M_current_position;	//!< The current position.
    ForwardIterator const M_end;			//!< The one-past-the-end position.
    std::vector<ScannerData<ForwardIterator> > M_stack;	//!< A stack of stored positions.
    int M_stack_index;					//!< Next free place on stack.
  public:
    //! @brief Construct a Scanner object.
    //
    // @param iter : Iterator to the first character.
    // @param iter : Iterator one-past-the-end.
    Scanner(ForwardIterator& iter, ForwardIterator const end) :
#if DEBUG_PARSER
	M_current_position(&iter),
#endif
        M_end(end), M_stack_index(0)
#if DEBUG_PARSER
	{ }
#else
        { M_current_position.init(&iter); }
#endif

    int push_position(void)
    {
      size_t size = M_stack.size();
      if (G_UNLIKELY(size <= M_stack_index))
	M_stack.push_back(M_current_position);
      else
	M_stack[M_stack_index] = M_current_position;
      return M_stack_index++;
    }

    void pop_position(int index)
    {
      M_current_position = M_stack[index];
      M_stack_index = index;
    }

#if DEBUG_PARSER
    //! @brief A debug helper routine.
    //
    // This function is intended for debugging only.
    // It writes to debug channel dc::parser and escapes non-printable characters.
    void print_line(void)
    {
      std::string s(M_current_position.M_line_start, *M_current_position.M_iter);
      Dout(dc::parser, "Parsed: \"" << buf2str(s.data(), s.length()) << "\".");
    }
#endif

    //! @brief Return the first character.
    //
    // This function must be called directly after creation of the Scanner object,
    // before calling any of the other member functions. It should only be called
    // once.
    //
    // @returns The first character.
    typename ForwardIterator::value_type first_character(void) throw(EndOfFileReached)
    {
      if (G_UNLIKELY(*M_current_position.M_iter == M_end))
	throw end_of_file_reached;
      M_current_position.M_number_of_characters = 1;
#if DEBUG_PARSER
      M_current_position.M_line_start = *M_current_position.M_iter;
#endif
      return **M_current_position.M_iter;
    }

    //! @brief Make the next character the current character.
    //
    // @returns The new current character.
    typename ForwardIterator::value_type next_character(void) //throw(EndOfFileReached)
    {
      if (G_UNLIKELY(++*M_current_position.M_iter == M_end))
      {
#if DEBUG_PARSER
	print_line();
#endif
//	throw end_of_file_reached;
      }
      ++M_current_position.M_column;
      return **M_current_position.M_iter;
    }

    //! @brief Eat all white space character, return the first non-white-space.
    //
    // @param c : A reference to the current character.
    //
    // Upon return, \a current_character will contain the first non-white-space character.
    // If \a current_character is not a space upon entry, then the function does nothing.
    void eat_white_space(typename ForwardIterator::value_type& current_character)
    {
      while (is_white_space(current_character))
      {
	if (G_UNLIKELY(is_eol(current_character)))
	  eat_eol(current_character);
	else
	  current_character = next_character();
      }
    }

    //! @brief Eat all characters left in the current line up till but not including the EOL.
    void eat_line(typename ForwardIterator::value_type& current_character)
    {
      while (!is_eol(current_character))
	current_character = next_character();
    }

    //! @brief Parse the next character and return true if it equals \a literal.
    bool parse_char(typename ForwardIterator::value_type& current_character, char literal)
    {
      current_character = next_character();
      if (current_character != literal)
	return false;
      current_character = next_character();
      return true;
    }

    //! @brief Return true if the string after the current character matches \a literal.
    bool parse_str(typename ForwardIterator::value_type& current_character, char const* literal)
    {
      current_character = next_character();
      for (char const* p = literal; *p; ++p)
      {
	if (current_character != *p)
	  return false;
	current_character = next_character();
      }
      return true;
    }

    //! @brief Eat a single comment, if any.
    //
    // This function should only ever be called directly after
    // a call to eat_white_space.
    //
    // @returns True if a comment was eaten.
    bool eat_comment(typename ForwardIterator::value_type& current_character)
    {
#if DEBUG_PARSER
      assert(!is_white_space(current_character));
#endif
      if (G_UNLIKELY(is_comment_start(current_character)))
      {
	if (current_character == '{')
	{
	  while (current_character != '}')
	    current_character = next_character();
	  current_character = next_character();
	}
	else // current_character == ';'
	{
	  eat_line(current_character);
	  eat_eol(current_character);
	}
	return true;
      }
      return false;
    }

    //! Eat all white space and all comments encountered, if any.
    void eat_white_space_and_comments(typename ForwardIterator::value_type& current_character)
    {
      eat_white_space(current_character);
      while(eat_comment(current_character))
	eat_white_space(current_character);
    }

    //! @brief Eat one or more EOL sequences.
    //
    // The current position must be on an EOL character (is_eol(c) is true).
    // This function also eats escaped lines (lines starting with a '%'),
    // such lines are completely ignored and not counted as empty lines.
    //
    // @returns True if more than one EOL sequence was eaten.
    bool eat_eol(typename ForwardIterator::value_type& current_character)
    {
#if DEBUG_PARSER
      assert(is_eol(current_character));
#endif
      unsigned int line = M_current_position.M_line + 1;
      do
      {
	++M_current_position.M_line;
	bool saw_carriage_return = (current_character == '\r');
	current_character = next_character();
	if (saw_carriage_return && current_character == '\n')
	  current_character = next_character();
        if (current_character == '%')
	{
	  eat_line(current_character);
	  ++line;	// We don't count escaped lines as empty lines.
	}
      }
      while (is_eol(current_character));
#if DEBUG_PARSER
      print_line();
      M_current_position.M_line_start = *M_current_position.M_iter;
#endif
      M_current_position.M_number_of_characters += M_current_position.M_column;
      M_current_position.M_column = 0;
      return M_current_position.M_line > line;
    }

    //! @brief Decodes a string.
    //
    // The current position must be a quote character.
    // After this function returns, the current position
    // is the character after the second quote.
    void decode_string(typename ForwardIterator::value_type& current_character)
    {
      do
      {
	current_character = next_character();
      }
      while(current_character != '"');
      // Eat closing quote.
      current_character = next_character();
    }

    //! @brief Return the current line number.
    unsigned int line(void) const { return M_current_position.M_line; }
    //! @brief Return the column.
    unsigned int column(void) const { return M_current_position.M_column + 1; }
    //! @brief Return the total number of characters parsed thus far.
    //
    // The current character is not counted.
    unsigned int number_of_characters(void) const { return M_current_position.M_number_of_characters + M_current_position.M_column; }

    //! @brief Return who is expected to move at this moment.
    Color to_move(void) const { return M_current_position.M_to_move; }

    //! @brief Reset the game state.
    void reset_game_state(void)
    {
      M_current_position.M_to_move = white;
    }

#ifdef CWDEBUG
    template<typename T>
    friend std::ostream& operator<<(std::ostream& os, Scanner<T> const& scanner);
#endif
  };

#ifdef CWDEBUG
//! @brief Debug helper function.
template<typename T>
std::ostream& operator<<(std::ostream& os, Scanner<T> const& const_scanner)
{
  Scanner<T> scanner(const_scanner);
  if (*scanner.M_current_position.M_iter == scanner.M_end)
    os << "<EOF>";
  else
  {
    char c = **scanner.M_current_position.M_iter;
    try
    {
      do
      {
	os << libcwd::char2str(c);
      }
      while (!is_eol(c = scanner.next_character()));
    }
    catch(EndOfFileReached&)
    {
      os << "<EOF>";
    }
  }
  return os;
}
#endif

typedef Scanner<MemoryBlockList::iterator> scanner_t;

namespace {

//! @brief Decode a tagname. 
//
// A tagname must begin with an alpha-numeric character.
// The rest of the characters are either alpha-numberic or underscores.
//
// @returns True if a non-empty tagname was found.
inline bool decode_tagname(char& c, scanner_t& scanner)
{
  if (G_UNLIKELY(!is_tagname_begin(c)))
    return false;
  while(is_tagname_continuation(c))
    c = scanner.next_character();
  return true;
}

//! @brief Decode a string, if any.
//
// This function demands that the string is on one line: EOL characters are not allowed in the string.
//
// @returns True if a string was found and decoded.
inline bool correct_string(char& c, scanner_t& scanner)
{
  if (c != '"')
    return false;
  // Eat the first quote.
  c = scanner.next_character();
  // Find the second quote, but also stop if we run into an EOL.
  while(!is_quote_or_eol(c))
    c = scanner.next_character();
  // Note a correct string if we ran into an EOL.
  if (c != '"')
    return false;
  // Eat the second quote.
  c = scanner.next_character();
  return true;
}

//! @brief Decode a tag pair.
//
// The current position must be on a '['.
// @returns True if a correctly formatted tag pair was found and decoded.
inline bool correct_tag_pair(char& c, scanner_t& scanner)
{
#if DEBUG_PARSER
  assert(c == '[');
#endif
  // Skip the '['.
  c = scanner.next_character();
  scanner.eat_white_space(c);
  if (G_UNLIKELY(!decode_tagname(c, scanner)))
    return false;
  scanner.eat_white_space(c);
  if (G_UNLIKELY(!correct_string(c, scanner)))
    return false;
  scanner.eat_white_space(c);
  if (G_UNLIKELY(c != ']'))
    return false;
  // Skip the ']'.
  c = scanner.next_character();
  return true;
}

inline bool tag_pair(char& c, scanner_t& scanner)
{
#if DEBUG_PARSER
  assert(c == '[');
#endif
  // Skip the '['.
  c = scanner.next_character();
  scanner.eat_white_space_and_comments(c);
  if (G_UNLIKELY(!decode_tagname(c, scanner)))
    return false;
  scanner.eat_white_space_and_comments(c);
  if (G_UNLIKELY(is_tag_separator_junk(c)))
  {
    // Allow stupidity like [Annotator: "Me"], or [Result = "1-0"].
    c = scanner.next_character();
    scanner.eat_white_space_and_comments(c);
  }
  if (G_UNLIKELY(c != '"'))
    return false;
  scanner.decode_string(c);
  scanner.eat_white_space_and_comments(c);
  if (G_UNLIKELY(c != ']'))
    return false;
  // Skip the ']'.
  c = scanner.next_character();
  return true;
}

bool decode_movetext_section_white(char& c, scanner_t& scanner) throw(ParseError)
{
  throw ParseError();
}

bool decode_movetext_section_black(char& c, scanner_t& scanner) throw(ParseError)
{
  throw ParseError();
}

bool decode_movetext_section(char& c, scanner_t& scanner)
{
  if (scanner.to_move() == white)
    return decode_movetext_section_white(c, scanner);
  else
    return decode_movetext_section_black(c, scanner);
}

bool decode_game_termination(char& c, scanner_t& scanner)
{
  char d = c;
  c = scanner.next_character();
  if (d == '0' && c == '-')
    return scanner.parse_char(c, '1');
  else if (d == '1' && c == '/')
    return scanner.parse_str(c, "2-1/2");
  return d == '*';
}

} // namespace

void DatabaseSeekable::read_thread(void)
{
  Debug(debug::init_thread());
  Dout(dc::notice, "DatabaseSeekable::read_thread started.");

  timespec start_time_real, end_time_real;
  timespec start_time_process, end_time_process;
  timespec start_time_thread, end_time_thread;

  clock_gettime(CLOCK_REALTIME, &start_time_real);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time_process);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start_time_thread);

  scanner_t scanner(M_buffer->begin(), M_buffer->end());

  try
  {
    int PGN_game_start;
    int PGN_movetext_section_start;

    bool saw_empty_line = true;			// The start of the file has the same status as empty line.

    // Read first character if any.
    char c = scanner.first_character();

    // Loop to find the start of the next game in case of parse errors.
    for (;;)
    {
      // We're going to parse a new game. Start with resetting the game state.
      scanner.reset_game_state();

      try
      {

	//
	// Start with eating leading junk.
	//

	// Eat leading white spaces.
	scanner.eat_white_space(c);

	do
	{
	  if (c == '[')
	  {
	    PGN_game_start = scanner.push_position();

	    // Demand a syntactically correct tag pair if we saw junk and there is no separating empty line before it.
	    // Otherwise our less restrictive tag pair parser is used.
	    if ((saw_empty_line && tag_pair(c, scanner)) ||
		(!saw_empty_line && correct_tag_pair(c, scanner)))
	    {
	      // Found the start of a PGN game.
	      Dout(dc::parser, "After first tag pair of PGN game: " << scanner.line() << ':' << scanner.column());
	      break;
	    }
	  }
	  // Eat this whole line.
	  scanner.eat_line(c);
	  // Plus the EOL, and possible following empty lines.
	  saw_empty_line = scanner.eat_eol(c);
	}
	while(1);

	//
	// We found the beginning of the first PGN file and parsed the first tag pair.
	// Next, parse all remaining tag pairs.
	//

	scanner.eat_white_space_and_comments(c);
	while(c == '[')
	{
	  if (G_UNLIKELY(!tag_pair(c, scanner)))
	    break; 
	  scanner.eat_white_space_and_comments(c);
	}

	// Decode optional movetext section.
	if (c == '1')
	{
	  // Because it is optional, we have to remember the position
	  // in case it fails. This could be a game termination (1-0 or 1/2-1/2).
          PGN_movetext_section_start = scanner.push_position();
	}
	if (!decode_movetext_section(c, scanner))
	{
	  // It failed, so pop the position.
	  scanner.pop_position(PGN_movetext_section_start);
	  c = '1';
	}
	
	if (decode_game_termination(c, scanner))
	{
	  // Eat any possible final comments.
	  scanner.eat_white_space_and_comments(c);

	  // Since this was a clean exit, start processing of
	  // next game as if we just saw an empty line (we probably did anyway).
	  saw_empty_line = true;
	  continue;
	}

	// Missing game termination, or parse error...
	Dout(dc::parser, "Parsing stopped at " << scanner.line() << ':' << scanner.column() << " at \"" << scanner << "\".");
      }
      catch(ParseError&)
      {
	// Eat the rest until the next start of a PGN game.
	continue;
      }
      break;
    }
  }
  catch(EndOfFileReached&)
  {
  }

  clock_gettime(CLOCK_REALTIME, &end_time_real);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time_process);
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end_time_thread);

  end_time_real -= start_time_real;
  end_time_process -= start_time_process;
  end_time_thread -= start_time_thread;

#if 0
  if (!info.hit)
  {
    std::cout << "Failure to parse anything." << std::endl;
  }
  else
    std::cout << info.length << " characters have been parsed successfully." << std::endl;
#endif

  std::cout << "Number of characters: " << scanner.number_of_characters() << '\n';
  std::cout << "Number of lines: " << scanner.line() << '\n';

  std::cout << "Real time                                 : " << end_time_real << " seconds.\n";
  std::cout << "Process time                              : " << end_time_process << " seconds.\n";
  std::cout << "Run time read_thread                      : " << end_time_thread << " seconds.\n";

  double t = end_time_thread.tv_sec + end_time_thread.tv_nsec * 1e-9;
  std::cout << "Speed: " << (scanner.number_of_characters() / t / 1048576) << " MB/s." << std::endl;

  M_processing_finished.emit();
}

void DatabaseSeekable::processing_finished(void)
{
  assert(M_buffer->closed());
  delete M_buffer;
  M_buffer = NULL;
  M_slot_open_finished(M_bytes_read);
}

} // namespace pgn
} // namespace cwchess
