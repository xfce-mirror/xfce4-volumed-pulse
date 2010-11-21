AC_INIT([xfce4-volumed], [0.1.11], [http://bugs.launchpad.net/xfce4-volumed])
AM_INIT_AUTOMAKE([AC_PACKAGE_TARNAME()], [AC_PACKAGE_VERSION()])

AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER(config.h)

AC_PROG_CC
AM_PROG_CC_C_O

PKG_CHECK_MODULES(GLIB, [glib-2.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(XFCONF, [libxfconf-0])
AC_SUBST(XFCONF_CFLAGS)
AC_SUBST(XFCONF_LIBS)

PKG_CHECK_MODULES(GSTREAMER, [gstreamer-0.10])
AC_SUBST(GSTREAMER_CFLAGS)
AC_SUBST(GSTREAMER_LIBS)

PKG_CHECK_MODULES(GSTREAMER_AUDIO, [gstreamer-audio-0.10])
AC_SUBST(GSTREAMER_AUDIO_CFLAGS)
AC_SUBST(GSTREAMER_AUDIO_LIBS)

PKG_CHECK_MODULES(LIBKEYBINDER, [keybinder])
AC_SUBST(LIBKEYBINDER_CFLAGS)
AC_SUBST(LIBKEYBINDER_LIBS)

AC_ARG_WITH(libnotify,
   AS_HELP_STRING([--with-libnotify], [enable notifications (default=autodetect)]),
   with_libnotify=$withval,
   with_libnotify="autodetect")

AC_ARG_ENABLE(debug,
   [--enable-debug enable debug symbols (default=no)],
   [case "${enableval}" in
      yes)  enable_debug=yes ;;
      no)   enable_debug=no ;;
      *) AC_MSG_ERROR([wrong value "${enableval}" for --enable-debug, use "yes" or "no" (default).]) ;;
esac],[])

HAVE_LIBNOTIFY=0
if test "$with_libnotify" = "autodetect"; then
   PKG_CHECK_EXISTS([libnotify], HAVE_LIBNOTIFY=1, )
fi
if test "$with_libnotify" = "yes" || test "$HAVE_LIBNOTIFY" = "1"; then
   with_libnotify="yes"
   PKG_CHECK_MODULES(LIBNOTIFY, [libnotify])
   HAVE_LIBNOTIFY=1
   AC_SUBST(LIBNOTIFY_CFLAGS)
   AC_SUBST(LIBNOTIFY_LIBS)
   AC_DEFINE(HAVE_LIBNOTIFY, 1, [LIBNOTIFY support])
fi
AC_SUBST(HAVE_LIBNOTIFY)
AM_CONDITIONAL(HAVE_LIBNOTIFY, [test "$HAVE_LIBNOTIFY" = "1"])


if test "x$enable_debug" = "xyes"; then
  CFLAGS="-g"
else
  CFLAGS="-O2 -DNDEBUG"
fi

AC_OUTPUT([
Makefile
src/Makefile
data/Makefile
])

echo "----------------------------------------"
echo "Xfce4-Volumed"
echo "	Installing in : ${prefix}"
echo "	Using LibNotify : ${with_libnotify}"
echo "	Compiler : ${CC}"
echo "	Compiler flags : ${CFLAGS}"
echo ""
echo "	Configuration is done OK."
echo "" 
