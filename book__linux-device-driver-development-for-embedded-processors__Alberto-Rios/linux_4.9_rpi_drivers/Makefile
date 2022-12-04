
obj-m += helloworld_rpi.o helloworld_rpi_with_parameters.o helloworld_rpi_with_timing.o
obj-m += helloworld_rpi_char_driver.o helloworld_rpi_class_driver.o misc_rpi_driver.o
obj-m += hellokeys_rpi.o ledRGB_rpi_platform.o ledRGB_rpi_class_platform.o led_rpi_UIO_platform.o
obj-m += io_rpi_expander.o ltc3206_rpi_led_class.o 
obj-m += int_rpi_key.o int_rpi_key_wait.o keyled_rpi_class.o
obj-m += linkedlist_rpi_platform.o
obj-m += sdma_rpi_m2m.o
obj-m += i2c_rpi_accel.o adxl345_rpi.o 
obj-m += ltc2607_rpi_dual_device.o ltc2422_rpi_dual.o ltc2422_rpi_trigger.o
obj-m += adxl345_rpi_iio.o


KERNEL_DIR ?= $(HOME)/linux

all:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
		SUBDIRS=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
		SUBDIRS=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10:


