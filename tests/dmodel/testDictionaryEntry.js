const {DModel} = imports.gi;

const InstanceOfMatcher = imports.tests.InstanceOfMatcher;

describe('Dictionary Object Model', function() {
    let dictionaryObject, jsonld;

    beforeEach(function() {
        jasmine.addMatchers(InstanceOfMatcher.customMatchers);

        jsonld = {
            '@id': 'ekn:///23456789012345678901',
            'word': 'entreaty',
            'definition': 'An earnest request or petition; a plea.',
            'partOfSpeech': 'noun',
            'title': 'June 23rd',
        };
        dictionaryObject = DModel.DictionaryEntry.new_from_json(jsonld);
    });

    describe('type', function() {
        it('should be a DModel.Content', function() {
            expect(dictionaryObject).toBeA(DModel.Content);
        });
    });

    describe('JSON-LD marshaler', function() {
        it('should construct from a JSON-LD document', function() {
            expect(dictionaryObject).toBeDefined();
        });

        it('should marshal properties', function() {
            expect(dictionaryObject.word).toBe('entreaty');
            expect(dictionaryObject.definition).toBe('An earnest request or petition; a plea.');
            expect(dictionaryObject.part_of_speech).toBe('noun');
        });

        it('should inherit properties set by parent class (DModel.Content)', function() {
            expect(dictionaryObject.title).toBe('June 23rd');
        });
    });
});
