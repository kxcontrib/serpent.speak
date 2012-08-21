import unittest
from q import _k
from datetime import datetime, date, time, timedelta
# extract _k.K class methods

for m in ('func k knk ktd err'
          ' ka kb kg kh ki kj ke kf kc ks km kd kz ku kv kt kp'
          ' kdd ktt kzz knz kpz'
          ' I F S K xT xD').split():
    globals()[m] = getattr(_k.K, '_'+m)
del m
q = lambda *args: k(0,*args)
q("\\e 0") # disable q's debug on error
KXVER=int(q('.Q.k').inspect('f'))
if KXVER >= 3:
    kguid = _k.K._kguid


def kstr(x):
    return k(0, '-3!', x).inspect('s')

def eq(a, b):
    test = k(0, '~', a, b).inspect('g')
    if not test:
        print 'ERROR:', kstr(a), '<>', kstr(b)
    return test

class AtomTestCase(unittest.TestCase):
    def test_kb(self):
        self.failUnless(eq(kb(1), k(0, '1b')))
    def test_kg(self):
        self.failUnless(eq(kg(1), k(0, '0x01')))
    def test_kh(self):
        self.failUnless(eq(kh(1), k(0, '1h')))
    def test_ki(self):
        self.failUnless(eq(ki(1), k(0, '1i')))
    def test_kj(self):
        self.failUnless(eq(kj(1), k(0, '1j')))
        self.failUnless(eq(kj(9223372036854775806), k(0, '9223372036854775806j')))
    def test_ke(self):
        self.failUnless(eq(ke(1), k(0, '1e')))
    def test_kf(self):
        self.failUnless(eq(kf(1), k(0, '1f')))
    def test_kc(self):
        self.failUnless(eq(kc('x'), k(0, '"x"')))
    def test_ks(self):
        self.failUnless(eq(ks('abc'), k(0, '`abc')))
    def test_km(self):
        self.failUnless(eq(km(1), k(0, '2000.02m')))
    def test_kd(self):
        self.failUnless(eq(kd(1), k(0, '2000.01.02')))
    def test_kdd(self):
        self.failUnless(eq(kdd(date(2000,1,2)), k(0, '2000.01.02')))
    def test_ktt(self):
        self.failUnless(eq(ktt(time(10,11,12,1000)), k(0, '10:11:12.001')))
    def test_kzz(self):
        self.failUnless(eq(kzz(datetime(2000,1,2,10,11,12,1000)), k(0, '2000.01.02T10:11:12.001')))
    def test_kpz(self):
        self.failUnless(eq(kpz(datetime(2000,1,2,10,11,12,1000)), k(0, '2000.01.02D10:11:12.001000000')))
    def test_knz(self):
        self.failUnless(eq(knz(timedelta(100,20,1000)), k(0, '100D00:00:20.001000000')))
    def test_kz(self):
        self.failUnless(eq(kz(1.5), k(0, '2000.01.02T12:00:00.000')))
    def test_ku(self):
        self.failUnless(eq(ku(1), k(0, '00:01')))
    def test_kv(self):
        self.failUnless(eq(kv(1), k(0, '00:00:01')))
    def test_kt(self):
        self.failUnless(eq(kt(1), k(0, '00:00:00.001')))

class ListTestCase(unittest.TestCase):
    def test_kp(self):
        self.failUnless(eq(kp("abc"), k(0, '"abc"')))
    def test_I(self):
        self.failUnless(eq(I([]), k(0, '`int$()')))
        self.failUnless(eq(I([1,2]), k(0, '1 2i')))
    def test_F(self):
        self.failUnless(eq(F([]), k(0, '`float$()')))
        self.failUnless(eq(F([1., 2.]), k(0, '1 2f')))
    def test_S(self):
        self.failUnless(eq(S([]), k(0, '`symbol$()')))
        self.failUnless(eq(S(['aa', 'bb']), k(0, '`aa`bb')))
    def test_K(self):
        self.failUnless(eq(K([]), k(0, '()')))
        self.failUnless(eq(K([ki(0), kf(1)]), k(0, '(0i;1f)')))

class TableDictTestCase(unittest.TestCase):
    def test_xD(self):
        self.failUnless(eq(xD(S('abc'), I(range(3))), k(0, '`a`b`c!0 1 2i')))

    def test_xT(self):
        a = S('XYZ')
        b = kp('xyz')
        c = I([1,2,3])
        self.failUnless(eq(xT(xD(S('abc'), K([a,b,c]))),
                           k(0, '([]a:`X`Y`Z;b:"xyz";c:1 2 3i)')))
    def test_ktd(self):
        x = k(0, '([a:1 2 3]b:10 20 30)')
        y = k(0, '([]a:1 2 3;b:10 20 30)')
        self.failUnless(eq(ktd(x),y))

