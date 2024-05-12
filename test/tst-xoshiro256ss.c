#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "xoshiro256ss.h"
#include "xoshiro256starstar_orig.h"

static int TEST_RT = 0;

#define TEST_FAIL(...)                                                         \
	{                                                                      \
		fprintf(stderr, "FAIL %s:%d ", __FILE__, __LINE__);            \
		fprintf(stderr, __VA_ARGS__);                                  \
		fprintf(stderr, "\n");                                         \
		TEST_RT = -1;                                                  \
	}

static uint64_t
splitmix64(uint64_t *state)
{
	uint64_t rt = (*state += UINT64_C(0x9E3779B97f4A7C15));
	rt	    = (rt ^ (rt >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	rt	    = (rt ^ (rt >> 27)) * UINT64_C(0x94D049BB133111EB);

	return rt ^ (rt >> 31);
}

/*
 * Verify if the intialization is performed expliticly like this:
 *
 * 1. Split the seed with splitmix64
 * 2  Perform 128 rounds of Xoshiro to escape the zeroland
 * 2. Jump 4 times, to split the state into 4 paralel PRNGs (as colums)
 *
 */

static void
seed_global_test_rng(uint64_t seed)
{
	uint64_t smx = seed;
	uint64_t s[4];

	s[0] = splitmix64(&smx);
	s[1] = splitmix64(&smx);
	s[2] = splitmix64(&smx);
	s[3] = splitmix64(&smx);
	xoshiro256starstar_orig_set(s);
	for (size_t _ = 0; _ < 128; _++)
		xoshiro256starstar_orig_next();
}

void
test_init(void)
{
	const uint64_t seed = UINT64_C(0x100e881);
	uint64_t       exp_st[16];

	seed_global_test_rng(seed);

	xoshiro256starstar_orig_get(exp_st);
	xoshiro256starstar_orig_jump();
	xoshiro256starstar_orig_get(exp_st + 4);
	xoshiro256starstar_orig_jump();
	xoshiro256starstar_orig_get(exp_st + 8);
	xoshiro256starstar_orig_jump();
	xoshiro256starstar_orig_get(exp_st + 12);
	/* traspose the matix */
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < i; j++) {
			uint64_t x	  = exp_st[i * 4 + j];
			uint64_t y	  = exp_st[j * 4 + i];
			exp_st[i * 4 + j] = y;
			exp_st[j * 4 + i] = x;
		}
	}

	struct xoshiro256ss rng;
	if (xoshiro256ss_init(&rng, seed) < 0) {
		TEST_FAIL("PRNG init");
		return;
	}

	/* Verify */
	for (size_t i = 0; i < 16; i++) {
		if (exp_st[i] != rng.s[i]) {
			TEST_FAIL("PRGN init, wrong state at %zu", i);
			break;
		}
	}
}

void
test_zeroinit(void)
{
	struct xoshiro256ss rng;
	if (xoshiro256ss_init(&rng, UINT64_C(0x00)) < 0) {
		TEST_FAIL("PRNG init");
		return;
	}
	for (size_t i = 0; i < 16; i++) {
		if (rng.s[i] == 0)
			TEST_FAIL("state contains 0x00 at %zu", i);
	}
}

void
test_filln_aligned_01(void)
{
	uint64_t seed = UINT64_C(0x834333c);

#define SIZE (32)
	uint64_t		expct[4][SIZE];
	_Alignas(0x20) uint64_t buf[4 * SIZE];

	for (size_t b = 0; b < 4; b++) {
		seed_global_test_rng(seed);
		for (size_t _ = 0; _ < b; _++)
			xoshiro256starstar_orig_jump();
		for (size_t i = 0; i < SIZE; i++)
			expct[b][i] = xoshiro256starstar_orig_next();
	}

	struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, seed);
	xoshiro256ss_filln(&rng, buf, SIZE * 4);

	for (size_t b = 0; b < 4; b++) {
		for (size_t i = 0; i < SIZE; i++) {
			if (buf[b + 4 * i] != expct[b][i]) {
				TEST_FAIL(
					"result differs at block %zu, index %zu",
					b, i);
				return;
			}
		}
	}
}

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	test_init();
	test_zeroinit();
	test_filln_aligned_01();

	return TEST_RT;
}
