#include <stddef.h>
#include <stdint.h>

#include "xoshiro256ss.h"


uint64_t xoshiro256ss_splitmix64(uint64_t *state) {
	uint64_t rt = (*state += UINT64_C(0x9E3779B97f4A7C15));
	rt = (rt ^ (rt >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	rt = (rt ^ (rt >> 27)) * UINT64_C(0x94D049BB133111EB);

	return rt ^ (rt >> 31);
}

