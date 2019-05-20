#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "./Sys/gpio.h"
#include "./Sys/timers.h"
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>



#define CLOCK_RATE 1 //Not in second!
#define SCL_SPEED 250000

#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1
#define ST77XX_NOP     0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID   0x04
#define ST77XX_RDDST   0x09

#define ST77XX_SLPIN   0x10
#define ST77XX_SLPOUT  0x11
#define ST77XX_PTLON   0x12
#define ST77XX_NORON   0x13

#define ST77XX_INVOFF  0x20
#define ST77XX_INVON   0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON  0x29
#define ST77XX_CASET   0x2A
#define ST77XX_RASET   0x2B
#define ST77XX_RAMWR   0x2C
#define ST77XX_RAMRD   0x2E

#define ST77XX_PTLAR   0x30
#define ST77XX_COLMOD  0x3A
#define ST77XX_MADCTL  0x36

#define ST77XX_MADCTL_MY  0x80
#define ST77XX_MADCTL_MX  0x40
#define ST77XX_MADCTL_MV  0x20
#define ST77XX_MADCTL_ML  0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1   0xDA
#define ST77XX_RDID2   0xDB
#define ST77XX_RDID3   0xDC
#define ST77XX_RDID4 0xDD

#define BIT(pin) (1 << (pin))
uint32_t clock_speed;
void ToBinary(const char *name, uint32_t hex, int bit_len)
{
	int i = 0;
	printf("%s\n", name);
	printf("\t0x%X\n", hex);
	printf("\t0b");
	for (i = bit_len - 1; i >= 0; i--) 
		printf("%d", (hex >> i) & 0x1);
	putc('\n', stdout);
	
}
void CharToBinary(const char *name, char hex, int bit_len)
{
	int i = 0;
	printf("%s\n", name);
	printf("\t0b");
	for (i = bit_len - 1; i >= 0; i--) 
		printf("%d", (hex >> i) & 0x1);
	putc('\n', stdout);
	
}
int CPOL = 0, CPHA = 0;
//uint8_t pin_rs, pin_cs, pin_scl, pin_sca;
#define PIN_RS 18//Register select pin (A0)
#define PIN_CS 8 //Chip select pin
#define PIN_SCL 11// 11 // Serial clock line
#define PIN_SDA 10//10// Serial data line
#define PIN_VCC 17
#define PIN_VCCI 27
#define LED_POS 3
#define PIN_RESET 4
#define NC 52
//#define 

void ConfigPin(uint8_t pin, uint8_t fnc)
{
	if (pin < 10) {
		pin -= 0;	
		SEL_FNC(GPFSEL0, pin, fnc);
	}
	else if (pin < 20) {
		pin -= 10;
		SEL_FNC(GPFSEL1, pin, fnc);
	}
	else if (pin < 30) {
		pin -= 20;	
		SEL_FNC(GPFSEL2, pin, fnc);
	}
}
int CheckPin(uint8_t pin, uint8_t fnc) 
{
	if (pin < 10) {
		pin -= 0;
		return ((GPFSEL0 >> (pin * 3)) & fnc) == fnc;

	}
	else if (pin < 20) {
		pin -= 10;
		return ((GPFSEL1 >> (pin * 3)) & fnc) == fnc;
	}
	else if (pin < 30) {
		pin -= 20;
		return ((GPFSEL2 >> (pin * 3)) & fnc) == fnc;
	}
	return 1;

}

void hw_reset()
{
	GPCLR0 = BIT(PIN_RESET);
	microsleep(1000); 
	GPSET0 = BIT(PIN_RESET);
	microsleep(500000);
}

/*
void pause(uint32_t iteration)
{
	static int i =0;
	for(i = 0; i < iteration; i++);
}
*/
const uint32_t sda_mask = BIT(PIN_SDA), scl_mask = BIT(PIN_SCL), rs_mask = BIT(PIN_RS);
	
