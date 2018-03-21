# Copyright 2018 Endless Mobile, Inc.

version = configuration_data()
version.set('EKN_CONTENT_MAJOR_VERSION', version_components[0])
version.set('EKN_CONTENT_MINOR_VERSION', version_components[1])
version.set('EKN_CONTENT_MICRO_VERSION', version_components[2])
version_h = configure_file(configuration: version,
    input: 'eknc-version.h.in', output: 'eknc-version.h')

installed_headers = [
    'eos-knowledge-content.h',
    version_h,
    'eknc-article-object-model.h',
    'eknc-audio-object-model.h',
    'eknc-content-object-model.h',
    'eknc-dictionary-object-model.h',
    'eknc-domain.h',
    'eknc-engine.h',
    'eknc-image-object-model.h',
    'eknc-media-object-model.h',
    'eknc-object-model.h',
    'eknc-query-object.h',
    'eknc-query-results.h',
    'eknc-set-object-model.h',
    'eknc-utils.h',
    'eknc-video-object-model.h',
]
private_headers = [
    'eknc-content-object-model-private.h',
    'eknc-database-manager-private.h',
    'eknc-domain-private.h',
    'eknc-media-object-model-private.h',
    'eknc-query-object-private.h',
    'eknc-utils-private.h',
]
sources = [
    'eknc-article-object-model.c',
    'eknc-audio-object-model.c',
    'eknc-content-object-model.c',
    'eknc-database-manager.c',
    'eknc-dictionary-object-model.c',
    'eknc-domain.c',
    'eknc-engine.c',
    'eknc-image-object-model.c',
    'eknc-media-object-model.c',
    'eknc-object-model.c',
    'eknc-query-object.c',
    'eknc-query-results.c',
    'eknc-set-object-model.c',
    'eknc-utils.c',
    'eknc-video-object-model.c',
]

include = include_directories('..')

enum_sources = gnome.mkenums_simple('eknc-enums', install_header: true,
    install_dir: join_paths(get_option('includedir'), api_name),
    sources: installed_headers)

main_library = library('@0@-@1@'.format(meson.project_name(), api_version),
    enum_sources, sources, installed_headers, private_headers,
    c_args: ['-DG_LOG_DOMAIN="@0@"'.format(namespace_name),
        '-DCOMPILING_EKN_CONTENT'],
    dependencies: [gio, glib, gobject, json_glib, libendless, shard, soup,
        xapian_glib],
    include_directories: include, install: true,
    link_depends: 'lib@0@.map'.format(subdir_name), soversion: api_version,
    version: libtool_version)

introspection_sources = [sources, enum_sources, installed_headers]

gnome.generate_gir(main_library, extra_args: ['--warn-all', '--warn-error'],
    identifier_prefix: 'Eknc', include_directories: include,
    includes: ['EosShard-0', 'Gio-2.0', 'GLib-2.0', 'GObject-2.0', 'Json-1.0',
        'Soup-2.4', 'Xapian-1.0'],
    install: true, namespace: namespace_name, nsversion: api_version,
    sources: introspection_sources, symbol_prefix: 'eknc')

install_headers(installed_headers, subdir: api_name)
