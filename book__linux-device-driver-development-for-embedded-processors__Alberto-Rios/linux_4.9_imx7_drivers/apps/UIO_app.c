

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#define BUFFER_LENGHT 128

#define GPDAT1_offset 	   0x00
#define GPDIR1_offset 	   0x04
#define GPDAT6_offset 	   0x50000
#define GPDIR6_offset 	   0x50004

#define GPIO1_DIR_MASK 1 << 2
#define GPIO1_DATA_MASK 1 << 2

#define GPIO6_DIR_MASK (1 << 20 | 1 << 21)
#define GPIO6_DATA_MASK (1 << 20 | 1 << 21)

#define LED_RED_MASK 1 << 2
#define LED_GREEN_MASK 1 << 20
#define LED_BLUE_MASK 1 << 21

#define UIO_SIZE "/sys/class/uio/uio0/maps/map0/size"

int main()
{
	int ret, devuio_fd;
	unsigned int uio_size;
	int GPDIR1_read, GPDIR1_write; 
	int GPDIR6_read, GPDIR6_write; 
	int GPDAT1_read, GPDAT1_write; 
	int GPDAT6_read, GPDAT6_write; 
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

	printf("Mapping done\n");

	/* Set GPIO1_IO_2 direction bit to output */
	GPDIR1_read = *(int *)(demo_driver_map + GPDIR1_offset);
	GPDIR1_write = GPDIR1_read | (GPIO1_DIR_MASK);
	*(int *)(demo_driver_map + GPDIR1_offset) = GPDIR1_write;

	printf("DIR1 set done\n");
	
	GPDIR6_read = *(int *)(demo_driver_map + GPDIR6_offset);
	GPDIR6_write = GPDIR6_read | (GPIO6_DIR_MASK);
	*(int *)(demo_driver_map + GPDIR6_offset) = GPDIR6_write;

	printf("DIR6 set done\n");

	/* set all leds to 0 output */
	GPDAT1_read = *(int *)(demo_driver_map + GPDAT1_offset);
	GPDAT1_write = GPDAT1_read & ~(GPIO1_DATA_MASK);
	*(int *)(demo_driver_map + GPDAT1_offset) = GPDAT1_write;

	GPDAT6_read = *(int *)(demo_driver_map + GPDAT6_offset);
	GPDAT6_write = GPDAT6_read & ~(GPIO6_DATA_MASK);
	*(int *)(demo_driver_map + GPDAT6_offset) = GPDAT6_write;
	
	/* control the LED */
	do {
		printf("Enter led value: on, off, or exit :\n");
		scanf("%[^\n]%*c", sendstring);
		if(strncmp(led_on, sendstring, 3) == 0)
		{
			GPDAT1_read = *(int *)(demo_driver_map + GPDAT1_offset);
			GPDAT1_write = GPDAT1_read | (GPIO1_DATA_MASK);
			*(int *)(demo_driver_map + GPDAT1_offset) = GPDAT1_write;
		}
		else if(strncmp(led_off, sendstring, 2) == 0)
		{
			GPDAT1_read = *(int *)(demo_driver_map + GPDAT1_offset);
			GPDAT1_write = GPDAT1_read & ~(GPIO1_DATA_MASK);
			*(int *)(demo_driver_map + GPDAT1_offset) = GPDAT1_write;
		}
		else if(strncmp(Exit, sendstring, 4) == 0)
		printf("Exit application\n");

		else {
			printf("Bad value\n");
			GPDAT1_read = *(int *)(demo_driver_map + GPDAT1_offset);
			GPDAT1_write = GPDAT1_read & ~(GPIO1_DATA_MASK);
			*(int *)(demo_driver_map + GPDAT1_offset) = GPDAT1_write;
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

