

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define BUFFER_LENGHT 128
#define GPIO4_GDIR_offset 0x04
#define GPIO_DIR_MASK 1<<29
#define GPIO_DATA_MASK 1<<29

#define PIO_SODR1_offset 0x50
#define PIO_CODR1_offset 0x54
#define PIO_CFGR1_offset 0x44
#define PIO_MSKR1_offset 0x40

#define PIO_PB0_MASK (1 << 0)
#define PIO_PB5_MASK (1 << 5)
#define PIO_PB6_MASK (1 << 6)
#define PIO_CFGR1_MASK (1 << 8)

#define PIO_MASK_ALL_LEDS (PIO_PB0_MASK | PIO_PB5_MASK | PIO_PB6_MASK)

#define UIO_SIZE "/sys/class/uio/uio0/maps/map0/size"

int main()
{
	int ret, devuio_fd;
	unsigned int uio_size;
	void *temp;
	void *demo_driver_map;
	char sendstring[BUFFER_LENGHT];
	char *led_on = "on";
	char *led_off = "off";
	char *Exit = "exit";

	printf("Starting led example\n");
	devuio_fd = open("/dev/uio0", O_RDWR | O_SYNC);
	if (devuio_fd < 0){
		perror("Failed to open the device");
		exit(EXIT_FAILURE);
	}

	/* read the size that has to be mapped */
	FILE *size_fp = fopen(UIO_SIZE, "r");
	fscanf(size_fp, "0x%08X", &uio_size);
	fclose(size_fp);

	/* do the mapping */
	demo_driver_map = mmap(NULL, uio_size, PROT_READ|PROT_WRITE, MAP_SHARED, devuio_fd, 0);
	if(demo_driver_map == MAP_FAILED) {
		perror("devuio mmap");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}

	temp = demo_driver_map + PIO_MSKR1_offset;
	*(int *)temp |= PIO_MASK_ALL_LEDS;

	/* select output */
	temp = demo_driver_map + PIO_CFGR1_offset;
	*(int *)temp |= PIO_CFGR1_MASK;

	/* clear all the leds */
	temp = demo_driver_map + PIO_SODR1_offset;
	*(int *)temp |= PIO_MASK_ALL_LEDS;

	/* control the LED */
	do {
		printf("Enter led value: on, off, or exit :\n");
		scanf("%[^\n]%*c", sendstring);
		if(strncmp(led_on, sendstring, 3) == 0)
		{
			temp = demo_driver_map + PIO_CODR1_offset;
			*(int *)temp |= PIO_PB0_MASK;
		}
		else if(strncmp(led_off, sendstring, 2) == 0)
		{
			temp = demo_driver_map + PIO_SODR1_offset;
			*(int *)temp |= PIO_PB0_MASK;
		}
		else if(strncmp(Exit, sendstring, 4) == 0)
		printf("Exit application\n");

		else {
			printf("Bad value\n");
			temp = demo_driver_map + PIO_SODR1_offset;
			*(int *)temp |= PIO_PB0_MASK;
			return -EINVAL;
		}

	} while(strncmp(sendstring, "exit", strlen(sendstring)));

	ret = munmap(demo_driver_map, uio_size);
	if(ret < 0) {
		perror("devuio munmap");
		close(devuio_fd);
		exit(EXIT_FAILURE);
	}

	close(devuio_fd);
	printf("Application termined\n");
	exit(EXIT_SUCCESS);
}

