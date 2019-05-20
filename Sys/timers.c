#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "timers.h"
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
volatile uint32_t *timer = NULL;
volatile uint32_t *systimer = NULL;
uint32_t itn_per_tick = 0;
uint32_t resolution = 0;

int MapSysTimerMem()
{
	uint8_t mem_fd;
	if(systimer != NULL)
		return -2;
	// System time can be directly accessed through /dev/mem and an appropriate offset.
	if((mem_fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) == -1) {
		fputs("Failed opening /dev/mem\n", stderr);
		return -1;
	}

	systimer = (uint32_t *)mmap(NULL, 
				TIMERS_SIZE, 
				PROT_READ, MAP_SHARED, 
				mem_fd, 
				SYSTIMER_OFFSET);
	
	// Since the desired pointer is probably retrieved, close the file now.
	if(close(mem_fd) < 0) {
		fputs("Failed closing /dev/mem.\n", stderr);
	}


	//If retrieval of the pointer fails return -1.
	if(systimer == MAP_FAILED) {
		fputs("Failed mapping the system timer.\n", stderr);
		return -1;
	}
	return 0;

}

int UnmapSysTimerMem()
{
	if(systimer != NULL) {
		if(munmap((void *)systimer, TIMERS_SIZE) < 0) {
			fputs("System Timer : Failed removing the memory mapping.\n", stderr);
			return -1;
		}
	}	
	
	return 0;
}
int MapTimerMem()
{
//	int i = 0;
	uint8_t mem_fd;
	timer = NULL;
	// System time can be directly accessed through /dev/mem and an appropriate offset.
	if((mem_fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) == -1) {
		fputs("Failed opening /dev/mem\n", stderr);
		return -1;
	}

	// Save the pointer of /dev/mem + TIMER_OFFSET in "time"
	timer = (uint32_t *)mmap(NULL, 
				TIMERS_SIZE, 
				PROT_READ | PROT_WRITE, MAP_SHARED, 
				mem_fd, 
				TIMER_OFFSET);

	// Since the desired pointer is probably retrieved, close the file now.
	if(close(mem_fd) < 0) {
		fputs("Failed closing /dev/mem.\n", stderr);
	}


	//If retrieval of the pointer is failed, return -1.
	if(timer == MAP_FAILED) {
		fputs("Failed mapping the timer.\n", stderr);
		return -1;
	}
	return 0;

}

int UnmapTimerMem()
{
	if(timer != NULL) {
		if(munmap((void *)timer, TIMERS_SIZE) < 0) {
			fputs("Timer : Failed removing the memory mapping.\n", stderr);
			return -1;
		}
	}	
	
	return 0;
}
/****************************************
 *********** DELAY FUNCTIONS ************
 ***************************************/

#define LOAD_VALUE 0xFFFFFFFF
#define MINIMUM 0xFFFFFFF0
inline long microsleep_loop(uint32_t usec) 
{
/* example from wiringPi.c by Gordon Henderson; 
 * struct timeval tNow, tLong, tEnd;
 * gettimeofday(&tNow, NULL);
 * tLong.tv_sec = usec / 1000000;
 * tLong.tv_sec = usec % 1000000;
 * timeradd(&tNow,&tLong,&tEnd);
 * while(timercmp(&tNow,&tEnd,<))
 * gettimeofday(&tNow,NULL);*/

//	uint32_t i = 0;
	usec *= resolution;
	while(itn_per_tick < usec) {
		usec -= itn_per_tick;		
	}
	usec = 0;
	
	return 0;
}
/*
 * Timer Control (Cf. bcm2835 datasheet):
 * 
 * 1st digit :
 * 0x2 = Prescale is clock (No prescale)
 * 0x6 = Prescale is clock/16
 * 0xA = Prescale is clock/256
 * 	Bits [02:00] = 10 (23-bit counter)
 *	Bits [03:00] = PRESCALE MODE:
 * 
 * 2nd digit :
 * 0xA = IRQ/TIMER enabled
 * 0x2 = IRQ enabled
 * 0x8 = Timer enabled
 * 	Bits [07:04] = 1010 (IRQ/Timer enabled)
 * 	Bits [07:04] = 0010 (IRQ enabled)
 * 	Bits [07:04] = 1000 (Timer enabled)
 *
 * 3rd digit :
 * 0x3 = Halt timer in debug mode and let FRC running.
 * 0x1 = Halt timer in debug mode and disable FRC
 * 	Bit 8 = 0 : Timers runs in debug halted mode
 * 		1 : Timers halts in debug halted mode
 * 	Bit 9 = Enable free running counter
 * 	Bits [11:10] = 00
 *
 * 4th digit :
 * 0x0 = Unused
 * 	Bits [15:12] = 0000
 * 5~6th digit :
 * 0x0 = Prescale of FRC
 *
 * Bits [31:16] = 0x0 (Unused)
 */

