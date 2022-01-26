dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_PREREQ([2.69])
AC_INIT(gSTM, [1.3.7], [dwjwilson@lavabit.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC

PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.22])
PKG_CHECK_MODULES(XML, [libxml-2.0])

PKG_CHECK_MODULES(APPINDICATOR, [appindicator3-0.1])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  pixmaps/Makefile
  gAskpass/Makefile
  gstm.spec
])

AC_OUTPUT

