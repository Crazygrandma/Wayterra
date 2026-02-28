project('wayterra', 'c',
  default_options: ['warning_level=3']
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wlroots = dependency('wlroots-0.20', required: true)
wayland_server = dependency('wayland-server', required: true)
xkbcommon = dependency('xkbcommon', required: true)
libdrm = dependency('libdrm', required: true)
glesv2 = dependency('glesv2', required: true)

executable(
  'wayterra',
  'wayterra.c',
  'util.h',
  'util.c',
  dependencies: [wlroots, wayland_server, xkbcommon, libdrm, glesv2],
  install: true,
)
