"""Python interface to the Q language

The following examples are adapted from the "Kdb+ Database and Language Primer"
by Dennis Shasha <http://kx.com/q/d/primer.htm>

>>> y = q('(`aaa; `bbbdef; `c)'); y[0]
k('`aaa')

Unlike in Q, in python function call syntax uses '()' and
indexing uses '[]':
>>> z = q('(`abc; 10 20 30; (`a; `b); 50 60 61)')
>>> z(2, 0)
k('`a')
>>> z[q('0 2')] # XXX: Should be able to write this as z[0,2]
k('(`abc;`a`b)')


Dictionaries

>>> fruitcolor = q('`cherry`plum`tomato!`brightred`violet`brightred')
>>> fruitcolor['plum']
k('`violet')
>>> fruitcolor2 = q('`grannysmith`plum`prune!`green`reddish`black')
>>> q(',', fruitcolor, fruitcolor2)
k('`cherry`plum`tomato`grannysmith`prune!`brightred`reddish`brightred`green`black')

Tables from Dictionaries

>>> d = q('`name`salary! (`tom`dick`harry;30 30 35) ')
>>> e = q.flip(d)
>>> e[1]
k('`name`salary!(`dick;30)')
>>> q('{select name from x}', e)
k('+(,`name)!,`tom`dick`harry')
>>> q('{select sum salary from x}', e).salary
k(',95')

>>> e2 = q.xkey('name', e)
>>> q('+', e2, e2)
k('(+(,`name)!,`tom`dick`harry)!+(,`salary)!,60 60 70')
>>> q.keys(e2)
k(',`name')
>>> q.cols(e2)
k('`name`salary')

Temporal Primitives

>>> x = datetime(2004,7,3,16,35,24,980000)
>>> K(x)
k('2004.07.03T16:35:24.980')
>>> K(x.date()), K(x.time())
(k('2004.07.03'), k('16:35:24.980'))
>>> K(timedelta(200,200,200))
k('200D00:03:20.000200000')

Input/Output

>>> import os
>>> r,w = os.pipe()
>>> h = K(w)(kp("xyz"))
>>> os.read(r, 100)
'xyz'
>>> os.close(r); os.close(w)
"""
__version__='3.1.0'
__metaclass__ = type
import os
QVER = os.environ.get('QVER')
del os

# Q sets QVER environment variable to communicate its version info to
# python. If it is not set - most likely you are attempting to run
# q.py under regular python.
if QVER is None:
    raise NotImplementedError("loading q in stock python is not implemented")
_k = __import__('_k' + QVER.replace('.', '_')) 
from datetime import datetime, date, time, timedelta
kerr = _k.error
class K_call_proxy:
    def __get__(self, obj, objtype):
        if obj is None:
            return self
        if obj.inspect('t') == 100:
            return obj._call_lambda
        return obj._call


