# $Id$

package_exec = `echo $(PACKAGE) | sed '$(program_transform_name)'`

autostartdir = $(sysconfdir)/xdg/autostart

autostart_in_files = xfce4-volumed-pulse.desktop.in
%.desktop: %.desktop.in
	$(AM_V_GEN) sed -e "s^\@PACKAGE_EXEC\@^$(package_exec)^g" < $< > $@
autostart_DATA = $(autostart_in_files:.desktop.in=.desktop)

install-data-hook:
	if [ x"$(PACKAGE)" != x"$(package_exec)" ]; then \
		mv $(DESTDIR)/$(autostartdir)/xfce4-volumed-pulse.desktop \
			$(DESTDIR)/$(autostartdir)/$(package_exec).desktop ; \
	fi

DISTCLEANFILES = $(autostart_DATA)

EXTRA_DIST = $(autostart_in_files)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
