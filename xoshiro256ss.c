#include <stddef.h>
#include <stdint.h>

#include "xoshiro256ss.h"


static uint64_t splitmix64(uint64_t *state) {
	uint64_t rt = (*state += UINT64_C(0x9E3779B97f4A7C15));
	rt = (rt ^ (rt >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
	rt = (rt ^ (rt >> 27)) * UINT64_C(0x94D049BB133111EB);

	return rt ^ (rt >> 31);
}


static inline uint64_t rotl64(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}


static uint64_t x256ss_scalar_next(uint64_t *s) {
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



static void x256ss_scalar_jump128(uint64_t *s) {
	static const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

	uint64_t s0 = 0;
	uint64_t s1 = 0;
	uint64_t s2 = 0;
	uint64_t s3 = 0;
	for(size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
		for(size_t b = 0; b < 64; b++) {
			if (JUMP[i] & UINT64_C(1) << b) {
				s0 ^= s[0];
				s1 ^= s[1];
				s2 ^= s[2];
				s3 ^= s[3];
			}
			x256ss_scalar_next(s);	
		}
	s[0] = s0;
	s[1] = s1;
	s[2] = s2;
	s[3] = s3;
}



int 
xoshiro256ss_init(struct xoshiro256ss *rng, uint64_t seed) {

	uint64_t tmp[4];
	uint64_t spl_tmp = seed;
	tmp[0] = splitmix64(&spl_tmp); 
	tmp[1] = splitmix64(&spl_tmp); 
	tmp[2] = splitmix64(&spl_tmp); 
	tmp[3] = splitmix64(&spl_tmp); 
	for (size_t _ = 0; _  < 16; _++)
		x256ss_scalar_next(tmp);
	
	rng->s[0*4 + 0]	= tmp[0];
	rng->s[1*4 + 0]	= tmp[1];
	rng->s[2*4 + 0]	= tmp[2];
	rng->s[3*4 + 0]	= tmp[3];
	x256ss_scalar_jump128(tmp);
	rng->s[0*4 + 1]	= tmp[0];
	rng->s[1*4 + 1]	= tmp[1];
	rng->s[2*4 + 1]	= tmp[2];
	rng->s[3*4 + 1]	= tmp[3];
	x256ss_scalar_jump128(tmp);
	rng->s[0*4 + 2]	= tmp[0];
	rng->s[1*4 + 2]	= tmp[1];
	rng->s[2*4 + 2]	= tmp[2];
	rng->s[3*4 + 2]	= tmp[3];
	x256ss_scalar_jump128(tmp);
	rng->s[0*4 + 3]	= tmp[0];
	rng->s[1*4 + 3]	= tmp[1];
	rng->s[2*4 + 3]	= tmp[2];
	rng->s[3*4 + 3]	= tmp[3];

	return 0;
}


