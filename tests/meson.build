# Copyright 2018 Endless Mobile, Inc.

javascript_tests = [
    'dmodel/testArticle.js',
    'dmodel/testArticleArchive.js',
    'dmodel/testAudio.js',
    'dmodel/testContent.js',
    'dmodel/testContentFromJson.js',
    'dmodel/testDatadir.js',
    'dmodel/testDictionaryEntry.js',
    'dmodel/testDomain.js',
    'dmodel/testEngine.js',
    'dmodel/testImage.js',
    'dmodel/testMedia.js',
    'dmodel/testQuery.js',
    'dmodel/testQueryResults.js',
    'dmodel/testSet.js',
    'dmodel/testShardOpenZim.js',
    'dmodel/testUtils.js',
    'dmodel/testVideo.js',
]

jasmine = find_program('jasmine')
include_path = '@0@:@1@'.format(meson.source_root(), meson.build_root())
built_library_path = join_paths(meson.build_root(), meson.project_name())
eknvfs_dir = join_paths(meson.build_root(), 'eknvfs')
test_content_path = join_paths(meson.current_source_dir(), 'testcontent')
tests_environment = environment()
tests_environment.set('GJS_PATH', include_path)
tests_environment.prepend('GI_TYPELIB_PATH', built_library_path)
tests_environment.prepend('LD_LIBRARY_PATH', built_library_path)
tests_environment.set('G_TEST_SRCDIR', meson.current_source_dir())
tests_environment.set('G_TEST_BUILDDIR', meson.current_build_dir())
tests_environment.set('GIO_MODULE_DIR', eknvfs_dir)
tests_environment.prepend('XDG_DATA_DIRS', test_content_path)
tests_environment.set('LC_ALL', 'C')

args = ['--no-config']
if (jasmine_report_argument != '')
    args += [jasmine_report_argument]
endif

foreach test_file : javascript_tests
    srcdir_file = join_paths(meson.current_source_dir(), test_file)
    test(test_file, jasmine, env: tests_environment,
        args: args + [srcdir_file])
endforeach
