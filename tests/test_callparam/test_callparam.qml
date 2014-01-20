import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    id: page
    width: 300
    height: 300

    Python {
        Component.onCompleted: {
            // We use call_sync() instead of call() in order
            // to guarantee the right ordering of Python's output
            // and the output of the JS engine for the error

            // This should fail with an error:
            // "Not a parameter list in call to print: 123"
            call_sync('print', 123);

            // This should work and print "123" on the console
            call_sync('print', [123]);
        }

        onError: {
            console.log('Received error: ' + traceback);
        }
    }
}
