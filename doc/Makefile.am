SUBDIRS = .

EXTRA_DIST = main.css mainpage.dox download.dox
BUILT_SOURCES =
TAGFILES =

if REAL_MAINTAINER
HTMLDIR=@OUTPUT_DIRECTORY@/cwchessboard
else
HTMLDIR=cwchessboard

distclean-local:
	rm -rf $(HTMLDIR)
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in

# Needed to work around EXTRA_DIST bug.
dist-hook:
	mkdir $(distdir)/images
	cp -p $(srcdir)/images/*.png $(distdir)/images

maintainer-clean-local:

all-local: doxygen.config html.header html.footer @REAL_MAINTAINER_TRUE@$(top_builddir)/cwchessboard-@VERSION@.tar.gz
	@REAL_MAINTAINER_TRUE@rm -rf $(HTMLDIR)
	test -L $(top_srcdir)/CwChessboard.c || \
	    (chmod u+w $(top_srcdir) && cd $(top_srcdir) && rm -f CwChessboard.c && ln -s CwChessboard.cc CwChessboard.c)
	doxygen doxygen.config 2>&1 | egrep -v '^$$|Pango-WARNING \*\*: Error loading GPOS table 5503' || true
	sed -i -re 's%<h[[:digit:]] .*[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+ *</h[[:digit:]]>%%' $(HTMLDIR)/index.html
	@REAL_MAINTAINER_TRUE@sed -i -e 's%@''VERSION@%@VERSION@%g' $(HTMLDIR)/download.html
	for f in $$(find $(HTMLDIR) -name '*.html'); do \
	  sed -i -re 's%<tr><td colspan="2"><div class="groupText"><br><br></div></td></tr>%%;s%(<!--([^-]*)(-[^-]+)*)--([^>][^-]*(-[^-]+)*-- *>)%\1\&#45\&#45\4%;s% ((&amp;|\*)+)(([^ &]|&[^a])|$$)%\1 \3%g' $$f; \
	done
	mkdir -p $(HTMLDIR)/images
	$(INSTALL) -m 644 $(srcdir)/images/*.png $(HTMLDIR)/images
	mv $(HTMLDIR)/doxygen.png $(HTMLDIR)/images
	$(INSTALL) -m 644 $(srcdir)/main.css $(HTMLDIR)
	@REAL_MAINTAINER_TRUE@$(INSTALL) -m 644 $(top_builddir)/cwchessboard-@VERSION@.tar.gz $(HTMLDIR)

@REAL_MAINTAINER_TRUE@$(top_builddir)/cwchessboard-@VERSION@.tar.gz:
	$(MAKE) $(AM_MAKEFLAGS) -C .. dist

