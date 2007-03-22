import unittest
import _k
from datetime import datetime, date, time
# extract _k.K class methods

for m in ('func k knk ktd err'
          ' ka kb kg kh ki kj ke kf kc ks km kd kz ku kv kt kp'
          ' I F S K xT xD').split():
    globals()[m] = getattr(_k.K, '_'+m)
del m

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
        self.failUnless(eq(ki(1), k(0, '1')))
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
        self.failUnless(eq(I([1,2]), k(0, '1 2')))
    def test_F(self):
        self.failUnless(eq(F([]), k(0, '`float$()')))
        self.failUnless(eq(F([1., 2.]), k(0, '1 2f')))
    def test_S(self):
        self.failUnless(eq(S([]), k(0, '`symbol$()')))
        self.failUnless(eq(S(['aa', 'bb']), k(0, '`aa`bb')))
    def test_K(self):
        self.failUnless(eq(K([]), k(0, '()')))
        self.failUnless(eq(K([ki(0), kf(1)]), k(0, '(0;1f)')))

class TableDictTestCase(unittest.TestCase):
    def test_xD(self):
        self.failUnless(eq(xD(S('abc'), I(range(3))), k(0, '`a`b`c!0 1 2')))

    def test_xT(self):
        a = S('XYZ')
        b = kp('xyz')
        c = I([1,2,3])
        self.failUnless(eq(xT(xD(S('abc'), K([a,b,c]))),
                           k(0, '([]a:`X`Y`Z;b:"xyz";c:1 2 3)')))
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
        l = [1,2,3]
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

class CallsTestCase(unittest.TestCase):
    def test_dot(self):
        x = k(0, '+')._dot(I([1, 2]))
        y = ki(3)
        self.failUnless(eq(x, y))

    def test_a1(self):
        x = k(0, 'neg')._a1(ki(1))
        y = ki(-1)
        self.failUnless(eq(x, y))

    def test_a2(self):
        x = k(0, '+')._a2(ki(1), ki(2))
        y = ki(3)
        self.failUnless(eq(x, y))

    def test_a3(self):
        x = k(0, 'list')._a3(ki(1), ki(2), ki(3))
        y = I([1,2,3])
        self.failUnless(eq(x, y))

if __name__ == '__main__':
    unittest.main()
