dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_PREREQ([2.69])
AC_INIT(gSTM, [1.3.6], [dwjwilson@lavabit.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
#AM_SILENT_RULES([yes])

AC_PROG_CC

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=gstm
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])

AM_GLIB_GNU_GETTEXT

LT_INIT

PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.22])
PKG_CHECK_MODULES(XML, [libxml-2.0])

PKG_CHECK_MODULES(APPINDICATOR, [appindicator3-0.1])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  pixmaps/Makefile
  po/Makefile.in
  gAskpass/Makefile
  gstm.spec
])

AC_OUTPUT

