#ifndef DEBUG_H
#define DEBUG_H

#ifndef CWDEBUG

#ifdef __cplusplus
#include <iostream>     // std::cerr
#include <cstdlib>      // std::exit, EXIT_FAILURE
#else
#include <stdio.h>	// fprintf
#include <stdlib.h>	// exit, EXIT_FAILURE
#endif

#define AllocTag1(p)
#define AllocTag2(p, desc)
#define AllocTag_dynamic_description(p, data)
#define AllocTag(p, data)
#define Debug(STATEMENT...)
#define Dout(cntrl, data)
#define DoutEntering(a, b)
#define DoutFatal(cntrl, data) LibcwDoutFatal(, , cntrl, data)
#define ForAllDebugChannels(STATEMENT...)
#define ForAllDebugObjects(STATEMENT...)
#define LibcwDebug(dc_namespace, STATEMENT...)
#define LibcwDout(dc_namespace, d, cntrl, data)
#ifdef __cplusplus
#define LibcwDoutFatal(dc_namespace, d, cntrl, data) do { ::std::cerr << data << ::std::endl; ::std::exit(EXIT_FAILURE); } while(1)
#else
#define LibcwDoutFatal(dc_namespace, d, cntrl, data) do { fprintf(stderr, "DoutFatal\n"); exit(EXIT_FAILURE); } while(1)
#endif
#define LibcwdForAllDebugChannels(dc_namespace, STATEMENT...)
#define LibcwdForAllDebugObjects(dc_namespace, STATEMENT...)
#define NEW(x) new x
#define CWDEBUG_ALLOC 0
#define CWDEBUG_MAGIC 0
#define CWDEBUG_LOCATION 0
#define CWDEBUG_LIBBFD 0
#define CWDEBUG_DEBUG 0
#define CWDEBUG_DEBUGOUTPUT 0
#define CWDEBUG_DEBUGM 0
#define CWDEBUG_DEBUGT 0
#define CWDEBUG_MARKER 0

#else // CWDEBUG

// This must be defined before <libcwd/debug.h> is included and must be the
// name of the namespace containing your `dc' (Debug Channels) namespace
// (see below).  You can use any namespace(s) you like, except existing
// namespaces (like ::, ::std and ::libcwd).
#define DEBUGCHANNELS ::debug::channels
#include <libcwd/debug.h>

namespace debug {

  void init(void);		// Initialize debugging code from main().
  void init_thread(void);	// Initialize debugging code from new threads.

  namespace channels {		// This is the DEBUGCHANNELS namespace, see above.
    namespace dc {		// 'dc' is defined inside DEBUGCHANNELS.

      using namespace libcwd::channels::dc;
      using libcwd::channel_ct;

      // Add the declaration of new debug channels here
      // and add their definition in a custom debug.cc file.
      extern channel_ct event;
      extern channel_ct motion_event;
      extern channel_ct cwchessboardwidget;
      extern channel_ct place;
      extern channel_ct countboard;
      extern channel_ct clipboard;
      extern channel_ct parser;

    } // namespace dc
  } // namespace DEBUGCHANNELS

//! @brief Interface for marking scopes with indented debug output.
//
// Creation of the object increments the debug indentation. Destruction
// of the object automatically decrements the indentation again.
struct Indent {
  int M_indent;                 //!< The extra number of spaces that were added to the indentation.
  //! Construct an Indent object.
  Indent(int indent) : M_indent(indent) { if (M_indent > 0) libcwd::libcw_do.inc_indent(M_indent); }
  //! Destructor.
  ~Indent() { if (M_indent > 0) libcwd::libcw_do.dec_indent(M_indent); }
};

} // namespace debug

//! Debugging macro.
//
// Print "Entering " << \a data to channel \a cntrl and increment
// debugging output indentation until the end of the current scope.
#define DoutEntering(cntrl, data) \
  int __gtktest_debug_indentation = 2;                                                                                  \
  {                                                                                                                     \
    LIBCWD_TSD_DECLARATION;                                                                                             \
    if (LIBCWD_DO_TSD_MEMBER_OFF(::libcwd::libcw_do) < 0)                                                               \
    {                                                                                                                   \
      ::libcwd::channel_set_bootstrap_st __libcwd_channel_set(LIBCWD_DO_TSD(::libcwd::libcw_do) LIBCWD_COMMA_TSD);      \
      bool on;                                                                                                          \
      {                                                                                                                 \
        using namespace LIBCWD_DEBUGCHANNELS;                                                                           \
        on = (__libcwd_channel_set|cntrl).on;                                                                           \
      }                                                                                                                 \
      if (on)                                                                                                           \
        Dout(cntrl, "Entering " << data);                                                                               \
      else                                                                                                              \
        __gtktest_debug_indentation = 0;                                                                                \
    }                                                                                                                   \
  }                                                                                                                     \
  debug::Indent __gtktest_debug_indent(__gtktest_debug_indentation);

// A global variable used to signal that we want to open gdb at some other point.
extern bool open_gdb;

#endif // CWDEBUG

#include <cassert>
#include "debug_ostream_operators.h"
#if defined(GTKMM) && defined(HAVE_GTK_DEBUG_OSTREAM_OPERATORS_H)
#include "gtk_debug_ostream_operators.h"
#endif

#endif // DEBUG_H

