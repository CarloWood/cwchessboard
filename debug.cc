// cwchessboard -- A C++ chessboard tool set for gtkmm
//
//! @file debug.cc Debug specific code.
//
// Copyright (C) 2006 - 2010 Carlo Wood
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
#include <cstdlib>
#include "debug.h"
#if LIBCWD_THREAD_SAFE
#include <glibmm/thread.h>
#include <cstdio>
#endif

#ifdef CWDEBUG

namespace debug {
  namespace channels {

    namespace dc {

      // Add new debug channels here.
      channel_ct event("EVENT");
      channel_ct motion_event("MOTION_EVENT");
      channel_ct cwchessboardwidget("WIDGET");
      channel_ct place("PLACE");
      channel_ct countboard("COUNTBOARD");
      channel_ct clipboard("CLIPBOARD");
      channel_ct parser("PARSER");

    } // namespace dc

  } // namespace DEBUGCHANNELS

#if LIBCWD_THREAD_SAFE
// Since cwchessboard is a graphical application, only debugging uses cout.
// Therefore we can put the mutex here.
Glib::Mutex* cout_mutex;
#endif

// Initialize debugging code from new threads.
void init_thread(void)
{
  // Everything below needs to be repeated at the start of every
  // thread function, because every thread starts in a completely
  // reset state with all debug channels off etc.

#if LIBCWD_THREAD_SAFE		// For the non-threaded case this is set by the rcfile.
  // Turn on all debug channels by default.
  ForAllDebugChannels(while(!debugChannel.is_on()) debugChannel.on());
  // Turn off specific debug channels.
  Debug(dc::bfd.off());
  Debug(dc::malloc.off());
#endif

  Debug(libcw_do.on());

  // Turn on debug output.
  // Only turn on debug output when the environment variable SUPPRESS_DEBUG_OUTPUT is not set.
  Debug(if (getenv("SUPPRESS_DEBUG_OUTPUT") == NULL) libcw_do.on());
#if LIBCWD_THREAD_SAFE
  if (!Glib::thread_supported())
    Glib::thread_init();
  if (!cout_mutex)
    cout_mutex = new Glib::Mutex;
  Debug(libcw_do.set_ostream(&std::cout, cout_mutex));

  // Set the thread id in the margin.
  char margin[22];
  sprintf(margin, "%-20lu ", pthread_self());
  Debug(libcw_do.margin().assign(margin, 21));
#else
  Debug(libcw_do.set_ostream(&std::cout));
#endif

  // Write a list of all existing debug channels to the default debug device.
  Debug(list_channels_on(libcw_do));
}

// Initialize debugging code from main().
void init(void)
{
  // You want this, unless you mix streams output with C output.
  // Read  http://gcc.gnu.org/onlinedocs/libstdc++/27_io/howto.html#8 for an explanation.
  // We can't use it, because other code uses printf to write to the console.
#if 0
  Debug(set_invisible_on());
  std::ios::sync_with_stdio(false);	// Cause "memory leaks" ([w]cin, [w]cout and [w]cerr filebuf allocations).
  Debug(set_invisible_off());
#endif

  // This will warn you when you are using header files that do not belong to the
  // shared libcwd object that you linked with.
  Debug(check_configuration());

#if CWDEBUG_ALLOC
  // Remove all current (pre- main) allocations from the Allocated Memory Overview.
  libcwd::make_all_allocations_invisible_except(NULL);
#endif

  Debug(read_rcfile());

  init_thread();
}

} // namespace debug

bool open_gdb = false;

#endif // CWDEBUG

