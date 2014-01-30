
AC_INIT([spdr], [0.1.0], [https://github.com/rioki/spdr/issues])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AM_SILENT_RULES([yes])
AM_PROG_AR

AC_LANG([C++])

AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11(noext, mandatory)

LT_INIT([shared static win32-dll])

AC_SEARCH_LIBS([socket], [wsock32 ws2_32])

AC_CONFIG_FILES([Makefile spdr.pc])

AC_OUTPUT