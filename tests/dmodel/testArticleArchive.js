const {DModel, Gio, GLib} = imports.gi;

const InstanceOfMatcher = imports.tests.InstanceOfMatcher;

const ByteArray = imports.byteArray;

describe ('Article Object Model With Archive', function () {
    let domain, archive_model, tempdir;

    beforeAll(function () {
        tempdir = GLib.Dir.make_tmp('dmodel-test-domain-XXXXXX');
        GLib.setenv('XDG_DATA_HOME', tempdir, true);
    });

    beforeEach(function (done) {
        jasmine.addMatchers(InstanceOfMatcher.customMatchers);
        domain = new DModel.Domain({
            app_id: 'com.endlessm.fake_test_app.en',
        });
        domain.init(null);

        let shards = domain.get_shards();
        DModel.default_vfs_set_shards(shards);

        domain.get_object("ekn:///c8c307a582fbbfd835ccc3888fece34711ed8c68", null, function (domain, result) {
            archive_model = domain.get_object_finish(result);
            done();
        });
    });

    afterEach(function () {
        function clean_out(file, cancellable) {
            let enumerator = file.enumerate_children('standard::*',
                Gio.FileQueryInfoFlags.NOFOLLOW_SYMLINKS, cancellable);
            let info;
            while ((info = enumerator.next_file(cancellable))) {
                let child = enumerator.get_child(info);
                if (info.get_file_type() === Gio.FileType.DIRECTORY)
                    clean_out(child, cancellable);
                child.delete(cancellable);
            }
        }
        clean_out(Gio.File.new_for_path(tempdir), null);
    });

    afterAll(function () {
        Gio.File.new_for_path(tempdir).delete(null);
    });

    describe ('Model', function () {
        it ('is the correct class', function () {
            expect(archive_model).not.toBe(null);
            expect(archive_model).toBeA(DModel.Article);
        });
    });

    describe ('get_archive_member_content_stream', function () {
        it ('returns a stream for index.html', function () {
            let stream = archive_model.get_archive_member_content_stream('index.html');
            expect(stream).not.toBe(null);
            expect(stream).toBeA(Gio.InputStream);
        });

        it ('returns null for a non-existent member', function () {
            let stream = archive_model.get_archive_member_content_stream('does-not-exist');
            expect(stream).toBe(null);
        });

        it ('returns valid content for index.html', function () {
            let stream = archive_model.get_archive_member_content_stream('index.html');
            let data_bytes = stream.read_bytes(4096, null).get_data();
            let data = ByteArray.toString(data_bytes);
            expect(data).toContain('<!DOCTYPE html>');
        });
    });
});
