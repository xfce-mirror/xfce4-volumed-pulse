volumed_pulse_sources = [
  'main.c',
  'xvd_data_types.h',
  'xvd_keys.c',
  'xvd_keys.h',
  'xvd_pulse.c',
  'xvd_pulse.h',
  'xvd_xfconf.c',
  'xvd_xfconf.h',
]

if libnotify.found()
  volumed_pulse_sources += [
    'xvd_notify.c',
    'xvd_notify.h',
  ]
endif

volumed_pulse = executable(
  'xfce4-volumed-pulse',
  volumed_pulse_sources,
  sources: xfce_revision_h,
  dependencies: [
    glib,
    gtk,
    libnotify,
    libpulse,
    libpulsemainloopglib,
    keybinder,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
)
