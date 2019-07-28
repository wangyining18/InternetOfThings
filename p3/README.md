Partner: Yongming Fan (email: fanyo@indiana.edu)

After logging into the server, run the server.py with the port number using command: $ python3 server.py 23333

Wait for the data to be received

Use command "make" to compile and create client.cpp and firmware.bin

Use command "make usb" to program Argon to be uploaded to server through Argon.

Server will save the post-filter audio and perform an power-calculation on the audio data. 

Open a web brower and enter the url with the correct port number: http://iot.lukefahr.org:23333
