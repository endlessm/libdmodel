const {DModel} = imports.gi;

const InstanceOfMatcher = imports.tests.InstanceOfMatcher;

const MOCK_AUDIO_DATA = {
    '@id': 'ekn:///df7132f8777966dd1169ee2aa0106f09b640f1aa',
    'title': 'Shooting Stars',
    'duration': '9000',
    'transcript': 'It\'s late and I\'m awake...',
};

describe('Audio Object Model', function () {
    let audioObject;

    beforeEach(function () {
        jasmine.addMatchers(InstanceOfMatcher.customMatchers);
        audioObject = DModel.Audio.new_from_json(MOCK_AUDIO_DATA);
    });

    describe('type', function () {
        it('should be a DModel.Content', function () {
            expect(audioObject).toBeA(DModel.Content);
        });
    });

    describe('JSON-LD marshaler', function () {
        it('should construct from a JSON-LD document', function () {
            expect(audioObject).toBeDefined();
        });

        it('should marshal properties', function () {
            expect(audioObject.duration).toBe(9000);
            expect(audioObject.transcript).toBe('It\'s late and I\'m awake...');
        });

        it('should inherit properties set by parent class (DModel.Content)', function () {
            expect(audioObject.title).toBe('Shooting Stars');
        });
    });
});
