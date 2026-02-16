project('tinywl', 'c',
  default_options: ['warning_level=3']
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wlroots = dependency('wlroots-0.20', required: true)
wayland_server = dependency('wayland-server', required: true)
xkbcommon = dependency('xkbcommon', required: true)
libdrm = dependency('libdrm', required: true)

executable(
  'wayterra',
  'tinywl.c',
  'config.h',
  'server.h',
  'tinywl_buffer.h',
  'tinywl_input.h',
  'tinywl_output.h',
  'tinywl_xdg.h',
  dependencies: [wlroots, wayland_server, xkbcommon, libdrm],
  install: true,
)
