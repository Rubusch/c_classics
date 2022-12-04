
obj-m += helloworld_sam.o helloworld_sam_with_parameters.o helloworld_sam_with_timing.o
obj-m += helloworld_sam_char_driver.o helloworld_sam_class_driver.o misc_sam_driver.o
obj-m += hellokeys_sam.o ledRGB_sam_platform.o ledRGB_sam_class_platform.o led_sam_UIO_platform.o
obj-m += io_sam_expander.o ltc3206_sam_led_class.o
obj-m += int_sam_key.o int_sam_key_wait.o keyled_sam_class.o
obj-m += linkedlist_sam_platform.o
obj-m += sdma_sam_m2m.o sdma_sam_mmap.o
obj-m += i2c_sam_accel.o adxl345_sam.o
obj-m += ltc2607_sam_dual_device.o ltc2422_sam_dual.o ltc2422_sam_trigger.o
obj-m += adxl345_sam_iio.o

KERNEL_DIR ?= $(HOME)/my-linux-sam

all:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- \
		SUBDIRS=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-poky-linux-gnueabi- \
		SUBDIRS=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10:


