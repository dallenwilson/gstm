## Process this file with automake to produce Makefile.in

uidir = $(pkgdatadir)/ui
ui_DATA = gaskpass.ui


AM_CFLAGS = \
	-Wall -g -Og -ggdb -fno-common \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
        -DPACKAGE_SRC_DIR=\""$(abs_srcdir)"\" \
        -DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
        $(GTK3_CFLAGS)

bin_PROGRAMS = gaskpass

gaskpass_SOURCES = \
	main.c main.h \
	gaskpass.c gaskpass.h

gaskpass_LDFLAGS = -Wl, -export-dynamic

gaskpass_LDADD = $(GTK3_LIBS)
EXTRA_DIST = $(ui_DATA)

# Remove ui directory on uninstall
uninstall-local:
	-rm -r $(uidir)
	-rm -r $(pkgdatadir)
