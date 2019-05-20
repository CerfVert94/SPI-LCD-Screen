#include "MapMem.h"
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>
#include "gpio.h"

void *MapMem(const char *path, int fopen_flags, void *addr, int prot, int mmem_flags, off_t offset)
{
	uint8_t fd;
	// GPIO can be directly accessed through /dev/gpiomem
	if((fd = open("/dev/gpiomem", O_RDWR | O_SYNC )) == -1) {
		fputs("Failed opening /dev/gpiomem\n", stderr);
		return FAILURE_NORMAL;
	}

	// Save the pointer of /dev/gpiomem in "gpiomem"
	gpiomem = (uint32_t *)mmap(NULL, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpiomem_fd, 0);

	// Since the desired pointer is probably retrieved, close the file now.
	if(close(gpiomem_fd) < 0) {
		fputs("Failed closing /dev/gpiomem.\n", stderr);
	}

	//If retrieval of the pointer is failed, return -1.
	if(gpiomem == MAP_FAILED) {
		fputs("Failed mapping the GPIO selection registers.\n", stderr);
		return FAILURE_NORMAL;
	}
	return SUCCESS;

}
