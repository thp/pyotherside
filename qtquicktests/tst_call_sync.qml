import QtTest 1.0

PythonRectangle {
    TestCase {
        name: "call_sync_with_parameters"
        when: ready

        function test_call_sync_with_parameters() {
            var result = py.call_sync('test_functions.function_that_takes_one_parameter', [1]);
            compare(result, 1);
        }
    }
}
