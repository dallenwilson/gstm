## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_BINDIR=\""$(prefix)/bin"\" \
	@PACKAGE_CFLAGS@
	$(XML_CFLAGS)
	$(X11_CFLAGS)

bin_PROGRAMS = gstm

gstm_SOURCES = \
	main.c common.h \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	conffile.c conffile.h \
	fniface.c fniface.h \
	fnssht.c fnssht.h \
	notarea.c eggtrayicon.c eggtrayicon.h

gstm_LDADD = @PACKAGE_LIBS@ $(XML_LIBS) $(X11_LIBS) $(INTLLIBS)