class K(_k.K):
    """a handle to q objects

    >>> k('2005.01.01 2005.12.04')
    k('2005.01.01 2005.12.04')

    Iteration over simple lists produces python objects
    >>> list(q("`a`b`c`d"))
    ['a', 'b', 'c', 'd']

    Iteration over q tables produces q dictionaries
    >>> list(q("([]a:`x`y`z;b:1 2 3)"))
    [k('`a`b!(`x;1)'), k('`a`b!(`y;2)'), k('`a`b!(`z;3)')]

    Iteration over a q dictionary iterates over its key
    >>> list(q('`a`b!1 2'))
    ['a', 'b']
    
    as a consequence, iteration over a keyed table is the same as
    iteration over its key table
    >>> list(q("([a:`x`y`z]b:1 2 3)"))
    [k('(,`a)!,`x'), k('(,`a)!,`y'), k('(,`a)!,`z')]
    
    Callbacks into python
    >>> def f(x, y):
    ...     return x + y
    >>> q('{[f]f(1;2)}', f)
    k('3')

    Buffer protocol:
    >>> x = kp('xxxxxx')
    >>> import os; r,w = os.pipe()
    >>> os.write(w, 'abcdef') == os.fdopen(r).readinto(x)
    True
    >>> os.close(w); x
    k('"abcdef"')

    Array protocol:
    >>> ','.join([k(x).__array_typestr__
    ...  for x in ('0b;0x00;0h;0;0j;0e;0.0;" ";`;2000.01m;2000.01.01;'
    ...            '2000.01.01T00:00:00.000;00:00;00:00:00;00:00:00.000')
    ...  .split(';')])"""
    __doc__ += """
    '<b1,<u1,<i2,<i4,<i8,<f4,<f8,<S1,|O%d,<i4,<i4,<f8,<i4,<i4,<i4'
    """ % _k.SIZEOF_VOID_P
    try:
        import numpy
    except ImportError:
        pass
    else:
        del numpy
        if _k.SIZEOF_VOID_P == 4:
            dtypes = dict(i_dtype='', j_dtype=', dtype=int64')
        else:
            dtypes = dict(i_dtype=', dtype=int32', j_dtype='')
        __doc__ += """
    Numpy support
    ---------------

    >>> from numpy import asarray, array
    >>> asarray(k("1010b"))
    array([ True, False,  True, False], dtype=bool)

    >>> asarray(k("0x102030"))
    array([16, 32, 48], dtype=uint8)

    >>> asarray(k("0 1 2h"))
    array([0, 1, 2], dtype=int16)

    >>> asarray(k("0 1 2"))
    array([0, 1, 2]{i_dtype})

    >>> asarray(k("0 1 2j"))
    array([0, 1, 2]{j_dtype})

    >>> asarray(k("0 1 2e"))
    array([ 0.,  1.,  2.], dtype=float32)

    >>> asarray(k("0 1 2.0"))
    array([ 0.,  1.,  2.])

    Date-time data-types expose their underlying data:

    >>> asarray(k(",2000.01m"))
    array([0]{i_dtype})

    >>> asarray(k(",2000.01.01"))
    array([0]{i_dtype})

    >>> asarray(k(",2000.01.01T00:00:00.000"))
    array([ 0.])

   """.format(**dtypes)
        del dtypes
    try:
        import Numeric
    except ImportError:
        pass
    else:
        del Numeric
        __doc__ += """
    Numeric support
    ---------------

    >>> from Numeric import asarray, array
    >>> asarray(k("1 2 3h"))
    array([1, 2, 3],'s')
    >>> K(array([1, 2, 3],'i'))
    k('1 2 3')

    K scalars behave like Numeric scalars
    >>> asarray([1,2,3]) + asarray(k('0.5'))
    array([ 1.5,  2.5,  3.5])
    >>> K(array(1.5))
    k('1.5')
    """
    __doc__ += """
    Low level interface
    -------------------

    The K type provides a set of low level functions that are similar
    to the C API provided by the k.h header. The C API functions that
    return K objects in C are implemented as class methods that return
    instances of K type.

    Atoms:
    >>> K._kb(True), K._kg(5), K._kh(42), K._ki(-3), K._kj(2**40), K._ke(3.5)
    (k('1b'), k('0x05'), k('42h'), k('-3'), k('1099511627776j'), k('3.5e'))

    >>> K._kf(1.0), K._kc('x'), K._ks('xyz')
    (k('1f'), k('"x"'), k('`xyz'))

    >>> K._kd(0), K._kz(0.0), K._kt(0)
    (k('2000.01.01'), k('2000.01.01T00:00:00.000'), k('00:00:00.000'))


    Tables and dictionaries:
    >>> x = K._xD(k('`a`b`c'), k('1 2 3')); x, K._xT(x)
    (k('`a`b`c!1 2 3'), k('+`a`b`c!1 2 3'))

    Keyed table:
    >>> t = K._xD(K._xT(K._xD(k(",`a"), k(",1 2 3"))),
    ...           K._xT(K._xD(k(",`b"), k(",10 20 30"))))
    >>> K._ktd(t)
    k('+`a`b!(1 2 3;10 20 30)')

    """
    # Lighten the K objects by preventing the automatic creation of
    # __dict__ and __weakref__ for each instance.
    __slots__ = ()
    def __new__(self, x):
        tx = type(x)
        if tx is K:
            return x
        try:
            array_struct = x.__array_struct__
        except AttributeError:
            pass
        else:
            return K._from_array_interface(array_struct)
        c = converters[tx]
        return c(x)

    def _call(self, *args):
        """call the k object

        Arguments are automatically converted to appropriate k objects
        >>> k('+')(date(1999,12,31), 2)
        k('2000.01.02')

        Strings are converted into symbols, use kp to convert to char
        vectors:
        >>> map(k('{x}'), ('abc', kp('abc')))
        [k('`abc'), k('"abc"')]

        """
        return super(K, self).__call__(*map(K, args))

    def _call_lambda(self, *args, **kwds):
        """call the k lambda

        >>> f = q('{[a;b]a-b}')
        >>> assert f(1,2) == f(1)(2) == f(b=2)(1) == f(b=2,a=1)
        >>> f(1,a=2)
        Traceback (most recent call last):
        ...
        TypeError: {[a;b]a-b} got multiple values for argument 'a'
        """
        if not kwds:
            return self._call(*args)
        names = self._k(0, '{(value x)1}', self)
        kargs = [nil]*len(names)
        l = len(args)
        kargs[:l] = args
        for i,n in enumerate(names):
            v = kwds.get(n)
            if v is not None:
                if i >= l:
                    kargs[i] = v
                else:
                    raise TypeError("%s got multiple values for argument '%s'"
                                    % (self, n))
        return self._call(*(kargs or ['']))

    __call__ = K_call_proxy()

    def __getitem__(self, x):
        """
        >>> k("10 20 30 40 50")[k("1 3")]
        k('20 40')
        >>> k("`a`b`c!1 2 3")['b']
        k('2')
        """
        return self._k(0, "@", self, K(x))

    def __getattr__(self, a):
        """table columns can be accessed via dot notation

        >>> q("([]a:1 2 3; b:10 20 30)").a
        k('1 2 3')
        >>> q("([a:1 2 3]b:10 20 30)").b
        k('10 20 30')
        """
        t = self.inspect('t')
        if t == 98:
            return self._k(0, '{x`%s}'%a, self)
        if t == 99:
            return self._k(0, '{(0!x)`%s}'%a, self)
        raise AttributeError

    def __int__(self):
        """converts K scalars to python int

        >>> map(int, map(q, '1b 2h 3 4e `5 6.0 2000.01.08'.split()))
        [1, 2, 3, 4, 5, 6, 7]
        """
        t = self.inspect('t')
        return int(self.inspect(fields[-t]))

    def __float__(self):
        """converts K scalars to python float

        >>> map(float, map(q, '1b 2h 3 4e `5 6.0 2000.01.08'.split()))
        [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0]
        """
        t = self.inspect('t')
        return float(self.inspect(fields[-t]))

    def __nonzero__(self):
        t = self.inspect('t')
        if t < 0:
            return self.inspect(fields[-t]) != 0
        else:
            return self.inspect('n') > 0

    def __eq__(self, other):
        """
        >>> K(1) == K(1)
        True
        >>> K(1) == None
        False
        """
        try:
            y = K(other)
        except KeyError:
            return False
        return bool(k('~')(self, other))

    def __ne__(self, other):
        """
        >>> K(1) != K(2)
        True
        """
        return bool(k('~~')(self, other))

    def __len__(self):
        """the length of the object

        >>> len(q("1 2 3"))
        3
        >>> len(q("([]a:10?5;b:0)"))
        10
        >>> len(q("`a`b`c!1 2 3"))
        3
        """

        t = self.inspect('t')
        if 0 <= t < 98:
            return self.inspect('n')
        if t in (98,99):
            return int(self._k(0, 'count', self))
        raise NotImplementedError

    def __contains__(self, item):
        """membership test

        >>> 1 in q('1 2 3')
        True

        >>> 'abc' not in q('(1;2.0;`abc)')
        False
        """
        if self.inspect('t'):
            x = q('in', item, self)
        else:
            x = q('{sum x~/:y}', item, self)
        return bool(x)

    def __get__(self, client, cls):
        """allow K objects use as descriptors"""
        if client is None:
            return self
        return self._a1(client)

    def keys(self):
        """returns q('key', self)

        Among other uses, enables interoperability between q and
        python dicts.
        >>> dict(q('`a`b!1 2'))
        {'a': k('1'), 'b': k('2')}
        >>> d = {}; d.update(q('`a`b!1 2'))
        >>> d
        {'a': k('1'), 'b': k('2')}

        An elegant idiom to unpack q tables:
        >>> u = locals().update
        >>> for r in q('([]a:`x`y`z;b:1 2 3;c:"XYZ")'):
        ...     u(r); print a, b, c
        x 1 X
        y 2 Y
        z 3 Z
                    
        """
        return self._k(0, 'key', self)
    
    __doc__ += """
    Q objects can be used in Python arithmetic expressions

    >>> x,y,z = map(K, (1,2,3))
    >>> print x + y, x * y, z/y, x|y, x&y, abs(-z)
    3 2 1.5 2 1 3

    Mixing Q objects with python numbers is allowed
    >>> 1/q('1 2 4')
    k('1 0.5 0.25')
    >>> q.til(5)**2
    k('0 1 4 9 16f')
    """

