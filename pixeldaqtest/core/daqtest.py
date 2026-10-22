class TestReturnValue(object):
    def __init__(self, value):
        self.x = value

# Test ReturnValues
PASS = TestReturnValue('PASS')
FAIL = TestReturnValue('FAIL')


class TestBase(object):

    def __init__(self, rod):
        self.rod=rod
        self.requires_is = False

    def initialize():  # needed at some point?
        pass

    def execute():     # the actual test
        pass

    def error_message(self):
        print(self.rod, self.__class__, ' test failed')

    def pass_message(self):
        print(self.rod, self.__class__.__name__, ' test passed')

