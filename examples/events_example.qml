import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    width: 400
    height: width

    Python {
        Component.onCompleted: {
            // Add the directory of this .qml file to the search path
            addImportPath(Qt.resolvedUrl('.'));

            setHandler('append', function (message, number) {
                output.text += '\n' + message + number;
            });

            setHandler('color', function(color) {
                output.color = color;
            });

            // Import the main module
            importModule('events_example', function () {
                console.log('Event example module is now imported');
            });
        }

        onReceived: console.log('Unhandled event: ' + data)
    }

    Text {
        id: output
        anchors.centerIn: parent
    }
}