class IterTestCase(unittest.TestCase):
    def test_bool(self):
        self.failUnlessEqual(list(k(0,'101b')), [True, False, True])

    def test_byte(self):
        self.failUnlessEqual(list(k(0,'0x010203')), [1,2,3])

    def test_char(self):
        self.failUnlessEqual(list(kp('abc')), ['a','b','c'])

    def test_short(self):
        self.failUnlessEqual(list(k(0, '1 2 3h')), [1, 2, 3])

    def test_int(self):
        l = [1,2,3,None]
        self.failUnlessEqual(list(I(l)), l)

    def test_long(self):
        self.failUnlessEqual(list(k(0, '1 2 3j')), [1, 2, 3])

    def test_date(self):
        self.failUnlessEqual(list(k(0, '"d"$0 1 2')),[date(2000,1,d) for d in [1, 2, 3]])
        self.failUnlessEqual(list(k(0, '-0W 0N 0Wd')),[date(1,1,1), None, date(9999,12,31),])

    def test_month(self):
        self.failUnlessEqual(list(k(0, '"m"$0 1 2')),[date(2000,m,1) for m in [1, 2, 3]])
        self.failUnlessEqual(list(k(0, '-0W 0N 0Wm')),[date(1,1,1), None, date(9999,12,1),])
                             
    def test_datetime(self):
        self.failUnlessEqual(list(k(0, '"z"$0.5 1.5 2.5')),[datetime(2000,1,d,12) for d in [1, 2, 3]])
        self.failUnlessEqual(list(k(0, '-0w 0n 0wz')),[datetime(1,1,1), None,
                                                       datetime(9999,12,31,23,59,59,999999),])
        
    def test_time(self):
        self.failUnlessEqual(list(k(0, '"t"$0 1 2 0N')),[time(0,0,0,m*1000) for m in range(3)]+[None])

    def test_minute(self):
        self.failUnlessEqual(list(k(0, '"u"$0 1 2 0N')),[time(0,m,0) for m in range(3)]+[None])

    def test_second(self):
        self.failUnlessEqual(list(k(0, '"v"$0 1 2')),[time(0,0,s) for s in range(3)])

    def test_symbol(self):
        l = ['a', 'b', 'c']
        self.failUnlessEqual(list(S(l)), l)

    def test_float(self):
        l = [1.2, 2.3, 3.4]
        self.failUnlessEqual(list(F(l)), l)

    def test_generic(self):
        l = kf(1.2), ki(2), ks('xyz')
        self.failUnlessEqual(map(kstr, K(l)), map(kstr, l))

    def test_dict(self):
        d = k(0, '`a`b`c!1 2 3')
        self.failUnlessEqual(list(d), list('abc'))

    def test_table(self):
        t = k(0, '([]a:`X`Y`Z;b:"xyz";c:1 2 3)')
        d = [k(0, '`a`b`c!(`X;"x";1)'),
             k(0, '`a`b`c!(`Y;"y";2)'),
             k(0, '`a`b`c!(`Z;"z";3)'),]
        for x,y in zip(t, d):
            self.failUnless(eq(x,y))

    if KXVER >= 3:
        def test_guid(self):
            s = '0x16151413121110090807060504030201'
            l = long(s, 16)
            self.assertTrue(eq(kguid(l), k(0, '0x0 sv ' + s)))
            self.assertEqual([l], list(k(0, 'enlist 0x0 sv ' + s)))

class CallsTestCase(unittest.TestCase):
    def test_dot(self):
        x = k(0, '+')._dot(I([1, 2]))
        y = ki(3)
        self.failUnless(eq(x, y))

#     def test_a0(self):
#         x = k(0, '{1}')._a0()
#         y = ki(1)
#         self.failUnless(eq(x, y))

    def test_a1(self):
        x = k(0, 'neg')._a1(ki(1))
        y = ki(-1)
        self.failUnless(eq(x, y))

#     def test_a2(self):
#         x = k(0, '+')._a2(ki(1), ki(2))
#         y = ki(3)
#         self.failUnless(eq(x, y))

#     def test_a3(self):
#         x = k(0, 'plist')._a3(ki(1), ki(2), ki(3))
#         y = I([1,2,3])
#         self.failUnless(eq(x, y))

class StrTestCase(unittest.TestCase):
    def test_pass(self):
        x = 'abc'
        self.failUnlessEqual(str(ks(x)), x)
        self.failUnlessEqual(str(kp(x)), x)
        x = 'a'
        self.failUnlessEqual(str(kc(x)), x)

    def test_misc(self):
        self.failUnlessEqual(str(kb(1)), '1b')
        self.failUnlessEqual(str(kh(1)), '1h')
        if KXVER >= 3:
            self.failUnlessEqual(str(ki(1)), '1i')
            self.failUnlessEqual(str(kj(1)), '1')
        else:
            self.failUnlessEqual(str(ki(1)), '1')
            self.failUnlessEqual(str(kj(1)), '1j')
        self.failUnlessEqual(str(kf(1)), '1f')
        
class ReprTestCase(unittest.TestCase):
    def test_pass(self):
        x = 'abc'
        self.failUnlessEqual(repr(ks(x)), "k('`abc')")
        self.failUnlessEqual(repr(kp(x)), "k('\"abc\"')")
        x = 'a'
        self.failUnlessEqual(repr(kc(x)), "k('\"a\"')")

    def test_misc(self):
        self.failUnlessEqual(repr(kb(1)), "k('1b')")
        self.failUnlessEqual(repr(kh(1)), "k('1h')")
        if KXVER >= 3:
            self.failUnlessEqual(repr(ki(1)), "k('1i')")
            self.failUnlessEqual(repr(kj(1)), "k('1')")
        else:
            self.failUnlessEqual(repr(ki(1)), "k('1')")
            self.failUnlessEqual(repr(kj(1)), "k('1j')")
        self.failUnlessEqual(repr(kf(1)), "k('1f')")
        
class JoinTestCase(unittest.TestCase):
    def test_byte(self):
        x = q('0x0102')
        x._ja(3)
        y = q('0x010203')
        self.failUnless(eq(x, y))
        
    def test_short(self):
        x = q('1 2h')
        x._ja(3)
        y = q('1 2 3h')
        self.failUnless(eq(x, y))
        
    def test_int(self):
        x = q('1 2')
        x._ja(3)
        y = q('1 2 3')
        self.failUnless(eq(x, y))
        
    def test_long(self):
        x = q('1 2j')
        x._ja(3)
        y = q('1 2 3j')
        self.failUnless(eq(x, y))

    def test_real(self):
        x = q('1 2e')
        x._ja(3)
        y = q('1 2 3e')
        self.failUnless(eq(x, y))

    def test_float(self):
        x = q('1 2f')
        x._ja(3)
        y = q('1 2 3f')
        self.failUnless(eq(x, y))

    def test_str(self):
        x = q('"ab"')
        x._ja('c')
        y = q('"abc"')
        self.failUnless(eq(x, y))
        
    def test_sym(self):
        x = q('`a`b')
        x._ja('c')
        y = q('`a`b`c')
        self.failUnless(eq(x, y))

class ErrorTestCase(unittest.TestCase):
    def test_simple(self):
        self.failUnlessRaises(_k.error, q, "1+`")
        self.failUnlessRaises(_k.error, q("1+"), ks(''))
        self.failUnlessRaises(_k.error, q("+"), ki(1), ks(''))

    def test_nested(self):
        self.failUnlessRaises(_k.error, q, "{{{'`xyz}0}0}", ki(0))

class ArrayStructTestCase(unittest.TestCase):
    def test_type(self):
        s = q('1 2 3').__array_struct__
        self.assertEqual(type(s).__name__, 'PyCObject')

    def test_error(self):
        x = q('()!()')
        self.assertRaises(AttributeError, lambda: x.__array_struct__)


try:
    mv = memoryview
except NameError:
    pass
else:
    import struct
    class NewBufferTestCase(unittest.TestCase):
        def test_simple_view(self):
            data = [
                #('::', 'P', 8),
                ('0b', '?', 1, False),
                ('0x0', "B", 1, 0),
                ('0h', "h", 2, 0),
                ('0i', "i", 4, 0),
                ('0j', "q", 8, 0),
                ('0e', "f", 4, 0.0),
                ('0f', "d", 8, 0.0),
                ('" "', "c", 1, ' '),
                ('2000.01m', "i", 4, 0),
                ('2000.01.01', "i", 4, 0),
                ('2000.01.01T00:00', "d", 8, 0),
                ('00:00', "i", 4, 0),
                ('00:00:00', "i", 4, 0),
                ('00:00:00.000', "i", 4, 0),
                ]
            if KXVER >= 3:
                data.append(('0Ng', "16B", 16, 0))
                
            for x, f, s, u in data:
                m = memoryview(q(x))
                self.assertEqual(m.ndim, 0)
                self.assertEqual(m.format, f)
                self.assertEqual(m.itemsize, s)
                v = struct.unpack(f, m.tobytes())
                self.assertEqual(v[0], u, x)
                m = memoryview(q("enlist " + x))
                self.assertEqual(m.ndim, 1)
                self.assertEqual(m.shape, (1,))
                self.assertEqual(m.strides, (s,))
                self.assertEqual(m.format, f)
                self.assertEqual(m.itemsize, s)
                v = struct.unpack(f, m[0])
                self.assertEqual(v[0], u)
                
try:
    from numpy import array
except ImportError:
    pass
else:
    class NumPyTestCase(unittest.TestCase):
        pass

if __name__ == '__main__':
    unittest.main()
