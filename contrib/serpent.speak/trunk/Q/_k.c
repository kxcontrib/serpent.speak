/* -*- mode: c; c-basic-offset: 8 -*- */
static char __version__[] = "$Revision: 1.40 $";
/*
  K object layout (32 bit):
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
+-------+---+---+-------+--------
|   r   | t | u |   n   |  G0 ...  
+-------+---+---+-+-----+--------
                |g|
                +-+-+
                | h |
                +---+---+
                |   i   |
                +-------+-------+
                |       j       |
                +-------+ ------+
                |   e   |
                +-------+-------+
                |       f       |
                +---+---+-------+
                |   s   |
                +-------+
                |   k   |
                +-------+
*/
#include "Python.h"
#include "k.h"
/* these should be in k.h */
ZK km(I i){K x = ka(-KM);xi=i;R x;}
ZK ku(I i){K x = ka(-KU);xi=i;R x;}
ZK kv(I i){K x = ka(-KV);xi=i;R x;}
#include <stdlib.h>
typedef struct {
        PyObject_HEAD
        K x;
} KObject;
static PyTypeObject K_Type;

#define K_Check(op) PyObject_TypeCheck(op, &K_Type)
#define K_CheckExact(op) ((op)->ob_type == &K_Type)

#include <stdio.h>
#include <ctype.h>
#ifndef  Py_RETURN_NONE
#   define Py_RETURN_NONE Py_INCREF(Py_None); return Py_None
#endif
#if PY_VERSION_HEX < 0x2050000
typedef int Py_ssize_t;
typedef Py_ssize_t (*readbufferproc)(PyObject *, Py_ssize_t, void **);
typedef Py_ssize_t (*writebufferproc)(PyObject *, Py_ssize_t, void **);
typedef Py_ssize_t (*segcountproc)(PyObject *, Py_ssize_t *);
#endif

PyDoc_STRVAR(module_doc,
"q interface module.\n"
"\n"
"Provides a K object - python handle to q datatypes.\n"
"\n"
"This module is implemented as a thin layer on top of C API, k.h. Most\n"
"functions described in http://kx.com/q/c/c.txt are\n"
"implemented in this module. Names of the functions are formed by\n"
"prefixing the names with an _. Note that this module\n"
"is not intended for the end user, but rather as a building block for\n"
"the higher level q module\n"
"\n"
"Summary of q datatypes\n"
"type: KB KG KH KI KJ KE KF KC KS KD KT KZ 0(nested list)\n"
"code:  1  4  5  6  7  8  9 10 11 14 19 15\n"
">>> KB,KG,KH,KI,KJ,KE,KF,KC,KS,KD,KT,KZ\n"
"(1, 4, 5, 6, 7, 8, 9, 10, 11, 14, 19, 15)\n"
"\n"
"t| size literal      q         c                \n"
"------------------------------------------------\n"
"b  1    0b           boolean   unsigned char	 \n"
"x  1    0x0          byte      unsigned char	 \n"
"h  2    0h           short     short	         \n"
"i  4    0            int       int	         \n"
"j  8    0j           long      long long	 \n"
"e  4    0e           real      float	         \n"
"f  8    0.0          float     double	         \n"
"c  1    " "          char      unsigned char	 \n"
"s  .    `            symbol    unsigned char*	 \n"
"m  4    2000.01m     month     int	         \n"
"d  4    2000.01.01   date      int	         \n"
"z  8    dateTtime    datetime  double	         \n"
"u  4    00:00        minute	                 \n"
"v  4    00:00:00     second	                 \n"
"t  4    00:00:00.000 time	                 \n"
"*  4    `s$`         enum	                 \n"
);
/* K objects */
static PyObject *ErrorObject;
static PyObject *
KObject_FromK(PyTypeObject *type, K x)
{
	if (!type) 
		type = &K_Type;
	if (!x) {
		extern void clr(void);
		extern char* es;
		PyErr_SetString(ErrorObject, es);
		clr();
		return NULL;
	}
	if (xt == -128)
		return PyErr_Format(ErrorObject, xs?xs:"not set"),r0(x),NULL;
	KObject *self = (KObject*)type->tp_alloc(type, 0);
	if (self)
		self->x = x;
	return (PyObject*)self;
}
/* converter function */
static int
getK(PyObject *arg, void *addr)
{
	if (!K_Check(arg)) {
		return PyErr_BadArgument();
	}
	K r = ((KObject*)arg)->x;
	/*
	if (!r) {
		return PyErr_BadArgument();
	}
	*/
	*(K*)addr = r;
	return 1;
}

static void
K_dealloc(KObject *self)
{
	if (self->x) {
		r0(self->x);
	}
	self->ob_type->tp_free(self);
}

static PyObject*
K_dot(KObject *self, PyObject *args)
{
	R K_Check(args)
		? KObject_FromK(self->ob_type, 
				dot(self->x, ((KObject*)args)->x))
		: PyErr_Format(PyExc_TypeError, "expected a K object, not %s",
			       args->ob_type->tp_name);
}

extern K a1(K,K);
static PyObject*
K_a1(KObject *self, PyObject *arg)
{
	R K_Check(arg)
		? KObject_FromK(self->ob_type, a1(self->x, ((KObject*)arg)->x))
		: PyErr_Format(PyExc_TypeError, "expected a K object, not %s",
			       arg->ob_type->tp_name);
}

extern K a2(K,K,K);
static PyObject*
K_a2(KObject *self, PyObject *args)
{
	K x, y;
	if (PyArg_ParseTuple(args, "O&O&", &getK, &x, &getK, &y))
		R KObject_FromK(self->ob_type, a2(self->x, x, y));
	return NULL;
}

extern K a3(K,K,K,K);
static PyObject*
K_a3(KObject *self, PyObject *args)
{
	K x, y, z;
	if (PyArg_ParseTuple(args, "O&O&O&", &getK, &x, &getK, &y, &getK, &z))
		R KObject_FromK(self->ob_type, a3(self->x, x, y, z));
	return NULL;
}

static PyObject*
K_call(KObject *self, PyObject *args, PyObject *kw)
{
	I n = PyTuple_GET_SIZE(args);
	K x;
	if (kw) {
		
	}
	x = ktn(0,n);

	return KObject_FromK(self->ob_type, k(0, ".", self->x, x));
}

static PyObject*
K_str(KObject *self)
{
	K x = self->x;
	switch (xt) {
	case KC:
		return PyString_FromStringAndSize(xC, xn);
	case -KS:
		return PyString_FromString(xs);
	}
	return PyString_FromFormat("<%s object of type %d at 0x%p>",
				   self->ob_type->tp_name, xt, x);
}

/** Array interface **/

/* Array Interface flags */
#define FORTRAN       0x002
#define ALIGNED       0x100
#define NOTSWAPPED    0x200
#define WRITEABLE     0x400

typedef struct {
	int version;
	int nd;
	char typekind;
	int itemsize;
	int flags;
	Py_intptr_t *shape;
	Py_intptr_t *strides;
	void *data;
} PyArrayInterface;
static char typechars[] = "ObXXuiiiffSOXiifXiif";

