
import Qt 4.7
import com.thpinfo.python 1.0

Rectangle {
    width: 400
    height: 400
    color: 'blue'

    ListView {
        anchors.fill: parent

        model: listModel
        delegate: Text {
            text: pyValue
        }
    }

    ListModel {
        id: listModel
    }

    Python {
        id: py
        Component.onCompleted: {
            py.importModule('pyotherside')
            console.log(py.evaluate('pyotherside.demo()'))
            var data = eval(py.evaluate('pyotherside.demo()'))
            for (var i=0; i<data.length; i++) {
                listModel.append({'pyValue': data[i]})
            }
        }
    }
}

