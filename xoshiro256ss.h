#ifndef XOSHIRO256SS_H
#define XOSHIRO256SS_H

#include <stddef.h>
#include <stdint.h>

struct xoshiro256ss {
	_Alignas(32) uint64_t s[16];
};

int
xoshiro256ss_init(struct xoshiro256ss *rng, uint64_t seed);

size_t
xoshiro256ss_filln(struct xoshiro256ss *rng, uint64_t *buf, size_t n);

size_t
xoshiro256ss_filln_f64n(struct xoshiro256ss *rng, double *buf, size_t n);

enum xoshiro256ss_jumpkind {
	XOSHIRO256SS_JUMP128 = 0,
	XOSHIRO256SS_JUMP192 = 1,
};

void
xoshiro256ss_jump(struct xoshiro256ss *rng, enum xoshiro256ss_jumpkind);

#endif // XOSHIRO256STARSTAR_H
