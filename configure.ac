#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([PSRXML],[1.05],[mkeith@pulsarastronomy.net])

AC_CONFIG_AUX_DIR([autoconf])



SWIN_OPTIONS_SET

#AC_CONFIG_SRCDIR([Makefile.am])
# Create static libraries by default
AC_DISABLE_SHARED

# Enable large-file support
AC_SYS_LARGEFILE

# Determine the machine endian
AC_C_BIGENDIAN

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_CC
AC_PROG_CPP
AM_PROG_CC_C_O

AC_PROG_INSTALL
AC_PROG_LIBTOOL


AM_PATH_XML2(2.0.0, have_xml=yes, have_xml=no)


if test X$have_xml != Xyes; then
	AC_ERROR("Can't find your libxml2 installation")
fi


MJK_LIB_CRYPTO

AC_SUBST(XML_CPPFLAGS)





AM_INIT_AUTOMAKE([foreign subdir-objects])

AM_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/psrxml/Makefile src/tools/Makefile src/tests/Makefile])

AC_OUTPUT
