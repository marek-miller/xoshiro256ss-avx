#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

/*
 * Verify if the intialization is performed expliticly like this:
 *
 * 1. Split the seed with splitmix64
 * 2. Jump 8 times, to split the state into paralel PRNGs (as colums)
 *
 */
static void
seed_global_test_rng(uint64_t seed)
{
	uint64_t smx = seed;
	uint64_t s[4];

	for (size_t i = 0; i < 4; i++)
		s[i] = xoshiro256ss_splitmix64(&smx);
	xoshiro256starstar_orig_set(s);
}

static void
test_init(void)
{
	const uint64_t seed = UINT64_C(0x100e881);
	uint64_t       expct[XOSHIRO256SS_WIDTH][4];

	seed_global_test_rng(seed);
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH; i++) {
		xoshiro256starstar_orig_jump();
		xoshiro256starstar_orig_get(expct[i]);
	}

	struct xoshiro256ss rng;
	int		    rt = xoshiro256ss_init(&rng, seed);
	if (rt < 0)
		TEST_FAIL("init() reported error: %d", rt);
	if (rt != XOSHIRO256SS_TECH)
		TEST_FAIL("init() reported wrong technology: %d (should be %d)",
			rt, XOSHIRO256SS_TECH);

	/* Verify */
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH; i++) {
		for (size_t j = 0; j < 4; j++) {
			if (expct[i][j] != rng.s[XOSHIRO256SS_WIDTH * j + i]) {
				TEST_FAIL(
					"PRGN init, wrong state at i=%zu, j=%zu",
					i, j);
				break;
			}
		}
	}
}

static void
test_zeroinit(void)
{
	struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, UINT64_C(0x00));
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH * 4; i++) {
		if (rng.s[i] == 0)
			TEST_FAIL("state contains 0x00 at %zu", i);
	}
}

static void
test_filln_aligned01(void)
{
	uint64_t seed = UINT64_C(0x834333c);

#define SIZE (3 * 1024 * 1024)
	uint64_t *expct = malloc(sizeof *expct * XOSHIRO256SS_WIDTH * SIZE);
	uint64_t *buf =
		aligned_alloc(64, sizeof *buf * XOSHIRO256SS_WIDTH * SIZE);
	if (!(expct && buf)) {
		TEST_FAIL("memory allocation");
		goto exit;
	}

	for (size_t b = 0; b < XOSHIRO256SS_WIDTH; b++) {
		seed_global_test_rng(seed);
		for (size_t _ = 0; _ < b + 1; _++)
			xoshiro256starstar_orig_jump();
		for (size_t i = 0; i < SIZE; i++)
			expct[b * SIZE + i] = xoshiro256starstar_orig_next();
	}

	struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, seed);
	xoshiro256ss_filln(&rng, buf, SIZE);

	for (size_t b = 0; b < XOSHIRO256SS_WIDTH; b++) {
		for (size_t i = 0; i < SIZE; i++) {
			if (buf[i * XOSHIRO256SS_WIDTH + b] !=
				expct[b * SIZE + i]) {
				TEST_FAIL(
					"result differs at block %zu, index %zu",
					b, i);
				goto exit;
			}
		}
	}
#undef SIZE
exit:
	free(expct);
	free(buf);
}

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	test_init();
	test_zeroinit();
	test_filln_aligned01();

	if (TEST_RT == 0)
		printf("OK\n");

	return TEST_RT;
}
