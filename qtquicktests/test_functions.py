import pymodel

def function_that_takes_one_parameter(parameter):
    '''For tst_model_add_one:call_sync_with_parameters'''
    assert parameter == 1
    return 1

class ModelWrapper:
    def __init__(self, sorted=False):
        self.sorted = sorted

    def activate(self, bridge):
        index = pymodel.FlatModelIndexIndex()
        if self.sorted:
            self.python_side = pymodel.SortedListModelPythonSide(bridge, index)
        else:
            self.python_side = pymodel.ListModelPythonSide(bridge, index)
        self.other_side = pymodel.ListModelOtherSide(self.python_side, index)
        return self.other_side


def setitem(obj, idx, val):
    '''See definition. Useful to avoid obtuse calls on the QML side.'''
    obj[idx] = val
