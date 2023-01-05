# See LICENSE.WTFPL
#
include $(srcdir)/cwm4/root_makefile_top.am

AM_CPPFLAGS = -iquote $(top_builddir) -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

SUBDIRS = @CW_SUBDIRS@ doc

EXTRA_DIST += sys.h CwChessboard-CONST.h CwChessboard.h CwChessboardCodes.h debug.h debug_ostream_operators.h ChessboardWidget.h \
	     Array.h Code.h Move.h BitBoard.h ChessNotation.h MoveIterator.h ChessPosition.h Color.h Index.h \
	     Piece.h Type.h Flags.h PieceIterator.h EnPassant.h CastleFlags.h Direction.h BitBoardTest.h \
	     ChessPositionTest.h CodeTest.h ColorTest.h FlagsTest.h IndexTest.h PieceTest.h TypeTest.h CountBoard.h \
	     MoveIterator.inl  PieceIterator.inl candidates_table.cxx direction_table.cxx ChessPositionWidget.h Promotion.h \
	     ChessGame.h MetaData.h GameNode.h PgnGame.h PgnGrammar.h chattr.h \
	     PgnDatabase.h PgnGame.h GameNode.h Referenceable.h ChessGame.h MetaData.h MemoryBlockList.h \
	     LICENSE.GPL LICENSE.WTFPL autogen_versions autogen.sh gen.sh
TAGS_FILES = @GLOBAL_TAGS_FILES@

CPPSOURCES = Direction.cxx ChessNotation.cxx MoveIterator.cxx ChessPosition.cxx Code.cxx CastleFlags.cxx
GUISOURCES = $(CPPSOURCES) ChessPositionWidget.cxx CwChessboard.cxx ChessboardWidget.cxx Referenceable.cxx MemoryBlockList.cxx

# The source code needed for a C application.
TSTC_SRC = tstc.c CwChessboard.c
# The source code needed for a C++ application.
#TSTCPP_SRC = tstcpp.cxx $(GUISOURCES)
# The source code needed for tstchessposition.
TSTCHESSPOSITION_SRC = tstchessposition.cxx $(CPPSOURCES)
# The source code needed for tstbenchmark
TSTBENCHMARK_SRC = tstbenchmark.cxx $(CPPSOURCES)
# The source code needed for tstpgnread
TSTPGNREAD_SRC = tstpgnread.cxx PgnDatabase.cxx chattr.tab.cpp MemoryBlockList.cxx $(CPPSOURCES)
# The source code needed for tsticonv
TSTICONV_SRC = tsticonv.cxx
# The source code needed for tstpgn
TSTPGN_SRC = tstpgn.cxx PgnDatabase.cxx chattr.tab.cpp MemoryBlockList.cxx $(CPPSOURCES)
# The source code needed for tstspirit
TSTSPIRIT_SRC = tstspirit.cxx

# CppUnit testsuite.
TESTSUITE_SRC = testsuite.cxx $(CPPSOURCES)

if LIBCWD_USED
CPPSOURCES += debug.cxx debug_ostream_operators.cxx
endif

#noinst_PROGRAMS = testsuite tstchessposition tstc tstcpp tstbenchmark tstpgnread tsticonv tstpgn tstspirit
noinst_PROGRAMS = testsuite tstchessposition tstc tstbenchmark tstpgnread tsticonv tstpgn tstspirit

tstc_SOURCES = $(TSTC_SRC)
tstc_CFLAGS = -std=c99 @GTK2_FLAGS@ @GLIB2_CFLAGS@
tstc_LDADD = @GTK2_LIBS@ @GLIB2_LIBS@ -lm

#tstcpp_SOURCES = $(TSTCPP_SRC)
#tstcpp_CXXFLAGS = @LIBCWD_R_FLAGS@ @gtkmm_CFLAGS@
#tstcpp_LDADD = cwds/libcwds_r.la @gtkmm_LIBS@ -lm

tstchessposition_SOURCES = $(TSTCHESSPOSITION_SRC)
tstchessposition_CXXFLAGS = @LIBCWD_R_FLAGS@
tstchessposition_LDADD = cwds/libcwds_r.la

testsuite_SOURCES = $(TESTSUITE_SRC)
testsuite_CXXFLAGS = @LIBCWD_R_FLAGS@ $(CPPUNIT_CFLAGS)
testsuite_LDADD = cwds/libcwds_r.la $(CPPUNIT_LIBS)

tstbenchmark_SOURCES = $(TSTBENCHMARK_SRC)
tstbenchmark_CXXFLAGS = -std=c++20 -DLIBCWD_THREAD_SAFE=0 @LIBCWD_FLAGS@
tstbenchmark_LDADD = cwds/libcwds.la

