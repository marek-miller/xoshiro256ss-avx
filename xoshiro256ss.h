#ifndef XOSHIRO256SS_H
#define XOSHIRO256SS_H

#include <stddef.h>
#include <stdint.h>

/* 
 * Specify AVX technology.  
 * 	1 - AVX2
 * 	2 - AVX512
 */
#ifndef XOSHIRO256SS_TECH
#define XOSHIRO256SS_TECH 1 
#endif

#define XOSHIRO256SS_WIDTH (8)

/*
 * TODO: write section about aligment requirements
 * for dynamically allocated structs.
 */

struct xoshiro256ss {
	_Alignas(64) uint64_t s[4 * XOSHIRO256SS_WIDTH];
};

/* Returns:
 *  1 - AVX2
 *  2 - AVX512
 * -1 - error (e.g. alignment)
 */
int
xoshiro256ss_init(struct xoshiro256ss *rng, uint64_t seed);

/* 
 * The buffer must be aligned to 64 bytes, 
 * and the length must be multiple of 8.
 * 
 * Returns: number of invocations to the parellel PRNG (= n)
 */
size_t
xoshiro256ss_filln(struct xoshiro256ss *rng, uint64_t *buf, size_t n);

void
xoshiro256ss_jump128(struct xoshiro256ss *rng);

void
xoshiro256ss_jump192(struct xoshiro256ss *rng);


uint64_t
xoshiro256ss_splitmix64(uint64_t *st);

double
xoshiro256ss_u64_to_f64n(uint64_t x);

#endif /* XOSHIRO256SS_H */
