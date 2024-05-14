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
	
	uint64_t steps;
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
 * Returns: 
 *  cumulative number of steps taken by the generator
 */
uint64_t
xoshiro256ss_filln(struct xoshiro256ss *rng, uint64_t *buf, size_t n);

uint64_t
xoshiro256ss_filln_f64n(struct xoshiro256ss *rng, double *buf, size_t n);


enum {
	XOSHIRO256SS_JUMP128 = 1,
	XOSHIRO256SS_JUMP192 = 2,
};

void
xoshiro256ss_jump(struct xoshiro256ss *rng, int flag);

#endif /* XOSHIRO256SS_H */
