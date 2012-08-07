/* -*- mode: c; c-basic-offset: 8 -*- */
#include "k.h"
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
static int (*Py_Main)(int argc, char **argv);
K py(K x)
{
	char *error;
	void *h;
	K r;
	h = dlopen("libpython2.6.so", RTLD_NOW|RTLD_GLOBAL);
	if (!h)
		return krr(dlerror());
	dlerror();    /* Clear any existing error */
	Py_Main = dlsym(h, "Py_Main");
	if ((error = dlerror()))
		return krr(dlerror());
	if (x->t) {
		R krr("type");
	}
	I const n = x->n;
        I m = 0;
	DO(n, K y; if ((y = kK(x)[i])->t != KC) R krr("atype");
	   m += y->n + 1);
	char** const argv = malloc(sizeof(char*) * n);
	if (!argv) {
		R krr("memory");
	}
        char* const buf = malloc(m);
	if (!buf) {
		free(argv);
		R krr("memory");
	}
	char* p = buf;
	DO(n, K y = kK(x)[i]; 
	   argv[i] = memcpy(p, kG(y), y->n);
	   p += y->n; *p++ = '\0');
	r = ki(Py_Main(n, argv));
	dlclose(h);
	free(argv);
	free(buf);
	R r;
}