tstpgnread_SOURCES = $(TSTPGNREAD_SRC)
tstpgnread_CXXFLAGS = -DLIBCWD_THREAD_SAFE=0 @LIBCWD_FLAGS@ @giomm_CFLAGS@
tstpgnread_LDADD = cwds/libcwds.la -lboost_system @giomm_LIBS@

tsticonv_SOURCES = $(TSTICONV_SRC)
tsticonv_CXXFLAGS = @LIBCWD_R_FLAGS@ @glibmm_CFLAGS@
tsticonv_LDADD = cwds/libcwds_r.la @glibmm_LIBS@

tstpgn_SOURCES = $(TSTPGN_SRC)
tstpgn_CXXFLAGS = @LIBCWD_R_FLAGS@ --param large-function-growth=500 @giomm_CFLAGS@
tstpgn_LDADD = cwds/libcwds_r.la -lboost_system @giomm_LIBS@

tstspirit_SOURCES = $(TSTSPIRIT_SRC)
tstspirit_CXXFLAGS = -DLIBCWD_THREAD_SAFE=0 @LIBCWD_FLAGS@
tstspirit_LDADD = cwds/libcwds.la -lboost_system

if MAINTAINER_MODE
BUILT_SOURCES = $(srcdir)/gen.cpp $(srcdir)/CwChessboard-CONST.h PgnGrammar.h chattr.tab.cpp CwChessboard.c sys.h
noinst_PROGRAMS += table_gen

table_gen_SOURCES = table_gen.cxx
table_gen_CXXFLAGS =
table_gen_LDFLAGS =

CwChessboard.c:
	ln -sf $(srcdir)/CwChessboard.cxx CwChessboard.c

$(srcdir)/gen.cpp: $(srcdir)/gen.sh $(srcdir)/CwChessboard.cxx
	(cd $(srcdir) && ./gen.sh CwChessboard.cxx)

$(srcdir)/CwChessboard-CONST.h: $(srcdir)/gen.cpp
	$(CXX) -o $(srcdir)/gen $(srcdir)/gen.cpp \
	  && $(srcdir)/gen > $(srcdir)/CwChessboard-CONST.h
	rm -f $(srcdir)/gen

PgnGrammar.h: $(srcdir)/PgnGrammar.h.in
	$(srcdir)/generate_PgnGrammar.h.sh

chattr.tab.cpp: table_gen
	./table_gen > chattr.tab.cpp

if REAL_MAINTAINER
bumpversion:
	if test $$(cat $(srcdir)/.last-release) -eq $$(grep '^define(CW_VERSION_REVISION' $(srcdir)/configure.ac | sed -e 's/[^0-9]*//g'); then \
	  sed -i -r -e 's/^(define\(CW_VERSION_REVISION,[[:space:]]*)([[:digit:]]*)(.*)/\1'$$(($$(cat $(srcdir)/.last-release) + 1))'\3/' $(srcdir)/configure.ac; \
	  $(MAKE) configure; \
	fi

mydistcheck:
	$(MAKE) CXXFLAGS="-g -O2" distcheck

release: bumpversion dist mydistcheck all
	rsync -rtz --copy-unsafe-links --chmod=go+r -e /usr/bin/ssh --delete \
	    --exclude=.svn --exclude=env.source ~/www/cwchessboard/. xs1:WWW/cwchessboard && \
	grep '^define(CW_VERSION_REVISION' $(srcdir)/configure.ac | sed -e 's/[^0-9]*//g' > $(srcdir)/.last-release
endif

all-local:
	@if test -e $(top_srcdir)/../.libcwdrc-src -a ! -e .libcwdrc; then \
	  echo "ln -s $(top_srcdir)/../.libcwdrc-src .libcwdrc"; \
	  ln -s $(top_srcdir)/../.libcwdrc-src .libcwdrc; \
	fi
	@if test -e $(top_srcdir)/../.gdbinit-src -a ! -e .gdbinit; then \
	  echo "ln -s $(top_srcdir)/../.gdbinit-src .gdbinit"; \
	  ln -s $(top_srcdir)/../.gdbinit-src .gdbinit; \
	fi
endif

clean-local:
	rm -f core.[0-9]*
	test ! -L .gdbinit || rm .gdbinit
	test ! -L .libcwdrc || rm .libcwdrc
	@MAINTAINER_MODE_TRUE@rm -f gdb.cmds troep* *.cgd @PACKAGE@-@VERSION@.tar.gz

distclean-local:
	rm -f $(srcdir)/config.h.in~ sys.h
	@MAINTAINER_MODE_TRUE@rm -f CwChessboard.c

include $(srcdir)/cwm4/root_makefile_bottom.am
