import QtQuick 2.0
import io.thp.pyotherside 1.3

Rectangle {
    width: 100
    height: 100

    Python {
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('qrc_example', function (success) {
                console.log('module imported: ' + success);
                addImportPath(Qt.resolvedUrl('below'));
                importModule('qrc_example_below', function (success) {
                    console.log('also imported: ' + success);
                });
            });
        }
    }
}
