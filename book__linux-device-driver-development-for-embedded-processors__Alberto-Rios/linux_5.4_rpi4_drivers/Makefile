
obj-m := helloworld_rpi4.o helloworld_rpi4_with_parameters.o helloworld_rpi4_char_driver.o helloworld_rpi4_class_driver.o misc_rpi4_driver.o hellokeys_rpi4.o ledRGB_rpi4_platform.o ledRGB_rpi4_class_platform.o led_rpi4_UIO_platform.o io_rpi4_expander.o ltc3206_rpi4_led_class.o int_rpi4_key.o int_rpi4_key_wait.o keyled_rpi4_class.o linkedlist_rpi4_platform.o sdma_rpi4_m2m.o i2c_rpi4_accel.o adxl345_rpi4.o ltc2607_rpi4_dual_device.o ltc2422_rpi4_dual.o ltc2422_rpi4_trigger.o adxl345_rpi4_iio.o helloworld_rpi4_with_timing.o


KERNEL_DIR ?= $(HOME)/linux

all:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
		M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) \
		ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- \
		M=$(PWD) clean

deploy:
	scp *.ko root@10.0.0.10:/home/


