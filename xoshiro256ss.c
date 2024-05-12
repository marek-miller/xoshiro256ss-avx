#include <stddef.h>
#include <stdint.h>

#include "xoshiro256ss.h"

static uint64_t
splitmix64(uint64_t *state)
{
	uint64_t rt;

	rt = (*state += UINT64_C(0x9E3779B97f4A7C15));
	rt = (rt ^ (rt >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	rt = (rt ^ (rt >> 27)) * UINT64_C(0x94D049BB133111EB);

	return rt ^ (rt >> 31);
}

static inline uint64_t
rotl64(const uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}

static uint64_t
scalar_next(uint64_t *s)
{
	const uint64_t rt = rotl64(s[1] * 5, 7) * 9;

	const uint64_t t = s[1] << 17;
	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];
	s[2] ^= t;
	s[3] = rotl64(s[3], 45);

	return rt;
}

static void
scalar_jump128(uint64_t *s)
{
	static const uint64_t JUMP[4] = { 0x180ec6d33cfd0aba,
		0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

	uint64_t s0 = 0, s1 = 0, s2 = 0, s3 = 0;
	for (size_t i = 0; i < 4; i++)
		for (size_t b = 0; b < 64; b++) {
			if (JUMP[i] & UINT64_C(1) << b) {
				s0 ^= s[0];
				s1 ^= s[1];
				s2 ^= s[2];
				s3 ^= s[3];
			}
			scalar_next(s);
		}
	s[0] = s0;
	s[1] = s1;
	s[2] = s2;
	s[3] = s3;
}

int
xoshiro256ss_init(struct xoshiro256ss *rng, uint64_t seed)
{
	uint64_t smx = seed;
	uint64_t tmp[4];

	tmp[0] = splitmix64(&smx);
	tmp[1] = splitmix64(&smx);
	tmp[2] = splitmix64(&smx);
	tmp[3] = splitmix64(&smx);
	for (size_t _ = 0; _ < 128; _++)
		scalar_next(tmp);

	rng->s[0 * 4 + 0] = tmp[0];
	rng->s[1 * 4 + 0] = tmp[1];
	rng->s[2 * 4 + 0] = tmp[2];
	rng->s[3 * 4 + 0] = tmp[3];
	scalar_jump128(tmp);
	rng->s[0 * 4 + 1] = tmp[0];
	rng->s[1 * 4 + 1] = tmp[1];
	rng->s[2 * 4 + 1] = tmp[2];
	rng->s[3 * 4 + 1] = tmp[3];
	scalar_jump128(tmp);
	rng->s[0 * 4 + 2] = tmp[0];
	rng->s[1 * 4 + 2] = tmp[1];
	rng->s[2 * 4 + 2] = tmp[2];
	rng->s[3 * 4 + 2] = tmp[3];
	scalar_jump128(tmp);
	rng->s[0 * 4 + 3] = tmp[0];
	rng->s[1 * 4 + 3] = tmp[1];
	rng->s[2 * 4 + 3] = tmp[2];
	rng->s[3 * 4 + 3] = tmp[3];

	return 0;
}

/*
 * This furncion assumes two things:
 *  1. that the buffer pointed to by buf is aligned to 32 bytes,
 *  2. that the length of the buffer is divisible by 4, i.e.
 *     the buffer contains a multiple of 32 bytes.
 * The function can take a buffer of length 0, i.e. when n=0,
 * as long as buf points to valid memory.
 */
extern size_t
filln_aligned(struct xoshiro256ss *rng, uint64_t *buf, size_t n);

static size_t
filln_unaligned(struct xoshiro256ss *rng, uint64_t *buf, size_t n)
{
	size_t rt = 0;

	_Alignas(0x20) uint64_t buf_alg[4];
	for (size_t b = 0; b < n / 4; b++) {
		rt += filln_aligned(rng, buf_alg, 4);
		buf[b * 4 + 0] = buf_alg[0];
		buf[b * 4 + 1] = buf_alg[1];
		buf[b * 4 + 2] = buf_alg[2];
		buf[b * 4 + 3] = buf_alg[3];
	}
	if (n % 4 > 0) {
		rt += filln_aligned(rng, buf_alg, 4);
		for (size_t i = 0; i < n % 4; i++)
			buf[(n / 4) * 4 + i] = buf_alg[i];
	}

	return rt;
}

size_t
xoshiro256ss_filln(struct xoshiro256ss *rng, uint64_t *buf, size_t n)
{
	size_t rt = 0;
	/* buf points to uint64_t, which is always aligned to 8 bytes */
	size_t algmt64 = ((uintptr_t)buf & 0x1f) / 8;
	size_t offst   = (4 - algmt64) % 4;
	if (algmt64 > 0) {
		rt += filln_unaligned(rng, buf, offst > n ? offst : n);
		if (offst > n)
			return rt;
	}

	size_t blk_alg = (n - offst) / 4 * 4;
	rt += filln_aligned(rng, buf + offst, blk_alg);
	if (n > offst + blk_alg)
		rt += filln_unaligned(
			rng, buf + offst + blk_alg, n - offst - blk_alg);

	return rt;
}
