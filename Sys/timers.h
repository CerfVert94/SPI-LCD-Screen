#include <stdint.h>
#define TIMERS_SIZE 	0x800

#define TIMER_OFFSET	0x2000B000
#define SYSTIMER_OFFSET	0x20003000

#define TIMER_LOAD		timer[0x400 >> 2] 
#define TIMER_VALUE 		timer[0x404 >> 2]
#define TIMER_CONTROL 		timer[0x408 >> 2]
#define TIMER_IRQ_WR 		timer[0x40C >> 2]
#define TIMER_IRQ_RD 		timer[0x410 >> 2]
#define TIMER_IRQ_MASKED 	timer[0x414 >> 2]
#define TIMER_RELOAD		timer[0x418 >> 2]
#define TIMER_PREDIV 		timer[0x41C >> 2]
#define TIMER_FRC		timer[0x420 >> 2]


#define SYSTIMER_CS	systimer[0x00]
#define SYSTIMER_CLO 	systimer[0x04]
#define SYSTIMER_CHI 	systimer[0x08]
#define SYSTIMER_C0 	systimer[0x0C]
#define SYSTIMER_C1 	systimer[0x10]
#define SYSTIMER_C2 	systimer[0x14]
#define SYSTIMER_C3	systimer[0x18]

extern volatile uint32_t *systimer;
extern volatile uint32_t *timer;
extern uint32_t itn_per_tick;
extern uint32_t resolution;
int MapTimerMem();
int UnmapTimerMem();

int MapSysTimerMem();
int UnmapSysTimerMem();

long microsleep(uint32_t usec);
long microsleep_loop(uint32_t usec);


//uint32_t it_per_countdown(const uint32_t count);
//int calibrate_timing(uint32_t repeat);
uint32_t measure_timing(uint32_t usec, uint32_t resolution);
int write_tickdata(uint32_t itn_per_tick, uint32_t resolution); 
int read_tickdata(void);

