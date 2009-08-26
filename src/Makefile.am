bin_PROGRAMS = xfce4-volumed

xfce4_volumed_SOURCES = \
	main.c				\
	xvd_data_types.h	\
	xvd_keys.c			\
	xvd_keys.h			\
	xvd_mixer.c			\
	xvd_mixer.h			\
	xvd_notify.c		\
	xvd_notify.h		\
	xvd_xfconf.c		\
	xvd_xfconf.h		

INCLUDES = 						\
	@GLIB_CFLAGS@				\
	@XFCONF_CFLAGS@				\
	@GSTREAMER_CFLAGS@			\
	@GSTREAMER_AUDIO_CFLAGS@	\
	@XCB_KEYSYMS_CFLAGS@		\
	@LIBNOTIFY_CFLAGS@			

xfce4_volumed_LDADD = 		\
	@GLIB_LIBS@				\
	@XFCONF_LIBS@			\
	@GSTREAMER_LIBS@		\
	@GSTREAMER_AUDIO_LIBS@	\
	@XCB_KEYSYMS_LIBS@		\
	@LIBNOTIFY_LIBS@		
