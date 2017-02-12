# Define package version here.
#
# CW_VERSION_MAJOR               : Increment when the whole library was rewritten; or some VERY MAJOR change was made.
# CW_VERSION_MINOR               : Increment when major changes have occured, like removing interfaces.
# CW_VERSION_REVISION            : Increment every public release; or set to 0 when CW_VERSION_MINOR was incremented.

define(CW_VERSION_MAJOR, 2)
define(CW_VERSION_MINOR, 0)
define(CW_VERSION_REVISION, 10)

# Package name and bug reporting.
define(CW_PACKAGE_NAME, [cwchessboard])
define(CW_BUGREPORT, [carlo@alinoe.com])

# CW_COMPILE_FLAGS may contain warning flags, but also default debug flags (-g*)
# and optimization flags (-O*) that which will be stripped when not required.
define(CW_COMPILE_FLAGS, [-W -Wall -Woverloaded-virtual -Wundef -Wpointer-arith -Wwrite-strings -Winline])
# CW_THREADS can be [no] (single-threaded), [yes] (multi-threaded) or [both] (single and multi-threaded applications).
define(CW_THREADS, [both])

# Include cwm4 header.
m4_include([cwm4/configure_ac_top.m4])

# Set GLOBAL_TAGS_FILES.
if test -z "$CTAGS_ROOT_SRCDIRS"; then
  GLOBAL_TAGS_FILES=
else
  abssrcdir=$(cd $srcdir; pwd)
  GLOBAL_TAGS_FILES="`find $abssrcdir \( -type f -name '*.h' -o -name '*.inl' \) -printf " %h/%f"; \
     ``for d in $CTAGS_ROOT_SRCDIRS; do \
         find $d -follow \( -type f -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cc' \
	     -o -name '*.cpp' -o -name '*.cxx' -o -name '*.C' -o -name '*.inl' \) -printf " %h/%f"; \
       done`"
fi
AC_SUBST(GLOBAL_TAGS_FILES)

# CppUnit testsuite (if you get an error here: apt-get install libcppunit-dev)
AM_PATH_CPPUNIT(1.12.0)

# We use doxygen for documentation.
CW_DOXYGEN

# Detect libgtkmm et al.
PKG_CHECK_MODULES([gtkmm], [gtkmm-2.4 >= 2.8.0])
PKG_CHECK_MODULES([gdk_pixbuf], [gdk-pixbuf-xlib-2.0])
PKG_CHECK_MODULES([giomm], [giomm-2.4])
PKG_CHECK_MODULES([glibmm], [glibmm-2.4])
PKG_CHECK_MODULES([gthread], [gthread-2.0])

# Check for libraries.
CW_LIB_LIBGTK2

# Generate sys.h.
CW_CONFIG_FILE([.], [sys.h])

# Used in sys.h to force recompilation when the compiler version changes.
CW_PROG_CXX_FINGER_PRINTS
CC_FINGER_PRINT="$cw_prog_cc_finger_print"
AC_SUBST([CC_FINGER_PRINT])
CXX_FINGER_PRINT="$cw_prog_cxx_finger_print"
AC_SUBST([CXX_FINGER_PRINT])
CXXCPP_FINGER_PRINT="$cw_prog_cxxcpp_finger_print"
AC_SUBST([CXXCPP_FINGER_PRINT])

# Use in sys.h to force recompilation when CXXFLAGS change.
CXXFLAGS_FINGER_PRINT=[$(echo $CXXFLAGS | sed -e 's/-W[a-z-]* *//g')]
AC_SUBST([CXXFLAGS_FINGER_PRINT])

# Include cwm4 footer.
m4_include([cwm4/configure_ac_bottom.m4])
