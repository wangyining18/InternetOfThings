#include "Particle.h"
#include "application.h"
#include "wave.h"

unsigned int localPort = 8888;

UDP Udp;

IPAddress remoteIP(18,218,51,247);

int port = 23333;

int chunk_size=16000;


void setup() {
    Serial.begin(9600);

    Udp.begin(8888);
}


void loop() {
    if (WAVE_SAMPLE_RATE != 16000){
        Serial.printf("Warning:  Incorrect sampling rate!\n");
    }

    Udp.sendPacket((uint8_t*)(&WAVE_DATA[0]) , sizeof(float)*chunk_size, remoteIP, port);
}
