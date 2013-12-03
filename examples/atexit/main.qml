
import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    width: 200
    height: 200
    color: 'red'

    Python {
        id: python
        Component.onCompleted: {
            addImportPath('examples/atexit');
            importModule_sync('main');
        }

        onError: console.log('error in python: ' + traceback);
    }
}
