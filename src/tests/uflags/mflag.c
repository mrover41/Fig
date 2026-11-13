#include <utest/mflag.h>
#include <stddef.h>

size_t mindex = 0;

static const bool *mask = NULL;
static size_t mask_size = 0;

void _tmalloc(const bool *arr, size_t arr_size) {
	mask = arr;
	mask_size = arr_size;
	mindex = 0;
}

void _tmalloc_disable() {
	mask_size = 0;
	mindex = 0;
}

bool _gmalloc() {
	if (mask_size == 0) return false;

	bool val = mask[mindex];
    mindex = (mindex + 1) % mask_size;

	return val;
}
