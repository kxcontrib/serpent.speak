"""q client

"""
from q import *
class Client(object):
    """
    >>> c = Client(port=1234)
    >>> x = c('t:([]a:();b:())')
    >>> x = c("insert", 't', (1,'x'))
    >>> x = c.insert('t', (2,'y'))
    >>> x = c("insert", 't', (3,'z'))
    >>> c("select sum a from t")
    k('+(,`a)!,,6')
    >>> del c
    """
    def __init__(self, host='', port=5001):
        self._h = q("hopen`:%s:%s" %(host, port))
    def __del__(self):
        q.hclose(self._h)
    def __call__(self, query, *args):
        if args:
            return K._a1(self._h, K._K([kp(query)]+map(K,args)))
        return K._a1(self._h, kp(query))
    def __getattr__(self, attr):
        def f(*args):
            return K._a1(self._h, K._K([kp(attr)]+map(K,args)))
        f.__name__ = attr
        return f

def _test():
    import doctest
    doctest.testmod()


if __name__ == "__main__":
    _test()
