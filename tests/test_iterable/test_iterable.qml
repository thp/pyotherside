import QtQuick 2.0
import io.thp.pyotherside 1.3

Python {
    property var tests: ([])

    Component.onCompleted: {
        addImportPath(Qt.resolvedUrl('.'));

        importModule('test_iterable', function () {
            function test_next() {
                console.log('================================');
                if (tests.length == 0) {
                    console.log('Tests completed');
                    Qt.quit();
                } else {
                    var test = tests.pop();
                    console.log('-> ' + test.name);
                    call(test.func, [], function (reply) {
                        if (reply === undefined || reply === null) {
                            error('Got undefined or null');
                            return;
                        }

                        // Sort, because a Python set is unordered (to make expected work below)
                        reply.sort(function (a, b) { return a - b; });

                        console.log('Got:      ' + reply);
                        console.log('Expected: ' + test.expected);
                        if (reply.toString() !== test.expected.toString()) {
                            error('Results do not match');
                            return;
                        }
                        test_next();
                    });
                }
            }

            tests.unshift({
                name: 'Getting set returns JS array',
                func: 'test_iterable.get_set',
                expected: [1, 2, 3]
            });
            tests.unshift({
                name: 'Getting generator expression returns JS array',
                func: 'test_iterable.get_iterable_generator_expression',
                expected: [0, 2, 4, 6]
            });
            tests.unshift({
                name: 'Getting generator returns JS array',
                func: 'test_iterable.get_iterable_generator',
                expected: [0, 3, 6, 9, 12]
            });

            test_next();
        });
    }

    onError: {
        console.log('Error: ' + traceback);
        console.log('Tests failed');
        Qt.quit();
    }
}
