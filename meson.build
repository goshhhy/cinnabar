project('cinnabar', 'c', default_options : ['buildtype=debugoptimized'])

xcb = dependency('xcb')
sulfur = dependency('sulfur')
executable('cinnabar', 'src/main.c', dependencies : [xcb, sulfur], install : true)
