import QtQuick 2.0
import io.thp.pyotherside 1.0

Rectangle {
    id: page
    width: 300
    height: 300

    Component.onCompleted: {
        py.addImportPath(Qt.resolvedUrl('.').substr('file://'.length));
        py.setHandler("test-errors", page.testErrors);
        py.importModule("test_errors", null);
    }

    Python {
        id: py

        onError: {
            console.log("PYTHON ERROR: " + traceback);
            msg.text += '\n' + traceback;
        }
    }

    Text {
        id: msg
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        text: "Testing (you should see errors appearing here)..."
        wrapMode: Text.Wrap
    }

    function testErrors() {
        console.log("starting");
        page.nonexistentMethod();
        console.log("finished");
    }
}
