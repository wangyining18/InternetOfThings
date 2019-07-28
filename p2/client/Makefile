
DEVICE_NAME		?= iot

all: firmware.bin

# generate a CPP Header file given a WAV file
wave.h: elephant.wav
	python3 wav_to_header.py -wav elephant.wav -cpp wave.h

# builds the Argon firmeware given the CPP files
firmware.bin: client.cpp wave.h 
	particle compile argon client.cpp wave.h --saveTo firmware.bin
	
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
	rm -rf wave.h firmware.bin
