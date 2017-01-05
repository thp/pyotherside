import QtQuick 2.3

import io.thp.pyotherside 1.5

Rectangle {
    property bool ready: false;
    property alias py: py;
    Python {
        id: py
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('test_functions', function() {
	        ready = true;
            });
        }
    }
}
