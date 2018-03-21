# Copyright 2018 Endless Mobile, Inc.

project('eos-knowledge-content', 'c', version: '0.0.0', license: 'LGPL2.1+',
    meson_version: '>= 0.42.0',
    default_options: ['c_std=c99', 'warning_level=3'])

gnome = import('gnome')
pkg = import('pkgconfig')

version_components = meson.project_version().split('.')
api_version = version_components[0]
api_name = '@0@-@1@'.format(meson.project_name(), api_version)
libtool_version = '0.0.0'
subdir_name = 'ekncontent'  # FIXME: make this equal to project_name
namespace_name = 'EosKnowledgeContent'

# Build options

jasmine_report_argument = ''
junit_reports_dir = get_option('jasmine_junit_reports_dir')
if junit_reports_dir != ''
    jasmine_report_argument = '--junit=@0@/${log/.log/.js.xml}'.format(junit_reports_dir)
endif

# Dependencies

libendless_req = '>= 0.5'
shard_req = '>= 0.1.2'
xapian_glib_req = '>= 1.7'

gio = dependency('gio-2.0')
glib = dependency('glib-2.0')
gmodule = dependency('gmodule-2.0')
gobject = dependency('gobject-2.0')
json_glib = dependency('json-glib-1.0')
libendless = dependency('endless-0', version: libendless_req)
shard = dependency('eos-shard-0', version: shard_req)
soup = dependency('libsoup-2.4')
xapian_glib = dependency('xapian-glib-1.0', version: xapian_glib_req)

giomoduledir = gio.get_pkgconfig_variable('giomoduledir')

subdir('eknvfs')
subdir(subdir_name)

# Dependency object for gtk-doc and tests to use
main_library_dependency = declare_dependency(dependencies: [glib, gobject],
    include_directories: include, link_with: main_library)

# Data files

install_data('overrides/@0@.js'.format(namespace_name),
    install_dir: join_paths(get_option('datadir'), 'gjs-1.0', 'overrides'))

# Generated Files

config = configuration_data()
config.set_quoted('EKN_CONTENT_VERSION', meson.project_version())
configure_file(configuration: config, output: 'config.h')

requires = ['glib-2.0', 'gio-2.0', 'gobject-2.0', 'json_glib-1.0', 'soup-2.4']
requires_private = [
    'eos-shard-0 @0@'.format(shard_req),
    'xapian_glib-1.0 @0@'.format(xapian_glib_req),
    'endless-0 @0@'.format(libendless_req),
]
pkg.generate(filebase: api_name, libraries: [main_library],
    description: 'Software development kit for applications for the developing world',
    name: meson.project_name(), subdirs: api_name, requires: requires,
    requires_private: requires_private,
    url: 'http://endlessm.github.io/eos-knowledge-lib',
    version: meson.project_version())

subdir('tests')

if get_option('documentation')
    subdir('docs/reference/@0@'.format(subdir_name))
endif

# Summary

message('\n'.join([
    '@0@ @1@ (@2@)'.format(meson.project_name(), meson.project_version(),
        api_name),
    '-------------------',
    'Options:',
    '     Documentation: @0@'.format(get_option('documentation')),
    '  Test reports dir: @0@'.format(get_option('jasmine_junit_reports_dir')),
    '',
    'Directories:',
    '    Install prefix: @0@'.format(get_option('prefix')),
    '   GIO Modules dir: @0@'.format(giomoduledir),
    ''
]))

