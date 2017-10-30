import QtQuick 2.0
import io.thp.pyotherside 1.5

Text {
    id: txt

    Python {
        Component.onCompleted: {
            importModule('sys', function() {
                var args = evaluate('sys.argv');
                for (var i=0; i<args.length; i++) {
                    txt.text += 'Arg ' + i + ': "' + args[i] + '"\n';
                }
            });
        }
    }
}
