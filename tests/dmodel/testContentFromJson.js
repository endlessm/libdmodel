const {DModel, Json} = imports.gi;

const InstanceOfMatcher = imports.tests.InstanceOfMatcher;

describe('Object model from json node', function () {
    beforeEach(function () {
        jasmine.addMatchers(InstanceOfMatcher.customMatchers);
    });

    it('errors with malformed data', function () {
        let node = Json.from_string(JSON.stringify([1, 2, 3]));
        expect(() => { DModel.model_from_json_node(node); }).toThrow();
    });

    it('errors with missing @type', function () {
        let node = Json.from_string(JSON.stringify({}));
        expect(() => { DModel.model_from_json_node(node); }).toThrow();
    });

    it('errors with unknown @type', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'foobar',
        }));
        expect(() => { DModel.model_from_json_node(node); }).toThrow();
    });

    it('creates a content object model', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'ekn://_vocab/ContentObject',
        }));
        let model = DModel.model_from_json_node(node);
        expect(model).toBeA(DModel.Content);
    });

    it('creates a article object model', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'ekn://_vocab/ArticleObject',
        }));
        let model = DModel.model_from_json_node(node);
        expect(model).toBeA(DModel.Article);
    });

    it('creates a set object model', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'ekn://_vocab/SetObject',
        }));
        let model = DModel.model_from_json_node(node);
        expect(model).toBeA(DModel.Set);
    });

    it('creates a media object model', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'ekn://_vocab/MediaObject',
        }));
        let model = DModel.model_from_json_node(node);
        expect(model).toBeA(DModel.Media);
    });

    it('creates a video object model', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'ekn://_vocab/VideoObject',
        }));
        let model = DModel.model_from_json_node(node);
        expect(model).toBeA(DModel.Video);
    });

    it('creates a image object model', function () {
        let node = Json.from_string(JSON.stringify({
            '@type': 'ekn://_vocab/ImageObject',
        }));
        let model = DModel.model_from_json_node(node);
        expect(model).toBeA(DModel.Image);
    });
});
