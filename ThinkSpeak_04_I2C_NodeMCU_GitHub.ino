/* soft for NodeMCU v3
I2C  Master
I2C communication between two Arduinos.
load data from arduino Nano /solar controller/ and send data to Thingspeak via NodeMCU 
*/ 
//LED
#define LEDPIN D4 

//library for NodeMCU
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

//Thingspeak stuff
//#include "ThingSpeak.h"
#include <ThingSpeak.h>
unsigned long myChannelNumber1 = *****;               //replace ***** with your ChannelNumber
const char * myWriteAPIKey1 = "****************";     //replace **************** with your WriteAPIKey
const char* server = "api.thingspeak.com";

//WiFi
WiFiClient client;

//Upload the sketch to ESP OTA (Over The Air)
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
const char* host = "**********";     //replace with your host name

// I2C defs
#include <Wire.h> //https://www.electronicwings.com/nodemcu/nodemcu-i2c-with-arduino-ide
//#include <I2C.h>  //http://dsscircuits.com/index.php/articles/66-arduino-i2c-master-library
// https://github.com/rambo/I2C
#define PACKET_SIZE 12 // I2C packet size
#define addrSlaveI2C 21    // ID of I2C slave

typedef struct sensorData_t {
float tempK;                         // float type have 4 bytes!
float tempB;
float tempT;
};

typedef union I2C_Packet_t {
 sensorData_t sensor;
 byte I2CPacket[sizeof(sensorData_t)];
};

I2C_Packet_t leakinfo;
  
// Function prototype
int getData();

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int resetCounter = 0;
int cc = 0;

void setup() {
  
//Serial 
  Serial.begin(9600);
  
//I2C  
  Wire.begin(D1, D2); /* join i2c bus with SDA=D1 and SCL=D2 of NodeMCU */
  //I2c.begin(); 
  //I2c.timeOut(30000);
  
//WiFi
  Serial.println("Connecting ...");
  WiFi.mode(WIFI_STA);
  
  wifiMulti.addAP("WIFI0001", "*************");      //replace WIFI0001 with your WiFi(1) name and password ***********    
  wifiMulti.addAP("WIFI0002", "*************");      ////replace WIFI0002 with your WiFi(2) name and password ***********  
  
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    Serial.print('.');
    digitalWrite(LEDPIN,0);
    //Serial.println("Connection Failed! Rebooting...");
    delay(2000);
    digitalWrite(LEDPIN,1);
    //ESP.restart();
    delay(1000);
  }
  Serial.println('\n');
  Serial.print("SSID:\t\t");
  Serial.println(WiFi.SSID());              // SSID 
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // IP 

  
  //Thingspeak
  ThingSpeak.begin(client);
  
  //update sketch to NodeMCU over air
  
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname(host);

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"***");   //replace *** with your OTA password
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
} //END SETUP

//++++ LOOP ++++

void loop() {
  
  //upload sketch to NodeMCU over air
  ArduinoOTA.handle();
  
  int i2cStatus = getData();
  
     float s1 = leakinfo.sensor.tempK;
     float s2 = leakinfo.sensor.tempB;
     float s3 = leakinfo.sensor.tempT;
     
 Serial.println();
 Serial.print(s1);
 Serial.print("\t");
 Serial.print(s2);
 Serial.print("\t");
 Serial.println(s3);   
   
  // Print Update Response to Serial Monitor
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }
  
  // Update ThingSpeak
  //upload data to server
    if (client.connect(server, 80)) {
    ThingSpeak.setField(1, s1);
    ThingSpeak.setField(2, s2);
    ThingSpeak.setField(3, s3);
    
    ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
    }
   
 delay(53994);
} // END LOOP

//***********************************************

// I2C Request data from slave
int getData()
{
  byte byteArray[PACKET_SIZE];
  Wire.requestFrom(addrSlaveI2C, PACKET_SIZE); /* request & read data of size PACKET_SIZE from slave */
  while(Wire.available()){
   for (int k=0; k < PACKET_SIZE; k++)
    { 
      leakinfo.I2CPacket[k] = Wire.read();
    }
    //char c = Wire.read();
  //Serial.print(c);
 }
// Serial.println();
 delay(300);
}
 // end getData
