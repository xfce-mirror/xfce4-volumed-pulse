# $Id$

autostartdir = $(sysconfdir)/xdg/autostart

autostart_in_files = xfce4-volumed-pulse.desktop.in
autostart_DATA = $(autostart_in_files:.desktop.in=.desktop)
%.desktop: %.desktop.in Makefile
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

DISTCLEANFILES = $(autostart_DATA) $(autostart_in_files)

EXTRA_DIST = $(autostart_in_files)	\
	meson.build

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
