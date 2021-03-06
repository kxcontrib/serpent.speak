import unittest
from q import _k
from datetime import datetime, date, time, timedelta

import sys
PY3K = sys.hexversion >= 0x3000000

# extract _k.K class methods
for m in ('func k knk ktd err'
          ' ka kb kg kh ki kj ke kf kc ks km kd kz ku kv kt kp'
          ' kdd ktt kzz knz kpz b9 d9'
          ' I J F S K xT xD').split():
    globals()[m] = getattr(_k.K, '_'+m)
del m
q = lambda *args: k(0,*args)
q("\\e 0") # disable q's debug on error
KXVER=int(q('.Q.k').inspect(b'f'))
if KXVER >= 3:
    kguid = _k.K._kguid


def kstr(x):
    return k(0, '-3!', x).inspect(b's')

class K_TestCase(unittest.TestCase):
    def assert_k_is(self, x, y):
        test = k(0, '~[%s;]' % y, x).inspect(b'g')
        if not test:
            raise self.failureException(kstr(x) + ' <> ' + y)

class AtomTestCase(K_TestCase):
    def test_ka(self):
        self.assert_k_is(ka(-_k.KB, 1), '1b')
        self.assert_k_is(ka(-_k.KJ, 1), '1j')
    def test_kb(self):
        self.assert_k_is(kb(1), '1b')
    def test_kb(self):
        self.assert_k_is(kb(1), '1b')
    def test_kg(self):
        self.assert_k_is(kg(1), '0x01')
    def test_kh(self):
        self.assert_k_is(kh(1), '1h')
    def test_ki(self):
        self.assert_k_is(ki(1), '1i')
    def test_kj(self):
        self.assert_k_is(kj(1), '1j')
        self.assert_k_is(kj(9223372036854775806), '9223372036854775806j')
    def test_ke(self):
        self.assert_k_is(ke(1), '1e')
    def test_kf(self):
        self.assert_k_is(kf(1), '1f')
    def test_kc(self):
        self.assert_k_is(kc(b'x'), '"x"')
    def test_ks(self):
        self.assert_k_is(ks('abc'), '`abc')
    def test_km(self):
        self.assert_k_is(km(1), '2000.02m')
    def test_kd(self):
        self.assert_k_is(kd(1), '2000.01.02')
    def test_kdd(self):
        self.assert_k_is(kdd(date(2000,1,2)), '2000.01.02')
    def test_ktt(self):
        self.assert_k_is(ktt(time(10,11,12,1000)), '10:11:12.001')
    def test_kzz(self):
        self.assert_k_is(kzz(datetime(2000,1,2,10,11,12,1000)), '2000.01.02T10:11:12.001')
    def test_kpz(self):
        self.assert_k_is(kpz(datetime(2000,1,2,10,11,12,1000)), '2000.01.02D10:11:12.001000000')
    def test_knz(self):
        self.assert_k_is(knz(timedelta(100,20,1000)), '100D00:00:20.001000000')
    def test_kz(self):
        self.assert_k_is(kz(1.5), '2000.01.02T12:00:00.000')
    def test_ku(self):
        self.assert_k_is(ku(1), '00:01')
    def test_kv(self):
        self.assert_k_is(kv(1), '00:00:01')
    def test_kt(self):
        self.assert_k_is(kt(1), '00:00:00.001')

    def test_id(self):
        x, y = ki(1), ki(1)
        self.assertNotEqual(x._id(), y._id())

class ListTestCase(K_TestCase):
    def test_kp(self):
        self.assert_k_is(kp("abc"), '"abc"')
    def test_I(self):
        self.assert_k_is(I([]), '`int$()')
        self.assert_k_is(I([1,2]), '1 2i')
    def test_J(self):
        self.assert_k_is(J([]), '`long$()')
        self.assert_k_is(J([1,2]), '1 2j')
    def test_F(self):
        self.assert_k_is(F([]), '`float$()')
        self.assert_k_is(F([1., 2.]), '1 2f')
    def test_S(self):
        self.assert_k_is(S([]), '`symbol$()')
        self.assert_k_is(S(['aa', 'bb']), '`aa`bb')
    def test_K(self):
        self.assert_k_is(K([]), '()')
        self.assert_k_is(K([ki(0), kf(1)]), '(0i;1f)')

