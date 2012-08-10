import unittest
from q import *
q("\\e 0") # disable q's debug on error

class CallTestCase(unittest.TestCase):
    def test_a0(self):
        self.failUnlessEqual(q('{1}')(), q('1'))
    def test_a1(self):
        self.failUnlessEqual(q('::')(1), q('1i'))
    def test_a2(self):
        self.failUnlessEqual(q('+')(1,2), q('3i'))
    def test_a3(self):
        self.failUnlessEqual(q('?')(q('10b'), 1, 2), q('1 2i'))
    def test_err(self):
        try:
            q("{'`test}", 0)
        except kerr, e:
            self.failUnlessEqual(str(e), 'test')
        q("f:{'`test}")
        try:
            q("{f[x]}")(42)
        except kerr, e:
            self.failUnlessEqual(str(e), 'test')
        
if __name__ == '__main__':
    unittest.main()
