/* -*- mode: c; c-basic-offset: 8 -*- */
#include "k.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
static int (*Py_Main)(int argc, char **argv);
K py(K f, K x, K lib)
{
	char *error;
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
	char** const argv = malloc(sizeof(char*) * (xn+1));
	P(!argv, krr("memory"));
        char* const buf = malloc(m);
	P(!buf,(free(argv),krr("memory")));
	argv[0] = f->s;
	char* p = buf;
	DO(xn,
	   K y = kK(x)[i]; 
	   argv[i+1] = memcpy(p, kG(y), y->n);
	   p += y->n; *p++ = '\0');
	r = ki(Py_Main(xn+1, argv));
	dlclose(h);
	free(argv);
	free(buf);
	R r;
}
