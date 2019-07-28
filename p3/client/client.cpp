#include "Particle.h"
#include "application.h"


UDP Udp;
IPAddress remoteIP(18,218,51,247);
int port = 23333;
unsigned int localPort = 8888;
int chunk_size = 1000;


int ledPin = D7;   
int analogPin = A4;


int i = 0;

int prevValue = 0;
int currValue = 0;


char* response;
int res = 1;
int buffer[16000];

void button_handler(system_event_t event, int duration, void* )
{
    if (!duration) { 
        digitalWrite(ledPin, HIGH);
    }
    else { // just released
        digitalWrite(ledPin, LOW);
    }
}


void setup() {
    // test
    /*digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);*/

    Serial.begin(9600);
    Udp.begin(8888);

    pinMode(analogPin, INPUT);
    pinMode(ledPin, OUTPUT);
    
    System.on(button_status, button_handler);
}


void loop() {
    // Waiting the button
    Serial.printf("Waiting for the button\n");
    while (1){
        Serial.printf("Waiting for the button\n");
        if (System.buttonPushed() > 1) {
            break;
        }
    }
    
    
    // Record the voice
    Serial.println("Started recording!");
    while (i != 16000){
        currValue = micros();
        if ((currValue - prevValue) >= 62){
            buffer[i] = analogRead(analogPin);
            Serial.printf("%d \n",buffer[i]);
            prevValue = currValue;
            i++;
        }
    }
    Serial.printf("Finished record\n");

    
    // Send the data to the server
    Serial.printf("Sending the data to the server\n");  
    int j = 0;
    while (j < 16000) {
        Udp.sendPacket((uint8_t*)(&buffer[j]), sizeof(int)*chunk_size, remoteIP, port);
        j += chunk_size;
    }   
    Serial.printf("Finished send data\n");
    delay(2000);

    // Receive the data from server
    Serial.printf("Waiting data from server\n");
    int res = Udp.receivePacket((uint8_t*)response, 2);
    Serial.printf("Received data from server\n");


    res = Udp.read(response, 2);
    Serial.println("Response received.");

    if (res == 0){
        Serial.println("440Hz has not been detected");
    }
    else{
        Serial.println("440Hz has been detected");
        digitalWrite(ledPin, HIGH);
        delay(1000);
        digitalWrite(ledPin, LOW);
        delay(1000);
    }
    
    Serial.printf("Finished. Press the mode button to restart\n");
}


