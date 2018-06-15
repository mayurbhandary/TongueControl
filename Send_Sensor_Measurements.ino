/*
  Wireless Servo Control, with ESP as Access Point

  Usage: 
    Connect phone or laptop to "ESP_XXXX" wireless network, where XXXX is the ID of the robot
    Go to 192.168.4.1. 
    A webpage with four buttons should appear. Click them to move the robot.

  Installation: 
    In Arduino, go to Tools > ESP8266 Sketch Data Upload to upload the files from ./data to the ESP
    Then, in Arduino, compile and upload sketch to the ESP

  Requirements:
    Arduino support for ESP8266 board
      In Arduino, add URL to Files > Preferences > Additional Board Managers URL.
      See https://learn.sparkfun.com/tutorials/esp8266-thing-hookup-guide/installing-the-esp8266-arduino-addon

    Websockets library
      To install, Sketch > Include Library > Manage Libraries... > Websockets > Install
      https://github.com/Links2004/arduinoWebSockets
    
    ESP8266FS tool
      To install, create "tools" folder in Arduino, download, and unzip. See 
      https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md#uploading-files-to-file-system

  Hardware: 
  * NodeMCU Amica DevKit Board (ESP8266 chip)
  * Motorshield for NodeMCU 
  * 2 continuous rotation servos plugged into motorshield pins D1, D2
  * Ultra-thin power bank 
  * Paper chassis

*/

#include <Arduino.h>


#include <Hash.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>


#include <Wire.h>

#include "IO_LPS22HB.h"

#include "debug.h"
#include "file.h"
#include "server.h"

IO_LPS22HB lps22hb;

bool connection;
int connectioncount=0;
uint8_t connectionid=0;
// WiFi AP parameters
char ap_ssid[13];
char* ap_password = "";

// WiFi STA parameters
char* sta_ssid = 
  "Butthole";
char* sta_password = 
  "...";

char* mDNS_name = "paperbot";

String html;
String css;

void setup() {
    setupPins();
    WiFi.hostname("Butthole");
    //setupSTA(sta_ssid, sta_password);
    setupAP(ap_ssid, ap_password);
  

    setupHTTP();
    setupWS(webSocketEvent);
    //setupMDNS(mDNS_name);

    //Serial.println("IoThings LPS22HB Arduino Test");
   
    lps22hb.begin(93);
  
    byte who_am_i = lps22hb.whoAmI();
    if (who_am_i != LPS22HB_WHO_AM_I_VALUE) {
      DEBUG("Error while retrieving WHO_AM_I byte...");
    }


}

void loop() {
    wsLoop();
    httpLoop();
    sendSensors(connectionid);
    delay(35);
}

void webSocketEvent(uint8_t id, WStype_t type, uint8_t * payload, size_t length) {
  
    switch(type) {
        case WStype_DISCONNECTED:
            DEBUG("Web socket disconnected, id = ", id);
            break;
        case WStype_CONNECTED: 
        {
            if(connectioncount<1){//only the first connection will have data sent to it. This means the python script must be run first.
              connectionid=id;
              connectioncount++;
            }
            
            connection=true;
            // IPAddress ip = webSocket.remoteIP(id);
            // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", id, ip[0], ip[1], ip[2], ip[3], payload);
            DEBUG("Web socket connected, id = ", id);

            // send message to client
            //wsSend(id, "Connected to ");
            //wsSend(id, ap_ssid);
            break;
        }
    }
}

void sendSensors(uint8_t id){
  char buff[10];
  int p=(int)(lps22hb.readPressure()*10);
  itoa(p,buff,10);
  wsSend(id,buff);
}

void setupPins() {
    // setup Serial, LEDs and Motors
    Serial.begin(115200);
    DEBUG("Started serial.");
    pinMode(LED_PIN, OUTPUT);    //Pin D0 is LED
}


