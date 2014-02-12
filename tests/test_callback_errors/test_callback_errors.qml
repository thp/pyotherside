import QtQuick 2.0
import io.thp.pyotherside 1.2

// Test if PyOtherSide correctly reports JS errors happening in callbacks
// in signal error(string traceback) for both imports and function calls.

Python {
    property var tests: ([])

    function test_next() {
        if (tests.length) {
            tests.pop()();
        } else {
            console.log('Tests done');
            Qt.quit();
        }
    }

    Component.onCompleted: {
        tests.unshift(function () {
            console.log('Expecting ReferenceError for "invalid" on import');
            importModule('os', function (success) {
                invalid;
            });
        });
        tests.unshift(function() {
            console.log('Expecting TypeError for "lock" property');
            call('os.getcwd', [], function (result) {
                console.lock(result);
            });
        });
        test_next();
    }

    onError: {
        // Remove full path to .qml file
        var msg = traceback.replace(Qt.resolvedUrl('.'), '');
        console.log('Got error: ' + msg);
        test_next();
    }
}
