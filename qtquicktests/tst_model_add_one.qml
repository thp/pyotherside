import QtTest 1.0

PythonListView {
    TestCase {
        name: "model_add_one"
        when: ready

        function get_model_item(row) {
            return view.model.data(view.model.index(row, 0), 258);
        }

        function test_model_add_one() {
            py.call_sync(py.getattr(python_side_model, 'append'), [1]);
            compare(view.count, 1);
            compare(get_model_item(0), 1);
        }
    }
}
