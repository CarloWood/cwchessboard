# See LICENSE.WTFPL
#
include $(srcdir)/cwm4/root_makefile_top.am

SUBDIRS = @CW_SUBDIRS@ doc

EXTRA_DIST += sys.h CwChessboard-CONST.h CwChessboard.h CwChessboardCodes.h debug.h debug_ostream_operators.h ChessboardWidget.h \
	     Array.h Code.h Move.h BitBoard.h ChessNotation.h MoveIterator.h ChessPosition.h Color.h Index.h \
	     Piece.h Type.h Flags.h PieceIterator.h EnPassant.h CastleFlags.h Direction.h BitBoardTest.h \
	     ChessPositionTest.h CodeTest.h ColorTest.h FlagsTest.h IndexTest.h PieceTest.h TypeTest.h CountBoard.h \
	     MoveIterator.inl  PieceIterator.inl candidates_table.cc direction_table.cc ChessPositionWidget.h Promotion.h \
	     ChessGame.h MetaData.h GameNode.h PgnGame.h PgnGrammar.h chattr.h \
	     PgnDatabase.h PgnGame.h GameNode.h Referenceable.h ChessGame.h MetaData.h MemoryBlockList.h \
	     LICENSE.GPL LICENSE.WTFPL autogen_versions autogen.sh gen.sh
TAGS_FILES = @GLOBAL_TAGS_FILES@

CPPSOURCES = Direction.cc ChessNotation.cc MoveIterator.cc ChessPosition.cc Code.cc CastleFlags.cc
GUISOURCES = $(CPPSOURCES) ChessPositionWidget.cc CwChessboard.cc ChessboardWidget.cc Referenceable.cc MemoryBlockList.cc

# The source code needed for a C application.
TSTC_SRC = tstc.c CwChessboard.c
# The source code needed for a C++ application.
TSTCPP_SRC = tstcpp.cc $(GUISOURCES)
# The source code needed for tstchessposition.
TSTCHESSPOSITION_SRC = tstchessposition.cc $(CPPSOURCES)
# The source code needed for tstbenchmark
TSTBENCHMARK_SRC = tstbenchmark.cc $(CPPSOURCES)
# The source code needed for tstpgnread
TSTPGNREAD_SRC = tstpgnread.cc PgnDatabase.cc chattr.tab.cc MemoryBlockList.cc $(CPPSOURCES)
# The source code needed for tsticonv
TSTICONV_SRC = tsticonv.cc
# The source code needed for tstpgn
TSTPGN_SRC = tstpgn.cc PgnDatabase.cc chattr.tab.cc MemoryBlockList.cc $(CPPSOURCES)
# The source code needed for tstspirit
TSTSPIRIT_SRC = tstspirit.cc

# CppUnit testsuite.
TESTSUITE_SRC = testsuite.cc $(CPPSOURCES)

if LIBCWD_USED
CPPSOURCES += debug.cc debug_ostream_operators.cc
TSTSPIRIT_SRC += debug.cc
endif
if REAL_MAINTAINER
GUISOURCES += gtk_debug_ostream_operators.cc
endif

noinst_PROGRAMS = testsuite tstchessposition tstc tstcpp tstbenchmark tstpgnread tsticonv tstpgn tstspirit

tstc_SOURCES = $(TSTC_SRC)
tstc_CFLAGS = -std=c99 @GTK2_FLAGS@
tstc_LDADD = @GTK2_LIBS@ -lm

tstcpp_SOURCES = $(TSTCPP_SRC)
tstcpp_CXXFLAGS = @CWD_FLAGS@ @gtkmm_CFLAGS@ -DGTKMM -DLIBCWD_THREAD_SAFE=0
tstcpp_LDADD = @CWD_LIBS@ @gtkmm_LIBS@ -lm

tstchessposition_SOURCES = $(TSTCHESSPOSITION_SRC)
tstchessposition_CXXFLAGS = @CWD_FLAGS@ @gtkmm_CFLAGS@ -DGTKMM -DLIBCWD_THREAD_SAFE=0
tstchessposition_LDADD = @CWD_LIBS@ @gtkmm_LIBS@ -lm

testsuite_SOURCES = $(TESTSUITE_SRC)
testsuite_CXXFLAGS = @CWD_FLAGS@ @gtkmm_CFLAGS@ -DGTKMM $(CPPUNIT_CFLAGS) -DLIBCWD_THREAD_SAFE=0
testsuite_LDADD = @CWD_LIBS@ @gtkmm_LIBS@ -lm $(CPPUNIT_LIBS)

