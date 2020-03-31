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

INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	gstm.spec \
	gstm.desktop \
	README.md \
	autogen.sh \
	gstm.anjuta \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache

desktopdir = $(datadir)/applications
desktop_DATA = gstm.desktop

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)