static int
k_typekind(K x)
{
	int t = abs(x->t);
	if (t < sizeof(typechars) - 1)
		return typechars[t];
	return 'X';
}

static int
k_itemsize(K x)
{
	static int itemsizes[] = {
		sizeof(void*),
		1, /* bool */
		0, 0, 1, /* byte */
		2, /* short */
		4, /* int */
		8, /* long */
		4, /* float */
		8, /* real */
		1, /* char */
		sizeof(void*), /* symbol */
		0, 4, /* month */
		4, /* date */
		8, /* datetime */
		0, 4, /* minute */
		4, /* second */
		8, /* time */
	};
	int t = abs(x->t);
	if (t < sizeof(itemsizes)/sizeof(*itemsizes))
		return itemsizes[t];
	return 0;
}

static void
k_array_struct_free(void *ptr, void *arr)
{
	PyArrayInterface *inter	= (PyArrayInterface *)ptr;
	free(inter->shape);
        free(inter);
	Py_DECREF((PyObject *)arr);
}

static PyObject *
K_array_struct_get(KObject *self)
{
	K k = self->x;
	if (!k) {
		PyErr_SetString(PyExc_AttributeError, "Null k object");
		return NULL;
	}
        PyArrayInterface *inter;
	int typekind =  k_typekind(k);
	if (strchr("XO", typekind)) {
		PyErr_Format(PyExc_AttributeError, "Unsupported K type %d"
			     " (maps to %c)", k->t, (char)typekind);
		return NULL;
	}
        if (!(inter = (PyArrayInterface *)malloc(sizeof(PyArrayInterface))))
		goto fail_inter;
	int nd = (k->t >= 0); /* scalars have t < 0 in k4 */
        inter->version = 2;
        inter->nd = nd;
        inter->typekind = typekind;
        inter->itemsize = k_itemsize(k);
        inter->flags = ALIGNED|NOTSWAPPED|WRITEABLE;
	if (nd) {
		if (!(inter->shape = (Py_intptr_t*)malloc(sizeof(Py_intptr_t)*2)))
			goto fail_shape;
		inter->shape[0] = k->n;
		inter->strides = inter->shape + 1;
		inter->strides[0] = inter->itemsize;
		inter->data = kG(k);
	}
	else {
		inter->shape = inter->strides = NULL;
		inter->data = &k->g;
	}
	Py_INCREF(self);
	return PyCObject_FromVoidPtrAndDesc(inter, self, &k_array_struct_free);
 fail_shape:
	free(inter);
 fail_inter:
	return PyErr_NoMemory();
}

static PyObject *
K_array_typestr_get(KObject *self)
{
	K k = self->x;
	static int const one = 1;
	char const endian = "><"[(int)*(char const*)&one];
	char const typekind = k_typekind(k);
	return PyString_FromFormat("%c%c%d", typekind == 'O'?'|':endian,
				   typekind, k_itemsize(k));
}


static int
k_ktype(int typekind, int itemsize)
{
	switch (typekind) {
	case 'f':
		switch (itemsize) {
		case 4:
			return KE;
		case 8:
			return KF;
		default:
			return -1;
		}
	case 'i':
		switch (itemsize) {
		case 2:
			return KH;
		case 4:
			return KI;
		case 8:
			return KJ;
		default:
			return -1;
		}
	case 'b':
		return KB;
	case 'O':
		return KS;
	}
	return -1;
}

/* K class methods */
PyDoc_STRVAR(K_from_array_interface_doc, 
             "K object from __array_struct__"); 
static PyObject * 
K_from_array_interface(PyTypeObject *type, PyObject *arg) 
{
	PyArrayInterface *inter;
	K x;
	int t, s;
	if (!PyCObject_Check(arg) 
	    || ((inter=((PyArrayInterface *)PyCObject_AsVoidPtr(arg)))->version != 2)) 
		{
			PyErr_SetString(PyExc_ValueError, "invalid __array_struct__");
			return NULL;
		}
	if (inter->nd > 1) {
		PyErr_Format(PyExc_ValueError, "cannot handle nd=%d",
			     inter->nd);
		return NULL;
	}
	s = inter->itemsize;
	t = k_ktype(inter->typekind, s);
	if (t < 0) {
		PyErr_Format(PyExc_ValueError, "cannot handle type '%c%d'",
			     inter->typekind, inter->itemsize);
		return NULL;
	}
	x = inter->nd?ktn(t, inter->shape[0]):ka(-t);
	if (!x)
		return PyErr_NoMemory();
	if (xt == -128) {
		PyErr_SetString(ErrorObject, xs);
                return NULL;
	}
	if (t == KS) {
		PyObject** src =  (PyObject**)inter->data;
		if (inter->nd) {
			S* dest = xS;
			int n = inter->shape[0], i;
			dest = xS;
			for(i = 0; i < n; ++i) {
				PyObject* obj = src[i*inter->strides[0]/s];
				if (!PyString_Check(obj)) {
					PyErr_SetString(PyExc_ValueError, "non-string in object array");
					r0(x);
					return NULL;
				}
				dest[i] = sn(PyString_AS_STRING(obj),  PyString_GET_SIZE(obj));
			}
		} else {
			PyObject* obj = src[0];
			xs = sn(PyString_AS_STRING(obj),  PyString_GET_SIZE(obj));
		}
	}
	else {
		if (inter->nd) {
			int n = inter->shape[0];
			DO(n, memcpy(xG+i*s, inter->data+i*inter->strides[0], s));
		}
		else
			memcpy(&x->g, inter->data, inter->itemsize);
	}
	return KObject_FromK(type, x);
}

PyDoc_STRVAR(K_ktd_doc,
	     "flip from keyedtable(dict)");
static PyObject *
K_ktd(PyTypeObject *type, PyObject *args)
{
	K x = 0;
	if (!PyArg_ParseTuple(args, "O&", &getK, &x)) {
		return NULL;
	}
	if (!x) {
		PyErr_BadArgument();
		return NULL;
	}
	return KObject_FromK(type, ktd(r1(x)));
}

PyDoc_STRVAR(K_err_doc,
	     "sets a K error\n"
	     "\n"
	     ">>> K.err('test')\n");
static PyObject *
K_err(PyTypeObject *type, PyObject *args)
{
	S s;
	if (!PyArg_ParseTuple(args, "s:err", &s)) {
		return NULL;
	}
	krr(s);
	Py_RETURN_NONE;
}

