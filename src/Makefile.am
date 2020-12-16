## Process this file with automake to produce Makefile.in

## Created by Anjuta


uidir = $(pkgdatadir)/ui
ui_DATA = gstm.ui


AM_CFLAGS = \
	-Wall -g -Og -ggdb -fno-common \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(abs_srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	$(GTK3_CFLAGS) $(XML_CFLAGS) $(APPINDICATOR_CFLAGS)

bin_PROGRAMS = gstm

gstm_SOURCES = \
	main.c \
	gstm.h \
	gstm.c \
	callbacks.c \
	callbacks.h \
	main.h \
	interface.c \
	interface.h \
	common.h \
	conffile.h \
	conffile.c \
	fniface.h \
	fniface.c \
	fnssht.c \
	fnssht.h \
	support.c \
	support.h \
	systray.c \
	systray.h	


gstm_LDFLAGS = -Wl,--export-dynamic

gstm_LDADD = $(GTK3_LIBS) $(XML_LIBS) $(APPINDICATOR_LIBS)
EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)