tstbenchmark_SOURCES = $(TSTBENCHMARK_SRC)
tstbenchmark_CXXFLAGS = -std=c++11 @CWD_FLAGS@
tstbenchmark_LDADD = @CWD_LIBS@ -lm

tstpgnread_SOURCES = $(TSTPGNREAD_SRC)
tstpgnread_CXXFLAGS = @CWD_R_FLAGS@ @giomm_CFLAGS@ @gthread_CFLAGS@
tstpgnread_LDADD = @CWD_R_LIBS@ -lm @giomm_LIBS@ @gthread_LIBS@

tsticonv_SOURCES = $(TSTICONV_SRC)
tsticonv_CXXFLAGS = @CWD_FLAGS@ @glibmm_CFLAGS@
tsticonv_LDADD = @CWD_LIBS@ -lm @glibmm_LIBS@

tstpgn_SOURCES = $(TSTPGN_SRC)
tstpgn_CXXFLAGS = @CWD_R_FLAGS@ @giomm_CFLAGS@ @gthread_CFLAGS@ --param large-function-growth=500
tstpgn_LDADD = @CWD_R_LIBS@ -lm @giomm_LIBS@ @gthread_LIBS@

tstspirit_SOURCES = $(TSTSPIRIT_SRC)
tstspirit_CXXFLAGS = @CWD_FLAGS@
tstspirit_LDADD = @CWD_LIBS@ -lm

if MAINTAINER_MODE
BUILT_SOURCES = $(srcdir)/gen.cc $(srcdir)/CwChessboard-CONST.h $(srcdir)/PgnGrammar.h chattr.tab.cc sys.h
noinst_PROGRAMS += table_gen

table_gen_SOURCES = table_gen.cc
table_gen_CXXFLAGS =
table_gen_LDFLAGS =

CwChessboard.c:
	ln -sf $(srcdir)/CwChessboard.cc CwChessboard.c

$(srcdir)/gen.cc: $(srcdir)/gen.sh $(srcdir)/CwChessboard.cc
	(cd $(srcdir) && ./gen.sh CwChessboard.cc)

$(srcdir)/CwChessboard-CONST.h: $(srcdir)/gen.cc
	$(CXX) -o $(srcdir)/gen $(srcdir)/gen.cc \
	  && $(srcdir)/gen > $(srcdir)/CwChessboard-CONST.h
	rm -f $(srcdir)/gen

$(srcdir)/PgnGrammar.h: $(srcdir)/PgnGrammar.h.in
	grep -B 1000 'RULE DECLARATION START' $< > $(srcdir)/PgnGrammar.h
	awk 'BEGIN { b=0 } \
	    /definition\(PgnGrammar const& self\)/ { b=1 } \
	    /DEBUG MACROS START/ { b=0 } \
	    /^[[:space:]]*=[[:space:]]*$$/ { if (b) \
	      if (line == "blank" || line == "white_space" || line == "eol" || line == "escaped_data") \
	        printf("      no_tree_rule_t %s;\n", line); \
	      else \
	        printf("      rule_t %s;\n", line); } \
	    { if (b == 1) { sub(/[[:space:]]*/, ""); sub(/[[:space:]]*\/.*/, ""); line=$$0; } }' $< >> $(srcdir)/PgnGrammar.h
	grep -A 1000 'RULE DECLARATION END' $< | grep -B 1000 'DEBUG MACROS START' >> $(srcdir)/PgnGrammar.h
	awk 'BEGIN { b=0 } \
	    /definition\(PgnGrammar const& self\)/ { b=1 } \
	    /DEBUG MACROS START/ { b=0 } \
	    /^[[:space:]]*=[[:space:]]*$$/ { if (b == 1) printf("        BOOST_SPIRIT_DEBUG_TRACE_RULE(%s, TRACE_PGN_GRAMMAR%s);\n", a[1], a[2]) } \
	    { if (b == 1) { sub(/^[[:space:]]*/, ""); \
	      split($$0, a, /[[:space:]\/]*/); \
	      if (length(a[2]) > 0) a[2] = sprintf(" & %s", a[2]); } }' $< >> $(srcdir)/PgnGrammar.h
	grep -A 1000 'DEBUG MACROS END' $< >> $(srcdir)/PgnGrammar.h

chattr.tab.cc: table_gen
	./table_gen > chattr.tab.cc

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