for spec, verb in [('add', '+'), ('sub', '-'), ('mul', '*'), ('pow', 'xexp'),
                   ('div', '%'), ('rdiv', '{y%x}'), ('and', '&'), ('or', '|'),
                   ('mod', 'mod'), ('pos', '+:'), ('neg', '-:'), ('abs', 'abs')]:
    setattr(K, '__%s__' % spec, K._k(0, verb))
del spec, verb
for spec in 'add sub mul pow and or mod'.split():
    setattr(K, '__r%s__' % spec, getattr(K, '__%s__' % spec))
del spec

fields = " g@ ghijefgs iif iii"

def k(m, *args):
    return K._k(0, 'k)'+m, *map(K, args))

class _Q(object):
    def __call__(self, m, *args):
        return K._k(0, m, *map(K, args))

    def __getattr__(self, attr):
        k = K._k
        try:
            return k(0, attr)
        except kerr:
            pass
        raise AttributeError(attr)

    def __setattr__(self, attr, value):
        k = K._k
        k(0, "{%s::x}" % attr, value) 

    def __delattr__(self, attr):
        k = K._k
        k(0, "delete %s from `." % attr) 

__doc__ += """
Q variables can be accessed a attributes of the 'q' object:
>>> q.test = q('([]a:1 2;b:`x`y)')
>>> sum(q.test.a)
3
>>> del q.test
"""
q = _Q()
nil = q('(value +[;0])1')

