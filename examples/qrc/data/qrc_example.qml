import QtQuick 2.0
import io.thp.pyotherside 1.2

Rectangle {
    width: 100
    height: 100

    Python {
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('qrc_example', function (success) {
                console.log('module imported: ' + success);
            });
        }
    }
}
