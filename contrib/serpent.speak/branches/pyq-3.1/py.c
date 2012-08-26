/* -*- mode: c; c-basic-offset: 8 -*- */
#include "k.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#ifdef PY3K
static int (*Py_Main)(int argc, wchar_t **argv);
static wchar_t *(*c2w)(char*, size_t *);
static void (*PyMem_Free)(void *);
#else
static int (*Py_Main)(int argc, char **argv);
#endif
K py(K f, K x, K lib)
{
	int argc; char **argv;
	char *error, *p, *buf;
	void *h;
	K r;
	h = dlopen((const char *)kG(lib), RTLD_NOW|RTLD_GLOBAL);
	if (!h)
		R krr(dlerror());
	dlerror();    /* Clear any existing error */
	Py_Main = dlsym(h, "Py_Main");
	P((error = dlerror()),krr(error));
	P(xt, krr("argv type"));
        I m = 0;     /* buf length */
	DO(xn,
	   K y;
	   P((y = kK(x)[i])->t!=KC, krr("arg type"));
	   m += y->n+1);
	argc = xn+1;
	argv = malloc(sizeof(char*) * argc);
	P(!argv, krr("memory"));
        buf = malloc(m);
	P(!buf,(free(argv),krr("memory")));
	argv[0] = f->s;
	p = buf;
	DO(xn,
	   K y = kK(x)[i]; 
	   argv[i+1] = memcpy(p, kG(y), y->n);
	   p += y->n; *p++ = '\0');
#ifdef PY3K
	dlerror();    /* Clear any existing error */
        c2w = dlsym(h, "_Py_char2wchar");
        P((error = dlerror()),krr(error));
	dlerror();    /* Clear any existing error */
        PyMem_Free = dlsym(h, "PyMem_Free");
        P((error = dlerror()),krr(error));
	wchar_t **wargv = malloc(sizeof(wchar_t*)*argc);
	wchar_t **wargv_copy = malloc(sizeof(wchar_t*)*argc);
	DO(argc,P(!(wargv[i]=c2w(argv[i],NULL)),krr("decode")));
	memcpy(wargv_copy, wargv, sizeof(wchar_t*)*argc); 
	r = ki(Py_Main(argc, wargv));
	DO(argc,PyMem_Free(wargv_copy[i]));
	free(wargv_copy);
	free(wargv);
#else
	r = ki(Py_Main(argc, argv));
#endif
	dlclose(h);
	free(argv);
	free(buf);
	R r;
}
