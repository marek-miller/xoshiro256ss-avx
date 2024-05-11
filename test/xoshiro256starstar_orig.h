/*
 * A thin wrapper around the original implementation of Xoshiro256**
 * by Blackman and Vigna:
 *
 *   David Blackman and Sebastiano Vigna. 2021.
 *   Scrambled Linear Pseudorandom Number Generators.
 *   ACM Trans. Math. Softw. 47, 4,
 *   Article 36 (December 2021), 32 pages.
 *   https://doi.org/10.1145/3460772
 *
 * Their implementation uses a static internal state for the PRNG, hence our
 * wrapper in not thead-safe and is used only as a referente for the tests
 * suite.
 *
 */

#ifndef XOSHIRO256STARSTAR_ORIG_H
#define XOSHIRO256STARSTAR_ORIG_H

#include <stdint.h>

/*
 * Use a pointer to an array of 4 uint64_t to quickly
 * overwrite the state of the PRNG.
 */
void
xoshiro256starstar_orig_set(uint64_t *state);

uint64_t
xoshiro256starstar_orig_next(void);

void
xoshiro256starstar_orig_jump(void);

void
xoshiro256starstar_orig_long_jump(void);

#endif // XOSHIRO256STARSTAR_ORIG_H
