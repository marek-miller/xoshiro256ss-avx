#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "xoshiro256ss.h"
#include "xoshiro256starstar_orig.h"

static int TEST_RT = 0;

#define TEST_FAIL(...)                                                         \
	fprintf(stderr, "FAIL %s:%d ", __FILE__, __LINE__);                    \
	fprintf(stderr, __VA_ARGS__);                                          \
	fprintf(stderr, "\n");                                                 \
	TEST_RT = -1

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
void
test_init(void)
{
	const uint64_t seed	 = UINT64_C(0x100e881);
	uint64_t       splmix_st = seed;
	uint64_t       exp_st[16];

	exp_st[0] = splitmix64(&splmix_st);
	exp_st[1] = splitmix64(&splmix_st);
	exp_st[2] = splitmix64(&splmix_st);
	exp_st[3] = splitmix64(&splmix_st);
	xoshiro256starstar_orig_set(exp_st);
	for (size_t i = 0; i < 128; i++)
		xoshiro256starstar_orig_next();
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
			uint64_t x = exp_st[i*4 + j];
			uint64_t y = exp_st[j*4 + i];
			exp_st[i*4 + j] = y;
			exp_st[j*4 + i] = x;
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
		if (rng.s[i] == 0) {
			TEST_FAIL("state contains 0x00 at %zu", i);
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

	return TEST_RT;
}