inline long microsleep(uint32_t usec) 
{
	
//	uint32_t lastCount;


	TIMER_CONTROL = 0x0182; //Enable clock
	
	// Reset timer. 
	// Load value on next rising edge : 
	do {
		TIMER_LOAD = LOAD_VALUE;
	}while(TIMER_VALUE < MINIMUM);

	//Timer decrement automatically.
	usec = TIMER_VALUE - usec;	
	while(usec < TIMER_VALUE);
//	lastCount = TIMER_VALUE;
	
	TIMER_CONTROL = 0x0122; //Disable clock
	return 0;// LOAD_VALUE - lastCount;
}



// Functions related to adjusment for more precision
uint32_t measure_timing(uint32_t usec, uint32_t resolution) 
{
	int i = 0;
	clock_t clk_start, clk_end;
	uint32_t delay, itn, precise_itn, diff;
	double eps  = 1.0, min_eps = 1.0;//epsilon = magin of error

	assert(usec < (0xFFFFFFFF / resolution));

	for (itn = 1; itn <= resolution; itn++) {
		diff = 0;
		for(i = 0; i < 100; i++) {
			clk_start = clock();
			delay = usec * resolution;
			while(itn < delay)
				delay -= itn;
			clk_end = clock();

			diff += (clk_end - clk_start);
		}
		diff /= i;
		if(diff > usec)
			eps = (double)(diff - usec)/usec;
		else
			eps = (double)(usec - diff)/usec;
		if(min_eps >= eps) {
			precise_itn = itn;
			min_eps = eps;
//			printf("Margin of error :%lf\n", eps);
		}
	}
//	printf("The final yield of it/tick : %u\n", value);
	return write_tickdata(precise_itn, resolution);
}
#define PATH_TICKDATA "./tickdata"

int read_tickdata(void)
{
	int fd;
	uint32_t buf[2];
	//Write it/tick into a file "./tick_data"
	if((fd = open(PATH_TICKDATA, O_RDONLY)) == -1) {
		fputs("Read : Failed opening\n", stderr);
		return -1;
	}
	if((read(fd, buf, sizeof(uint32_t) * 2)) < 0)
		fputs ("Read :Failed reading\n",stderr);

	itn_per_tick = buf[0];
	resolution = buf[1];

	if(close(fd) < 0) {
		fputs("Read : Failed closing.\n", stderr);
		return -1;
	}
	return -1;
}

int write_tickdata(uint32_t itn_per_tick, uint32_t resolution) 
{
	int fd;
	uint32_t buf[2] = {itn_per_tick, resolution};
	//Write it/tick into a file "./tick_data"
	if((fd = open(PATH_TICKDATA, O_RDWR | O_CREAT, S_IRWXU)) == -1) {
		fputs("Write : Failed opening or creating.\n", stderr);
		return -1;
	}

	printf("Record %u itn per tick.\n", itn_per_tick);
	printf("Record resolution : %u.\n", resolution);

	if((write(fd, buf, sizeof(uint32_t) * 2)) <= 0)
		fputs ("Write : Failed writing.",stderr);
	
	if(close(fd) < 0) {
		fputs("Write : Failed closing.\n", stderr);
		return -1;
	}
	return 0;	
}













/*
The number of iteration per 'count' countdown
inline uint32_t it_per_countdown(const uint32_t count)
{
	 * Measure the number of loop iterated during
	 * "count" countdown of the timer. 
	uint32_t loop = 0, target = 0;

	TIMER_CONTROL = 0x0182; //Enable clock
	do {
		target = LOAD_VALUE - count;
		TIMER_LOAD = LOAD_VALUE; // Initialize counter.
	}while(TIMER_VALUE < LOAD_VALUE);

	while(TIMER_VALUE >  target){
		loop++;
	}
	TIMER_CONTROL = 0x0122; //Disable clock
	return loop;
}
int calibrate_timing(uint32_t it) 
{
	const int repeat_cntdown = 100;
	uint32_t cntdown = 0, i = 0, j = 0, value;
	double it_per_tick = 0.0, avg_it_per_tick = 0.0;

	if (timer == NULL)
		return -1;		

	 *  multiply 'countdown' by 10 each time, 'repeat' times.
	 *  Average the number of iteration per countdown (it/cntdown)
	 *  This yields the number of iteration per clock tick (it/tick) *

	cntdown = 0xA;

	for (i = 0; i <= repeat; i++) {	

		it_per_tick = 0.0;
		// Measure it/cntdown 100 times and sum up the measurement.
		for (j = 0; j < repeat_cntdown; j++) {
			it_per_tick += it_per_countdown(cntdown);
		}
		it_per_tick /= repeat_cntdown;	
		printf("%.0lf it per %u cntdown.\n", it_per_tick, cntdown);	
		it_per_tick /= cntdown; //average to get it/tick
		printf("%.0lf it per tick\n", it_per_tick);
		avg_it_per_tick += it_per_tick; //Sum up it/tick for various range of countdown.
		cntdown *= 10;
	}
	printf("Average : %.1f / %d\n", avg_it_per_tick, i);
	value = (uint32_t)(avg_it_per_tick / i);
	printf("The final yield of it/tick : %u\n", value);
	return write_tickdata(value);
}*/

