dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_PREREQ([2.69])
AC_INIT(gSTM, [1.3.7], [dwjwilson@lavabit.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC

PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.22])

PKG_CHECK_MODULES(XML, [libxml-2.0])

#PKG_CHECK_MODULES(APPINDICATOR, [appindicator3-0.1])

a=0
AC_CHECK_LIB(ayatana-appindicator3, app_indicator_new, [a=1])

if test $a == 1
then
    APPINDICATOR_CFLAGS="-I/usr/include/libayatana-appindicator3-0.1"
    APPINDICATOR_LIBS="-layatana-appindicator3"
    AC_DEFINE([AYATANA],[],[Use libayatana-appindicator rather than libappindicator.])

else
	AC_CHECK_LIB(appindicator3, app_indicator_new,[a=1])
	
	if test $a == 1
	then
		APPINDICATOR_CFLAGS="-I/usr/include/libappindicator3-0.1"
		APPINDICATOR_LIBS="-lappindicator3"
	fi
fi

if test $1 == 0
then
	AC_MSG_ERROR([An appindicator library is required. Install either libappindicator or libayatana-appindicator.])
fi

AC_SUBST([APPINDICATOR_CFLAGS])
AC_SUBST([APPINDICATOR_LIBS])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  pixmaps/Makefile
  gAskpass/Makefile
  gstm.spec
])

AC_OUTPUT

