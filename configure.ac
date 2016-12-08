dnl Define package version here.
define(CW_VERSION_MAJOR, 2)
define(CW_VERSION_MINOR, 0)
define(CW_VERSION_REVISION, 10)
define(CW_PACKAGE_NAME, cwchessboard)
define(CW_BUGREPORT, carlo@alinoe.com)

dnl Package name and version
AC_INIT(CW_PACKAGE_NAME, CW_VERSION_MAJOR.CW_VERSION_MINOR.CW_VERSION_REVISION, CW_BUGREPORT)

dnl Put resulting configuration defines in this header file.
AC_CONFIG_HEADERS([config.h])

dnl Ensure that the directory specified with --srcdir was correct
dnl by demanding that it contains `CwChessboard.h'.
AC_CONFIG_SRCDIR([CwChessboard.h])

dnl Because we use cwautomacros.
CW_AUTOMACROS([20081012])

dnl Set GLOBAL_TAGS_FILES.
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

dnl Automake options.
AM_INIT_AUTOMAKE

dnl Include maintainer mode targets
AM_MAINTAINER_MODE

dnl Check if we are the real maintainer
AM_CONDITIONAL(REAL_MAINTAINER, test -z "$MAINTAINER_MODE_TRUE" -a x"$TOPPROJECT" = x"/home/carlo/projects/cwchessboard")

if test -z "$REAL_MAINTAINER_TRUE"; then
AC_DEFINE(HAVE_GTK_DEBUG_OSTREAM_OPERATORS_H, [], [Only set if you are the real maintainer])
fi

dnl Demand at least version 2.57 of autoconf.
AC_PREREQ(2.57)

dnl CppUnit testsuite (if you get an error here: apt-get install libcppunit-dev)
AM_PATH_CPPUNIT(1.12.0)

dnl Check for compiler and preprocessor
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CXXCPP

dnl Add --enable-debug (DEBUG, DOXYGEN_DEBUG), --enable-libcwd (CWDEBUG, DOXYGEN_CWDEBUG),
dnl --enable-optimise and --enable-profile options. Update USE_LIBCWD, CWD_LIBS, CWD_FLAGS and CXXFLAGS accordingly.
dnl CW_OPG_FLAGS(-Woverloaded-virtual -Wsign-promo -Waddress -Warray-bounds -Wc++0x-compat -Wchar-subscripts -Wcomment -Wformat -Wmissing-braces -Wparentheses -Wreorder -Wreturn-type -Wsequence-point -Wsign-compare -Wstrict-aliasing -Wstrict-overflow=1 -Wswitch -Wtrigraphs -Wunknown-pragmas -Wunused-function -Wunused-label -Wunused-value -Wunused-variable -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-field-initializers -Wsign-compare -Wtype-limits)
CW_OPG_FLAGS([-Winline])
AM_CONDITIONAL(LIBCWD_USED, [test $USE_LIBCWD = yes])

dnl Checks for other programs.
AC_PROG_INSTALL

dnl We use doxygen for documentation.
CW_DOXYGEN

dnl Detect libgtkmm et al.
PKG_CHECK_MODULES([gtkmm], [gtkmm-2.4 >= 2.8.0])
PKG_CHECK_MODULES([gdk_pixbuf], [gdk-pixbuf-xlib-2.0])
PKG_CHECK_MODULES([giomm], [giomm-2.4])
PKG_CHECK_MODULES([glibmm], [glibmm-2.4])
PKG_CHECK_MODULES([gthread], [gthread-2.0])

if test -z "$REAL_MAINTAINER_FALSE"; then
OUTPUT_DIRECTORY=
fi
AC_SUBST(OUTPUT_DIRECTORY)

dnl Check for libraries.
CW_LIB_LIBGTK2

dnl Generate sys.h.
CW_CONFIG_FILE([.], [sys.h])

dnl Used in sys.h to force recompilation when the compiler version changes.
CW_PROG_CXX_FINGER_PRINTS
CC_FINGER_PRINT="$cw_prog_cc_finger_print"
AC_SUBST([CC_FINGER_PRINT])
CXX_FINGER_PRINT="$cw_prog_cxx_finger_print"
AC_SUBST([CXX_FINGER_PRINT])
CXXCPP_FINGER_PRINT="$cw_prog_cxxcpp_finger_print"
AC_SUBST([CXXCPP_FINGER_PRINT])

dnl Use in sys.h to force recompilation when CXXFLAGS change.
CXXFLAGS_FINGER_PRINT=[$(echo $CXXFLAGS | sed -e 's/-W[a-z-]* *//g')]
AC_SUBST([CXXFLAGS_FINGER_PRINT])

dnl Output the other configuration files.
AC_CONFIG_FILES(
	[Makefile]
	[doc/Makefile]
	[doc/doxygen.config]
	[doc/html.header]
	[doc/html.footer]
	)

AC_OUTPUT
