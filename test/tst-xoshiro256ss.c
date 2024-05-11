#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "xoshiro256ss.h"
#include "xoshiro256starstar_orig.h"


static int TST_RT = 0;

static uint64_t TST_INIT_STATE[4] = { 0x1, 0x11, 0x111, 0x1111 };

int
main(int argc, char **argv) 
{
	(void)argc;
	(void)argv;

	
	xoshiro256starstar_orig_set(TST_INIT_STATE);
	uint64_t smpl = xoshiro256starstar_orig_next();
	printf("smpl = %lu\n", smpl);

	return TST_RT;
}
