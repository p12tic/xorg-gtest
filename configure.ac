# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([Dummy X.org Testing Environment for Google Test],
        [0.1.0],
        [],
        [xorg-gtest])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])

m4_include([m4/gtest.m4])

# Initialize Automake
AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE

LIB_VERSION=0:1:0
AC_SUBST([LIB_VERSION])

# Initialize libtool
AC_PROG_LIBTOOL

LT_INIT

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES( XSERVER, x11 [xorg-server] )
 
# Check for Google Test
AC_CHECK_GTEST

AM_CONDITIONAL([HAVE_GTEST], [test "x$have_gtest" = xyes])

AC_SUBST([GTEST_LDFLAGS])
AC_SUBST([GTEST_CXXFLAGS])

AC_CONFIG_FILES([Makefile])
                
AC_SUBST(AM_CPPFLAGS, "-Wall -Werror")

AC_OUTPUT