class TableDictTestCase(K_TestCase):
    def test_xD(self):
        self.assert_k_is(xD(S('abc'), I(range(3))), '`a`b`c!0 1 2i')

    def test_xT(self):
        a = S('XYZ')
        b = kp('xyz')
        c = I([1,2,3])
        self.assert_k_is(xT(xD(S('abc'), K([a,b,c]))),
                         '([]a:`X`Y`Z;b:"xyz";c:1 2 3i)')
    def test_ktd(self):
        x = k(0, '([a:1 2 3]b:10 20 30)')
        y = '([]a:1 2 3;b:10 20 30)'
        self.assert_k_is(ktd(x),y)

class IterTestCase(K_TestCase):
    def test_bool(self):
        self.assertEqual(list(k(0,'101b')), [True, False, True])

    def test_byte(self):
        self.assertEqual(list(k(0,'0x010203')), [1,2,3])

    def test_char(self):
        self.assertEqual(list(kp('abc')), ['a','b','c'])

    def test_short(self):
        self.assertEqual(list(k(0, '1 2 3h')), [1, 2, 3])

    def test_int(self):
        l = [1,2,3,None]
        self.assertEqual(list(I(l)), l)

    def test_long(self):
        self.assertEqual(list(k(0, '1 2 3j')), [1, 2, 3])

    def test_date(self):
        self.assertEqual(list(k(0, '"d"$0 1 2')),[date(2000,1,d) for d in [1, 2, 3]])
        self.assertEqual(list(k(0, '-0W 0N 0Wd')),[date(1,1,1), None, date(9999,12,31),])

    def test_month(self):
        self.assertEqual(list(k(0, '"m"$0 1 2')),[date(2000,m,1) for m in [1, 2, 3]])
        self.assertEqual(list(k(0, '-0W 0N 0Wm')),[date(1,1,1), None, date(9999,12,1),])
                             
    def test_datetime(self):
        self.assertEqual(list(k(0, '"z"$0.5 1.5 2.5')),[datetime(2000,1,d,12) for d in [1, 2, 3]])
        self.assertEqual(list(k(0, '-0w 0n 0wz')),[datetime(1,1,1), None,
                                                       datetime(9999,12,31,23,59,59,999999),])
        
    def test_time(self):
        self.assertEqual(list(k(0, '"t"$0 1 2 0N')),[time(0,0,0,m*1000) for m in range(3)]+[None])

    def test_minute(self):
        self.assertEqual(list(k(0, '"u"$0 1 2 0N')),[time(0,m,0) for m in range(3)]+[None])

    def test_second(self):
        self.assertEqual(list(k(0, '"v"$0 1 2')),[time(0,0,s) for s in range(3)])

    def test_symbol(self):
        l = ['a', 'b', 'c']
        self.assertEqual(list(S(l)), l)

    def test_float(self):
        l = [1.2, 2.3, 3.4]
        self.assertEqual(list(F(l)), l)

    def test_generic(self):
        l = kf(1.2), ki(2), ks('xyz')
        self.assertEqual([kstr(x) for x in K(l)],
                         [kstr(x) for x in l])


    def test_dict(self):
        d = k(0, '`a`b`c!1 2 3')
        self.assertEqual(list(d), list('abc'))

    def test_table(self):
        t = k(0, '([]a:`X`Y`Z;b:"xyz";c:1 2 3)')
        d = ['`a`b`c!(`X;"x";1)',
             '`a`b`c!(`Y;"y";2)',
             '`a`b`c!(`Z;"z";3)',]
        for x,y in zip(t, d):
            self.assert_k_is(x, y)

    if KXVER >= 3:
        def test_guid(self):
            s = '0x16151413121110090807060504030201'
            l = int(s, 16)
            self.assert_k_is(kguid(l), '0x0 sv ' + s)
            self.assertEqual([l], list(k(0, 'enlist 0x0 sv ' + s)))

class CallsTestCase(K_TestCase):
    def test_dot(self):
        x = k(0, '+')._dot(I([1, 2]))
        y = '3i'
        self.assert_k_is(x, y)
        self.assertRaises(_k.error, q('+')._dot, q('``'))


    def test_a0(self):
         x = k(0, '{1}')._a0()
         self.assert_k_is(x, '1')

    def test_a1(self):
        x = k(0, 'neg')._a1(ki(1))
        y = '-1i'
        self.assert_k_is(x, y)

    def test_call(self):
        f = q('{5}')
        x = ki(42)
        self.assert_k_is(f(), '5')
        self.assert_k_is(f(x), '5')
        self.assertRaises(_k.error, f, x, x)

