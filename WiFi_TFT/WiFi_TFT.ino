/*
 * TFT + WiFi
 * 
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 * 
 */

#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // nzmichaelh's version - https://github.com/nzmichaelh/Adafruit-ST7735-Library
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

char tmpstr[32];

extern "C" {
#include "user_interface.h"
}

void setup( void ) {

  Serial.begin(115200);
  delay(2000); // wait for uart to settle and print Espressif blurb..
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

  Serial.println("Setup done");

}

void loop( void ) {

  tft.fillScreen( ST7735_BLACK );
  delay(500);
  tft.setTextColor( ST7735_WHITE );
  tft.setCursor( 0, 8 );
  tft.print( "Start-up ...." );
  sprintf( tmpstr, "ChipID %d\0", ESP.getChipId() );
  tft.setCursor( 0, 24 );
  tft.print( tmpstr );
  ElapsedStr(); // form str with hh:mm:ss
  tft.setCursor( 0, 40 );
  tft.print( tmpstr );
  delay(8000);

  Serial.println("Starting Scan...");
  Scan_Wifi_Networks();

  Serial.print( "Time elapsed so far: " );   
  Serial.print( millis() / 1000 );
  Serial.println( "sec." );
  delay(30000);

}

void Scan_Wifi_Networks( void ) {

  int c = 0;
  char myStr[22];

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  // Need to be in disconnected mode to Run network Scan!
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("Scaning Networks Complete..");
  Serial.print(n); Serial.println(" Networks have been Found");
  sprintf( myStr, "%d APs found\0", n );
  // tft.fillScreen( ST7735_BLACK );
  tft.setCursor( 0, 64 );
  tft.print( myStr ); // display the number of APs found
  delay(3000);
  
  if (n == 0) {

    tft.fillScreen( ST7735_BLACK );
    tft.setCursor( 0, 20 );
    tft.print( "No networks found" );
    Serial.println("No networks found");

  } else {

    tft.fillScreen( ST7735_BLACK );

    Serial.print(n); Serial.println(" networks found");
    for( int i=0; i<n; i++ ) {
      // Print SSID and RSSI for each network found
      Serial.print( i + 1 );
      Serial.print( ": " );
      Serial.print( WiFi.SSID(i) );
      Serial.print( " (" );
      Serial.print( WiFi.RSSI(i) ); // RSSI is negative number so '-' sign in front
      Serial.print( ")" );
      Serial.println( (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*" );
      delay(10);
      sprintf( myStr, "%s", WiFi.SSID(i) );
      myStr[15] = 0;
      sprintf( myStr, "%s %2d", myStr, WiFi.RSSI(i) );
      // sprintf( myStr, "%s %2d%1s", myStr, WiFi.RSSI(i), (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*" );
      tft.setCursor( 0, c );
      tft.print( myStr );
      // TFT is in color so lets print encryption sign in red
      if ( WiFi.encryptionType(i) != ENC_TYPE_NONE ) {
        tft.setTextColor( ST7735_RED );
        tft.print( "*" );
        tft.setTextColor( ST7735_WHITE );
      }
      // Serial.println( myStr );
      c = c + 16;
      if(c > 112) {
        delay(10000);
        tft.fillScreen( ST7735_BLACK );
        c = 0 ;
      }
      delay(500);

    }
  }

  Serial.println("");
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


/*
 * Arduino (Mini, Nano, Uno)   HY-1.8 SPI
 * D9                          Pin 07 (A0)
 * D10 (SS)                    Pin 10 (CS)
 * D11 (MOSI)                  Pin 08 (SDA)
 * D13 (SCK)                   Pin 09 (SCK)
 * D8                          Pin 06 (RESET)
 */
