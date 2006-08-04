/* -*- mode: c; c-basic-offset: 8 -*- */
static char __version__[] = "$Revision: 1.19 $";
#define PYK_K_MODULE 1
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
#include <stdlib.h>
typedef struct {
        PyObject_HEAD
        K _k;
} KObject;
static PyTypeObject K_Type;

#define K_Check(op) PyObject_TypeCheck(op, &K_Type)
#define K_CheckExact(op) ((op)->ob_type == &K_Type)

#include <stdio.h>
#include <ctype.h>
#ifndef  Py_RETURN_NONE
#   define Py_RETURN_NONE Py_INCREF(Py_None); return Py_None
#endif
#ifndef MODULE_NAME
#   define  MODULE_NAME _k
#endif
#define STRINGIFY(x) #x
#define XSTRINGIFY(x) STRINGIFY(x)

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
KObject_FromK(PyTypeObject *type, K k)
{
	if (!type) {
		type = &K_Type;
	}
	if (!k) {
		return PyErr_Format(PyExc_ValueError, "null k object");
	}
	KObject *self = (KObject*)type->tp_alloc(type, 0);
	if (self)
		self->_k = r1(k);
	return (PyObject*)self;
}
/* converter function */
static int
getK(PyObject *arg, void *addr)
{
	if (!K_Check(arg)) {
		return PyErr_BadArgument();
	}
	K r = ((KObject*)arg)->_k;
	if (!r) {
		return PyErr_BadArgument();
	}
	*(K*)addr = r;
	return 1;
}

static void
K_dealloc(KObject *self)
{
	if (self->_k) {
		r0(self->_k);
	}
	self->ob_type->tp_free(self);
}

static PyObject*
K_str(KObject *self)
{
	K x = self->_k;
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
	K k = self->_k;
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
	K k = self->_k;
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
	KObject *ret;
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
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(x);
		return NULL;
	}
	ret->_k = x;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_ktd_doc,
	     "flip from keyedtable(dict)");
static PyObject *
K_ktd(PyTypeObject *type, PyObject *args)
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
	k = ktd(r1(k0));
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
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

PyDoc_STRVAR(K_kb_doc,
	     "returns a K bool");
static PyObject *
K_kb(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	G g;
	if (!PyArg_ParseTuple(args, "b|O!", &g, &K_Type, &ret)) {
		return NULL;
	}
	K k = kb(g);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	assert(PyType_IsSubtype(type, &K_Type));
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kg_doc,
	     "returns a K byte");
static PyObject *
K_kg(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	G g;
	if (!PyArg_ParseTuple(args, "b|O!", &g, &K_Type, &ret)) {
		return NULL;
	}
	K k = kg(g);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kh_doc,
	     "returns a K short");
static PyObject *
K_kh(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	H h;
	if (!PyArg_ParseTuple(args, "h|O!", &h, &K_Type, &ret)) {
		return NULL;
	}
	K k = kh(h);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_ki_doc,
	     "returns a K int");
static PyObject *
K_ki(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	I i;
	if (!PyArg_ParseTuple(args, "i|O!", &i, &K_Type, &ret)) {
		return NULL;
	}
	K k = ki(i);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kj_doc,
	     "returns a K long (64 bits)");
static PyObject *
K_kj(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	J j;
	if (!PyArg_ParseTuple(args, "L|O!", &j, &K_Type, &ret)) {
		return NULL;
	}
	K k = kj(j);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_ke_doc,
	     "returns a K real (32 bits)");
static PyObject *
K_ke(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	E e;
	if (!PyArg_ParseTuple(args, "f|O!", &e, &K_Type, &ret)) {
		return NULL;
	}
	K k = ke(e);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kf_doc,
	     "returns a K float (64 bits)");
static PyObject *
K_kf(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	F f;
	if (!PyArg_ParseTuple(args, "d|O!", &f, &K_Type, &ret)) {
		return NULL;
	}
	K k = kf(f);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kc_doc,
	     "returns a K char");
static PyObject *
K_kc(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	G c;
	if (!PyArg_ParseTuple(args, "c|O!:K.kc", &c, &K_Type, &ret)) {
		return NULL;
	}
	K k = kc(c);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_ks_doc,
	     "returns a K symbol");
