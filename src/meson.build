project('wayterra', 'c',
  default_options: ['warning_level=3']
)

add_project_arguments('-DWLR_USE_UNSTABLE', language: 'c')

wlroots = dependency('wlroots-0.20', required: true)
wayland_server = dependency('wayland-server', required: true)
xkbcommon = dependency('xkbcommon', required: true)
libdrm = dependency('libdrm', required: true)
glesv2 = dependency('glesv2', required: true)

inc = include_directories('include','third_party')

executable(
  'wayterra',
  'wayterra.c',
  'src/error.c',
  'src/file_utils.c',
  'src/gl_utils.c',
  dependencies: [wlroots, wayland_server, xkbcommon, libdrm, glesv2],
  include_directories: inc,
)
