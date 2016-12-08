// cwchessboard -- A C++ chessboard tool set
//
//! @file tstspirit.cc A test application to test using boost::spirit::classic.
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

#define BOOST_SPIRIT_DEBUG

#include "sys.h"
#include <iostream>
#include <fstream>
#include <boost/spirit/include/classic_core.hpp>    
#include <boost/spirit/include/classic_multi_pass.hpp>
#include "PgnGrammar.h"
#include "debug.h"

namespace {

typedef char char_t;
//typedef boost::spirit::classic::multi_pass<std::istreambuf_iterator<char_t> > iterator_t;
struct MY_ITERATOR {
  public:
    typedef std::input_iterator_tag iterator_category;
    typedef char value_type;
    typedef ptrdiff_t difference_type;
    typedef char* pointer;
    typedef char& reference;

    friend bool operator==(MY_ITERATOR const& a, MY_ITERATOR const& b) { return false; }
    friend bool operator!=(MY_ITERATOR const& a, MY_ITERATOR const& b) { return true; }
    value_type& operator*(void) { return M_dummy; }
    value_type const& operator*(void) const { return M_dummy; }
    MY_ITERATOR& operator++(void) { return *this; }

    MY_ITERATOR(void) : M_dummy('?') { }
    MY_ITERATOR(MY_ITERATOR const& iter) : M_dummy(iter.M_dummy) { }
    MY_ITERATOR& operator=(MY_ITERATOR const& iter) { M_dummy = iter.M_dummy; }

  private:
    char M_dummy;
};

typedef MY_ITERATOR iterator_t;

typedef boost::spirit::classic::skip_parser_iteration_policy<boost::spirit::classic::space_parser> iter_policy_t;
typedef boost::spirit::classic::scanner_policies<iter_policy_t> scanner_policies_t;
typedef boost::spirit::classic::scanner<iterator_t, scanner_policies_t> scanner_t;

iter_policy_t iter_policy(boost::spirit::classic::space_p);
scanner_policies_t policies(iter_policy);

} // namespace *anonymous*

int main(int argc, char* argv[])
{
  Debug(debug::init());

  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == "/home/carlo/chess/freechess.ladder/0018.pgn")
      continue;
    std::cout << "Trying to parse file \"" << argv[i] << "\"." << std::endl;
    std::ifstream in(argv[i]);
    iterator_t first; // FIXME (boost::spirit::classic::make_multi_pass(std::istreambuf_iterator<char_t>(in)));
    iterator_t /*const*/ last; // FIXME (boost::spirit::classic::make_multi_pass(std::istreambuf_iterator<char_t>()));

    cwchess::pgn::grammar::PgnGrammar g;
    boost::spirit::classic::parse_info<iterator_t> info = boost::spirit::classic::parse(first, last, g);
    if (!info.hit)
    {
      std::cout << "Failure to parse anything." << std::endl;
      return 1;
    }
    else
      std::cout << info.length << " characters have been parsed successfully." << std::endl;
  }
}
