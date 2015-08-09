/*
 * OpenWeather
 * 
 * The MIT License (MIT)
 *
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

// Serial printing ON/OFF
#include "Arduino.h"
#define DEBUG true
#define Serial if(DEBUG)Serial

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <ArduinoJson.h>

#include <stdio.h>
#include <OLED_SSD1306.h>
#include "font.h"

/*
 * SSD1306 - 0.96" 128x64
 * SH1106  - 1.3"  132x64
 */

//// HW SPI pins
// #define SDA_pin 13
// #define SCL_pin 14
// I2C pins
// #define SDA_pin 4
// #define SCL_pin 5
//// ESP-01 pins
#define SDA_pin 0
#define SCL_pin 2

#define OLED_ADDRESS 0x3C
// #define OLED_ADDRESS 0x78
#define SCROLL_WORKS
// #undef SCROLL_WORKS

// 0.96" OLED
OLED_SSD1306 oled( OLED_ADDRESS );

// 1.3" OLED
// #define SH1106_LC_OFFSET 2
// OLED_SSD1306 oled( OLED_ADDRESS, SH1106_LC_OFFSET );

const char* ssid  = "MozzWiFi";
const char* pass  = "x";
const char* host  = "mozgy.t-com.hr";

const char* urlHost = "api.openweathermap.org";
String urlCall = "";

unsigned long foo;
char tmpstr[20];

extern "C" {
#include "user_interface.h"
uint32_t readvdd33(void);
}

void setup() {

  Serial.begin(115200);

  // print out all system information
  Serial.println();
  Serial.print("Heap: "); Serial.println(system_get_free_heap_size());
  Serial.print("Boot Vers: "); Serial.println(system_get_boot_version());
  Serial.print("CPU: "); Serial.println(system_get_cpu_freq());
  Serial.print("ChipID: "); Serial.println(ESP.getChipId());
  Serial.print("SDK: "); Serial.println(system_get_sdk_version());
  Serial.println();

  // Wire.begin( SDA_pin, SCL_pin );
  // Wire.setClock( 400000 );

  // Serial.println("OLED Init...");
  // oled.Init();
  // oled.DisplayFlipON();

  initWiFi();

  Serial.println("Setup done");

  foo = 0;

}

void loop() {

  unsigned long sec;
  float Vdd;
  char tmpstr2[10];

#define REALDATA
//#define HTTPDEBUG
#undef HTTPDEBUG

  if( foo == 0 ) {
    WiFiClient client;
    setupURL();

#ifdef REALDATA
    if( client.connect( urlHost, 80 ) ) {
      client.println( urlCall );
    }
#endif
#ifdef HTTPDEBUG
    Serial.println(urlHost);
    Serial.println(urlCall);
#endif

    // Read all the lines of the reply from server and print them to Serial
    String httpResponse;
    // TODO - check for overflow
#ifdef REALDATA
    char httpJson[1200] = "";
#else
    char httpJson[] = "{\
      \"coord\":{\"lon\":16.38,\"lat\":45.47},\
      \"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"Sky is Clear\",\"icon\":\"01d\"}],\
      \"base\":\"stations\",\
      \"main\":{\"temp\":301.21,\"pressure\":1016,\"humidity\":38,\"temp_min\":297.15,\"temp_max\":306.15},\
      \"id\":3190813,\"name\":\"Sisak\",\"cod\":200\
      }";
#endif

#ifdef REALDATA
    while( client.available() ) {
      String line;
      line = client.readStringUntil('\r');
      httpResponse += line;
      if( line.charAt(1) == '{' ) {
        unsigned int lnlen = line.length();
        line.toCharArray( httpJson, lnlen );
 #ifdef HTTPDEBUG
        Serial.println(line);
        Serial.print("JSON LEN : ");
        Serial.println(lnlen);
        Serial.println(httpJson);
 #endif
      }
    }
 #ifdef HTTPDEBUG
    Serial.println();
    Serial.print(httpResponse);
    Serial.print("HTTP LEN : ");
    Serial.println(httpResponse.length());
 #endif
#endif


    StaticJsonBuffer<2047> jsonBuffer;
    JsonObject &HttpData = jsonBuffer.parseObject( httpJson );

    if ( !HttpData.success() ) {
      Serial.println("parsing failed");
    }

    JsonObject &CoordData = HttpData["coord"];
    JsonObject &WeatherData = HttpData["weather"];
    JsonObject &MainData = HttpData["main"];
    JsonObject &SysData = HttpData["sys"];

    const char* CityBase = HttpData["base"];
    const char* CityName = HttpData["name"];
    double CityLon = CoordData["lon"];
    double CityLat = CoordData["lat"];
    unsigned long CityId = HttpData["id"];
    unsigned long CitySunrise = SysData["sunrise"];
    unsigned long CitySunset = SysData["sunset"];
    double CityTempMin = MainData["temp_min"];
    double CityTempMax = MainData["temp_max"];

    Serial.print("Buffer - ");
    Serial.println(jsonBuffer.size());
    Serial.print("JSON - ");
    Serial.println(HttpData.measureLength());
    Serial.print("JSON - ");
    Serial.println(HttpData.size());    
    Serial.print("- ");
    Serial.println(CityBase);
    Serial.print("- ");
    Serial.println(CityName);
    Serial.print("- ");
    Serial.println(CityId);
    Serial.print("- ");
    Serial.println(CityLon);
    Serial.print("- ");
    Serial.println(CityLat);
    Serial.print("- ");
    Serial.println(CitySunrise);
    Serial.print("- ");
    Serial.println(CitySunset);
    Serial.print("- ");
    Serial.println(CityTempMin);
    Serial.print("- ");
    Serial.println(CityTempMax);

    Serial.println();
    Serial.print( "Time elapsed so far: " );
    Serial.print( millis() / 1000 );
    Serial.println( "sec." );
    delay(30);
    client.stop();
  }
  delay(10);
  foo++;
  // 10 delay + 100000 -> 1000sec
  if( foo > 100000 ) {
    foo = 0;
  }

}