#define K_ATOM(a, T, t, doc)					\
	PyDoc_STRVAR(K_k##a##_doc, doc);			\
	static PyObject *					\
	K_k##a(PyTypeObject *type, PyObject *args)		\
	{							\
		T g;						\
		if (!PyArg_ParseTuple(args, #t ":K.k"#a, &g))	\
			return NULL;				\
		K x = k##a(g);					\
		if (!type) {					\
			type = &K_Type;				\
		}						\
		return KObject_FromK(type, x);			\
	}			
K_ATOM(a, I, i, "returns a K atom")
K_ATOM(b, G, b, "returns a K bool")
K_ATOM(g, G, b, "returns a K byte")
K_ATOM(h, H, h, "returns a K short")
K_ATOM(i, I, i, "returns a K int")
ZS th(I i){SW(i){CS(1,R "st")CS(2,R "nd")};R "th";}
PyDoc_STRVAR(K_I_doc,
	     "returns a K int list");
static PyObject *
K_I(PyTypeObject *type, PyObject *arg)
{
	PyObject *seq = PySequence_Fast(arg, "K._I: not a sequence");
	if (seq == NULL)
		return NULL;
	int i, n = PySequence_Fast_GET_SIZE(seq);
	K x = ktn(KI, n);
	for (i = 0; i < n; ++i) {
		PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
		if (!PyInt_Check(o)) {
			r0(x);
			PyErr_Format(PyExc_TypeError,
				     "K._I: %d-%s item is not an int", i+1, th(i+1));
			return NULL;
		}
		long item = PyInt_AS_LONG(o);
		if (sizeof(I) != sizeof(long) && item != (I)item) {
			r0(x);
			PyErr_Format(PyExc_TypeError,
				     "K._I: %d-%s item (%ld) is too big", i+1, th(i+1), item);
			return NULL;
		}
		xI[i] = (I)item;
	}
	return KObject_FromK(type, x);
}
K_ATOM(j, J, L, "returns a K long (64 bits)")
K_ATOM(e, E, f, "returns a K real (32 bits)")
K_ATOM(f, F, d, "returns a K float (64 bits)")
PyDoc_STRVAR(K_F_doc,
	     "returns a K float list");
static PyObject *
K_F(PyTypeObject *type, PyObject *arg)
{
	PyObject *seq = PySequence_Fast(arg, "K._F: not a sequence");
	if (seq == NULL)
		return NULL;
	int i, n = PySequence_Fast_GET_SIZE(seq);
	K x = ktn(KF, n);
	for (i = 0; i < n; ++i) {
		PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
		if (!PyFloat_Check(o)) {
			r0(x);
			PyErr_Format(PyExc_TypeError,
				     "K._F: %d-%s item is not an int", i+1, th(i+1));
			return NULL;
		}
		xF[i] = PyFloat_AS_DOUBLE(o);
	}
	return KObject_FromK(type, x);
}
K_ATOM(c, G, c,	"returns a K char")

PyDoc_STRVAR(K_ks_doc,
	     "returns a K symbol");
static PyObject *
K_ks(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	S s; I n;
	if (!PyArg_ParseTuple(args, "s#", &s, &n, &K_Type, &ret)) {
		return NULL;
	}
	K x = ks(sn(s,n));
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}

PyDoc_STRVAR(K_S_doc,
	     "returns a K symbol list");
static PyObject *
K_S(PyTypeObject *type, PyObject *arg)
{
	PyObject *seq = PySequence_Fast(arg, "K._S: not a sequence");
	if (seq == NULL)
		return NULL;
	int i, n = PySequence_Fast_GET_SIZE(seq);
	K x = ktn(KS, n);
	for (i = 0; i < n; ++i) {
		PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
		if (!PyString_Check(o)) {
			r0(x);
			PyErr_Format(PyExc_TypeError,
				     "K._S: %d-%s item is not a string", i+1, th(i+1));
			return NULL;
		}
		xS[i] = sn(PyString_AS_STRING(o), PyString_GET_SIZE(o));
	}
	return KObject_FromK(type, x);
}
K_ATOM(m, I, i, "returns a K month")
K_ATOM(d, I, i, "returns a K date")
K_ATOM(z, F, d, "returns a K datetime")
K_ATOM(u, I, i, "returns a K minute")
K_ATOM(v, I, i, "returns a K second")
K_ATOM(t, I, i, "returns a K time")
PyDoc_STRVAR(K_kp_doc,
	     "returns a K string");
static PyObject *
K_kp(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	S s; I n;
	if (!PyArg_ParseTuple(args, "s#", &s, &n, &K_Type, &ret)) {
		return NULL;
	}
	K x = kpn(s,n);
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}

PyDoc_STRVAR(K_K_doc,
	     "returns a K general list");
static PyObject *
K_K(PyTypeObject *type, PyObject *arg)
{
	PyObject *seq = PySequence_Fast(arg, "K._K: not a sequence");
	if (seq == NULL)
		return NULL;
	int i, n = PySequence_Fast_GET_SIZE(seq);
	K x = ktn(0, n);
	for (i = 0; i < n; ++i) {
		PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
		if (!K_Check(o)) {
			r0(x);
			PyErr_Format(PyExc_TypeError,
				     "K._K: %d-%s item is not a K object", i+1, th(i+1));
			return NULL;
		}
		xK[i] = r1(((KObject*)o)->x);
	}
	return KObject_FromK(type, x);
}


PyDoc_STRVAR(K_ktn_doc,
	     "returns a K list");
static PyObject *
K_ktn(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	I i1, i2;
	if (!PyArg_ParseTuple(args, "ii", &i1, &i2, &K_Type, &ret)) {
		return NULL;
	}
	K x = ktn(i1,i2);
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}

PyDoc_STRVAR(K_xT_doc,
	     "table from dictionary");
static PyObject *
K_xT(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	K k0, k;
	if (!PyArg_ParseTuple(args, "O&|O!", &getK, &k0, &K_Type, &ret)) {
		return NULL;
	}
	if (!k0) {
		PyErr_BadArgument();
		return NULL;
	}
	r1(k0);
	k = xT(k0);
	return KObject_FromK(type, k);
}

PyDoc_STRVAR(K_xD_doc,
	     "returns a K dict");
static PyObject *
K_xD(PyTypeObject *type, PyObject *args)
{
	K k1 = 0, k2 = 0;
	if (!PyArg_ParseTuple(args, "O&O&",
			      getK, &k1, getK, &k2))
		{
			return NULL;
		}
	if (!(k1 && k2)) {
		PyErr_BadArgument();
		return NULL;
	}
	K x = xD(r1(k1), r1(k2));
	assert(xt == XD);
	return KObject_FromK(type, x);
}


PyDoc_STRVAR(K_knk_doc,
	     "returns a K list");
static PyObject *
K_knk(PyTypeObject *type, PyObject *args)
{
	I n; K r;
	switch (PyTuple_Size(args)-1) {
	case 0: {
		if (!PyArg_ParseTuple(args, "i", &n)) {
			return NULL;
		}
		r = knk(n);
		break;
	}
	case 1: {
		K k1;
		if (!PyArg_ParseTuple(args, "iO&", &n,
				      getK, &k1))
			{
				return NULL;
			}
		r = knk(n,r1(k1));
		break;
	}
	case 2: {
		K k1,k2;
		if (!PyArg_ParseTuple(args, "iO&O&", &n,
				      getK, &k1,
				      getK, &k2))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2));
		break;
	}
	case 3: {
		K k1,k2,k3;
		if (!PyArg_ParseTuple(args, "iO&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3));
		break;
	}
	case 4: {
		K k1,k2,k3,k4;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4));
		break;
	}
	case 5: {
		K k1,k2,k3,k4,k5;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5));
		break;
	}
	case 6: {
		K k1,k2,k3,k4,k5,k6;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6));
		break;
	}
	case 7: {
		K k1,k2,k3,k4,k5,k6,k7;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7));
		break;
	}
	case 8: {
		K k1,k2,k3,k4,k5,k6,k7,k8;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),r1(k8));
		break;
	}
	case 9: {
		K k1,k2,k3,k4,k5,k6,k7,k8,k9;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&O&O&O&", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8,
				      getK, &k9))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),r1(k8),r1(k9));
		break;
	}
	default:
		PyErr_BadArgument();
		return NULL;
	}
	return KObject_FromK(type, r);
}

