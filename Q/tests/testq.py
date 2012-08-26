import unittest
from q import *
q("\\e 0") # disable q's debug on error
KXVER=int(q('.Q.k'))
class CallTestCase(unittest.TestCase):
    def test_a0(self):
        self.assertEqual(q('{1}')(), q('1'))
    def test_a1(self):
        self.assertEqual(q('::')(1), q('1i'))
    def test_a2(self):
        self.assertEqual(q('+')(1,2), q('3i'))
    def test_a3(self):
        self.assertEqual(q('?')(q('10b'), 1, 2), q('1 2i'))
    def test_err(self):
        try:
            q("{'`test}", 0)
        except kerr, e:
            self.assertEqual(str(e), 'test')
        q("f:{'`test}")
        try:
            q("{f[x]}")(42)
        except kerr, e:
            self.assertEqual(str(e), 'test')

if KXVER >= 3:
    class TestGUID(unittest.TestCase):
        def test_conversion(self):
            from uuid import UUID
            u = UUID('cc165d74-88df-4973-8dd1-a1f2e0765a80')
            self.assertEqual(int(K(u)), u.int)

class TestOrderedDict(unittest.TestCase):
    def test_conversion(self):
        import collections
        odict = getattr(collections, 'OrderedDict', None)
        if odict is None:
            self.skipTest("no OrderedDict in collections")
        od = odict([('a', 1.0), ('b', 2.0)])
        self.assertEqual(K(od), q('`a`b!1 2f'))

if __name__ == '__main__':
    unittest.main()
