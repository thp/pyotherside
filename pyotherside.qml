
import QtQuick 1.0
import com.thpinfo.python 1.0

Rectangle {
    width: 400
    height: 400

    ListView {
        anchors.fill: parent

        model: listModel
        delegate: Text {
            width: parent.width
            font.pixelSize: 30
            text: name
            font.bold: directory

            Rectangle {
                anchors.fill: parent
                color: (!directory && mouseArea.pressed)?'red':'yellow'
                opacity: (directory || mouseArea.pressed)?.5:0
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                onClicked: {
                    if (directory) {
                        py.chdir(name)
                    } else {
                        console.log('clicked on a file')
                    }
                }
            }
        }
    }

    ListModel {
        id: listModel

        function fill(data) {
            listModel.clear()

            // Python returns a list of dicts - we can simply append
            // each dict in the list to the list model
            for (var i=0; i<data.length; i++) {
                listModel.append(data[i])
            }
        }
    }

    Python {
        id: py
        property string path: '.'

        function chdir(newpath) {
            // Append new path to current path, then use Python to calculate the
            // real path from it (resolves e.g. '..' to parent directory)
            path = call('os.path.join', [path, newpath]);
            path = call('os.path.abspath', [path]);

            // Replace list model data with list-of-dicts from Python
            listModel.fill(call('pyotherside.demo', [path]));
        }

        Component.onCompleted: {
            // Import standard library modules
            importModule('os');
            console.log(call('os.listdir', ['.']));

            // Import custom modules
            addImportPath('.');
            importModule('pyotherside');

            // We can also evalulate arbitrary Python expressions
            console.log('ten squares:' + evaluate('[x*x for x in range(1, 11)]'));

            importModule('math');
            console.log('square root of 4.5: ' + call('math.sqrt', [4.5]));

            // Call built-in functions
            console.log(call('list', [[1, 2, 'b']]));
            console.log('Absolute value of -42: ' + call('abs', [-42]));

            // Fill the list model with files from current working directory
            chdir('.');
        }
    }
}

