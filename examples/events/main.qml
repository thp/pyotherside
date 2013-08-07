
import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    width: 400
    height: width

    Python {
        id: py

        Component.onCompleted: {
            // Add the Python library directory to the import path
            addImportPath('examples/events');

            // Import the main module
            importModule('main', function () {
                console.log('Main module is now imported');
            });
        }

        onReceived: {
            var command = data[0], args = data.splice(1);

            if (command == 'append') {
                output.text += '\n' + args;
            } else if (command == 'color') {
                output.color = args[0];
            }
        }
    }

    Text {
        id: output
        anchors.centerIn: parent
    }
}
