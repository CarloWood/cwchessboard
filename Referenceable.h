// cwchessboard -- A C++ chessboard tool set
//
//! @file Referenceable.h This file contains the declaration of class Referenceable.
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

#ifndef REFERENCEABLE_H
#define REFERENCEABLE_H

#ifndef USE_PCH
#include <glib.h>
#endif

namespace util {

class Referenceable {
  private:
    mutable volatile gint M_ref_count;
  protected:
    Referenceable(void) : M_ref_count(1) { }
    virtual ~Referenceable() { }
  public:
    void reference(void) const { g_atomic_int_inc(&M_ref_count); }
    void unreference(void) const { if (g_atomic_int_dec_and_test(&M_ref_count)) delete const_cast<Referenceable*>(this); }
};

} // namespace util

#endif	// REFERENCEABLE_H
