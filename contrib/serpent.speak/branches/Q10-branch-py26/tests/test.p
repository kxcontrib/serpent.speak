# -*- python -*-
import sys
from q import q
def test():
    pass
assert len(sys.argv) == len(q(".z.x")) + 1, "test argv"
q("p)print 'ok'")
sys.exit(0)