class StrTestCase(K_TestCase):
    def test_pass(self):
        x = 'abc'
        self.assertEqual(str(ks(x)), x)
        self.assertEqual(str(kp(x)), x)
        x = b'a'
        self.assertEqual(str(kc(x)), x.decode())

    def test_misc(self):
        self.assertEqual(str(kb(1)), '1b')
        self.assertEqual(str(kh(1)), '1h')
        if KXVER >= 3:
            self.assertEqual(str(ki(1)), '1i')
            self.assertEqual(str(kj(1)), '1')
        else:
            self.assertEqual(str(ki(1)), '1')
            self.assertEqual(str(kj(1)), '1j')
        self.assertEqual(str(kf(1)), '1f')
        
class ReprTestCase(unittest.TestCase):
    def test_pass(self):
        x = 'abc'
        self.assertEqual(repr(ks(x)), "k('`abc')")
        x = b'abc'
        self.assertEqual(repr(kp(x)), "k('\"abc\"')")
        x = b'a'
        self.assertEqual(repr(kc(x)), "k('\"a\"')")

    def test_misc(self):
        self.assertEqual(repr(kb(1)), "k('1b')")
        self.assertEqual(repr(kh(1)), "k('1h')")
        if KXVER >= 3:
            self.assertEqual(repr(ki(1)), "k('1i')")
            self.assertEqual(repr(kj(1)), "k('1')")
        else:
            self.assertEqual(repr(ki(1)), "k('1')")
            self.assertEqual(repr(kj(1)), "k('1j')")
        self.assertEqual(repr(kf(1)), "k('1f')")
        
class JoinTestCase(K_TestCase):
    def test_byte(self):
        x = q('0x0102')
        x._ja(3)
        y = '0x010203'
        self.assert_k_is(x, y)
        
    def test_short(self):
        x = q('1 2h')
        x._ja(3)
        y = '1 2 3h'
        self.assert_k_is(x, y)
        
    def test_int(self):
        x = q('1 2')
        x._ja(3)
        y = '1 2 3'
        self.assert_k_is(x, y)
        
    def test_long(self):
        x = q('1 2j')
        x._ja(3)
        y = '1 2 3j'
        self.assert_k_is(x, y)

    def test_real(self):
        x = q('1 2e')
        x._ja(3)
        y = '1 2 3e'
        self.assert_k_is(x, y)

    def test_float(self):
        x = q('1 2f')
        x._ja(3)
        y = '1 2 3f'
        self.assert_k_is(x, y)

    def test_str(self):
        x = q('"ab"')
        x._ja(b'c')
        y = '"abc"'
        self.assert_k_is(x, y)
        
    def test_sym(self):
        x = q('`a`b')
        x._ja(b'c')
        y = '`a`b`c'
        self.assert_k_is(x, y)

class ErrorTestCase(unittest.TestCase):
    def test_simple(self):
        self.assertRaises(_k.error, q, "1+`")
        self.assertRaises(_k.error, q("1+"), ks(''))
        self.assertRaises(_k.error, q("+"), ki(1), ks(''))

    def test_nested(self):
        self.assertRaises(_k.error, q, "{{{'`xyz}0}0}", ki(0))

class ArrayStructTestCase(unittest.TestCase):
    def test_type(self):
        s = q('1 2 3').__array_struct__
        if PY3K:
            self.assertEqual(type(s).__name__, 'PyCapsule')
        else:
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
                ('" "', "c", 1, b' '),
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


class SerializationTestCase(K_TestCase):
    def test_b9(self):
        self.assert_k_is(b9(-1, kb(1)),    b'0x010000000a000000ff01')
        self.assert_k_is(b9(-1, kc(b'a')), b'0x010000000a000000f661')
        self.assert_k_is(b9(-1, kg(1)),    b'0x010000000a000000fc01')
        self.assert_k_is(b9(-1, ki(1)),    b'0x010000000d000000fa01000000')
        self.assert_k_is(b9(-1, kj(1)),    b'0x0100000011000000f90100000000000000')
        self.assert_k_is(b9(-1, kp(b'')),  b'0x010000000e0000000a0000000000')
    def test_d9(self):
        self.assert_k_is(d9(q('0x010000000a000000ff00')), b"0b")

try:
    from numpy import array
except ImportError:
    pass
else:
    class NumPyTestCase(unittest.TestCase):
        pass

if __name__ == '__main__':
    unittest.main()
