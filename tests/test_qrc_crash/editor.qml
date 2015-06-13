import QtQuick 2.1
import QtQuick.Window 2.1
import io.thp.pyotherside 1.5

Window {
    visible: true

    Python {
        id: python
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('editor', function () {
                call('editor.Crasher', [], function () {
                });
            });
        }
    }
}
