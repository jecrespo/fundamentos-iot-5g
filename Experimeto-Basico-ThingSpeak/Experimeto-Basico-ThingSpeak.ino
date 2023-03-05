/*
  WriteSingleField
  
  Description: Writes a value to a channel on ThingSpeak every 20 seconds.
  
  Hardware: Arduino MKR NB 1500
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires MKRNB library.
  - Reqires GSM access (SIM card or credentials).
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2020, The MathWorks, Inc.
*/

#include <MKRNB.h>
#include "secrets.h"
#include "ThingSpeak.h"  // always include thingspeak header file after other header files and custom macros

// PIN Number
const char PINNUMBER[] = SECRET_PIN;

NBClient client;
GPRS gprs;
NB nbAccess;  //gsmAccess;

unsigned long myChannelNumber = SECRET_CH_ID;
const char* myWriteAPIKey = SECRET_WRITE_APIKEY;

int number = 0;

void setup() {
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  Serial.println("Starting Arduino web client.");
  boolean connected = false;

  // wait 10 seconds for connection:
  delay(10000);

  while (!connected) {
    if ((nbAccess.begin(PINNUMBER) == NB_READY) && (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("connected");

  //Get IP.
  IPAddress LocalIP = gprs.getIPAddress();
  Serial.print("Server IP address= ");
  Serial.println(LocalIP);

  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  Serial.println("Envio Datos");

  // set the fields with the values
  ThingSpeak.setField(1, 100);
  ThingSpeak.setField(2, 200);
  ThingSpeak.setField(3, 300);

  ThingSpeak.setStatus("OK");

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  Serial.print("Resultado envío: ");
  Serial.println(x);
  if (x == 200) {
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  Serial.println("Siguiente Envio");
  delay(20000);  // Wait 20 seconds to update the channel again
}
