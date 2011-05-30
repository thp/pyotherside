
import Qt 4.7
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
            path += '/' + newpath

            // Need to use eval to convert JSON-encoded string to JS data
            var data = eval(py.evaluate('pyotherside.demo("'+path+'")'))

            // Replace list model data with list-of-dicts from Python
            listModel.fill(data)
        }

        Component.onCompleted: {
            // Import the module named "pyotherside" to Python interpreter
            py.importModule('pyotherside')

            // Fill the list model with files from current working directory
            chdir('.')
        }
    }
}

