import unittest
from q import *

class CallTestCase(unittest.TestCase):
    def test_a0(self):
        self.failUnlessEqual(q('{1}')(), q('1'))
    def test_a1(self):
        self.failUnlessEqual(q('::')(1), q('1'))
    def test_a2(self):
        self.failUnlessEqual(q('+')(1,2), q('3'))
    def test_a3(self):
        self.failUnlessEqual(q('?')(q('10b'), 1, 2), q('1 2'))
        
if __name__ == '__main__':
    unittest.main()