/*
r=k(c,s,x,y,z,(K)0); decrements(r0) x,y,z. eventually program must do r0(r);
 if one of the parameters is reused you must increment, e.g.

 K x=ks("trade");
 k(-c,s,r1(x),..,(K)0);
 k(-c,s,r1(x),..,(K)0);
 ..
 r0(x);
*/
PyDoc_STRVAR(K_k_doc,
	     "k(c, m, ...) -> k object\n");
static PyObject *
K_k(PyTypeObject *type, PyObject *args)
{
	I c;char* m;K r;
	switch (PyTuple_Size(args)-2) {
	case 0: {
		if (!PyArg_ParseTuple(args, "is", &c, &m)) {
			return NULL;
		}
		r = k(c,m,(K)0);
		break;
	}
	case 1: {
		K k1;
		if (!PyArg_ParseTuple(args, "isO&", &c, &m,
				      getK, &k1))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),(K)0);
		break;
	}
	case 2: {
		K k1,k2;
		if (!PyArg_ParseTuple(args, "isO&O&", &c, &m,
				      getK, &k1,
				      getK, &k2))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),(K)0);
		break;
	}
	case 3: {
		K k1,k2,k3;
		if (!PyArg_ParseTuple(args, "isO&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),(K)0);
		break;
	}
	case 4: {
		K k1,k2,k3,k4;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),(K)0);
		break;
	}
	case 5: {
		K k1,k2,k3,k4,k5;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),(K)0);
		break;
	}
	case 6: {
		K k1,k2,k3,k4,k5,k6;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),(K)0);
		break;
	}
	case 7: {
		K k1,k2,k3,k4,k5,k6,k7;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),(K)0);
		break;
	}
	case 8: {
		K k1,k2,k3,k4,k5,k6,k7,k8;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),r1(k8),(K)0);
		break;
	}
	case 9: {
		K k1,k2,k3,k4,k5,k6,k7,k8,k9;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&O&O&O&", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8,
				      getK, &k9))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),r1(k8),r1(k9),(K)0);
		break;
	}
	default: 
		PyErr_BadArgument();
		return NULL;
	}
	return KObject_FromK(type, r);
}



PyDoc_STRVAR(K_inspect_doc,
	     "inspect(k, c, [, i]) -> python object\n"
	     "\n"
	     ""
	     );
static PyObject *
K_inspect(PyObject *self, PyObject *args)
{
	K k = ((KObject*)self)->x;
	if (!k) {
		PyErr_SetString(PyExc_ValueError, "null k object");
		return NULL;
	}
	int i = 0;
	char c;
	if (!PyArg_ParseTuple(args, "c|i:inspect", 
			      &c, &i))
		return NULL;
	switch (c) {
	case 'r': return PyInt_FromLong(k->r);
	case 't': return PyInt_FromLong(k->t);
	case 'u': return PyInt_FromLong(k->u);
	case 'n': return PyInt_FromLong(k->n);
		/* atoms */
	case 'g': return PyInt_FromLong(k->g);
	case 'h': return PyInt_FromLong(k->h);
	case 'i': return PyInt_FromLong(k->i);
	case 'j': return PyLong_FromLongLong(k->j);
	case 'e': return PyFloat_FromDouble(k->e);
	case 'f': return PyFloat_FromDouble(k->f);
	case 's': return (k->t == -KS
			  ? PyString_FromString((char*)k->s)
			  : k->t == KC
			  ? PyString_FromStringAndSize((char*)kG(k), k->n)
			  : PyString_FromFormat("<%p>", k->s));
	case 'k': return KObject_FromK(self->ob_type, r1(k->k));
		/* lists */
	case 'G': return PyInt_FromLong(kG(k)[i]);
	case 'H': return PyInt_FromLong(kH(k)[i]);
	case 'I': return PyInt_FromLong(kI(k)[i]);
	case 'J': return PyLong_FromLongLong(kJ(k)[i]);
	case 'E': return PyFloat_FromDouble(kE(k)[i]);
	case 'F': return PyFloat_FromDouble(kF(k)[i]);
	case 'S': return (k->t == KS
			  ? PyString_FromString((char*)kS(k)[i])
			  : PyString_FromFormat("<%p>", kS(k)[i]));
	case 'K': return KObject_FromK(self->ob_type, r1(kK(k)[i]));
	}
	return PyErr_Format(PyExc_KeyError, "no such field: '%c'", c);
}

/* Calling Python */
ZK
call_python_object(K type, K func, K x)
{
	I n; K *args, r;
	if (type->t!=-KJ||func->t!=-KJ||xt<0||xt>=XT) {
		R krr("type error");
	}
	n = xn; r1(x);
	if (xt != 0) {
		x = k(0, "(::),", x, (K)0);
		args = xK+1;
	}
	else {
		args = xK;
	}	
	PyObject* pyargs = PyTuple_New(n), *res;
	DO(n, PyTuple_SET_ITEM(pyargs, i, 
			       KObject_FromK((PyTypeObject*)type->k,r1(args[i]))));
	res = PyObject_CallObject((PyObject*)func->k, pyargs);
	Py_DECREF(pyargs); r0(x);
	if(!res)
		R krr("error in python");
	r = K_Check(res)
		? r1(((KObject*)res)->x)
		: krr("py-type error");
	Py_DECREF(res);
	return r;
}

ZK
kl(H n, V* cfunc)
{
	K x = ka(112);
	xu = n;
	xk = cfunc;
	R x;
}

static PyObject *
K_func(PyTypeObject *type, PyObject *func)
{
	K f = kl(3, call_python_object);
	K kfunc = kj(0);
	K ktype = kj(0);
	kfunc->k = (K)func;
	ktype->k = (K)type;
	K x = knk(3, f, ktype, kfunc);
	xt = 104; /* projection */
	return KObject_FromK(type, x);
}