def show(x, start=0, output=None, geometry=None):
    """pretty-print data to the console

    (similar to q.show, but uses python stdout by default)

    >>> x = q('([k:`x`y`z]a:1 2 3;b:10 20 30)')
    >>> show(x)
    k| a b 
    -| ----
    x| 1 10
    y| 2 20
    z| 3 30

    The first optional argument, 'start' specifies the first row to be
    printed (negative means from the end):
    >>> show(x, 2)
    k| a b 
    -| ----
    z| 3 30
    
    >>> show(x, -2)
    k| a b 
    -| ----
    y| 2 20
    z| 3 30

    The geometry is the height and width of the console
    >>> show(x, geometry=[4,6])
    k| a..
    -| -..
    x| 1..
    ..

    """
    if output is None:
        import sys;
        output = sys.stdout
    if geometry is None:
        geometry = q.value(kp("\\c"))
    if start < 0:
        start += q.count(x)
    S = q('{` sv .Q.S[x;y;z]}')
    output.write(buffer(S(geometry,start,x)))

def inttok(x):
    """converts python int to k

    >>> inttok(2**40)
    k('1099511627776j')
    >>> inttok(42)
    k('42')
    >>> inttok(2**65)
    Traceback (most recent call last):
    ...
    OverflowError: long too big to convert
    """
    try:
        return K._ki(x)
    except OverflowError:
        return K._kj(x)


datetimetok = K._kzz
__doc__ += """
datetimetok converts python datetime to k (DEPRECATED)

>>> datetimetok(datetime(2006,5,3,2,43,25,999000))
k('2006.05.03T02:43:25.999')
"""

