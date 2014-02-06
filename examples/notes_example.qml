import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    width: 300
    height: 200

    TextInput {
        id: ti

        anchors.fill: parent

        onTextChanged: {
            py.call('notes_example.notes.set_contents', [text], function() {
                console.log('Changes sent to Python');
            });
        }

        Python {
            id: py
            Component.onCompleted: {
                // Add the directory of this .qml file to the search path
                addImportPath(Qt.resolvedUrl('.'));

                importModule('notes_example', function () {
                    console.log('imported python module');
                    call('notes_example.notes.get_contents', [], function(result) {
                        console.log('got contents from Python: ' + result);
                        ti.text = result;
                    });
                });
            }
        }
    }
}
