import QtQuick 2.0
import io.thp.pyotherside 1.2

Python {
    property var tests: ([])
    property var someJsDate: (new Date())
    property date someDate: '2014-02-12'
    // XXX: Not possible, see https://qt-project.org/forums/viewthread/8935
    // property time someTime: '11:22:33'

    Component.onCompleted: {
        addImportPath(Qt.resolvedUrl('.'));

        importModule('test_datetime', function () {
            var someTime = call_sync('test_datetime.get_time_value', []);
            // Note: Qt doesn't support microseconds, so these are trimmed
            // to milliseconds during conversion between data types
            console.log('Got time value from Python: ' + someTime);
            var someDateFromPython = call_sync('test_datetime.get_date_value', []);
            console.log('Got date value from Python: ' + someDateFromPython);
            var someDateTimeFromPython = call_sync('test_datetime.get_datetime_value', []);
            console.log('Got datetime value from Python: ' + someDateTimeFromPython);

            function test_next() {
                console.log('================================');
                if (tests.length == 0) {
                    console.log('Tests completed');
                    Qt.quit();
                } else {
                    var test = tests.pop();
                    console.log('-> ' + test.name);
                    call(test.func, [test.expected], function (reply) {
                        if (reply === undefined || reply === null) {
                            error('Got undefined or null');
                            return;
                        }
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
                name: 'Submit back and forth date (QML date property)',
                func: 'test_datetime.submit_date',
                expected: someDate
            });
            tests.unshift({
                name: 'Submit back and forth time (QML time from Python)',
                func: 'test_datetime.submit_time',
                expected: someTime
            });
            tests.unshift({
                name: 'Submit back and forth datetime (JS "new Date()")',
                func: 'test_datetime.submit_datetime',
                expected: someJsDate
            });
            tests.unshift({
                name: 'Submit back and forth date (JS var from Python)',
                func: 'test_datetime.submit_date',
                expected: someDateFromPython
            });
            tests.unshift({
                name: 'Submit back and forth datetime (JS var from Python)',
                func: 'test_datetime.submit_datetime',
                expected: someDateTimeFromPython
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
