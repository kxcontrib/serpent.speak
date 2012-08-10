/* -*- mode: c; c-basic-offset: 8 -*- */
static char __version__[] = "$Revision: 1.49 $";
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
#include "datetime.h"
#include "longintrepr.h"

#include "k.h"
#include "math.h"

/* these should be in k.h */
ZK km(I i){K x = ka(-KM);xi=i;R x;}
ZK kuu(I i){K x = ka(-KU);xi=i;R x;}
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
#define PyInt_FromSsize_t PyInt_FromLong
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
static K k_none;
static PyObject *ErrorObject;
static PyObject *
KObject_FromK(PyTypeObject *type, K x)
{
	if (!type) 
		type = &K_Type;
	if (xt == -128)
		return PyErr_SetString(ErrorObject,
				       xs?xs:(S)"not set"),r0(x),NULL;
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
				k(0, ".", r1(self->x), r1(((KObject*)args)->x), (K)0))
		: PyErr_Format(PyExc_TypeError, "expected a K object, not %s",
			       args->ob_type->tp_name);
}

/* extern K a1(K,K); */
static PyObject*
K_a0(KObject *self)
{
	K x = self->x;
	if (xt < 100) {
		Py_INCREF(self);
		return (PyObject*)self;
	}
	R KObject_FromK(self->ob_type, k(0, "@", r1(x), r1(k_none), (K)0));
}
static PyObject*
K_a1(KObject *self, PyObject *arg)
{
	R K_Check(arg)
		? KObject_FromK(self->ob_type, k(0, "@", r1(self->x), r1(((KObject*)arg)->x), (K)0))
		: PyErr_Format(PyExc_TypeError, "expected a K object, not %s",
			       arg->ob_type->tp_name);
}

static PyObject*
K_ja(KObject *self, PyObject *arg)
{
	switch (self->x->t) {
	case 0: {
		if (K_Check(arg))
			jk(&self->x, r1(((KObject*)arg)->x));
		else
			R PyErr_Format(PyExc_TypeError, "K._ja: expected K object, not %s", 
				       arg->ob_type->tp_name);
		break;
	}
	case KB: {
		if (PyBool_Check(arg)) {
			G a = arg == Py_True;
			ja(&self->x, &a);
		}
		else
			R PyErr_Format(PyExc_TypeError, "K._ja: expected bool, not %s", 
				       arg->ob_type->tp_name);
		break;
	}
	case KG: {
		G a = PyInt_AsLong(arg); /* XXX: need bounds checking */
		if (a == (G)-1 && PyErr_Occurred())
			R NULL;
		else
			ja(&self->x, &a);
		break;
	}
	case KH: {
		H a = PyInt_AsLong(arg);  /* XXX: need bounds checking */
		if (a == -1 && PyErr_Occurred())
			R NULL;
		else
			ja(&self->x, &a);
		break;
	}
	case KI: {
		I a = PyInt_AsLong(arg);  /* XXX: need bounds checking */
		if (a == -1 && PyErr_Occurred())
			R NULL;
		else
			ja(&self->x, &a);
		break;
	}
	case KJ: {
		J a = PyInt_AsLong(arg);  /* XXX: need bounds checking */
		if (a == -1 && PyErr_Occurred())
			R NULL;
		else
			ja(&self->x, &a);
		break;
	}
	case KE: {
		E a = PyFloat_AsDouble(arg);  /* XXX: need bounds checking */
		if (a == -1 && PyErr_Occurred())
			R NULL;
		else
			ja(&self->x, &a);
		break;
	}
	case KF: {
		F a = PyFloat_AsDouble(arg);  /* XXX: need bounds checking */
		if (a == -1 && PyErr_Occurred())
			R NULL;
		else
			ja(&self->x, &a);
		break;
	}
	case KC: {
		char *a; Py_ssize_t n;
		if (-1 == PyString_AsStringAndSize(arg, &a, &n))
			R NULL;
		if (n != 1)
			R PyErr_Format(PyExc_TypeError, "K.ja: a one-, not %zd-character string", n);
		ja(&self->x, a);
		break;
	}
	case KS: {
		char *a; Py_ssize_t n;
		if (-1 == PyString_AsStringAndSize(arg, &a, &n))
			R NULL;
		js(&self->x, sn(a, n));
		break;
	}
	case KM: {
		
	}
	case KD: {

	}
	case KZ: {

	}
	case KU: {

	}
	case KV: {

	}
	case KT: {
		R PyErr_Format(PyExc_NotImplementedError, "appending to type %d", (int)self->x->t);
	}
	}
	Py_RETURN_NONE;
}

