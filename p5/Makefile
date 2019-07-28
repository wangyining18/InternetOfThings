
DEVICE_NAME		?= iot

all: firmware.bin

# builds the Argon firmeware given the CPP files
firmware.bin: client.cpp
	particle compile argon client.cpp --saveTo firmware.bin
	
# flashes the Argon device over the web
web: 
	particle flash ${DEVICE_NAME} firmware.bin 

# flashes the Argon device over USB
usb: firmware.bin
	particle flash --usb firmware.bin

# flashes the Argon device over USB
serial: firmware.bin
	particle flash --serial firmware.bin

.PHONY: clean
clean:
	rm -rf firmware.bin
