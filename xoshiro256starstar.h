#ifndef XOSHIRO256STARSTAR_H
#define XOSHIRO256STARSTAR_H

#include <stddef.h>
#include <stdint.h>

struct xoshiro256starstar {
	_Alignas(32) uint64_t s[16];
};

struct xoshiro256starstar_smpl {
	_Alignas(32) uint64_t a[4];
};

int xoshiro256starstar_init(struct xoshiro256starstar *rng, uint64_t seed);

size_t xoshiro256starstar_filln(struct xoshiro256starstar *rng,
	struct xoshiro256starstar_smpl *buf, size_t n);

inline double u64_to_f64n(uint64_t a)
{
	return (a >> 11) * 0x1.0p-53;
}

#endif // XOSHIRO256STARSTAR_H
