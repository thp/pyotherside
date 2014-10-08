import QtQuick 2.0
// Note that we import 1.3 here instead of >= 1.4
import io.thp.pyotherside 1.3

Python {
    Component.onCompleted: {
        var foo = undefined;

        // qml: Received error: Import PyOtherSide 1.4 or newer to use getattr()
        getattr(foo, 'test');

        var func = eval('[]');

        // qml: Received error: Function not found: '' (unexpected EOF while parsing (<string>, line 0))
        call(func, [], function (result) {});

        // qml: Received error: Function not found: '' (unexpected EOF while parsing (<string>, line 0))
        var result = call_sync(func, []);

        Qt.quit();
    }

    onError: {
        console.log('Received error: ' + traceback);
    }
}
