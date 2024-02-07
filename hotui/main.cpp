#include <dlfcn.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "../src/game/client/ui_rect.h"

typedef void (*plugin_ptr_t)();

int main()
{
	char *error;
	void * handle = dlopen("sample.so", RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		exit(1);
	}

	dlerror();    /* Clear any existing error */
	plugin_ptr_t tick_ptr;
	tick_ptr = (plugin_ptr_t)dlsym(handle, "ui_tick");



	error = dlerror();
	if (error != NULL) {
		fprintf(stderr, "%s\n", error);
		exit(1);
	}

	tick_ptr();

	dlclose(handle);
}

