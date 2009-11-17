desktopdir = $(sysconfdir)/xdg/autostart

# Desktop file for the application
desktop_in_in_files = xfce4-volumed.desktop.in.in
desktop_in_files = $(desktop_in_in_files:.desktop.in.in=.desktop.in)
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

xfce4-volumed.desktop.in: xfce4-volumed.desktop.in.in
	$(AM_V_GEN) ( \
		sed -e "s^@PREFIX@^$(prefix)^" \
			$< > $@ \
	)

EXTRA_DIST = $(desktop_DATA)

