import QtQuick 2.0
import io.thp.pyotherside 1.5

Item {
    width: 320
    height: 480

    PyGLArea {
        id: glArea
        anchors.fill: parent
        property var t: 0

        SequentialAnimation on t {
            NumberAnimation { to: 1; duration: 2500; easing.type: Easing.InQuad }
            NumberAnimation { to: 0; duration: 2500; easing.type: Easing.OutQuad }
            loops: Animation.Infinite
            running: true
        }

        onTChanged: {
            if (renderer) {
                py.call(py.getattr(renderer, 'set_t'), [t], update);
            }
        }
    }

    Rectangle {
        color: Qt.rgba(1, 1, 1, 0.7)
        radius: 10
        border.width: 1
        border.color: "white"
        anchors.fill: label
        anchors.margins: -10
    }

    Text {
        id: label
        color: "black"
        wrapMode: Text.WordWrap
        text: "The background here is a squircle rendered with raw OpenGL using a PyGLArea. This text label and its border is rendered using QML"
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 20
    }

    Python {
        id: py

        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('renderer', function () {
                call('renderer.Renderer', [], function (renderer) {
                    glArea.renderer = renderer;
                });
            });
        }

        onError: console.log(traceback);
    }
}
