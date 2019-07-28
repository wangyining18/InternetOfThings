Partner: Yongming Fan (email: fanyo@indiana.edu)

In this project, Argon will blink an LED using interrupts generated from a timer. To blink the LED, use the following command to build the project.

$ make all PLATFORM=argon APP=timer PARTICLE_DEVELOP=1 USE_SWD_JTAG=y MODULAR=n

Then, use the following command to flash the Argon.

$ make all program-dfu PLATFORM=argon APP=timer PARTICLE_DEVELOP=1 USE_SWD_JTAG=y MODULAR=n
