PYVER:"2.7"

\d .p
@[`p 2:(`i;1);"libpython",@[`.;`PYVER],".so\000"]
e:{e0 x,"\000";}
\d .
p)import sys, os; sys.executable = 'q'
p)from q import q
p)script = str(q(".z.f"))
p)sys.argv = [script] + map(str, q(".z.x"))
p)sys.modules['__main__'].__file__ = script
p)sys.path.insert(0, os.path.dirname(script))
p)del sys, os, script
