// Copyright 2016 Endless Mobile, Inc.

const {DModel} = imports.gi;

describe('Utils', function () {
    describe('parallel init', function () {
        it('handles an empty list', function () {
            expect(function () {
                DModel.utils_parallel_init([], 0, null);
            }).not.toThrow();
        });
    });
});
