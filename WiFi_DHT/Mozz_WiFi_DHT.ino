/*
 * 
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 * 
 * MIT Licence
 *
 */

// Serial printing ON/OFF
#include "Arduino.h"
#define DEBUG true
#define Serial if(DEBUG)Serial
#define DEBUG_OUTPUT Serial

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

ADC_MODE(ADC_VCC);

#include <Ticker.h>
Ticker tickerDHTScan;
#define WAITTIME 600
boolean tickerFired;

#include "DHT.h"
#define DHTPIN 2
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = "MozzWiFi";
const char* pass     = "xx";
const char* host     = "xx";
const char* urlHost  = "192.168.4.1";

char tmpstr[40];

void setup() {

  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println( ESP.getFreeHeap() );
  Serial.print( F("Boot Vers: ") ); Serial.println( ESP.getBootVersion() );
  Serial.print( F("CPU: ") ); Serial.print( ESP.getCpuFreqMHz() ); Serial.println("MHz");
  Serial.print( F("SDK: ") ); Serial.println( ESP.getSdkVersion() );
  Serial.print( F("Chip ID: ") ); Serial.println( ESP.getChipId() );
  Serial.print( F("Flash ID: ") ); Serial.println( ESP.getFlashChipId() );
  Serial.print( F("Flash Size: ") ); Serial.println( ESP.getFlashChipRealSize() );
  Serial.print( F("Vcc: ") ); Serial.println( ESP.getVcc() );
  Serial.println();

  dht.begin();

//  initWiFi();

  tickerDHTScan.attach( WAITTIME, flagDHTScan );
  tickerFired = true;

}

void loop() {

  if( tickerFired ) {
    tickerFired = false;
    while( ! doSomethingWithDHT() );

    ElapsedStr( tmpstr );
    Serial.println( tmpstr );
    Serial.print( F("Heap: ") ); Serial.println( ESP.getFreeHeap() );
  }

}

void flagDHTScan( void ) {
  tickerFired = true;
}

boolean doSomethingWithDHT( void ) {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
//  // Read temperature as Fahrenheit
//  float f = dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
//  if (isnan(h) || isnan(t) || isnan(f)) {
  if ( isnan(h) || isnan(t) ) {
    // Serial.println( "Failed to read from DHT sensor!" );
    Serial.print(".");
    delay(10);
    return false;
  }

//  // Compute heat index
//  // Must send in temp in Fahrenheit!
//  float hi = dht.computeHeatIndex(f, h);

  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C ");
//  Serial.print(f);
//  Serial.print(" *F\t");
//  Serial.print("Heat index: ");
//  Serial.print(hi);
//  Serial.println(" *F");
  Serial.println();

//  sendSensorData( "Room", h, t );
  sendSensorData( "Attic", h, t );

  return true;
}

void sendSensorData( const char *id, float hum, float temp ) {

  WiFiClient client;
  int mV = ESP.getVcc();

  initWiFi();

  client.stop();
  uint8_t i = 0;
  while( !client.connect( urlHost, 80 ) && i++ < 30 ) {
    Serial.println( "Cannot connect!" );
    // connect can BUGout, no idea yet why
    delay(500);
  }

  String WiFiString = "GET /dht/insertdata.php?hash=dht&s=";
    WiFiString += id;
    WiFiString += "&h=";
    WiFiString += hum;
    WiFiString += "&t=";
    WiFiString += temp;
    WiFiString += "&vcc=";
    WiFiString += mV;
    WiFiString += " HTTP/1.1\r\n";
    WiFiString += "Host: ";
    WiFiString += host;
    WiFiString += "\r\n";
    WiFiString += "Connection: close\r\n";
    WiFiString += "\r\n";

  Serial.println( WiFiString );
  client.println( WiFiString );

  client.stop();

}

void initWiFi(void) {

  Serial.print( "Connecting to " );
  Serial.println( ssid );
  WiFi.mode( WIFI_STA );
  WiFi.begin( );
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ){
     WiFi.begin( ssid, pass );
     Serial.println("Retrying connection...");
  }
  Serial.println( "WiFi connected" );
  Serial.print( "IP address: " );
  Serial.println( WiFi.localIP() );

}

void ElapsedStr( char *str ) {

  unsigned long sec, minute, hour;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  sprintf( str, "Elapsed " );
  if ( hour == 0 ) {
    sprintf( str, "%s   ", str );
  } else {
    sprintf( str, "%s%2d:", str, hour );
  }
  if ( minute >= 10 ) {
    sprintf( str, "%s%2d:", str, minute );
  } else {
    if ( hour != 0 ) {
      sprintf( str, "%s0%1d:", str, minute );
    } else {
      sprintf( str, "%s ", str );
      if ( minute == 0 ) {
        sprintf( str, "%s  ", str );
      } else {
        sprintf( str, "%s%1d:", str, minute );
      }
    }
  }
  if ( ( sec % 60 ) < 10 ) {
    sprintf( str, "%s0%1d", str, ( sec % 60 ) );
  } else {
    sprintf( str, "%s%2d", str, ( sec % 60 ) );
  }

}
