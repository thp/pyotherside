import QtQuick 2.3

import io.thp.pyotherside 1.5

Rectangle {
    property bool sorted: false;
    property var python_side_model;
    property var model_wrapper;
    property bool ready: false;
    property alias view: view;
    property alias py: py;
    ListView {
        id: view
        model: model
        delegate: Row {
            Text { text: model.display }
        }
        anchors.fill: parent
    }
    PythonItemModel {
        id: model
    }
    Python {
        id: py
        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('test_functions', function() {
                call('test_functions.ModelWrapper', [sorted], function(result) {
                    model.model = getattr(result, 'activate');
                    python_side_model = getattr(result, 'python_side')
                    model_wrapper = result;
                    ready = true;
                });
            });
        }
    }
}
