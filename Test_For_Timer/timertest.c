#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "../Sys/gpio.h"
#include "../Sys/timers.h"

#define NCHECK 200
#define CLK_NANO_TIME 1000
double test_microsleep_loop(uint32_t usec)
{
	int i = 0, pass = 0, total = 0;
	clock_t clk_start, clk_end;
	uint32_t duration;
	double tol, success; 
	int res;

	for(tol = 0.05; tol >= 0.00; tol -= 0.01) {
		for (i = 0; i < NCHECK; i++) {
			total++;
			clk_start = clock();

			microsleep_loop(usec);
			clk_end = clock();
			assert(res != -1);

			duration = clk_end - clk_start;
			if(usec * (1.0 - tol) <= duration &&  duration <= usec * (1.0 + tol))
				pass++;	
			//printf("%u/%u %.2lf\n", duration, nsec, tol);
		}
	}
	printf("%d/%d Passed with the tolerance ranging from 0~5%% with step size of +1%%\n",pass, total);
	success = (double)pass/total * 100.0;
//	printf("usleep_loop success rate : %.2lf \n", success);
	return success;
}
double test_microsleep(uint32_t usec)
{
	int i = 0, pass = 0, total = 0;
	clock_t clk_start, clk_end;
	uint32_t duration/*,real_delay*/;
	double tol, success; 
	int res;

	for(tol = 0.05; tol >= 0.00; tol -= 0.01) {
		for (i = 0; i < NCHECK; i++) {
			total++;

			clk_start = clock();
			res = microsleep(usec);
			clk_end = clock();
			
			assert(res != -1);
		
			duration = clk_end - clk_start;
			if(usec * (1.0 - tol) <= duration && duration <= usec * (1.0 + tol))
				pass++;	
		}
	}
	printf("%d/%d Passed with the tolerance ranging from 0~5%% with step size of +1%%\n",pass, total);
	success = (double)pass / total * 100.0;
//	printf("usleep success rate : %.2lf\n", success);
	return success;
}

#define TIMER_DELAY 0
#define LOOP_DELAY 1
#define SUCCESS_THRESHOLD 0.95
int main(int argc, char *argv[])
{
	uint32_t usec_a = 0, usec_b = 0, step = 0, usec = 0;
	bool error = false;
	double success_rate[2];
	switch (argc) {

		case 1:
		fputs("Need the duration of delay.\n",stdout);
		error = true;
		break;

		case 3: 
		fputs("Need the step of increment between duration1 and duration2.\n",stdout);
		error = true;
		break;

		case 4:
		step = (uint32_t) atoi(argv[3]);
		usec_b = (uint32_t) atoi(argv[2]);
		case 2: 
		usec_a = (uint32_t) atoi(argv[1]);
		/* duration1 must be greater than 0.
		 * duration2 must be greater than duration1.
		 * step must be greater than the difference of both. */
		if(usec_a <= 0 && usec_b > usec_a && (usec_b - usec_a) > step) {
			fputs("Invalid arguments.\n",stdout);
			error = true;
		}	
		break;

		default:
		fputs("Too many arguments.\n",stdout);
		error = true;
		break;
	}
	if (error) {
		fputs("Ex: ./timertest [num1] [num2 -- optional] ",stderr);
	       	fputs("[step -- mandatory if num2 entered]\n",stderr);
		return -1;
	}
		
	// Test for memory mapping.
	assert(MapTimerMem() != -1); 
	usec = usec_a;
	while(usec <= usec_b) {
		printf("For %u us:\n", usec);
		measure_timing(usec, 1000);
		// Read the tick data.
		read_tickdata();

		assert(itn_per_tick != 0);
		fputc('\t',stdout);
		success_rate[TIMER_DELAY] = test_microsleep(usec);
		fputc('\t',stdout);
		success_rate[LOOP_DELAY] = test_microsleep_loop(usec);
		printf("\tmicrosleep success rate : %.2lf\n", success_rate[TIMER_DELAY]);
		printf("\tmicrosleep_loop success rate : %.2lf\n", success_rate[LOOP_DELAY] );
//		getchar();
		usec += step;

	}
	assert(UnmapTimerMem() != -1);
	return 0;
}
