#include "ThingsBoard.h"
#include <ESP8266WiFi.h>
#define WIFI_AP             "Jasmeetop"
#define WIFI_PASSWORD       "123456789"

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
#define TOKEN               "CNP3pr03HjjgacruD1Vu"
#define THINGSBOARD_SERVER  "demo.thingsboard.io"

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD   115200

#include <Adafruit_NeoPixel.h>
//Hacked from the original Adafruit library demo
 
#define PIN 12   //my control pin D6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);
 

#define uploadtime 510
uint32_t tblastupload = 0;
uint32_t tsLastglow = 0;

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

 
#define REPORTING_PERIOD_MS 500

 
// Connections : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0, buzzer-d5, 
PulseOximeter pox;
 
float BPM, SpO2;
uint32_t tsLastReport = 0;
 
void onBeatDetected()
{
    Serial.println("Beat Detected!");
    tone(14, 1000, 100);
    for(int i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, strip.Color(0, 100, 0));
    }
    strip.show();
    tsLastglow = millis();
}

void setup() {
  // initialize serial for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
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
  pox.begin();
  pox.setOnBeatDetectedCallback(onBeatDetected);
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
}


void loop() 
{
  if (millis() - tsLastglow > 300)
    {
      for(int i=0; i<strip.numPixels(); i++) 
      {
        strip.setPixelColor(i, strip.Color(50, 50, 50));
      }
      strip.show();
    }
    pox.update();
 
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
      {
          Serial.print("Heart rate:");
          Serial.print(BPM);
          Serial.print(" bpm / SpO2:");
          Serial.print(SpO2);
          Serial.println(" %");
          tsLastReport = millis();
      }
  
    if (millis() - tblastupload > uploadtime)
      {
        tblastupload = millis();
        Serial.println("Sending data...");
        tb.sendTelemetryInt("BPM", BPM);
        tb.sendTelemetryFloat("SpO2", SpO2);
      
        tb.loop();
      }
}

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    WiFi.begin(WIFI_AP, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Connected to AP");
  }
}
