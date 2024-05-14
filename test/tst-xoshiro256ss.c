#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "xoshiro256ss.h"
#include "xoshiro256starstar_orig.h"

static _Atomic(int) TEST_RT = 0;

#define TEST_FAIL(...)  do {                                                   \
		fprintf(stderr, "FAIL %s:%d ", __FILE__, __LINE__);            \
		fprintf(stderr, __VA_ARGS__);                                  \
		fprintf(stderr, "\n");                                         \
		TEST_RT = -1;                                                  \
	} while(0)

static uint64_t
splitmix64(uint64_t *st)
{
	uint64_t rt;

	rt = (*st += UINT64_C(0x9E3779B97f4A7C15));
	rt = (rt ^ (rt >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	rt = (rt ^ (rt >> 27)) * UINT64_C(0x94D049BB133111EB);

	return rt ^ (rt >> 31);
}

/*
 * Verify if the intialization is performed expliticly like this:
 *
 * 1. Split the seed with splitmix64
 * 2. Jump 8 times, to split the state into paralel PRNGs (as colums)
 *
 */
static void
seed_orig_rng(uint64_t *s, uint64_t seed)
{
	uint64_t smx = seed;
	for (size_t i = 0; i < 4; i++)
		s[i] = splitmix64(&smx);
}

static void
test_init(void)
{
	_Alignas(0x40) struct xoshiro256ss rng;

	const uint64_t	seed = UINT64_C(0x100e881);
	uint64_t	s[4];
	uint64_t	expct[XOSHIRO256SS_WIDTH][4];
	seed_orig_rng(s, seed);
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH; i++) {
		xoshiro256starstar_orig_jump(s);
		memcpy(expct[i], s, 8 * 4);
	}


	int rt = xoshiro256ss_init(&rng, seed);
	if (rt <  0)
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
				return;
			}
		}
	}
}

static void
test_zeroinit(void)
{
	_Alignas(0x40) struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, UINT64_C(0x00));
	for (size_t i = 0; i < XOSHIRO256SS_WIDTH * 4; i++) {
		if (rng.s[i] == 0)
			TEST_FAIL("state contains 0x00 at %zu", i);
			return;
	}
}

static void
test_filln_aligned01(void)
{
	uint64_t seed = UINT64_C(0x834333c);
	uint64_t s[4];
#define SIZE UINT64_C(3 * 1024 * 1024)
	uint64_t *expct = malloc(sizeof *expct * XOSHIRO256SS_WIDTH * SIZE);
	uint64_t *buf =
		aligned_alloc(0x40, sizeof *buf * XOSHIRO256SS_WIDTH * SIZE);
	if (!(expct && buf)) {
		TEST_FAIL("memory allocation");
		goto exit;
	}

	for (size_t b = 0; b < XOSHIRO256SS_WIDTH; b++) {
		seed_orig_rng(s, seed);
		for (size_t _ = 0; _ < b + 1; _++)
			xoshiro256starstar_orig_jump(s);
		for (size_t i = 0; i < SIZE; i++)
			expct[b * SIZE + i] = xoshiro256starstar_orig_next(s);
	}

	_Alignas(0x40) struct xoshiro256ss rng;
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

static inline double
to_double(uint64_t x)
{
	const union {
		uint64_t i;
		double	 d;
	} u = { .i = UINT64_C(0x3FF) << 52 | x >> 12 };
	return u.d - 1.0;
}

static void
test_filln_aligned02_f64n(void)
{
	uint64_t seed = UINT64_C(0x1e42fffc);
	uint64_t s[4];
#define SIZE UINT64_C(7 * 1024 * 1024)
	double *expct = malloc(sizeof *expct * XOSHIRO256SS_WIDTH * SIZE);
	double *buf =
		aligned_alloc(0x40, sizeof *buf * XOSHIRO256SS_WIDTH * SIZE);
	if (!(expct && buf)) {
		TEST_FAIL("memory allocation");
		goto exit;
	}

	for (size_t b = 0; b < XOSHIRO256SS_WIDTH; b++) {
		seed_orig_rng(s, seed);
		for (size_t _ = 0; _ < b + 1; _++)
			xoshiro256starstar_orig_jump(s);
		for (size_t i = 0; i < SIZE; i++)
			expct[b * SIZE + i] =
				to_double(xoshiro256starstar_orig_next(s));
	}

	_Alignas(0x40) struct xoshiro256ss rng;
	xoshiro256ss_init(&rng, seed);
	xoshiro256ss_filln_f64n(&rng, buf, SIZE);

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


/* Split tests into a few threads */

#define TEST_THRDS (4)
static thrd_t test_thrd[TEST_THRDS];
static int test_thrd_res[TEST_THRDS];

int test_thrd_worker01(void *p) {
	(void)p;

	test_init();

	return 0;
}

int test_thrd_worker02(void *p) {
	(void)p;

	test_zeroinit();

	return 0;
}

int test_thrd_worker03(void *p) {
	(void)p;

	test_filln_aligned01();

	return 0;
}

int test_thrd_worker04(void *p) {
	(void)p;

	test_filln_aligned02_f64n();

	return 0;
}

static thrd_start_t test_thrd_worker[TEST_THRDS] = {
	test_thrd_worker01,
	test_thrd_worker02,
	test_thrd_worker03,
	test_thrd_worker04,
};

int
main(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	
	for (int i = 0; i < TEST_THRDS; i++) 
		if (thrd_create(&test_thrd[i], test_thrd_worker[i], NULL) != thrd_success)
			TEST_FAIL("cannot create thread %d", i);

	for (int i = 0; i < TEST_THRDS; i++)
		thrd_join(test_thrd[i], &test_thrd_res[i]);

	if (TEST_RT == 0)
		printf("OK\n");

	return TEST_RT;
}
