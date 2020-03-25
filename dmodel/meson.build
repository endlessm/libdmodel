# Copyright 2018 Endless Mobile, Inc.

version = configuration_data()
version.set('DMODEL_MAJOR_VERSION', version_components[0])
version.set('DMODEL_MINOR_VERSION', version_components[1])
version.set('DMODEL_MICRO_VERSION', version_components[2])
version_h = configure_file(configuration: version,
    input: 'dm-version.h.in', output: 'dm-version.h')

installed_headers = [
    'dmodel.h',
    version_h,
    'dm-article.h',
    'dm-audio.h',
    'dm-base.h',
    'dm-content.h',
    'dm-dictionary-entry.h',
    'dm-domain.h',
    'dm-engine.h',
    'dm-image.h',
    'dm-macros.h',
    'dm-media.h',
    'dm-query.h',
    'dm-query-results.h',
    'dm-set.h',
    'dm-utils.h',
    'dm-video.h',
]
private_headers = [
    'dm-content-private.h',
    'dm-database-manager-private.h',
    'dm-domain-private.h',
    'dm-media-private.h',
    'dm-query-private.h',
    'dm-utils-private.h',
]
sources = [
    'dm-article.c',
    'dm-audio.c',
    'dm-base.c',
    'dm-content.c',
    'dm-database-manager.c',
    'dm-dictionary-entry.c',
    'dm-domain.c',
    'dm-engine.c',
    'dm-image.c',
    'dm-media.c',
    'dm-query.c',
    'dm-query-results.c',
    'dm-set.c',
    'dm-utils.c',
    'dm-video.c',
]

include = include_directories('..')

enum_sources = gnome.mkenums_simple('dm-enums', install_header: true,
    install_dir: join_paths(get_option('includedir'), api_name),
    sources: installed_headers)

main_library = library('@0@-@1@'.format(meson.project_name(), api_version),
    enum_sources, sources, installed_headers, private_headers,
    c_args: ['-DG_LOG_DOMAIN="@0@"'.format(namespace_name),
        '-DCOMPILING_DMODEL'],
    dependencies: [gio, glib, gobject, json_glib, libarchive, libendless,
        shard, soup, xapian_glib],
    include_directories: include, install: true,
    link_depends: 'lib@0@.map'.format(meson.project_name()),
    soversion: api_version, version: libtool_version)

introspection_sources = [sources, enum_sources, installed_headers]

gnome.generate_gir(main_library, extra_args: ['--warn-all', '--warn-error'],
    identifier_prefix: 'Dm', include_directories: include,
    includes: ['EosShard-0', 'Gio-2.0', 'GLib-2.0', 'GObject-2.0', 'Json-1.0',
        'Soup-2.4', 'Xapian-2.0'],
    install: true, namespace: namespace_name, nsversion: api_version,
    sources: introspection_sources, symbol_prefix: 'dm')

install_headers(installed_headers, subdir: api_name)
