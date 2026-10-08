#include <utest/mflag.h>

bool FLAG = false;

void _tmalloc(bool set) {
	FLAG = set;
}

bool _gmalloc() {
	return FLAG;
}
