import unittest
from q import *
q("\\e 0") # disable q's debug on error
KXVER=int(q('.Q.k'))
PY3K=str is not bytes
class TestBuiltinConversions(unittest.TestCase):
    def test_none(self):
        self.assertEqual(K(None), q("::"))

    def test_bool(self):
        self.assertEqual(K(True), q("1b"))
        self.assertEqual(K(False), q("0b"))

    def test_int(self):
        self.assertEqual(K(1), q("1"))
        self.assertRaises(OverflowError, K, 2**100)

    @unittest.skipIf(PY3K, "long and int are unified in Py3K")
    def test_long(self):
        self.assertEqual(K(1L), q("1j"))
        self.assertRaises(OverflowError, K, 2**100)

    def test_str(self):
        self.assertEqual(K(''), q("`"))

    def test_float(self):
        self.assertEqual(K(1.0), q("1f"))
        self.assertEqual(K(float('nan')), q("0Nf"))
        self.assertEqual(K(float('inf')), q("0w"))

    def test_datetime(self):
        self.assertEqual(K(date(2000, 1, 1)),
                         q("2000.01.01"))
        self.assertEqual(K(time(0)), q("00:00t"))
        self.assertEqual(K(datetime(2000, 1, 1)),
                         q("2000.01.01T00:00"))
        self.assertEqual(K(timedelta(1)), q("1D"))

    def test_list(self):
        self.assertEqual(K([]), q("()"))
        self.assertEqual(K([1]), q("enlist 1"))
        self.assertEqual(K([1, 2]), q("1 2"))
        self.assertEqual(K([1, None]), q("1 0N"))

        self.assertEqual(K([1.0]), q("enlist 1f"))
        self.assertEqual(K([1.0, 2.0]), q("1 2f"))
        self.assertEqual(K([1.0, float('nan')]), q("1 0n"))

        self.assertEqual(K(['']), q("enlist`"))
        self.assertEqual(K(['a', 'b'), q("`a`b"))


    def test_tuple(self):
        self.assertEqual(K(()), q("()"))
        t = (False, 0, 0.0, '', kp(b''), date(2000,1,1))
        self.assertEqual(K(t), q("(0b;0;0f;`;\"\";2000.01.01)"))

class CallTestCase(unittest.TestCase):
    def test_a0(self):
        self.assertEqual(q('{1}')(), q('1'))
    def test_a1(self):
        self.assertEqual(q('::')(1), q('1'))
    def test_a2(self):
        self.assertEqual(q('+')(1,2), q('3'))
    def test_a3(self):
        self.assertEqual(q('?')(q('10b'), 1, 2), q('1 2'))
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
