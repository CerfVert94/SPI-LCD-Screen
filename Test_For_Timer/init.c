#include "./Sys/timers.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
	uint32_t it_per_tick = 0;
	int degree;
	if (argc < 2) {
		fputs("Need the degree of precision.\n",stdout);
		return -1;
	}
	degree = atoi(argv[1]);
	if(degree <= 0) {
		fputs("Invalid degree of precision.\n",stdout);
		return -1;
	}
	assert(MapTimerMem() != -1);

	assert(calibrate_timing(degree) != -1);
	it_per_tick = read_tickdata();
	printf("it/tick : %u\n", it_per_tick);
	assert(it_per_tick != 0);
	assert(UnmapTimerMem() != -1);


	return 0;
}
