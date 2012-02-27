# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([Dummy X.org Testing Environment for Google Test],
        [0.1.1],
        [],
        [xorg-gtest])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])

m4_include([m4/gtest.m4])

# Initialize Automake
AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

LIB_VERSION=0:0:0
AC_SUBST([LIB_VERSION])

# Initialize libtool
AC_PROG_LIBTOOL

LT_INIT

# Require X.Org macros 1.16 or later for XORG_TESTSET_CFLAG
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.16 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.16)
XORG_DEFAULT_OPTIONS

# Checks for programs.
AC_PROG_CXX
AC_PROG_INSTALL

PKG_CHECK_MODULES( XSERVER, x11 [xorg-server] )
 
# Check for Google Test
AC_CHECK_GTEST

AS_IF([test "x$ac_cv_lib_gtest_main" != xyes],
      AC_MSG_ERROR([package 'gtest' not found]))

AC_SUBST([GTEST_CPPFLAGS])

AC_SUBST(DUMMY_CONF_PATH, "$datadir/xorg/gtest/dummy.conf")

AC_CONFIG_FILES([Makefile
                 xorg-gtest.pc])
                
AC_SUBST(AM_CPPFLAGS, "-Wall -Werror")

AC_CONFIG_FILES([doc/Makefile examples/Makefile src/Makefile])

AC_OUTPUT
