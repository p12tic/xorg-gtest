# Checks whether the gtest library is available on the system
# Allows for adjusting the include and library path.
# Sets have_gtest=yes if the library is present and
# reports the compiler and linker flags in 
# GTEST_CXXFLAGS AND GTEST_LDFLAGS, respectively.
AC_DEFUN([AC_CHECK_GTEST],
[
  AC_ARG_WITH([gtest-include-path],
  [AS_HELP_STRING([--with-gtest-include-path],
    [location of the Google test headers, defaults to /usr/include])],
  [GTEST_CXXFLAGS="-I$withval"],
  [GTEST_CXXFLAGS='-I/usr/include'])

  AC_ARG_WITH([gtest-lib-path],
  [AS_HELP_STRING([--with-gtest-lib-path], [location of the Google test libraries])],
  [GTEST_LDFLAGS="-L$withval -lpthread"],
  [GTEST_LDFLAGS='-lgtest -lpthread'])

  AC_HAVE_LIBRARY( [gtest], 
                [have_gtest=yes], 
                 AC_MSG_WARN([package 'gtest' not found: tests disabled]))

]) # AC_CHECK_GTEST