static PyObject *
K_ks(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	S s; I n;
	if (!PyArg_ParseTuple(args, "s#|O!", &s, &n, &K_Type, &ret)) {
		return NULL;
	}
	K k = ks(sn(s,n));
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
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
				     "K._S: %d-th item is not a string", i);
			return NULL;
		}
		xS[i] = sn(PyString_AS_STRING(o), PyString_GET_SIZE(o));
	}
	KObject *ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(x);
		return NULL;
	}
	ret->_k = x;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kd_doc,
	     "returns a K date");
static PyObject *
K_kd(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	I i;
	if (!PyArg_ParseTuple(args, "i|O!", &i, &K_Type, &ret)) {
		return NULL;
	}
	K k = kd(i);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kz_doc,
	     "returns a K datetime");
static PyObject *
K_kz(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	F f;
	if (!PyArg_ParseTuple(args, "d|O!", &f, &K_Type, &ret)) {
		return NULL;
	}
	K k = kz(f);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kt_doc,
	     "returns a K time");
static PyObject *
K_kt(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	I i;
	if (!PyArg_ParseTuple(args, "i|O!", &i, &K_Type, &ret)) {
		return NULL;
	}
	K k = kt(i);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}

PyDoc_STRVAR(K_kp_doc,
	     "returns a K string");
static PyObject *
K_kp(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	S s;
	if (!PyArg_ParseTuple(args, "s|O!", &s, &K_Type, &ret)) {
		return NULL;
	}
	K k = kp(s);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}
PyDoc_STRVAR(K_ktn_doc,
	     "returns a K list");
static PyObject *
K_ktn(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	I i1, i2;
	if (!PyArg_ParseTuple(args, "ii|O!", &i1, &i2, &K_Type, &ret)) {
		return NULL;
	}
	K k = ktn(i1,i2);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
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
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}
PyDoc_STRVAR(K_xD_doc,
	     "returns a K dict");
static PyObject *
K_xD(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	K k1 = 0, k2 = 0;
	if (!PyArg_ParseTuple(args, "O&O&|O!",
			      getK, &k1, getK, &k2,
			      &K_Type, &ret))
		{
			return NULL;
		}
	if (!(k1 && k2)) {
		PyErr_BadArgument();
		return NULL;
	}
	K k = xD(r1(k1), r1(k2));
	assert(k->t == XD);
	if (!k) {
		return PyErr_NoMemory();
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = k;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(k);
		return NULL;
	}
	ret->_k = k;
	return (PyObject*)ret;
}


PyDoc_STRVAR(K_knk_doc,
	     "returns a K list");
