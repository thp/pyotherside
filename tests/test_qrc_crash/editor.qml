import QtQuick 2.0
import io.thp.pyotherside 1.5

Rectangle {
    width: 300
    height: 300

    Python {
        id: python
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('editor', function () {
                call('editor.Crasher', [], function (result) {
                    console.log('Got result: ' + result);
                    call('editor.foo', [result], function (res2) {
                        console.log('res2 = ' + res2);
                    });
                });
            });
        }
    }
}