static PyMethodDef 
K_methods[] = {
	{"_func", (PyCFunction)K_func,METH_O|METH_CLASS, "func"}, 
	{"_dot", (PyCFunction)K_dot,  METH_O, "dot"},
	{"_a1", (PyCFunction)K_a1,  METH_O, "a1"},
	{"_a2", (PyCFunction)K_a2,  METH_VARARGS, "a2"},
	{"_a3", (PyCFunction)K_a3,  METH_VARARGS, "a3"},
	{"_k",	(PyCFunction)K_k,  METH_VARARGS|METH_CLASS, K_k_doc},
	{"_knk",(PyCFunction)K_knk, METH_VARARGS|METH_CLASS, K_knk_doc},
	{"_ktd",(PyCFunction)K_ktd, METH_VARARGS|METH_CLASS, K_ktd_doc},
	{"_err",(PyCFunction)K_err, METH_VARARGS|METH_CLASS, K_err_doc},
	{"_ka",	(PyCFunction)K_ka, METH_VARARGS|METH_CLASS, K_ka_doc},
	{"_kb",	(PyCFunction)K_kb, METH_VARARGS|METH_CLASS, K_kb_doc},
	{"_kg",	(PyCFunction)K_kg, METH_VARARGS|METH_CLASS, K_kg_doc},
	{"_kh",	(PyCFunction)K_kh, METH_VARARGS|METH_CLASS, K_kh_doc},
	{"_ki",	(PyCFunction)K_ki, METH_VARARGS|METH_CLASS, K_ki_doc},
	{"_I",	(PyCFunction)K_I, METH_O|METH_CLASS, K_I_doc},
	{"_kj",	(PyCFunction)K_kj, METH_VARARGS|METH_CLASS, K_kj_doc},
	{"_ke",	(PyCFunction)K_ke, METH_VARARGS|METH_CLASS, K_ke_doc},
	{"_kf",	(PyCFunction)K_kf, METH_VARARGS|METH_CLASS, K_kf_doc},
	{"_F",	(PyCFunction)K_F, METH_O|METH_CLASS, K_F_doc},
	{"_kc",	(PyCFunction)K_kc, METH_VARARGS|METH_CLASS, K_kc_doc},
	{"_ks",	(PyCFunction)K_ks, METH_VARARGS|METH_CLASS, K_ks_doc},
	{"_S",	(PyCFunction)K_S, METH_O|METH_CLASS, K_S_doc},
	{"_km",	(PyCFunction)K_km, METH_VARARGS|METH_CLASS, K_km_doc},
	{"_kd",	(PyCFunction)K_kd, METH_VARARGS|METH_CLASS, K_kd_doc},
	{"_kz",	(PyCFunction)K_kz, METH_VARARGS|METH_CLASS, K_kz_doc},
	{"_ku",	(PyCFunction)K_ku, METH_VARARGS|METH_CLASS, K_ku_doc},
	{"_kv",	(PyCFunction)K_kv, METH_VARARGS|METH_CLASS, K_kv_doc},
	{"_kt",	(PyCFunction)K_kt, METH_VARARGS|METH_CLASS, K_kt_doc},
	{"_kp",	(PyCFunction)K_kp, METH_VARARGS|METH_CLASS, K_kp_doc},
	{"_ktn",(PyCFunction)K_ktn, METH_VARARGS|METH_CLASS, K_ktn_doc},
	{"_xT",	(PyCFunction)K_xT, METH_VARARGS|METH_CLASS, K_xT_doc},
	{"_xD",	(PyCFunction)K_xD, METH_VARARGS|METH_CLASS, K_xD_doc},
	{"_K",	(PyCFunction)K_K, METH_O|METH_CLASS, K_K_doc},

	{"_from_array_interface", (PyCFunction)K_from_array_interface,
                                   METH_O|METH_CLASS, K_from_array_interface_doc},

	{"inspect", (PyCFunction)K_inspect, METH_VARARGS, K_inspect_doc},
	{NULL,		NULL}		/* sentinel */
};

static Py_ssize_t
K_buffer_getreadbuf(KObject *self, Py_ssize_t index, const void **ptr)
{
        if ( index != 0 ) {
                PyErr_SetString(PyExc_SystemError,
                                "Accessing non-existent K object segment");
                return -1;
        }
	K x = self->x;
	switch (xt) {
	case -KS:
		*ptr = (void *)xs;
		return strlen(xs);
	case KC: case KG:
		*ptr = (void *)xG;
		return xn;
	}
	PyErr_Format(PyExc_NotImplementedError,
		     "Buffer protocol not implemented for type %d", (I)xt);
	return -1;
}

static Py_ssize_t
K_buffer_getwritebuf(KObject *self, Py_ssize_t index, const void **ptr)
{
        if ( index != 0 ) {
                PyErr_SetString(PyExc_SystemError,
                                "Accessing non-existent K object segment");
                return -1;
        }
	K x = self->x;
	switch (xt) {
	case KC: case KG:
		*ptr = (void *)xG;
		return xn;
	}
	PyErr_Format(PyExc_NotImplementedError,
		     "Buffer protocol not implemented for type %d", (I)xt);
	return -1;
}

static Py_ssize_t
K_buffer_getsegcount(KObject *self, Py_ssize_t *lenp)
{
        if ( lenp )
                *lenp = self->xn;
        return 1;
}

static PyBufferProcs K_as_buffer = {
        (readbufferproc)K_buffer_getreadbuf,
        (writebufferproc)K_buffer_getwritebuf,
        (segcountproc)K_buffer_getsegcount,
        NULL,
};

static PyGetSetDef K_getset[] = {
	{"__array_struct__", (getter)K_array_struct_get, NULL,
         "Array protocol: struct"},
	{"__array_typestr__", (getter)K_array_typestr_get, NULL,
         "Array protocol: typestr"},
	{NULL, NULL, NULL, NULL},  /* Sentinel */
};
static PyObject *k_iter(KObject *o);

static PyTypeObject K_Type = {
	/* The ob_type field must be initialized in the module init function
	 * to be portable to Windows without using C++. */
	PyObject_HEAD_INIT(NULL)
	0,			/*ob_size*/
	"_k.K",	         	/*tp_name*/
	sizeof(KObject),	/*tp_basicsize*/
	0,			/*tp_itemsize*/
	/* methods */
	(destructor)K_dealloc, /*tp_dealloc*/
	0,			/*tp_print*/
	0,                      /*tp_getattr*/
        0,                      /*tp_setattr*/
	0,			/*tp_compare*/
	0,			/*tp_repr*/
	0,			/*tp_as_number*/
	0,                      /*tp_as_sequence*/
	0,			/*tp_as_mapping*/
	0,			/*tp_hash*/
        (ternaryfunc)K_call,    /*tp_call*/
        (reprfunc)K_str,        /*tp_str*/
        0,                      /*tp_getattro*/
        0,                      /*tp_setattro*/
        &K_as_buffer,           /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,     /*tp_flags*/
        0,                      /*tp_doc*/
        0,                      /*tp_traverse*/
        0,                      /*tp_clear*/
        0,                      /*tp_richcompare*/
        0,                      /*tp_weaklistoffset*/
        (getiterfunc)k_iter,    /*tp_iter*/
        0,                      /*tp_iternext*/
        K_methods,                      /*tp_methods*/
        0,                      /*tp_members*/
        K_getset,                      /*tp_getset*/
        0,                      /*tp_base*/
        0,                      /*tp_dict*/
        0,                      /*tp_descr_get*/
        0,                      /*tp_descr_set*/
        0,                      /*tp_dictoffset*/
        (initproc)0,            /*tp_init*/
        0,                      /*tp_alloc*/
        PyType_GenericNew,      /*tp_new*/
        0,                      /*tp_free*/
        0,                      /*tp_is_gc*/
};
/* --------------------------------------------------------------------- */