static PyObject *
K_knk(PyTypeObject *type, PyObject *args)
{
	KObject *ret = 0;
	I n;
	K r;
	switch (PyTuple_Size(args)-1) {
	case 0: {
		if (!PyArg_ParseTuple(args, "i|O!", &n, &K_Type, &ret)) {
			return NULL;
		}
		r = knk(n);
		break;
	}
	case 1: {
		K k1;
		if (!PyArg_ParseTuple(args, "iO&|O!", &n,
				      getK, &k1,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1));
		break;
	}
	case 2: {
		K k1,k2;
		if (!PyArg_ParseTuple(args, "iO&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2));
		break;
	}
	case 3: {
		K k1,k2,k3;
		if (!PyArg_ParseTuple(args, "iO&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3));
		break;
	}
	case 4: {
		K k1,k2,k3,k4;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4));
		break;
	}
	case 5: {
		K k1,k2,k3,k4,k5;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5));
		break;
	}
	case 6: {
		K k1,k2,k3,k4,k5,k6;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6));
		break;
	}
	case 7: {
		K k1,k2,k3,k4,k5,k6,k7;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7));
		break;
	}
	case 8: {
		K k1,k2,k3,k4,k5,k6,k7,k8;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = knk(n,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),r1(k8));
		break;
	}
	case 9: {
		K k1,k2,k3,k4,k5,k6,k7,k8,k9;
		if (!PyArg_ParseTuple(args, "iO&O&O&O&O&O&O&O&O&|O!", &n,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8,
				      getK, &k9,
				      &K_Type, &ret))
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
	if (!r) {
		return NULL;
	}
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = r1(r);
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(r);
		return NULL;
	}
	ret->_k = r1(r);
	return (PyObject*)ret;
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
	KObject *ret = 0;
	I c;
	char* m;
	K r;
	switch (PyTuple_Size(args)-2) {
	case 0: {
		if (!PyArg_ParseTuple(args, "is|O!", &c, &m, &K_Type, &ret)) {
			return NULL;
		}
		r = k(c,m,(K)0);
		break;
	}
	case 1: {
		K k1;
		if (!PyArg_ParseTuple(args, "isO&|O!", &c, &m,
				      getK, &k1,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),(K)0);
		break;
	}
	case 2: {
		K k1,k2;
		if (!PyArg_ParseTuple(args, "isO&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),(K)0);
		break;
	}
	case 3: {
		K k1,k2,k3;
		if (!PyArg_ParseTuple(args, "isO&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),(K)0);
		break;
	}
	case 4: {
		K k1,k2,k3,k4;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),(K)0);
		break;
	}
	case 5: {
		K k1,k2,k3,k4,k5;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),(K)0);
		break;
	}
	case 6: {
		K k1,k2,k3,k4,k5,k6;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),(K)0);
		break;
	}
	case 7: {
		K k1,k2,k3,k4,k5,k6,k7;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),(K)0);
		break;
	}
	case 8: {
		K k1,k2,k3,k4,k5,k6,k7,k8;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8,
				      &K_Type, &ret))
			{
				return NULL;
			}
		r = k(c,m,r1(k1),r1(k2),r1(k3),r1(k4),r1(k5),r1(k6),r1(k7),r1(k8),(K)0);
		break;
	}
	case 9: {
		K k1,k2,k3,k4,k5,k6,k7,k8,k9;
		if (!PyArg_ParseTuple(args, "isO&O&O&O&O&O&O&O&O&|O!", &c, &m,
				      getK, &k1,
				      getK, &k2,
				      getK, &k3,
				      getK, &k4,
				      getK, &k5,
				      getK, &k6,
				      getK, &k7,
				      getK, &k8,
				      getK, &k9,
				      &K_Type, &ret))
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
	if (!r)
		return PyErr_NoMemory();
	if (r->t == -128)
                return PyErr_Format(ErrorObject, "%s", r->s); 
	if (ret) {
		if (ret->_k) {
			r0(ret->_k);
		}
		ret->_k = r;
		Py_RETURN_NONE;
	}
	if (!type) {
		type = &K_Type;
	}
	ret = (KObject*)type->tp_alloc(type, 0);
	if (!ret) {
		r0(r);
		return NULL;
	}
	ret->_k = r;
	return (PyObject*)ret;
}



PyDoc_STRVAR(K_inspect_doc,
	     "inspect(k, c, [, i]) -> python object\n"
	     "\n"
	     ""
	     );
static PyObject *
K_inspect(PyObject *self, PyObject *args)
{
	K k = ((KObject*)self)->_k;
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
	case 'k': return KObject_FromK(self->ob_type, k->k);
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
	case 'K': return KObject_FromK(self->ob_type, kK(k)[i]);
	}
	return PyErr_Format(PyExc_KeyError, "no such attribute: '%c'", c);
}