void SPI_Mode0_Tx(uint32_t *data, int len)
{
	uint32_t data_mask;

	GPCLR0 = BIT(PIN_CS);
	data_mask = BIT((len - 1));
	while(len--)
	{
		if(*data & data_mask) {
			GPCLR0 = scl_mask;
			GPSET0 = sda_mask;
		}
		else {
			GPCLR0 = scl_mask;
			GPCLR0 = sda_mask;
		}
		GPSET0 = scl_mask;
		data_mask >>= 1;
	}
	GPSET0 = BIT(PIN_CS);
	
}
void send_command(uint8_t cmd)
{
	GPCLR0 = rs_mask;
	SPI_Mode0_Tx((uint32_t*)&cmd, 8);
	

}
void send_parameter(uint32_t param, int len)
{
	
	GPSET0 = rs_mask;
	SPI_Mode0_Tx(&param,len);	
}
void set_window(int x, int y)
{
	send_command(ST77XX_CASET);// 15: Column addr set, 4 args, no delay:
	send_parameter(0x00, 8);
	send_parameter(x + 2, 8); // Xsend_command(START = 2
	send_parameter(0x00, 8); 
	send_parameter(x + 3, 8); // XEND = 129
	send_command(ST77XX_RASET);// 16: Row addr set, 4 args, no delay:
	send_parameter(0x00, 8);
	send_parameter(y + 1, 8); // Xsend_command(START = 1
	send_parameter(0x00, 8);
	send_parameter(y + 2, 8); // XEND = 160

}
void write_display(uint32_t *ppPixel,int sizeX, int sizeY, const int8_t clr_res){
	int  i, len;
	uint32_t pixel_mask, default_mask = BIT((clr_res - 1));
	
	len = sizeY * sizeX;
	
	send_command(ST77XX_CASET);// 15: Column addr set, 4 args, no delay:
	send_parameter(0x00, 8);
	send_parameter(2, 8); // Xsend_command(START = 2
	send_parameter(0x00, 8); 
	send_parameter(129, 8); // XEND = 129
	send_command(ST77XX_RASET);// 16: Row addr set, 4 args, no delay:
	send_parameter(0x00, 8);
	send_parameter(1, 8); // Xsend_command(START = 1
	send_parameter(0x00, 8);
	send_parameter(160+2, 8); // XEND = 160


	send_command(ST77XX_RAMWR);
	
	GPSET0 = rs_mask;
	GPCLR0 = BIT(PIN_CS);
	for(i = 0; i < len; i++) {	
		pixel_mask = default_mask;
		while(pixel_mask) {
			if(ppPixel[i] & pixel_mask) {
				GPCLR0 = scl_mask;
				GPSET0 = sda_mask;
			}
			else {
				GPCLR0 = scl_mask;
				GPCLR0 = sda_mask;
			}
			GPSET0 = scl_mask;
			pixel_mask >>= 1;
		}
	}
	GPSET0 = BIT(PIN_CS);
}


uint32_t set_pixel(int r, int g, int b) 
{
	uint32_t pixel;
	pixel = ((r * 63) / 255) << 18 |
	          ((g * 63) / 255) << 10 |
		  ((b * 63) / 255) << 2;
	return pixel;
}

void Powerdown()
{
	hw_reset();
	GPCLR0 = BIT(PIN_RESET);
	GPCLR0 = BIT(PIN_VCCI);
	GPCLR0 = BIT(PIN_VCC);
	GPCLR0 = BIT(PIN_RS);
	GPCLR0 = BIT(PIN_CS);
	GPCLR0 = BIT(PIN_SDA);
	GPCLR0 = BIT(PIN_SCL);
	GPCLR0 = BIT(LED_POS);

}
int Setup()
{
	uint8_t pins[] = {PIN_VCC, PIN_VCCI, PIN_RESET, PIN_RS,
			   PIN_CS, PIN_SCL,PIN_SDA, LED_POS, NC};
	int i = 0;
	while(pins[i] != NC) {
		ConfigPin(pins[i], SEL_OUTPUT);

		if(!CheckPin(pins[i], SEL_OUTPUT)) {
			return -1;
		}
		i++;
	}	
	Powerdown();
	microsleep(100000);
	// Turn on the LED.
	GPSET0 = BIT(LED_POS);

	// Reset is active low.
	GPSET0 = BIT(PIN_RESET);

	//Power on (VCC -> VCCI)
	GPSET0 = BIT(PIN_VCC);	
	GPSET0 = BIT(PIN_VCCI);

	//Hardware reset.
	hw_reset();
	send_command(ST77XX_SWRESET);//1: Software reset, no args, w/delay
	microsleep(50000); // 50 ms delay
	send_command(ST77XX_SLPOUT); //2: Out of sleep mode, no args, w/delay
	microsleep(500000);// 255 = 500 ms delay
	send_command(ST77XX_COLMOD);//3: Set color mode, 1 arg + delay:
	send_parameter(0x06, 8); // 16-bit color
	microsleep(10000); // 10 ms delay
	send_command(ST77XX_MADCTL);//3: Set color mode, 1 arg + delay:
	send_parameter(0x06, 8); // 16-bit color
	microsleep(10000); // 10 ms delay
	send_command(ST77XX_INVOFF);// 17: Normal display on, no args, w/delay
	send_parameter(0x00, 8);
	send_command(ST77XX_NORON);// 17: Normal display on, no args, w/delay
	microsleep(10000); // 10 ms delay
	send_command(ST77XX_DISPON); // 18: Main screen turn on, no args, w/delay
	microsleep(500000);  // 255 = 500 ms delay	*/
	send_command(ST77XX_CASET);// 15: Column addr set, 4 args, no delay:
	send_parameter(0x00, 8);
	send_parameter(2, 8); // Xsend_command(START = 2
	send_parameter(0x00, 8); 
	send_parameter(129, 8); // XEND = 129
	send_command(ST77XX_RASET);// 16: Row addr set, 4 args, no delay:
	send_parameter(0x00, 8);
	send_parameter(1, 8); // Xsend_command(START = 1
	send_parameter(0x00, 8);
	send_parameter(160+2, 8); // XEND = 160


	return 0;
}

