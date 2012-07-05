# Initialize Autoconf
AC_PREREQ([2.60])
AC_INIT([Dummy X.org Testing Environment for Google Test],
        [0.3.0],
        [],
        [xorg-gtest])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])

# Initialize Automake
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CXX
AC_PROG_RANLIB

AC_LANG([C++])

# Require X.Org macros 1.17 or later for XORG_ENABLE_INTEGRATION_TESTS
m4_ifndef([XORG_MACROS_VERSION],
	  [m4_fatal([must install xorg-macros 1.17 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.17)
XORG_DEFAULT_OPTIONS
XORG_ENABLE_INTEGRATION_TESTS([yes])
XORG_WITH_DOXYGEN

PKG_CHECK_MODULES(X11, x11)
 
# Check for Google Test
CHECK_GTEST

AS_IF([test "x$have_gtest" != xyes],
      AC_MSG_ERROR([package 'gtest' not found]))

AC_SUBST([GTEST_CPPFLAGS])

# Check if we should include support for evemu
AC_ARG_WITH([evemu],
            [AS_HELP_STRING([--with-evemu],
                            [support Linux input device recording playback (default: enabled if available)])],
            [],
            [with_evemu=check])

# utouch-evemu was renamed to evemu, check for both
PKG_CHECK_MODULES(EVEMU, evemu, [have_evemu=yes],
                  [PKG_CHECK_MODULES(EVEMU, utouch-evemu, [have_evemu=yes], [have_evemu=no])])

AS_IF([test "x$with_evemu" == xyes && test "x$have_evemu" != xyes],
      AC_MSG_ERROR([package 'evemu' not found]))

AM_CONDITIONAL([HAVE_EVEMU], [test "x$have_evemu" = "xyes"])
AS_IF([test "x$have_evemu" = xyes], [AC_DEFINE([HAVE_EVEMU])])

AC_SUBST(SOURCEDIR, ['${prefix}/src/xorg-gtest'])
AC_SUBST(DUMMY_CONF_PATH, ['${datarootdir}/xorg/gtest/dummy.conf'])

# Check if we can build integration tests
AS_IF([test "x$enable_integration_tests" != xno],
      [AC_MSG_CHECKING([for X.org server])]
      [PKG_CHECK_EXISTS([xorg-server],
                        [have_xorg_server=yes],
                        [have_xorg_server=no])]
      [AC_MSG_RESULT([$have_xorg_server])])
AS_IF([test "x$enable_integration_tests" = xyes -a \
       "x$have_xorg_server" != xyes],
      [AC_MSG_ERROR([X.org server required for integration tests])])

AS_IF([test "x$have_xorg_server" = xyes],
      [MODULES=`$PKG_CONFIG --variable=moduledir --print-errors xorg-server`
       test "x$?" = "x0" && have_modules=yes])
AS_IF([test "x$have_modules" = xyes],
      [AC_CHECK_FILE([$MODULES/drivers/dummy_drv.so], [have_dummy_module=yes])])
AS_IF([test "x$enable_integration_tests" = xyes -a \
       "x$have_dummy_module" != xyes],
      [AC_MSG_ERROR([X.org Dummy video driver required for integration tests])])

AM_CONDITIONAL(ENABLE_XORG_GTEST_TESTS, [test "x$have_dummy_module" = xyes ])

AC_CONFIG_FILES([Makefile
                 aclocal/Makefile
                 data/Makefile
                 doc/Makefile
                 examples/Makefile
                 include/Makefile
                 src/Makefile
                 xorg-gtest.pc])

AC_OUTPUT
