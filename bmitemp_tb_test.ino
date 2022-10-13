#include "ThingsBoard.h"
#include <ESP8266WiFi.h>
#define WIFI_AP             "Jasmeetop"
#define WIFI_PASSWORD       "123456789"
#define TOKEN               "cZ3jBHNMYMs87thERMgk" //cZ3jBHNMYMs87thERMgk  cZ3jBHNMYMs87hjl cZ3jBHNMYMs87thERMgk cZ3jBHNMYMs87thERMgk
#define THINGSBOARD_SERVER  "demo.thingsboard.io"
#define SERIAL_DEBUG_BAUD   9600

#define uploadtime 510
uint32_t tblastupload = 0;

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

#include <NewPing.h>
#include "HX711.h"
#include <OneWire.h>
#include <DallasTemperature.h>

 
#define TRIGGER_PIN   15 //D8 of the nodeMCU
#define ECHO_PIN      13 //D7 of the nodeMCU
#define MAX_DISTANCE 300 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 500; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.

HX711 scale(14, 12); //dout 12->D5; SCK 12 ->D6
float weight,tmpr, height;
float calibration_factor = 22050.97;//22050.97 as on 27July

#define ONE_WIRE_BUS 2 //d4
// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);  
// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
 

void setup() 
{
  Serial.begin(SERIAL_DEBUG_BAUD);
  //sensors.begin();
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  InitWiFi();
  if (!tb.connected()) 
  {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN)) 
    {
      Serial.println("Failed to connect");
      return;
    }
  }
  
  pingTimer = millis(); // Start now.
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); 
}


void loop() 
{
  
      if (millis() >= pingTimer) 
      {   // pingSpeed milliseconds since last ping, do another ping.
        pingTimer += pingSpeed;      // Set the next ping time.
        echoCheck(); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
      }
      // Do other stuff here, really. Think of it as multi-tasking.
    
      scale.set_scale(calibration_factor); //Adjust to this calibration factor
      weight = scale.get_units(5); 
      Serial.print(" Weight: ");
      Serial.print(weight);
      Serial.print(" KG     ");
    
      sensors.requestTemperatures(); 
      //print the temperature in Celsius
      Serial.print(" Temperature: ");
      tmpr= (sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0;
      Serial.print(tmpr);
      
      Serial.println("F  ");
    
      if(Serial.available())
      {
          char temp = Serial.read();
          if(temp == 't')
          {
            scale.tare();  //Reset the scale to zero
          }
      }

  
    if (millis() - tblastupload > uploadtime)
      {
        tblastupload = millis();
        Serial.println("Sending data...");
        tb.sendTelemetryFloat("temprature", tmpr);
        tb.sendTelemetryFloat("Weight", weight);
        float actheight=203-height;
        tb.sendTelemetryFloat("Height", actheight);        
        float bmi= (weight/(actheight*actheight))*10000;
        tb.sendTelemetryFloat("bmi", bmi);
        tb.loop();
      }
}

void echoCheck() 
{ 

    Serial.print(" Height: ");
    height=sonar.ping_cm();
    Serial.print(height); // Send ping, get distance in cm and print result (0 = outside set distance range)
    Serial.print("cm");

}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() 
{
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) 
  {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
