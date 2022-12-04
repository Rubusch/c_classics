all: LTC2422_spidev

app : LTC2422_spidev.c
	$(CC) -o $@ $^
clean :
	rm LTC2422_spidev
deploy : LTC2422_spidev
	scp $^ root@10.0.0.10:
