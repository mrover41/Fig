#include <stdio.h>

#include <utest/mflag.h>

void *__real_malloc(size_t size);

void *__wrap_malloc(size_t size) {
	if (!_gmalloc()) return __real_malloc(size);
	printf("[MALLOC_W] Attemt malloc(%zu)\n", size);
	return NULL;
}
