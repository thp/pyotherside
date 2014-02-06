import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    width: 200
    height: 200
    color: 'red'

    Python {
        Component.onCompleted: {
            // Add the directory of this .qml file to the search path
            addImportPath(Qt.resolvedUrl('.'));

            importModule_sync('atexit_example');
        }

        onError: console.log('error in python: ' + traceback);
    }
}
