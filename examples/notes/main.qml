
import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    width: 300
    height: 200

    TextInput {
        id: ti

        anchors.fill: parent

        onTextChanged: {
            py.call('main.notes.set_contents', [text], function() {
                console.log('Changes sent to Python');
            });
        }

        Python {
            id: py
            Component.onCompleted: {
                addImportPath('examples/notes');
                importModule('main', function () {
                    console.log('imported python module');
                    call('main.notes.get_contents', [], function(result) {
                        console.log('got contents from Python: ' + result);
                        ti.text = result;
                    });
                });
            }
        }
    }

}
