/**
 * Issue #23: importModule runs forever
 * https://github.com/thp/pyotherside/issues/23
 **/

import QtQuick 2.0
import io.thp.pyotherside 1.3

Rectangle {
    width: 300
    height: 300

    Text {
        id: text
        anchors.centerIn: parent
    }

    Python {
        Component.onCompleted: {
            importModule('gi.repository.Gio', function() {
                console.log('import completed');
                call('gi.repository.Gio.Settings.new("org.gnome.Vino").keys', [], function(result) {
                    text.text = result.join('\n');
                });
            });
        }

        onError: console.log('Error: ' + traceback);
    }
}
