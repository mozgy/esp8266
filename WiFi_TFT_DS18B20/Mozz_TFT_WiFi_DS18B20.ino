/*
 * TFT + WiFi + DS18B20
 * 
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 * 
 */

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // nzmichaelh's version - https://github.com/nzmichaelh/Adafruit-ST7735-Library
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdio.h>

const char* ssid  = "MozzWiFi";
const char* pass  = "x";
const char* host  = "mozgy.t-com.hr";

/*
 * ESP8266-12        HY-1.8 SPI
 * GPIO5             Pin 06 (RESET)
 * GPIO2             Pin 07 (A0)
 * GPIO13 (HSPID)    Pin 08 (SDA)
 * GPIO14 (HSPICLK)  Pin 09 (SCK)
 * GPIO15 (HSPICS)   Pin 10 (CS)
 */
#define TFT_PIN_CS   15
#define TFT_PIN_DC   2
#define TFT_PIN_RST  5

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);

// (a 4.7K resistor is necessary)
#define ONE_WIRE_BUS 4

OneWire oneWire( ONE_WIRE_BUS );
DallasTemperature sensors( &oneWire );

char tmpstr[32];

extern "C" {
#include "user_interface.h"
}

void setup( void ) {

  // for debuging ..
  Serial.begin( 115200 );
  delay(100);
  // print out all system information
  Serial.println();
  Serial.print("Heap: "); Serial.println(system_get_free_heap_size());
  Serial.print("Boot Vers: "); Serial.println(system_get_boot_version());
  Serial.print("CPU: "); Serial.println(system_get_cpu_freq());
  Serial.println();

  tft.initR( INITR_BLACKTAB );
  tft.setTextWrap( false );
  tft.setTextColor( ST7735_WHITE );
  tft.setRotation( 3 );
  Serial.println("TFT Init...");

  sensors.begin();
  Serial.println("DS18B20 Init...");

  Serial.println("Setup done");

}

void loop() {

  float temp;
  char t[10];

  tft.fillScreen( ST7735_BLACK );

  sensors.requestTemperatures();
  Serial.print("Temperature for Device 1 is: ");
  temp = sensors.getTempCByIndex( 0 );
  Serial.print( temp );
  Serial.println();

  sprintf( tmpstr, "ChipID %d\0", ESP.getChipId() );
  tft.setCursor( 0, 24 );
  tft.print( tmpstr );

  dtostrf( temp, 3, 2, t);
  sprintf( tmpstr, "Temp %s\0", t );
  tft.setCursor( 0, 48 );
  tft.print( tmpstr );

  ElapsedStr(); // form str with hh:mm:ss
  tft.setCursor( 0, 72 );
  tft.print( tmpstr );

  delay(10000);

}

void ElapsedStr( void ) {

  unsigned long sec, minute, hour;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  sprintf( tmpstr, "Elapsed \0" );
  if ( hour == 0 ) {
    sprintf( tmpstr, "%s   \0", tmpstr );
  } else {
    sprintf( tmpstr, "%s%2d:\0", tmpstr, hour );
  }
  if ( minute == 0 ) {
    sprintf( tmpstr, "%s   \0", tmpstr );
  } else {
    sprintf( tmpstr, "%s%2d:\0", tmpstr, minute );
  }
  if ( ( sec % 60 ) < 10 ) {
    sprintf( tmpstr, "%s0%1d\0", tmpstr, ( sec % 60 ) );
  } else {
    sprintf( tmpstr, "%s%2d\0", tmpstr, ( sec % 60 ) );
  }
  // sprintf( tmpstr, "Elapsed %2d:%2d:%2d\0", ( sec / 3600 ), ( ( sec % 3600 ) / 60 ), ( sec % 60 ) );

}

