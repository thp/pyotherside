import QtTest 1.0

PythonListView {
    TestCase {
        name: "model_create"
        when: ready

        function test_model_created() {
            compare(view.count, 0);
        }
    }
}
