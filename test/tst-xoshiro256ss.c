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
}

void
test_init(void)
{
	const uint64_t seed = UINT64_C(0x100e881);
	uint64_t       exp_st[XOSHIRO256SS_WIDTH][4];

	seed_global_test_rng(seed);
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH; i++) {
		xoshiro256starstar_orig_jump();
		xoshiro256starstar_orig_get(exp_st[i]);
	}

	struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, seed);

	/* Verify */
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH; i++) {
		for (size_t j = 0; j < 4; j++) {
			if (exp_st[i][j] != rng.s[XOSHIRO256SS_WIDTH * j + i]) {
				TEST_FAIL("PRGN init, wrong state at %zu", i);
				break;
			}
		}
	}
}

void
test_zeroinit(void)
{
	struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, UINT64_C(0x00));
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH * 4; i++) {
		if (rng.s[i] == 0)
			TEST_FAIL("state contains 0x00 at %zu", i);
	}
}

void
test_filln_aligned(void)
{
	uint64_t seed = UINT64_C(0x834333c);

#define SIZE (64)
	uint64_t expct[XOSHIRO256SS_WIDTH][SIZE];
	for (size_t b = 0; b < XOSHIRO256SS_WIDTH; b++) {
		seed_global_test_rng(seed);
		for (size_t _ = 0; _ < b + 1; _++)
			xoshiro256starstar_orig_jump();
		for (size_t i = 0; i < SIZE; i++)
			expct[b][i] = xoshiro256starstar_orig_next();
	}

	struct xoshiro256ss_smpl buf[SIZE];
	struct xoshiro256ss	 rng;
	xoshiro256ss_init(&rng, seed);
	xoshiro256ss_filln(&rng, buf, SIZE);

	for (size_t b = 0; b < XOSHIRO256SS_WIDTH; b++) {
		for (size_t i = 0; i < SIZE; i++) {
			if (buf[i].s[b] != expct[b][i]) {
				TEST_FAIL(
					"result differs at block %zu, index %zu",
					b, i);
				return;
			}
		}
	}
#undef SIZE
}

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	test_init();
	test_zeroinit();
	test_filln_aligned();

	if (TEST_RT == 0)
		printf("OK\n");

	return TEST_RT;
}
