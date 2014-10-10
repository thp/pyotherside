import QtQuick 2.0
import io.thp.pyotherside 1.4

Rectangle {
    id: root
    width: 400
    height: 400

    Rectangle {
        id: foo
        x: 123
        y: 123
        width: 20
        height: 20
        color: 'blue'

        function dynamicFunction(a, b, c) {
            console.log('In QML, dynamicFunction got: ' + a + ', ' + b + ', ' + c);
            rotation += 4;
            return 'hello';
        }
    }

    Python {
        id: py

        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('qobject_reference', function () {
                call('qobject_reference.foo', [foo, py], function (result) {
                    console.log('In QML, got result: ' + result);
                    result.color = 'green';
                });
            });
        }
    }
}