PyDoc_STRVAR(_k_khp_doc,
	     "khp(host,port) -> connection handle\n"
	     "\n"
	     ">>> c = khp('localhost', 5001)\n");

static PyObject *
_k_khp(PyObject *self, PyObject *args)
{
	char *h;
	int p;
	if (!PyArg_ParseTuple(args, "si:khp", &h, &p))
		return NULL;
	return PyInt_FromLong(khp(h, p));
}

PyDoc_STRVAR(_k_ymd_doc,
	     "ymd(y,m,d) -> q date\n"
	     "\n"
	     ">>> ymd(2000, 1, 1)\n"
	     "0\n");

static PyObject *
_k_ymd(PyObject *self, PyObject *args)
{
	int y, m, d;
	if (!PyArg_ParseTuple(args, "iii:ymd", &y, &m, &d))
		return NULL;
	return PyInt_FromLong(ymd(y, m, d));
}


PyDoc_STRVAR(_k_kid_doc,
	     "kid(k) -> id of k object\n"
	     "\n");

static PyObject *
_k_kid(PyObject *self, PyObject *args)
{
	K k;
	if (!PyArg_ParseTuple(args, "O&:kid", getK, &k))
		return NULL;
	return PyInt_FromLong((long)k);
}


/* List of functions defined in the module */

static PyMethodDef _k_methods[] = {
	{"khp",		_k_khp,		METH_VARARGS, _k_khp_doc},
	{"ymd",		_k_ymd,		METH_VARARGS, _k_ymd_doc},
	{"kid",		_k_kid,		METH_VARARGS, _k_kid_doc},
	{NULL,		NULL}		/* sentinel */
};
/*********************** K Object Iterator **************************/

typedef struct {
	PyObject_HEAD
	char*			ptr;
	char*			end;
	KObject*                obj;
	size_t                  itemsize;
	int                     itemtype;
} kiterobject;

static PyTypeObject KObjectIter_Type;

#define KObjectIter_Check(op) PyObject_TypeCheck(op, &KObjectArrayIter_Type)

static PyObject *
k_iter(KObject *obj)
{
	kiterobject *it;

	if (!K_Check(obj)) {
		PyErr_BadInternalCall();
		return NULL;
	}

	it = PyObject_GC_New(kiterobject, &KObjectIter_Type);
	if (it == NULL)
		return NULL;

	Py_INCREF(obj);
	it->obj = obj;
	it->ptr = kG(obj->x);
	it->itemsize = k_itemsize(obj->x);
	if (!it->itemsize) {
		PyErr_Format(PyExc_NotImplementedError, "not iterable: t=%d", obj->xt);
		return NULL;
	}
	it->end = it->ptr + it->itemsize * obj->xn;
	it->itemtype = obj->xt;
	PyObject_GC_Track(it);
	return (PyObject *)it;
}

static PyObject *
kiter_next(kiterobject *it)
{
	PyObject *ret = NULL;
	if (it->ptr < it->end)
		switch (it->itemtype) {
		case KS: /* most common case: use list(ks) */
			ret = PyString_FromString(*(char**)it->ptr);
			break;
		case 0:
			ret = KObject_FromK(it->obj->ob_type, r1(*(K*)it->ptr));
			break;
		/* remaining cases are less common because array(x) *
		 * is a better option that list(x)                  */
		case KB:
			ret = PyBool_FromLong(*(char*)it->ptr);
			break;
		case KG:
		case KC:
			ret = PyString_FromStringAndSize(it->ptr, 1);
			break;
		case KH:
			ret = PyInt_FromLong(*(short*)it->ptr);
			break;
		case KI:
		case KM:
		case KD:
		case KV:
		case KT:
			ret = PyInt_FromLong(*(int*)it->ptr);
			break;
		case KJ:
			ret = PyLong_FromLongLong(*(long long*)it->ptr);
			break;
		case KE:
			ret = PyFloat_FromDouble(*(float*)it->ptr);
			break;
		case KF:
			ret = PyFloat_FromDouble(*(double*)it->ptr);
			break;
		}
	it->ptr += it->itemsize;
	return ret;
}

static void
kiter_dealloc(kiterobject *it)
{
	PyObject_GC_UnTrack(it);
	Py_XDECREF(it->obj);
	PyObject_GC_Del(it);
}

static int
kiter_traverse(kiterobject *it, visitproc visit, void *arg)
{
	if (it->obj != NULL)
		return visit((PyObject *)(it->obj), arg);
	return 0;
}

static PyTypeObject KObjectIter_Type = {
	PyObject_HEAD_INIT(NULL)
	0,                                      /* ob_size */
	"kiterator",                            /* tp_name */
	sizeof(kiterobject),                    /* tp_basicsize */
	0,                                      /* tp_itemsize */
	/* methods */
	(destructor)kiter_dealloc,	 	/* tp_dealloc */
	0,                                      /* tp_print */
	0,                                      /* tp_getattr */
	0,                                      /* tp_setattr */
	0,                                      /* tp_compare */
	0,                                      /* tp_repr */
	0,                                      /* tp_as_number */
	0,                                      /* tp_as_sequence */
	0,                                      /* tp_as_mapping */
	0,                                      /* tp_hash */
	0,                                      /* tp_call */
	0,                                      /* tp_str */
	PyObject_GenericGetAttr,                /* tp_getattro */
	0,                                      /* tp_setattro */
	0,                                      /* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,/* tp_flags */
	0,                                      /* tp_doc */
	(traverseproc)kiter_traverse,	        /* tp_traverse */
	0,					/* tp_clear */
	0,                                      /* tp_richcompare */
	0,                                      /* tp_weaklistoffset */
	PyObject_SelfIter,			/* tp_iter */
	(iternextfunc)kiter_next,		/* tp_iternext */
	0,					/* tp_methods */
};

