# Copyright 2018 Endless Mobile, Inc.

libeknvfs_sources = [
    'ekn-file-input-stream-wrapper.c',
    'ekn-file-input-stream-wrapper.h',
    'ekn-file.h',
    'ekn-file.c',
    'ekn-vfs.c',
]

libeknvfs = shared_module('eknvfs', libeknvfs_sources,
    c_args: ['-DG_LOG_DOMAIN="EknVfs"'], dependencies: [gio, gmodule, shard],
    install: true, install_dir: giomoduledir, link_depends: ['libeknvfs.map'])

meson.add_install_script('update_gio_modules.py')

