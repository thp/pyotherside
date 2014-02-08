import QtQuick 2.0
import io.thp.pyotherside 1.0

Python {
    Component.onCompleted: {
        addImportPath(Qt.resolvedUrl('.'));

        /**
         * Here, we test the broken behavior of the PyOtherSide 1.0 API
         * for imports with "." in the name:
         *  1. It uses PyImport_ImportModule() which does a "*"_import
         *  2. The variable in the globals dict that gets set is the full
         *     module name (and not the module after the "." for
         *     non-"*"-imports) including the dot, which is broken, anyway, as
         *     there's not way to retrieve that name in normal Python syntax
         *     (names cannot contain a "."), so for this test we use a dirty
         *     way of accessing they key via the globals() dict (just for
         *     testing - I hope nobody used that in old code, but we want to
         *     have a stable API, so we will drag this behavior along with the
         *     1.0 API support - new code should definitely use the 1.2 API)
         **/

        importModule('thp_io.pyotherside.nested', function () {
            console.log('"nested" imported successfully');

            // In API version 1.0, we expect the import to have done a "*"
            // import, and to add insult to injury, we assign the module
            // name with a ".", which basically makes the import unaccessible
            // from normal Python code (the entry in the globals dict contains
            // a ".", which isn't a valid name in Python), so we access the
            // globals dictionary directly
            console.log('repr of the module: ' + evaluate('repr(globals()["thp_io.pyotherside.nested"])'));
            call('globals()["thp_io.pyotherside.nested"].info', [], function (result) {
                console.log('from nested.info(): ' + result);
            });

            importModule('thp_io.pyotherside.nested.module', function () {
                console.log('"nested.module" imported successfully');
                // Globals hack - see above
                call('globals()["thp_io.pyotherside.nested.module"].info', [], function (result) {
                    console.log('from nested.module.info(): ' + result);
                    // Globals hack again - see above
                    console.log('nested.module.value: ' + evaluate('globals()["thp_io.pyotherside.nested.module"].value'));
                    Qt.quit();
                });
            });
        });
    }
}
