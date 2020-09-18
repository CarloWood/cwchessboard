// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file
//! @brief Debug specific code.
//
// Copyright (C) 2017 Carlo Wood
//
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

#include "sys.h"

#ifdef CWDEBUG
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START

// Add new debug channels here.
channel_ct event("EVENT");
channel_ct motion_event("MOTION_EVENT");
channel_ct widget("WIDGET");
channel_ct place("PLACE");
channel_ct countboard("COUNTBOARD");
channel_ct clipboard("CLIPBOARD");
channel_ct parser("PARSER");
channel_ct clip("CLIP");

NAMESPACE_DEBUG_CHANNELS_END

#endif // CWDEBUG