void setupURL(void) {

// api.openweathermap.org/data/2.5/forecast?id={city ID}

  urlCall = "GET ";
  // urlCall += "/data/2.5/forecast";
  urlCall += "/data/2.5/weather";
  urlCall += "?id=3190813"; // Sisak,HR
  urlCall += "&APPID=yy"; // Mozz's API key

  urlCall += " HTTP/1.1\r\n";
  urlCall += "Host: ";
  urlCall += host;
  urlCall += "\r\n";
  urlCall += "Connection: close\r\n";
  urlCall += "Accept: */*\r\n";
  urlCall += "User-Agent: Mozilla/4.0 (compatible; esp8266 Arduino IDE; Windows NT 5.1)\r\n";
  urlCall += "\r\n";


}

void initWiFi(void) {

  int WiFiCounter = 0;

  Serial.print( "Connecting to " );
  Serial.println( ssid );
  WiFi.mode( WIFI_STA );
  WiFi.begin( ssid, pass );

  while ( WiFi.status() != WL_CONNECTED && WiFiCounter < 30 ) {
    delay(1000);
    WiFiCounter++;
    Serial.print( "." );
  }
  Serial.println( "" );
  
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

/*
  oled.ClearDisplay();
//  oled.SendStrXY( "Start-up ....  ", 0, 1 );
  sprintf( tmpstr, "ChipID %9d", ESP.getChipId() );
  oled.SendStrXY( tmpstr, 1, 0 );
  // sprintf( tmpstr, "FlashID %06X", ESP.getFlashChipId() );
  sprintf( tmpstr, "SDK %12s", ESP.getSdkVersion() );
  oled.SendStrXY( tmpstr, 2, 0 );
  sprintf( tmpstr, "Flash %8dkB", ESP.getFlashChipSize() / 1024 );
  oled.SendStrXY( tmpstr, 3, 0 );
//  sprintf( tmpstr, "Vcc %d", ESP.getVcc() );
//  Vdd = (float)ESP.getVcc() / 1000.0;
//  sprintf( tmpstr, "Vcc %12d", readvdd33() );
  Vdd = (float)readvdd33() / 1000.0;
  dtostrf( Vdd, 6, 3, tmpstr2 );
  sprintf( tmpstr, "Vcc %12s", tmpstr2 );
  oled.SendStrXY( tmpstr, 5, 0 );
  ElapsedStr( tmpstr ); // form str with hh:mm:ss
  oled.SendStrXY( tmpstr, 6, 0 );
  delay(10000);
 */
