INCLUDES = \
	-I${top_srcdir} \
	-DG_LOG_DOMAIN=\"xfce4-volumed-pulse\" \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = \
	xfce4-volumed-pulse

xfce4_volumed_pulse_SOURCES = \
	main.c \
	xvd_data_types.h \
	xvd_keys.c \
	xvd_keys.h \
	xvd_pulse.c \
	xvd_pulse.h \
	xvd_xfconf.c \
	xvd_xfconf.h \

xfce4_volumed_pulse_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(XFCONF_CFLAGS) \
	$(LIBKEYBINDER_CFLAGS) \
	$(LIBPULSE_CFLAGS) \
	$(LIBPULSEMAINLOOPGLIB_CFLAGS) \
	$(PLATFORM_CFLAGS)

xfce4_volumed_pulse_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)

xfce4_volumed_pulse_LDADD = \
	$(GLIB_LIBS) \
	$(XFCONF_LIBS) \
	$(LIBKEYBINDER_LIBS) \
	$(LIBPULSE_LIBS) \
	$(LIBPULSEMAINLOOPGLIB_LIBS)

#
# Optional support for libnotify
#
if HAVE_LIBNOTIFY
xfce4_volumed_pulse_SOURCES += \
	xvd_notify.c \
	xvd_notify.h

xfce4_volumed_pulse_CFLAGS += \
	$(LIBNOTIFY_CFLAGS)

xfce4_volumed_pulse_LDADD += \
	$(LIBNOTIFY_LIBS)
endif

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
