bin_PROGRAMS = xfce4-volumed

xfce4_volumed_SOURCES = \
	main.c				\
	xvd_data_types.h	\
	xvd_keys.c			\
	xvd_keys.h			\
	xvd_pulse.c			\
	xvd_pulse.h			\
	xvd_notify.c		\
	xvd_notify.h		\
	xvd_xfconf.c		\
	xvd_xfconf.h

INCLUDES = 							\
	@GLIB_CFLAGS@					\
	@XFCONF_CFLAGS@					\
	@LIBPULSE_CFLAGS@				\
	@LIBPULSEMAINLOOPGLIB_CFLAGS@	\
	@LIBKEYBINDER_CFLAGS@			\
	@LIBNOTIFY_CFLAGS@

xfce4_volumed_LDADD = 			\
	@GLIB_LIBS@					\
	@XFCONF_LIBS@				\
	@LIBPULSE_LIBS@				\
	@LIBPULSEMAINLOOPGLIB_LIBS@	\
	@LIBKEYBINDER_LIBS@			\
	@LIBNOTIFY_LIBS@

