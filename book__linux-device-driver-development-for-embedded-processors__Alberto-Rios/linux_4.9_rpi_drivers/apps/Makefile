CC = ~/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc

all: LTC2422_spidev

app : LTC2422_spidev.c
	$(CC) -o $@ $^
clean :
	rm LTC2422_spidev
deploy : LTC2422_spidev
	scp $^ root@10.0.0.10:
