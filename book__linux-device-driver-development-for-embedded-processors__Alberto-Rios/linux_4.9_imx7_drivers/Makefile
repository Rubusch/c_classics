
//obj-m += ledRGB_imx_class_platform.o led_imx_UIO_platform.o 
//obj-m += adxl345_imx.o adxl345_imx_iio.o adxl345_imx_iio_filter.o i2c_imx_accel.o io_imx_expander.o 
//obj-m += ltc2607_imx_dual_device.o ltc2422_imx_dual.o ltc2422_imx_trigger.o
//obj-m += sdma_imx_m2m.o sdma_imx_sg_m2m.o sdma_imx_mmap.o 
//obj-m += helloworld_imx_char_driver.o helloworld_imx_class_driver.o misc_imx_driver.o hellokeys_imx.o helloworld_imx.o helloworld_imx_with_parameters.o helloworld_imx_with_timing.o
//obj-m += ledRGB_imx_platform.o ltc3206_imx_led_class.o int_imx_key.o int_imx_key_wait.o keyled_imx_class.o 

obj-m += helloworld_imx.o helloworld_imx_with_parameters.o helloworld_imx_with_timing.o
obj-m += helloworld_imx_char_driver.o helloworld_imx_class_driver.o misc_imx_driver.o
obj-m += hellokeys_imx.o ledRGB_imx_platform.o ledRGB_imx_class_platform.o led_imx_UIO_platform.o
obj-m += io_imx_expander.o ltc3206_imx_led_class.o 
obj-m += int_imx_key.o int_imx_key_wait.o keyled_imx_class.o
obj-m += linkedlist_imx_platform.o
obj-m += sdma_imx_m2m.o sdma_imx_sg_m2m.o sdma_imx_mmap.o
obj-m += i2c_imx_accel.o adxl345_imx.o 
obj-m += ltc2607_imx_dual_device.o ltc2422_imx_dual.o ltc2422_imx_trigger.o
obj-m += adxl345_imx_iio.o

KERNEL_DIR ?= $(HOME)/my-linux-imx

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


