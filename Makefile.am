## Process this file with automake to produce Makefile.in

SUBDIRS = src gAskpass po

EXTRA_DIST = \
	autogen.sh \
	gSTM.glade \
	gSTM.gladep \
	gstm.desktop

desktopdir = $(datadir)/applications
desktop_DATA = gstm.desktop


install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(datadir)/pixmaps/$(PACKAGE); \
	    fi \
	  done ; \
	  $(INSTALL_DATA) $(srcdir)/pixmaps/gSTM.png $(DESTDIR)$(datadir)/pixmaps; \
	fi

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