int SPI_Setup()
{
	if( wiringPiSPISetup (0, SCL_SPEED) < 0) {
		fputs("Error: spi setup.", stderr);
		return -1;
	}
	return 0;
}
#define SIZE_X 128
#define SIZE_Y 160

uint32_t *movie = NULL;
off_t movie_size = 0;
off_t open_movie()
{
	off_t size;
        uint8_t movie_fd;
        if(movie != NULL)
                return -1;
        // Open movie
        if((movie_fd = open("./LineRabbit.mmov", O_RDONLY)) == -1) {
                fputs("Failed opening the movie.\n", stderr);
                return -1;
        }
	size = lseek(movie_fd, 0, SEEK_END);
	if (size <= 0) {
		fputs("Empty file or unable to open.\n",stderr);
		return -1;	
	}
//	printf("Size of the movie: %d bytes/n", size);
	lseek(movie_fd, 0, SEEK_SET);
	

        // Save the pointer of /movie_fd
        movie = (uint32_t *)mmap(NULL, size, PROT_READ , MAP_SHARED, movie_fd, 0);

        // Since the desired pointer is probably retrieved, close the file now.
        if(close(movie_fd) < 0) {
                fputs("Failed closing the movie\n", stderr);
        }

        //If retrieval of the pointer is failed, return -1.
        if(movie == MAP_FAILED) {
                fputs("Failed mapping the movie\n", stderr);
                return -1;
        }
        return 0;
}
int close_movie(){
        if(movie != NULL) {
                if(munmap((uint32_t*)movie, movie_size ) < 0) {
                        fputs("Failed removing the memory mapping.\n", stderr);
                        return -1;
                }
        }
        return 0;
}





int main(void )
{	
	int i = 0;
//	uint32_t pixels[SIZE_Y * SIZE_X * 52],
//	uint32_t color[] = {0xFC0000, 0xFC00, 0xFC}
	int  len, frame = 52;
	if(MapGPIOMem()) {
		fputs("Failed opening GPIO", stderr);
		return -1;
	}

	if(MapTimerMem()) {
		fputs("Failed opening Timer", stderr);
		return -1;
	}


	printf("Setup start:\n");
	if (Setup() < 0){
		fputs("Abort\n",stderr);
		return -1;
	}
	printf("Setup Finished. \n");
/*
	k = 0;	
	for(i = 0; i < SIZE_Y * SIZE_X; i++) {
			pixels[0][i] = color[0];
			pixels[1][i] = color[1];
			pixels[2][i] = color[2];
			
	}
*/
	if((movie_size = open_movie()) < 0)
		return -1;
	i = 0;

//	printf("0x%X\n",set_pixel(255,0,0));
/*	for (int y = 0; y < 160; y++) 
		for (int x = 0; x < 128; x++){
			printf("%X\n",movie[i++]);	
			microsleep(500000);
		}*/
	printf("Loaded the movie!");
	
	len = SIZE_X * SIZE_Y;
	while(1) {
		for(i = 0; i < frame; i++) {
			write_display(&movie[len * i], SIZE_X,SIZE_Y, 24);
		}
	}
	close_movie();
	UnmapGPIOMem();
	UnmapTimerMem();
	return 0;
}
