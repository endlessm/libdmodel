const {DModel, Gio, GLib} = imports.gi;
const ByteArray = imports.byteArray;

const InstanceOfMatcher = imports.tests.InstanceOfMatcher;

describe('ShardOpenZim', function () {
    let domain, tempdir;

    beforeAll(function () {
        tempdir = GLib.Dir.make_tmp('dmodel-test-domain-XXXXXX');
        GLib.setenv('XDG_DATA_HOME', tempdir, true);
    });

    beforeEach(function () {
        jasmine.addMatchers(InstanceOfMatcher.customMatchers);
        domain = new DModel.Domain({
            app_id: 'com.endlessm.fake_zim_test_app.en',
        });
        domain.init(null);
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

    it('returns a PDF file from the database', function (done) {
        domain.get_object('ekn+zim:///A/article.pdf', null, function (domain, result) {
            let model = domain.get_object_finish(result);
            expect(model).not.toBe(null);
            expect(model.title).toBe('Flotación sucia');
            expect(model.content_type).toBe('application/pdf');
            expect(model).toBeA(DModel.Article);
            done();
        });
    });

    it('throws for an ID not our database', function (done) {
        domain.get_object('ekn+zim:///A/notfound.pdf', null, function (domain, result) {
            expect(() => domain.get_object_finish(result)).toThrow();
            done();
        });
    });

    it('returns a HTML file from the database and reads its content', function (done) {
        let shard = domain.get_shards()[0];
        let record = shard.find_by_id('A/lipsum.html');

        let stream = shard.stream_data(record, null);
        let html = ByteArray.toString(
            ByteArray.fromGBytes(
                // This is the best way I've found to read all bytes from a stream
                stream.read_bytes(4294967295, null),
            )
        );

        expect(html.indexOf('<title>Test ZIM document</title>')).toBe(146);

        done();
    });

    it('query a document in the database', function (done) {
        let query = new DModel.Query({
            search_terms: 'flotacion',
            match: DModel.QueryMatch.TITLE_SYNOPSIS,
        });
        domain.query(query, null, function (domain, result) {
            let models = domain.query_finish(result).get_models();
            expect(models.length).toBe(1);
            expect(models[0].title).toBe('Flotación sucia');
            expect(models[0]).toBeA(DModel.Article);
            done();
        });
    });
});