static PyMethodDef 
K_methods[] = {
	{"_k",	(PyCFunction)K_k,  METH_VARARGS|METH_CLASS, K_k_doc},
	{"_knk",(PyCFunction)K_knk, METH_VARARGS|METH_CLASS, K_knk_doc},
	{"_ktd",(PyCFunction)K_ktd, METH_VARARGS|METH_CLASS, K_ktd_doc},
	{"_err",(PyCFunction)K_err, METH_VARARGS|METH_CLASS, K_err_doc},
	{"_kb",	(PyCFunction)K_kb, METH_VARARGS|METH_CLASS, K_kb_doc},
	{"_kg",	(PyCFunction)K_kg, METH_VARARGS|METH_CLASS, K_kg_doc},
	{"_kh",	(PyCFunction)K_kh, METH_VARARGS|METH_CLASS, K_kh_doc},
	{"_ki",	(PyCFunction)K_ki, METH_VARARGS|METH_CLASS, K_ki_doc},
	{"_kj",	(PyCFunction)K_kj, METH_VARARGS|METH_CLASS, K_kj_doc},
	{"_ke",	(PyCFunction)K_ke, METH_VARARGS|METH_CLASS, K_ke_doc},
	{"_kf",	(PyCFunction)K_kf, METH_VARARGS|METH_CLASS, K_kf_doc},
	{"_kc",	(PyCFunction)K_kc, METH_VARARGS|METH_CLASS, K_kc_doc},
	{"_ks",	(PyCFunction)K_ks, METH_VARARGS|METH_CLASS, K_ks_doc},
	{"_S",	(PyCFunction)K_S, METH_O|METH_CLASS, K_S_doc},
	{"_kd",	(PyCFunction)K_kd, METH_VARARGS|METH_CLASS, K_kd_doc},
	{"_kz",	(PyCFunction)K_kz, METH_VARARGS|METH_CLASS, K_kz_doc},
	{"_kt",	(PyCFunction)K_kt, METH_VARARGS|METH_CLASS, K_kt_doc},
	{"_kp",	(PyCFunction)K_kp, METH_VARARGS|METH_CLASS, K_kp_doc},
	{"_ktn",(PyCFunction)K_ktn, METH_VARARGS|METH_CLASS, K_ktn_doc},
	{"_xT",	(PyCFunction)K_xT, METH_VARARGS|METH_CLASS, K_xT_doc},
	{"_xD",	(PyCFunction)K_xD, METH_VARARGS|METH_CLASS, K_xD_doc},

	{"_from_array_interface", (PyCFunction)K_from_array_interface,
                                   METH_O|METH_CLASS, K_from_array_interface_doc},

	{"inspect", (PyCFunction)K_inspect, METH_VARARGS, K_inspect_doc},
	{NULL,		NULL}		/* sentinel */
};

static int
K_init(KObject *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

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
	XSTRINGIFY(MODULE_NAME) ".K",	         	/*tp_name*/
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
        0,                      /*tp_call*/
        K_str,                  /*tp_str*/
        0,                      /*tp_getattro*/
        0,                      /*tp_setattro*/
        0,/*&K_as_buffer,         tp_as_buffer*/
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
        (initproc)K_init,       /*tp_init*/
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
	it->ptr = kG(obj->_k);
	it->itemsize = k_itemsize(obj->_k);
	if (!it->itemsize) {
		PyErr_Format(PyExc_NotImplementedError, "not iterable: t=%d", obj->_k->t);
		return NULL;
	}
	it->end = it->ptr + it->itemsize * obj->_k->n;
	it->itemtype = obj->_k->t;
	PyObject_GC_Track(it);
	return (PyObject *)it;
}

static PyObject *
kiter_next(kiterobject *it)
{
	PyObject *ret = NULL;
	assert(PyArrayIter_Check(it));
	if (it->ptr < it->end)
		switch (it->itemtype) {
		case KS: /* most common case: use list(ks) */
			ret = PyString_FromString(*(char**)it->ptr);
			break;
		case 0:
			ret = KObject_FromK(it->obj->ob_type, *(K*)it->ptr);
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

/* Initialization function for the module (*must* be called init_k) */
#define INIT(x) init ## x
#define XINIT(x) INIT(x)
PyMODINIT_FUNC
XINIT(MODULE_NAME)(void)
{
	PyObject *m;
	/* PyObject* c_api_object; */

	/* Finalize the type object including setting type of the new type
	 * object; doing it here is required for portability to Windows
	 * without requiring C++. */
	if (PyType_Ready(&K_Type) < 0)
		return;

	/* Create the module and add the functions */
	m = Py_InitModule3(XSTRINGIFY(MODULE_NAME), _k_methods, module_doc);
	if (!m)
		return;
	/* Add some symbolic constants to the module */
	if (ErrorObject == NULL) {
	  ErrorObject = PyErr_NewException(XSTRINGIFY(MODULE_NAME) ".error", NULL, NULL);
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

	PyModule_AddStringConstant(m, "__version__", __version__);
}