static K k_repr;
static PyObject*
K_str(KObject *self)
{
	K x = self->x;
	switch (xt) {
	case KC:
		return PyString_FromStringAndSize((S)xC, xn);
	case -KS:
		return PyString_FromString(xs);
	case -KC:
		return PyString_FromStringAndSize((S)&xg, 1);
	}
	x = k(0, "@", r1(k_repr), r1(x), (K)0);
	if (xt == -128)
		return PyErr_SetString(ErrorObject, xs?xs:(S)"not set"),r0(x),NULL;
	return PyString_FromStringAndSize((S)xC, xn);
}
static PyObject*
K_repr(KObject *self)
{
	K x = self->x;
	PyObject *f, *s, *r;
	x = k(0, "@", r1(k_repr), r1(x), (K)0);
	if (xt == -128) {
		r = PyString_FromFormat("<k object at %p of type %hd, '%s>", 
					self->x, self->xt, xs);
		r0(x); R r;
	}

	f = PyString_FromString("k(%r)");
	if (f == NULL)
		R NULL;
	s = PyString_FromStringAndSize((S)xC, xn);
	if (s == NULL)
		R NULL;
	r = PyString_Format(f, s);
	Py_DECREF(f);
	Py_DECREF(s);
	R r;
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
#if KXVER>=0
		16,
#else
		0,
#endif
		0, 1, /* byte */
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
static PyObject *K_K(PyTypeObject *type, PyObject *arg);
static PyObject *
K_call_any(KObject *self, PyObject *args)
{
	switch (PyTuple_GET_SIZE(args)) {
	case 0:
		R K_a0(self);
	case 1:
		R K_a1(self, PyTuple_GET_ITEM(args, 0));
	}
	R K_dot(self, K_K(self->ob_type, args));
}
# define K_call K_call_any

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
ZS th(I i){SW(i){CS(1,R "st")CS(2,R "nd")CS(3,R "rd")};R "th";}
PyDoc_STRVAR(K_I_doc,
	     "returns a K int list");
static PyObject *
K_I(PyTypeObject *type, PyObject *arg)
{
	long item;
	PyObject *seq = PySequence_Fast(arg, "K._I: not a sequence");
	if (seq == NULL)
		return NULL;
	int i, n = PySequence_Fast_GET_SIZE(seq);
	K x = ktn(KI, n);
	for (i = 0; i < n; ++i) {
		PyObject *o = PySequence_Fast_GET_ITEM(seq, i);
		if (PyInt_Check(o))
			item = PyInt_AS_LONG(o);
		else {
			if (o == Py_None)
				item = ni;
			else {
				r0(x);
				PyErr_Format(PyExc_TypeError,
					     "K._I: %d-%s item is not an int", i+1, th(i+1));
				return NULL;
			}
		}
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

PyDoc_STRVAR(K_kdd_doc,
	     "converts datetime.date to q date");
static PyObject *
K_kdd(PyTypeObject *type, PyDateTime_Date *arg)
{
	if (!PyDate_Check(arg))
		return PyErr_Format(PyExc_TypeError, "expected a date object, not %s",
				    arg->ob_type->tp_name);
	int y, m, d;
	y = PyDateTime_GET_YEAR(arg);
	m = PyDateTime_GET_MONTH(arg);
	d = PyDateTime_GET_DAY(arg);
	K x = kd(ymd(y,m,d));
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}
PyDoc_STRVAR(K_ktt_doc,
	     "converts datetime.time to q time");
static PyObject *
K_ktt(PyTypeObject *type, PyObject *arg)
{
	if (!PyTime_Check(arg))
		return PyErr_Format(PyExc_TypeError, "expected a time object, not %s",
				    arg->ob_type->tp_name);
	int h, m, s, u;
	h = PyDateTime_TIME_GET_HOUR(arg);
	m = PyDateTime_TIME_GET_MINUTE(arg);
	s = PyDateTime_TIME_GET_SECOND(arg);
	u = PyDateTime_TIME_GET_MICROSECOND(arg);
	K x = kt(((h*60+m)*60+s)*1000+u/1000);
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}
PyDoc_STRVAR(K_kzz_doc,
	     "converts datetime.datetime to q datetime");
static PyObject *
K_kzz(PyTypeObject *type, PyObject *arg)
{
	if (!PyDateTime_Check(arg))
		return PyErr_Format(PyExc_TypeError, "expected a date object, not %s",
				    arg->ob_type->tp_name);
	int y, m, d, h, u, s, i;
	y = PyDateTime_GET_YEAR(arg);
	m = PyDateTime_GET_MONTH(arg);
	d = PyDateTime_GET_DAY(arg);
	h = PyDateTime_DATE_GET_HOUR(arg);
	u = PyDateTime_DATE_GET_MINUTE(arg);
	s = PyDateTime_DATE_GET_SECOND(arg);
	i = PyDateTime_DATE_GET_MICROSECOND(arg);
	K x = kz(ymd(y,m,d)+(((h*60+u)*60+s)*1000+i/1000)/(24*60*60*1000.));
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}

#ifdef KN
PyDoc_STRVAR(K_knz_doc,
	     "converts datetime.datetime to q timespan");
static PyObject *
K_knz(PyTypeObject *type, PyObject *arg)
{
	if (!PyDelta_Check(arg))
		return PyErr_Format(PyExc_TypeError, "expected a timedelta object, not %s",
				    arg->ob_type->tp_name);
	int d, s, u;
	d = ((PyDateTime_Delta*)arg)->days;
	s = ((PyDateTime_Delta*)arg)->seconds;
	u = ((PyDateTime_Delta*)arg)->microseconds;
	K x = ktj(-KN, 1000000000ll*(d*24*60*60+s)+1000l*u);
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}

PyDoc_STRVAR(K_kpz_doc,
	     "converts datetime.timedelta to q timespan");
static PyObject *
K_kpz(PyTypeObject *type, PyObject *arg)
{
	if (!PyDateTime_Check(arg))
		return PyErr_Format(PyExc_TypeError, "expected a date object, not %s",
				    arg->ob_type->tp_name);
	int y, m, d, h, u, s, i;
	y = PyDateTime_GET_YEAR(arg);
	m = PyDateTime_GET_MONTH(arg);
	d = PyDateTime_GET_DAY(arg);
	h = PyDateTime_DATE_GET_HOUR(arg);
	u = PyDateTime_DATE_GET_MINUTE(arg);
	s = PyDateTime_DATE_GET_SECOND(arg);
	i = PyDateTime_DATE_GET_MICROSECOND(arg);
	K x = ktj(-KP, 1000000000ll*(((ymd(y,m,d)*24+h)*60+u)*60+s)
		  + 1000l*i);
	if (!type) {
		type = &K_Type;
	}
	return KObject_FromK(type, x);
}
#endif
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
K_ATOM(uu, I, i, "returns a K minute")
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

#if KXVER>=3
PyDoc_STRVAR(K_kguid_doc,
             "returns a K guid");
static PyObject *
K_kguid(PyTypeObject *type, PyObject *args)
{
	U u;
	PyLongObject *pylong;
	if (!PyArg_ParseTuple(args, "O!:K._kguid", &PyLong_Type, &pylong))
		return NULL;
	if (_PyLong_AsByteArray(pylong, u.g, 16, 0, 0) == -1)
		return NULL;
	if (!type)
		type = &K_Type;
	return KObject_FromK(type, ku(u));
}
#endif


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
	if (r == NULL) {
		PyErr_SetString(PyExc_OSError, "connection");
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
#if KXVER >=3
	case 'm': return PyInt_FromLong(k->m);
	case 'a': return PyInt_FromLong(k->a);
	case 'n': return PyInt_FromSsize_t(k->n);
#else
	case 'n': return PyInt_FromLong(k->n);
#endif
	case 'r': return PyInt_FromLong(k->r);
	case 't': return PyInt_FromLong(k->t);
	case 'u': return PyInt_FromLong(k->u);
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
			  : k->t == -KC
			  ? PyString_FromStringAndSize((char*)&k->g, 1)
			  : PyString_FromFormat("<%p>", k->s));
	case 'c': return PyString_FromStringAndSize((char*)&k->g, 1);
	case 'k': return (k->t == XT
			  ? KObject_FromK(self->ob_type, r1(k->k))
			  : PyString_FromFormat("<%p>", k->k));
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

PyDoc_STRVAR(K_id_doc,
	     "x._id(k) -> id of k object\n"
	     "\n");
static PyObject *
K_id(KObject *self)
{
	return PyInt_FromSsize_t((Py_ssize_t)self->x);
}

static PyMethodDef 
K_methods[] = {
	{"_func", (PyCFunction)K_func,METH_O|METH_CLASS, "func"}, 
	{"_dot", (PyCFunction)K_dot,  METH_O, "dot"},
	{"_a0", (PyCFunction)K_a0,  METH_NOARGS, "a0"},
	{"_a1", (PyCFunction)K_a1,  METH_O, "a1"},
	{"_ja", (PyCFunction)K_ja,  METH_O, "append atom"},
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
	{"_kdd",(PyCFunction)K_kdd, METH_O|METH_CLASS, K_kdd_doc},
	{"_kz",	(PyCFunction)K_kz, METH_VARARGS|METH_CLASS, K_kz_doc},
	{"_kzz",(PyCFunction)K_kzz, METH_O|METH_CLASS, K_kzz_doc},
#ifdef KN
	{"_knz",(PyCFunction)K_knz, METH_O|METH_CLASS, K_knz_doc},
	{"_kpz",(PyCFunction)K_kpz, METH_O|METH_CLASS, K_kpz_doc},
#endif
	{"_ku", (PyCFunction)K_kuu, METH_VARARGS|METH_CLASS, K_kuu_doc},
#if KXVER>=3
	{"_kguid",(PyCFunction)K_kguid, METH_VARARGS|METH_CLASS, K_kguid_doc},
#endif
	{"_kv",	(PyCFunction)K_kv, METH_VARARGS|METH_CLASS, K_kv_doc},
	{"_kt",	(PyCFunction)K_kt, METH_VARARGS|METH_CLASS, K_kt_doc},
	{"_ktt",(PyCFunction)K_ktt, METH_O|METH_CLASS, K_ktt_doc},
	{"_kp",	(PyCFunction)K_kp, METH_VARARGS|METH_CLASS, K_kp_doc},
	{"_ktn",(PyCFunction)K_ktn, METH_VARARGS|METH_CLASS, K_ktn_doc},
	{"_xT",	(PyCFunction)K_xT, METH_VARARGS|METH_CLASS, K_xT_doc},
	{"_xD",	(PyCFunction)K_xD, METH_VARARGS|METH_CLASS, K_xD_doc},
	{"_K",	(PyCFunction)K_K, METH_O|METH_CLASS, K_K_doc},

	{"_from_array_interface", (PyCFunction)K_from_array_interface,
                                   METH_O|METH_CLASS, K_from_array_interface_doc},

	{"inspect", (PyCFunction)K_inspect, METH_VARARGS, K_inspect_doc},
	{"_id", (PyCFunction)K_id,  METH_NOARGS, K_id_doc},
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
        (charbufferproc)K_buffer_getreadbuf,
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
	(reprfunc)K_repr,     	/*tp_repr*/
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

#if 0
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
#endif

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


/* List of functions defined in the module */

static PyMethodDef _k_methods[] = {
//	{"khp",		_k_khp,		METH_VARARGS, _k_khp_doc},
	{"ymd",		_k_ymd,		METH_VARARGS, _k_ymd_doc},
	{NULL,		NULL}		/* sentinel */
};
/*********************** K Object Iterator **************************/

typedef struct {
	PyObject_HEAD
	PyTypeObject* ktype;
	K x; I i,n;
} kiterobject;

static PyTypeObject KObjectIter_Type;

#define KObjectIter_Check(op) PyObject_TypeCheck(op, &KObjectArrayIter_Type)

static PyObject *
k_iter(KObject *obj)
{
	kiterobject *it; K x;

	if (!K_Check(obj)) {
		PyErr_BadInternalCall();
		return NULL;
	}

	it = PyObject_GC_New(kiterobject, &KObjectIter_Type);
	if (it == NULL)
		return NULL;
	Py_INCREF(it->ktype = obj->ob_type);
	x = obj->x;
	if (xt == XD)
		x = xx;
	it->x = r1(x);
	it->i = 0;
	if (!k_itemsize(x) && xt != XT) {
		PyErr_Format(PyExc_NotImplementedError, "not iterable: t=%d", xt);
		return NULL;
	}
	it->n = xt == XT ? kK(kK(xk)[1])[0]->n : xn;
	PyObject_GC_Track(it);
	return (PyObject *)it;
}


static K d2l, m2l, z2l, t2l, v2l, u2l;
static PyObject *
d2py(I d)
{
	switch (d) {
	case -wi: R PyDate_FromDate(1,1,1);
	case wi: R PyDate_FromDate(9999,12,31);
	case ni: Py_RETURN_NONE;
	}
	K y=kd(d),x=k(0,"@",r1(d2l),y,(K)0);
	PyObject *o = PyDate_FromDate(xI[0],xI[1],xI[2]);
	r0(x); R o;
}
static PyObject *
m2py(I d)
{
	switch (d) {
	case -wi: R PyDate_FromDate(1,1,1);
	case wi: R PyDate_FromDate(9999,12,1);
	case ni: Py_RETURN_NONE;
	}
	K y=km(d),x=k(0,"@",r1(m2l),y,(K)0);
	PyObject *o = PyDate_FromDate(xI[0],xI[1],1);
	r0(x); R o;
}
static PyObject *
z2py(F z)
{
	if (finite(z)) {
		K y=kz(z),x=k(0,"@",r1(z2l),y,(K)0);
		PyObject *o =  PyDateTime_FromDateAndTime(xI[0],xI[1],xI[2],
							  xI[3],xI[4],xI[5],
							  (I)round((z-floor(z))*24*60*60*1e6));
		r0(x); R o;
	}
	if (isnan(z)) Py_RETURN_NONE;
	R z<0?PyDateTime_FromDateAndTime(1,1,1,0,0,0,0)
	     :PyDateTime_FromDateAndTime(9999,12,31,23,59,59,999999);
}

static PyObject *
t2py(I t)
{
	if(t == ni) Py_RETURN_NONE;
	K y=kt(t),x=k(0,"@",r1(t2l),y,(K)0);
	PyObject *o = PyTime_FromTime(xI[0],xI[1],xI[2],
				       t%1000*1000);
	r0(x); R o;
}
static PyObject *
v2py(I t)
{
	if(t == ni) Py_RETURN_NONE;
	K y=kv(t),x=k(0,"@",r1(v2l),y,(K)0);
	PyObject *o = PyTime_FromTime(xI[0],xI[1],xI[2],0);
	r0(x); R o;
}
static PyObject *
u2py(I t)
{
	if(t == ni) Py_RETURN_NONE;
	K y=kuu(t),x=k(0, "@",r1(u2l),y,(K)0);
	PyObject *o = PyTime_FromTime(xI[0],xI[1],0,0);
	r0(x); R o;
}

static PyObject *
kiter_next(kiterobject *it)
{
	PyObject *ret = NULL;
	K x = it->x;
	I i = it->i, n = it->n;
	if (i < n)
		switch (xt) {
		case KS: /* most common case: use list(ks) */
			ret = PyString_FromString(xS[i]);
			break;
		case 0:
			ret = KObject_FromK(it->ktype, r1(xK[i]));
			break;
#if KXVER>=3
		case UU:
			ret = _PyLong_FromByteArray(xU[i].g, 16, 0, 0);
			break;
#endif
		/* remaining cases are less common because array(x) *
		 * is a better option that list(x)                  */
		case KB:
			ret = PyBool_FromLong(xG[i]);
			break;
		case KC:
			ret = PyString_FromStringAndSize((S)&xC[i], 1);
			break;
		case KG:
			ret = PyInt_FromLong(xG[i]);
			break;
		case KH:
			ret = xH[i]==nh?Py_INCREF(Py_None),Py_None:PyInt_FromLong(xH[i]);
			break;
		case KI:
			ret = xI[i]==ni?Py_INCREF(Py_None),Py_None:PyInt_FromLong(xI[i]);
			break;
		case KM:
			ret = m2py(xI[i]);
			break;
		case KD:
			ret = d2py(xI[i]);
			break;
		case KV:
			ret = v2py(xI[i]);
			break;
		case KU:
			ret = u2py(xI[i]);
			break;
		case KT:
			ret = t2py(xI[i]);
			break;
		case KZ:
			ret = z2py(xF[i]);
			break;
		case KJ:
			ret = xJ[i]==nj?Py_INCREF(Py_None),Py_None:PyLong_FromLongLong(xJ[i]);
			break;
		case KE:
			ret = PyFloat_FromDouble(xE[i]);
			break;
		case KF:
			ret = PyFloat_FromDouble(xF[i]);
			break;
		case XT:
			ret = KObject_FromK(it->ktype,k(0,"@",r1(x),ki(i),(K)0));
			break;
		}
	it->i++;
	return ret;
}

static void
kiter_dealloc(kiterobject *it)
{
	PyObject_GC_UnTrack(it);
	Py_XDECREF(it->ktype);
	r0(it->x);
	PyObject_GC_Del(it);
}

static int
kiter_traverse(kiterobject *it, visitproc visit, void *arg)
{
	if (it->ktype != NULL)
		return visit((PyObject *)(it->ktype), arg);
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

/* Initialization function for the module (*must* be called init_k + KXVER) */
#define CAT(x, y) x##y
#define XCAT(x, y) CAT(x, y)
#define SCAT(x, y) (x #y)
#define XSCAT(x, y) SCAT(x, y)
PyMODINIT_FUNC
XCAT(init_k, KXVER)(void)
{
	PyObject *m;
	/* PyObject* c_api_object; */
	PyDateTime_IMPORT;
	/* date/time to list translations */
	d2l=k(0,"`year`mm`dd$\\:",(K)0);
	m2l=k(0,"`year`mm$\\:",(K)0);
	z2l=k(0,"`year`mm`dd`hh`uu`ss$\\:",(K)0); 
	t2l=k(0,"`hh`mm`ss$\\:",(K)0);
	v2l=k(0,"`hh`mm`ss$\\:",(K)0);
	u2l=k(0,"`hh`mm$\\:",(K)0);
	k_none = k(0, "::", (K)0);
	k_repr = k(0, "-3!", (K)0);
	/* Finalize the type object including setting type of the new type
	 * object; doing it here is required for portability to Windows
	 * without requiring C++. */
	if (PyType_Ready(&K_Type) < 0)
		return;

	/* Create the module and add the functions */
	m = Py_InitModule3(XSCAT("_k", KXVER), _k_methods, module_doc);
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
}
