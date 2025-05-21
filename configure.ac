dnl ***************************
dnl *** Version information ***
dnl ***************************
XDT_VERSION_INIT([0.3.0])
m4_define([copyright_year], [2025])

dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_COPYRIGHT([Copyright (c) 2009-copyright_year() The Xfce development team. All rights reserved.])
AC_INIT([xfce4-volumed-pulse], [xdt_version], [https://gitlab.xfce.org/apps/xfce4-volumed-pulse])
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIR([m4])
AC_REVISION([xdt_version_build])
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 no-dist-gzip dist-bzip2 tar-ustar foreign])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl *******************************
dnl *** Check for UNIX variants ***
dnl *******************************
AC_USE_SYSTEM_EXTENSIONS
AC_SEARCH_LIBS([strerror],[cposix])

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.4])
LT_INIT([disable-static])

dnl translations
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl ********************************
dnl *** Support program renaming ***
dnl ********************************
AC_ARG_PROGRAM()

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
AM_PROG_CC_C_O
LT_PATH_LD()
AC_PROG_INSTALL()

dnl **********************************
dnl *** Check for standard headers ***
dnl **********************************
AC_CHECK_HEADERS([errno.h stdlib.h unistd.h])
AC_CHECK_FUNCS([daemon setsid])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.66])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.24])
XDT_CHECK_PACKAGE([XFCONF], [libxfconf-0], [4.18.0])
XDT_CHECK_PACKAGE([LIBPULSE], [libpulse], [0.9.19])
XDT_CHECK_PACKAGE([LIBPULSEMAINLOOPGLIB], [libpulse-mainloop-glib], [0.9.19])
XDT_CHECK_PACKAGE([LIBKEYBINDER], [keybinder-3.0], [0.2.0])

dnl **************************************
dnl *** Optional support for Libnotify ***
dnl **************************************
XDT_CHECK_OPTIONAL_PACKAGE([LIBNOTIFY], [libnotify], [0.1.3],
                           [libnotify], [Notification support])

dnl ***********************************
dnl *** Check for debugging support ***
dnl ***********************************
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl **************************************
dnl *** Check for linker optimizations ***
dnl **************************************
XDT_FEATURE_LINKER_OPTS()

dnl *********************************
dnl *** Substitute platform flags ***
dnl *********************************
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
data/Makefile
src/Makefile
po/Makefile.in
])
AC_OUTPUT

dnl ***************************
dnl *** Print configuration ***
dnl ***************************
echo
echo "Build Configuration:"
echo
echo "* Installation prefix:       $prefix"
echo "* Debug Support:             $enable_debug"
if test x"$LIBNOTIFY_FOUND" = x"yes"; then
echo "* Libnotify support:         yes"
else
echo "* Libnotify support:         no"
fi
