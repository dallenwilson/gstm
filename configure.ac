#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Basic package setup
AC_PREREQ([2.69])
AC_INIT([gstm], [1.2], [dwjwilson@lavabit.com])
AC_CONFIG_SRCDIR([src/common.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES(PACKAGE, libgnomeui-2.0)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.6)
PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)
PKG_CHECK_MODULES(X11, x11)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 mkdir setenv strcasecmp])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAG_LIBS)

# Translation stuff
GETTEXT_PACKAGE=gstm
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  pixmaps/Makefile
  po/Makefile.in
])

AC_OUTPUT

