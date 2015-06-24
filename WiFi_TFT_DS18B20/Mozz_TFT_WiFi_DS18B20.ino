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

uint8_t xTemp, yTemp;

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
  tft.fillScreen( ST7735_BLACK );
  Serial.println("TFT Init...");

  sensors.begin();
  Serial.println("DS18B20 Init...");

  xTemp = 0;
  yTemp = 0;

  sprintf( tmpstr, "ChipID %d\0", ESP.getChipId() );
  tft.setCursor( 0, 16 );
  tft.print( tmpstr );

  Serial.println("Setup done");

}

void loop() {

  float temp;
  char t[10];

  // tft.fillScreen( ST7735_BLACK );

  sensors.requestTemperatures();
  Serial.print("Temperature for Device 1 is: ");
  temp = sensors.getTempCByIndex( 0 );
  Serial.print( temp );
  Serial.println();

  dtostrf( temp, 3, 2, t);
  sprintf( tmpstr, "Temp %s\0", t );
  tft.fillRect( 0, 32, 80, 8, ST7735_BLACK );
  tft.setCursor( 0, 32 );
  tft.print( tmpstr );

  ElapsedStr(); // form str with hh:mm:ss
  tft.fillRect( 0, 40, 96, 8, ST7735_BLACK );
  tft.setCursor( 0, 40 );
  tft.print( tmpstr );

  // 1°C resolution -10 - +50
  // yTemp = 117 - temp + 0;

  // 2°C resolution +5 - +35
  // yTemp = 117 - temp * 2 + 30;

  // 3°C resolution +x - +x
  yTemp = 117 - temp * 3 + 45;

  tft.drawPixel( xTemp, yTemp, ST7735_YELLOW );
  xTemp++;
  if ( xTemp > 150 ) {
    xTemp = 0;
    tft.fillRect( 0, 60, 151, 60, ST7735_BLACK );
  }

  delay(10000);

}

void ElapsedStr( void ) {

  unsigned long sec, minute, hour;

  sec = millis() / 1000;
  minute = ( sec % 3600 ) / 60;
  hour = sec / 3600;
  sprintf( tmpstr, "Elapsed " );
  if ( hour == 0 ) {
    sprintf( tmpstr, "%s    ", tmpstr );
  } else {
    sprintf( tmpstr, "%s%3d:", tmpstr, hour );
  }
  if ( minute >= 10 ) {
    sprintf( tmpstr, "%s%2d:", tmpstr, minute );
  } else {
    if ( hour != 0 ) {
      sprintf( tmpstr, "%s0%1d:", tmpstr, minute );
    } else {
      sprintf( tmpstr, "%s ", tmpstr );
      if ( minute == 0 ) {
        sprintf( tmpstr, "%s  ", tmpstr );
      } else {
        sprintf( tmpstr, "%s%1d:", tmpstr, minute );
      }
    }
  }
  if ( ( sec % 60 ) < 10 ) {
    sprintf( tmpstr, "%s0%1d", tmpstr, ( sec % 60 ) );
  } else {
    sprintf( tmpstr, "%s%2d", tmpstr, ( sec % 60 ) );
  }
  // sprintf( tmpstr, "Elapsed %2d:%2d:%2d", ( sec / 3600 ), ( ( sec % 3600 ) / 60 ), ( sec % 60 ) );

}
