#! /usr/bin/env q
PYVER:"2.7"

args:{$["@"~last x;-1_x;x]} each .z.x
py:`py 2:(`py;3)
lib:"libpython",PYVER,".so\000"
`QVER setenv .Q.fmt[3;1].Q.k
r:py[.z.f;args;lib]
\\
