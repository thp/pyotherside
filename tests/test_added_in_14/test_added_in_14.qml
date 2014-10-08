import QtQuick 2.0
// Note that we import 1.3 here instead of >= 1.4
import io.thp.pyotherside 1.3

Python {
    Component.onCompleted: {
        var foo = undefined;

        // qml: Received error: Import PyOtherSide 1.4 or newer to use getattr()
        getattr(foo, 'test');

        // qml: Received error: Import PyOtherSide 1.4 or newer to use callMethod()
        callMethod(foo, 'something', [], function (result) {});

        // qml: Received error: Import PyOtherSide 1.4 or newer to use callMethod_sync()
        var result = callMethod_sync(foo, 'something', []);

        Qt.quit();
    }

    onError: {
        console.log('Received error: ' + traceback);
    }
}
