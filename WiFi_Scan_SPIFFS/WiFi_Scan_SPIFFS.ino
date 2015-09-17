/*
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
// #define Serial if(DEBUG)Serial
// #define DEBUG_OUTPUT Serial

ADC_MODE(ADC_VCC);

#include "ESP8266WiFi.h"
#include "FS.h"

#include <ArduinoJson.h>

char tmpstr[40];

uint8_t net_count;
uint8_t net_max;

File fh_netdata;
String line;

StaticJsonBuffer<2048> wifiDataJson;
JsonArray &WiFiNetworksArray  = wifiDataJson.createArray();

char netsJson[2048] = "";

extern "C" {
#include "user_interface.h"
}

void setup() {

  Serial.begin(115200);
  delay(10);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.print(F("Heap: ")); Serial.println(system_get_free_heap_size());
  Serial.print(F("Boot Vers: ")); Serial.println(system_get_boot_version());
  Serial.print(F("CPU: ")); Serial.println(system_get_cpu_freq());
  Serial.print(F("SDK: ")); Serial.println(system_get_sdk_version());
  Serial.print(F("Chip ID: ")); Serial.println(system_get_chip_id());
  Serial.print(F("Flash ID: ")); Serial.println(spi_flash_get_id());
  Serial.print(F("Vcc: ")); Serial.println(ESP.getVcc());
  Serial.println();

  bool result = SPIFFS.begin();
  if( !result ) {
    Serial.println(F("SPIFFS open failed!"));
  }

/*
  // comment format section after DEBUGING done
  result = SPIFFS.format();
  if( !result ) {
    Serial.println("SPIFFS format failed!");
  }
 */
//  SPIFFS.remove( "/netcnt.txt" );
  SPIFFS.remove( "/netdata.txt" );

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  net_count = 0;
  net_max = 0;

  Serial.println(F("Setup done"));
}

void loop() {

  do_wifiscan();

//  WiFiNetworksArray.prettyPrintTo( Serial );
//  Serial.println("");

  ElapsedStr( tmpstr );
  Serial.println( tmpstr );

  // Wait a bit before scanning again
  delay(120000);

}

void do_wifiscan( void ) {

  Serial.println(F("scan start"));

  // WiFi.scanNetworks will return the number of networks found
  net_count = WiFi.scanNetworks();
  Serial.println("scan done");
  if (net_count == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(net_count);
    Serial.println(" networks found");
    parse_networks( net_count );
  }
  Serial.println("");

}

bool find_ssid( const char *ssid ) {
  String ssidcopy = String( ssid );

  for ( JsonArray::iterator it = WiFiNetworksArray.begin(); it != WiFiNetworksArray.end(); ++it ) {
    const char *str = (*it)["ssid"];
    if ( ssidcopy == String( str ) ) {
      return true;
    }
  }
  return false;

}

bool read_netdata( ) {
  fh_netdata = SPIFFS.open("/netdata.txt", "r");

  if ( !fh_netdata ) {
    Serial.println(F("Data file doesn't exist yet. Creating it"));

    fh_netdata = SPIFFS.open("/netdata.txt", "w");
    if ( !fh_netdata ) {
      Serial.println(F("Data file creation failed"));
    }

    fh_netdata.println( "{\"count\":0,\"max\":0}" );
//    fh_netdata.println( "{\"count\":0,\"max\":0,\"networks\":[{\"ssid\":\"ssid\",\"rssi\":0,\"enc\":\"*\"}]}" );

  } else {

    line = fh_netdata.readStringUntil('\n');
    Serial.print(F("Line (read) "));Serial.println( line );

    // TODO: check for overflow
    line.toCharArray( netsJson, line.length() + 1 );

    StaticJsonBuffer<2048> jsonBuffer;
    JsonObject &WiFiData = jsonBuffer.parseObject( netsJson );

    if ( !WiFiData.success() ) {
      Serial.println(F("parsing failed"));
      return false;
    }
    int n = WiFiData["count"];
    if ( n == 0 ) {
      Serial.println(F("Last count read 0"));
      return true;
    }

    net_max = WiFiData["max"];
    Serial.print(F("Last max read "));Serial.println( net_max );

    JsonArray &tmp = WiFiData["networks"];
    for ( int i = 0; i < n; i++ ) {
      Serial.print("-- ");
      tmp[i]["ssid"].prettyPrintTo( Serial );
      Serial.print(" ");
      const char *ssid = tmp[i]["ssid"].asString();
      if ( !find_ssid( ssid ) ) {
        Serial.print(" - not found before");
        JsonObject &tmpAdd = wifiDataJson.createObject();
        tmpAdd["ssid"] = tmp[i]["ssid"].asString();
        tmpAdd["rssi"] = tmp[i]["rssi"].asString();
        tmpAdd["enc"] = tmp[i]["enc"].asString();
        WiFiNetworksArray.add( tmpAdd );
        Serial.print(" - ");
        tmp[i].printTo( Serial );
      }
      Serial.println("");
    }

  }
  fh_netdata.close();
  return true;
}

void write_netdata( int net_num ) {
  fh_netdata = SPIFFS.open("/netdata.txt", "w");

  if ( !fh_netdata ) {
    Serial.println(F("Something went wrong with data file !"));
  } else {

    StaticJsonBuffer<2048> jsonBuffer;
    JsonObject &WiFiData = jsonBuffer.createObject();
    WiFiData["count"] = net_num;
    WiFiData["max"] = net_max;

    JsonArray &WiFiDataArray  = WiFiData.createNestedArray("networks");

    for ( int i = 0; i < net_num; ++i ) {

      JsonObject &tmp = jsonBuffer.createObject();

      tmp["ssid"] = WiFi.SSID(i);
      tmp["rssi"] = WiFi.RSSI(i);
      tmp["enc"] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");

      WiFiDataArray.add( tmp );
    }

    WiFiData.printTo( fh_netdata );
    fh_netdata.println( "\n" );

// Serial DEBUG info
//    WiFiNetworksArray.printTo( Serial );
//    Serial.println("");
    WiFiData.printTo( Serial );
    Serial.println("");
//    Serial.println("");
//    WiFiData.prettyPrintTo( Serial );

  }
  fh_netdata.close();
}

void parse_networks( int net_num ) {

  read_netdata( );

/*
  for (int i = 0; i < net_num; ++i)
  {
    // Print SSID and RSSI for each network found
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(")");
    Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
    delay(10);
  }
 */

  if ( net_num > net_max ) {
    net_max = net_num;
  }

  write_netdata( net_num );
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
