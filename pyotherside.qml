
import Qt 4.7
import com.thpinfo.python 1.0

Rectangle {
    width: 400
    height: 400

    ListView {
        anchors.fill: parent

        model: listModel
        delegate: Text {
            font.pixelSize: 30
            text: pyValue
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    py.chdir(pyValue)
                }
            }
        }
    }

    ListModel {
        id: listModel
    }

    Python {
        id: py
        property string path: '.'

        function chdir(newpath) {
            path += '/' + newpath
            listModel.clear()
            var data = eval(py.evaluate('pyotherside.demo("'+path+'")'))
            for (var i=0; i<data.length; i++) {
                listModel.append({'pyValue': data[i]})
            }
        }

        Component.onCompleted: {
            py.importModule('pyotherside')
            var data = eval(py.evaluate('pyotherside.demo("'+path+'")'))
            for (var i=0; i<data.length; i++) {
                listModel.append({'pyValue': data[i]})
            }
        }
    }
}