static void k3io_init(void);
/* Initialization function for the module (*must* be called init_k) */
PyMODINIT_FUNC
init_k(void)
{
	PyObject *m;
	/* PyObject* c_api_object; */

	/* Finalize the type object including setting type of the new type
	 * object; doing it here is required for portability to Windows
	 * without requiring C++. */
	if (PyType_Ready(&K_Type) < 0)
		return;

	/* Create the module and add the functions */
	m = Py_InitModule3("_k", _k_methods, module_doc);
	if (!m)
		return;
	/* Add some symbolic constants to the module */
	if (ErrorObject == NULL) {
	  ErrorObject = PyErr_NewException("_k.error", NULL, NULL);
		if (ErrorObject == NULL)
			return;
	}
	Py_INCREF(ErrorObject);
	PyModule_AddObject(m, "error", ErrorObject);

	/* Add K */
	PyModule_AddObject(m, "K", (PyObject *)&K_Type);
	/* vector types */
	PyModule_AddIntConstant(m, "KB", KB);
	PyModule_AddIntConstant(m, "KG", KG);
	PyModule_AddIntConstant(m, "KH", KH);
	PyModule_AddIntConstant(m, "KI", KI);
	PyModule_AddIntConstant(m, "KJ", KJ);
	PyModule_AddIntConstant(m, "KE", KE);
	PyModule_AddIntConstant(m, "KF", KF);
	PyModule_AddIntConstant(m, "KC", KC);
	PyModule_AddIntConstant(m, "KS", KS);
	PyModule_AddIntConstant(m, "KM", KM);
	PyModule_AddIntConstant(m, "KD", KD);
	PyModule_AddIntConstant(m, "KZ", KZ);
	PyModule_AddIntConstant(m, "KU", KU);
	PyModule_AddIntConstant(m, "KV", KV);
	PyModule_AddIntConstant(m, "KT", KT);
	/* table, dict */
	PyModule_AddIntConstant(m, "XT", XT);
	PyModule_AddIntConstant(m, "XD", XD);

	PyModule_AddIntConstant(m, "SIZEOF_VOID_P", SIZEOF_VOID_P);
	PyModule_AddStringConstant(m, "__version__", __version__);
	k3io_init();
}


/* k3io - read and write kdb tables */
/* could be implemented as a loadable q module, but keeping it here
   saves an extra compilation unit and simplifies installation        */
#include <stddef.h>

struct header {
	I dummy[2];
	I type; 
	I length;
};
/* q types of k types */
ZH tt[]={0,KI,KF,KC,KS};
#undef NDEBUG
ZS
k3io_read_symbol(FILE* f)
{
	char buf[1024], *p = buf;
	while((*p++ = getc(f))) assert(p < buf + sizeof(buf));
	R sn(buf, p-buf-1);
}

ZK /* read atom from a stream */
k3io_read_atom(I t, FILE* f)
{
	K x = ka(-tt[t]);
	switch (t) {
	case 1:
		fread(&xi, sizeof(xi), 1, f);
		break;
	case 2:
		fseek(f, 4, SEEK_CUR);
		fread(&xf, sizeof(xf), 1, f);
		break;
	case 3:
		fread(&xg, sizeof(xg), 1, f);
		break;
	case 4: 
		xs = k3io_read_symbol(f);
		break;
	}
	R x;
}

ZK /* read vector from a stream */
k3io_read_vector(I t, I n, FILE* f)
{
	K x = ktn(tt[-t], n);
	switch (t) {
	case -1:
		fread(xI, sizeof(xi), n, f);
		break;
	case -2:
		fread(xF, sizeof(xf), n, f);
		break;
	case -3:
		fread(xG, sizeof(xg), n, f);
		break;
	case -4:
		DO(n,
		   G buf[1024];A p = buf;
		   while((*p++ = getc(f)));
		   xS[i] = sn(buf, p-buf-1));
		break;
	}
	R x;
}

ZV /* skip to the 8 byte boundary */
k3io_next_object(FILE *f)
{
	fseek(f, ~7&(7+ftell(f)), SEEK_SET);
}

ZK
k3io_read_attrs(FILE* f)
{
	K x;
	struct header h;
	fread(&h, offsetof(struct header, length), 1, f);
	if (h.type == 6)
		x = k(0, "::", (K)0);
	else {
		I n, i;
		fread(&n, 4, 1, f);
		x = xD(ktn(KS,n), ktn(0,n));
		for(i = 0; i < n ; ++i) {
			fread(&h, sizeof(h), 1, f);
			fread(&h, offsetof(struct header, length), 1, f);
			kS(xx)[i] = k3io_read_symbol(f);
			k3io_next_object(f);
			fread(&h, offsetof(struct header, length), 1, f);
			if (h.type < 0) {
				fread(&h.length, 4, 1, f);
				kK(xy)[i] = k3io_read_vector(h.type, h.length, f);
			}
			else 
				kK(xy)[i] = k3io_read_atom(h.type, f);
			k3io_next_object(f);
			fread(&h, sizeof(h), 1, f);
		}
	}
	k3io_next_object(f);
	R x;
}

ZK
k3io_read_table(K x)
{
	struct header h;
	FILE *f;
	if (xt != -KS)
		R krr("k3io_read_table: type");
	if (!(f = fopen(xs, "r")))
		R orr("k3io_read_table");
	if (1 != fread(&h, sizeof(h), 1, f)) {
		x = krr("k3io_read_table: format short");
		goto end;
	}
	if (h.dummy[0] != -3 || h.dummy[1] != 1 || h.type != 5) {
		x = krr("k3io_read_table: format data");
		goto end;
	}
	x = xD(ktn(KS,h.length), ktn(0,h.length));
	I i, n = h.length;
	for (i = 0; i < n; ++i) {
		K v, d;
		fread(&h, sizeof(h), 1, f);
		fread(&h, offsetof(struct header, length), 1, f);
		kS(xx)[i] = k3io_read_symbol(f);
		k3io_next_object(f);
		fread(&h, sizeof(h), 1, f);
		v = k3io_read_vector(h.type, h.length, f);
		k3io_next_object(f);
		d = k3io_read_attrs(f); 
		kK(xy)[i] = k(0, ".k3.convert", r1(v), r1(d), (K)0);
	}
	x = xT(x);
 end:
	fclose(f);
	R x;
}

ZK
k3io_read_splayed(K x)
{
	struct header h;
	FILE *f, *g; K buf;
	if (xt != -KS)
		R krr("k3io_read_splayed: type");
	size_t dlen = strlen(xs);
	S dir = xs; 
	buf = ktn(KC,dlen+4);
	if (!(f = fopen(strcat(strcpy(kC(buf), dir), "/.l"), "r")))
		R orr("k3io_read_splayed");
	r0(buf);
	if (1 != fread(&h, sizeof(h), 1, f)) {
		x = krr("k3io_read_splayed: format short");
		goto end;
	}
	if (h.dummy[0] != -3 || h.dummy[1] != 1 || h.type != 0 || h.length != 2) {
		x = krr("k3io_read_table: format data");
		goto end;
	}
	fread(&h, sizeof(h), 1, f);
	x = xD(NULL, ktn(0,h.length));
	xx = k3io_read_vector(h.type, h.length, f);
	k3io_next_object(f);
	fread(&h, sizeof(h), 1, f);
	I i, n = h.length;
	for (i = 0; i < n; ++i) {
		K v, d;
		S col = kS(xx)[i];
		buf = ktn(KC,dlen+2+strlen(col));
		g =  fopen(strcat(strcat(strcat(strcpy(kC(buf),dir),"/"),col),".l"), "r");
		r0(buf);
		fread(&h, sizeof(h), 1, g);
		v = k3io_read_vector(h.type, h.length, g);
		fclose(g);
		k3io_next_object(f);
		d = k3io_read_attrs(f);
		kK(xy)[i] = k(0, ".k3.convert", r1(v), r1(d), (K)0);
	}
	x = xT(x);
 end:
	fclose(f);
	R x;
}

