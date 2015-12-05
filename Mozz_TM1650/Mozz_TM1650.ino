/*
 * 
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 * 
 * MIT Licence
 *
 */

// Serial printing ON/OFF
#include <Wire.h>
#include <Arduino.h>
#define DEBUG true
#define Serial if(DEBUG)Serial
#define DEBUG_OUTPUT Serial

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <time.h>

ADC_MODE(ADC_VCC);

#include <Ticker.h>
Ticker tickerShowTime;
#define WAITTIME 1
boolean tickerFired;

// I2C pins
#define SDA_pin 4
#define SCL_pin 5

#include <TM1650.h>

TM1650 Disp4Seg;

const char* ssid     = "MozzWiFi";
const char* pass     = "xx";
const char* host     = "yy";

int timezone = 1;
int dst = -1; // bugged, doesnt count

uint16_t i,j,k;
uint8_t b;

void flagShowTime( void ) {
  tickerFired = true;
}

void initWiFi( void ) {

  Serial.print( "Connecting to " );
  Serial.println( ssid );
  WiFi.mode( WIFI_STA );
  WiFi.begin( );
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ){
     WiFi.begin( ssid, pass );
     Serial.println("Retrying connection...");
//    Serial.println("Connection Failed! Rebooting...");
//    delay(5000);
//    ESP.restart();
  }
  Serial.println( "WiFi connected" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.localIP() );

}

void SetupArduinoOTA( void ) {

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("disp7-esp");

  // ArduinoOTA.setPassword((const char *)"xx");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
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

}

void setup() {

  Serial.begin(115200);
  Serial.println( "Booting" );
  Serial.println();
  Serial.printf( "Sketch size: %u\n", ESP.getSketchSize() );
  Serial.printf( "Free size: %u\n", ESP.getFreeSketchSpace() );
  Serial.printf( "Heap: %u\n", ESP.getFreeHeap() );
  Serial.printf( "Boot Vers: %u\n", ESP.getBootVersion() );
  Serial.printf( "CPU: %uMHz\n", ESP.getCpuFreqMHz() );
  Serial.printf( "SDK: %s\n", ESP.getSdkVersion() );
  Serial.printf( "Chip ID: %u\n", ESP.getChipId() );
  Serial.printf( "Flash ID: %u\n", ESP.getFlashChipId() );
  Serial.printf( "Flash Size: %u\n", ESP.getFlashChipRealSize() );
  Serial.printf( "Vcc: %u\n", ESP.getVcc() );
  Serial.println();

  initWiFi();
  SetupArduinoOTA();
  Serial.println( "OTA Setup Done!" );

  Wire.begin( SDA_pin, SCL_pin );
  Wire.setClock( 400000 );

  Disp4Seg.Init();
  Serial.println( "TM1650 Setup Done!" );

  configTime( timezone * 3600, dst * 3600, "pool.ntp.org", "time.nist.gov", "tik.t-com.hr" );
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  i = 0;
  j = 0;
  k = 0;
  b = 1;

  tickerShowTime.attach( WAITTIME, flagShowTime );
  tickerFired = true;

}

void displayFun( uint16_t i ) {

  if( ( i % 100 ) == 0 ) {
//    Disp4Seg.SendControl( b );
//    Serial.printf( "Command bit - %d\n", b );
    Serial.printf( "Counter - %d\n", i );
    b = ( b << 1 ) & 0xff;
    if ( b == 0 ) {
      b = 1;
    }
  }

  ( i & 0x04 ) ? Disp4Seg.ColonON() : Disp4Seg.ColonOFF();

  Disp4Seg.SetBrightness( 4 );
  Disp4Seg.WriteNum( i );

//    j = i & 0x03;
//    k = ( i >> 3 ) & 0x07;
//    Serial.printf( "Brgithness %d at %d\n", k, j );
//    Disp4Seg.SetBrightness( k );

}

void loop() {

  ArduinoOTA.handle();

  if( tickerFired ) {
    tickerFired = false;
    time_t now = time(nullptr);
    struct tm* t = localtime( &now );
    int sec = t->tm_sec;
    int minute = t->tm_min;
    int hour = t->tm_hour;
//    Serial.println(ctime(&now));
//    Serial.printf( "%2d:%2d\n", minute, sec );

    ( sec & 0x01 ) ? Disp4Seg.ColonON() : Disp4Seg.ColonOFF();
    Disp4Seg.SetBrightness( 4 );
    Disp4Seg.WriteNum( hour * 100 + minute );
  }

}