datetok = K._kdd
__doc__ += """
datetok converts python date to k (DEPRECATED)

>>> datetok(date(2006,5,3))
k('2006.05.03')

"""

timetok = K._ktt
__doc__ += """
timetok converts python time to k (DEPRECATED)

>>> timetok(time(12,30,0,999000))
k('12:30:00.999')

"""

def _ni(x): raise NotImplementedError
_X = {str:K._S, int:K._I, float:K._F, date:_ni, time:_ni, datetime:_ni}
def listtok(x):
    """converts python list to k

    >>> listtok([])
    k('()')
    
    Type is determined by the type of the first element of the list
    >>> listtok(list("abc"))
    k('`a`b`c')
    >>> listtok([1,2,3])
    k('1 2 3')
    >>> listtok([0.5,1.0,1.5])
    k('0.5 1 1.5')

    All elements must have the same type for conversion
    >>> listtok([0.5,'a',5])
    Traceback (most recent call last):
      ...
    TypeError: K._F: 2-nd item is not an int
    
    """
    if x:
        return _X[type(x[0])](x)
    return K._ktn(0,0)

def tupletok(x):
    """converts python tuple to k

    Tuples are converted to general lists, strings in tuples are
    converted to char lists.

    >>> tupletok((kp("insert"), 't', (1, "abc")))
    k('("insert";`t;(1;`abc))')
    """
    return K._K(K(i) for i in x)
    
kp = K._kp

converters = {
    K: lambda x: x,
    type(None): lambda x: K._k(0, "::"),
    bool: K._kb,
    int: inttok,
    long: inttok,
    float: K._kf,
    date: K._kdd,
    datetime: K._kzz,
    time: K._ktt,
    str: K._ks,
    list: listtok,
    tuple: tupletok,
    type(lambda:0): K._func,
    buffer: K._kp
    }
try:
    converters[timedelta] = K._knz
except AttributeError:
    pass
###############################################################################
# Lazy addition of converters
###############################################################################
import __builtin__
_imp=__builtin__.__import__
def __import__(name, *args, **kwds): 
    m = _imp(name, *args, **kwds)
    if name == 'uuid':
        converters[m.UUID] = lambda u: K._kguid(u.int)
    return m
__builtin__.__import__ = __import__
###############################################################################
try:
    import MA, Numeric
except ImportError:
    pass
else:
    null = {}
    for typecode in "fislO":
        null[typecode] = K(Numeric.array([],typecode))[0]
    del typecode
    converters[MA.array] = lambda(a): K(a.filled(null[a.typecode()]))
    # Make sure MA does not mess with array repr:
    MA.set_print_limit()
    del MA, Numeric
__test__ = {}
try:
    from numpy import array
except ImportError:
    pass
else:
    __test__["array interface (vector)"] ="""
    >>> K._from_array_interface(array([1, 0, 1], bool).__array_struct__)
    k('101b')
    >>> K._from_array_interface(array([1, 2, 3], 'h').__array_struct__)
    k('1 2 3h')
    >>> K._from_array_interface(array([1, 2, 3], 'i').__array_struct__)
    k('1 2 3')
    >>> K._from_array_interface(array([1, 2, 3], 'q').__array_struct__)
    k('1 2 3j')
    >>> K._from_array_interface(array([1, 2, 3], 'f').__array_struct__)
    k('1 2 3e')
    >>> K._from_array_interface(array([1, 2, 3], 'd').__array_struct__)
    k('1 2 3f')
    """
    __test__["array interface (scalar)"] ="""
    >>> K._from_array_interface(array(1, bool).__array_struct__)
    k('1b')
    >>> K._from_array_interface(array(1, 'h').__array_struct__)
    k('1h')
    >>> K._from_array_interface(array(1, 'i').__array_struct__)
    k('1')
    >>> K._from_array_interface(array(1, 'q').__array_struct__)
    k('1j')
    >>> K._from_array_interface(array(1, 'f').__array_struct__)
    k('1e')
    >>> K._from_array_interface(array(1, 'd').__array_struct__)
    k('1f')
    """

def _test():
    import doctest, sys
    doctest.testmod(sys.modules[__name__])

if __name__ == "__main__":
    _test()