#include <sys/types.h>
#include <sys/stat.h>

ZK k3c[20];
ZS k3T[20];
ZI k3type[] = {-1,1,1,-2,-3,-4};
ZI k3size[] = { 4,0,0, 8, 1, 0};
ZI
write_header(FILE* f, I type, I length)
{
	struct header h = {{-3, 1}, type, length};
	if (!fwrite(&h, sizeof(h), 1, f))
		R orr("write_header"),-1;
	return 0;
}

ZI write_vector(FILE* f, K x);

ZI
write_scalar(FILE* f, K x)
{
	I t = -k3type[-xt-KI];
	struct header h = {{-3, 1}, 0, 0};
	h.type = t;
	if (!fwrite(&h, offsetof(struct header, length), 1, f))
		R orr("write_scalar"),-1;
	switch (t) {
	case 1:
		if (!fwrite(&xi, sizeof(I), 1, f))
			R orr("write_scalar"),-1;
		break;
	case 2:
		if (!fwrite(&h.length, sizeof(F), 1, f))
			R orr("write_scalar"),-1;
		if (!fwrite(&xf, sizeof(F), 1, f))
			R orr("write_scalar"),-1;
		break;
	}
	R 0;
}

ZI
write_object(FILE* f, K x)
{
	R (xt < 0)
		? write_scalar(f, x)
		: write_vector(f, x);
}

ZI
write_vector(FILE* f, K x)
{
	I t = xt;
	if (t > KT && t < 100)
		t = KI;
	if (t < KI || t > KS)
		R krr("type"),-1;
	I n = xn;
	if (write_header(f, k3type[t-KI], n))
		R -1;
	switch(k3type[t-KI]) {
	case 0: 
		DO(n,P(write_object(f, xK[i]),-1))break;
	case -1:
	case -2:       
		if(n != fwrite(xG, k3size[t-KI], n, f))
			R orr("write_vector"),-1;
		break;
	case -3:       
		if(n != fwrite(xG, 1, n, f) || putc(0,f))
			R orr("write_vector"),-1;
		break;
	case -4: {
		I i; S s;
		for (i = 0; i < n; ++i) {
			s = xS[i];
			if (1 != fwrite(s, 1+strlen(s), 1, f))
				R orr("write_vector"),-1;
		}
		break;
	}
	default:
		R krr("type"),-1;
	}
	return 0;
}

ZI
write_symbol(FILE* f, S s)
{
	struct header h = {{-3, 1}, 4};
	if (1 != fwrite(&h, offsetof(struct header, length), 1, f))
		R orr("write_symbol"),-1;
	if (1 != fwrite(s, strlen(s)+1, 1, f))
		R orr("write_symbol"),-1;
	R 0;
}

ZK
k3convert(K x, S* type)
{
	K r;
	if (xt < 0)
		R krr("type");
	if (xt > 19) {
		K s = k(0, "key", x, (K)0);
		if (s->t == -128)
			R krr(s->s);
		*type = s->s;
		r = ktn(KI, xn);
		memcpy(kI(r), xI, xn*sizeof(I));
		R r;
	}
	*type = k3T[xt];
	r = a1(k3c[xt], x);
	R r;
}


ZI
write_dict(FILE *f, K x)
{
	I n = xx->n, i;
	if (write_header(f, 5, n))
		R -1;
	for (i = 0; i < n; ++i) {
		S type;
		if (write_header(f, 0, 3))
			R -1;
		if (write_symbol(f, kS(xx)[i]))
			R -1;
		k3io_next_object(f);
		K c = k3convert(kK(xy)[i], &type);
		if (write_vector(f, c))
			R r0(c),-1;
		r0(c);
		k3io_next_object(f);
		if (type) {
			if (write_header(f, 5, 1))
				R -1;
			if (write_header(f, 0, 3))
				R -1;
			if (write_symbol(f, "T"))
				R -1;
			k3io_next_object(f);
			if (write_symbol(f, type))
				R -1;
			k3io_next_object(f);
		}
		if (write_header(f, 6, 0))
				R -1;
	}
	R 0;
}

ZK
k3io_write_table(K x, K y)
{
	K r;
	if (xt != -KS || *xs != ':')
		R krr("k3io_write_table: type x");
	if (y->t != XT)
		R krr("k3io_write_table: type y");
	FILE *f = fopen(xs+1, "w");
	if (!f)
		R orr("k3io_write_table");
	r = write_dict(f, y->k)?0:r1(x);
	if (fclose(f))
		R orr("k3io_write_table");
	R r;
}

ZK
k3io_write_vector(K x, K y)
{
	if (xt != -KS || *xs != ':')
		R krr("type");
	I t = y->t;
	if (t > KT && t < 100)
		t = KI;
	if (t < KI || t > KS)
		R krr("type");
	FILE* f = fopen(xs+1, "w");
	if (!f)
		R orr("k3io_write_vector");
	if (-1 == write_vector(f, y))
		x = NULL;
	else
		r1(x);
	R fclose(f),x;
}

ZK
k3io_write_splayed(K x, K y)
{
	if (xt != -KS)
		R krr("type");
	if (!mkdir(xs, 0777))
		R orr("k3io_write_splayed");
	if (y->t != XT)
		R krr("type");
	y = y->k; /* unflip */
	/*
	I i, n = kK(y)[0]->n;
	for (i = 0; i = n; ++i) {
		write_vector(xs, kK(kK(y)[0])[i]->s,
			     kK(kK(y)[1])[i])
	}
	*/
	R x;
}

void
k3io_init(void)
{
	k(0, "{.k3.rt::x}", r1(kl(1, k3io_read_table)), (K)0);
	k(0, "{.k3.rs::x}", r1(kl(1, k3io_read_splayed)), (K)0);
	k(0, "{.k3.wt::x}", r1(kl(2, k3io_write_table)), (K)0);
	k(0, "{.k3.ws::x}", r1(kl(2, k3io_write_splayed)), (K)0);
	k(0, "{.k3.wv::x}", r1(kl(2, k3io_write_vector)), (K)0);
	k(0, ".k3.convert:{"
	  "C:`date`time!(2035.01.01+;::);"
	  "$[(::)~y;x;$[null c:y`T;x;C[c]x]]}", (K)0);
	DO(20,k3c[i]=k(0,"::",(K)0));
	k3c[KB]=k3c[KG]=k3c[KH]=k(0,"`int$",(K)0);
	k3c[KE]=k(0,"`float$",(K)0); 
	k3c[KM]=k(0,"{-420+`int$x}",(K)0);
	k3T[KM]="month";
	k3c[KD]=k(0,"{-12784+`int$x}",(K)0);
	k3T[KD]="date";
	k3c[KZ]=k(0,"{-12784+`float$x}",(K)0); 
	k3T[KZ]="timestamp";
	k3c[KT]=k(0,"{(`float$x)%8.64e7}",(K)0); 
	k3T[KT]="time";
}

/* k3io end */
