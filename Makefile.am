## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src pixmaps gAskpass

ACLOCAL_AMFLAGS = -I m4

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS

EXTRA_DIST = \
	gstm.spec \
	gstm.desktop \
	README.md \
	autogen.sh \
	gstm.anjuta \
	debian

desktopdir = $(datadir)/applications
desktop_DATA = gstm.desktop

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
