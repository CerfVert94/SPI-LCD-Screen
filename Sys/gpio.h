#define GPIO_SIZE 4096
#define	GPFSEL0 gpiomem[0]
#define GPFSEL1 gpiomem[1]
#define GPFSEL2 gpiomem[2]
#define GPFSEL3 gpiomem[3]


#define GPSET0 gpiomem[7]
#define GPSET1 gpiomem[8]

#define GPCLR0 gpiomem[10]
#define GPCLR1 gpiomem[11]

#define GPLEV0 gpiomem[13]
#define GPLEV1 gpiomem[14]

#define SEL_FNC(gpfsel, pin, fnc) gpfsel = (gpfsel & (~((0x7)<<(pin*3)))) | (fnc << (pin * 3))

#define SEL_OUTPUT 0x1
#define SEL_INPUT 0x0

#define SUCCESS 0
#define FAILURE_NORMAL -1
#define FAILURE_OPENED -2
extern volatile uint32_t *gpiomem;
int MapGPIOMem();
int UnmapGPIOMem();
