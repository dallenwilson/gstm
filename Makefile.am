## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src pixmaps po gAskpass

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS

EXTRA_DIST = gstm.desktop

desktopdir = $(datadir)/applications
desktop_DATA = gstm.desktop

INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
