/*
 * Copyright (c) 2015. Mario Mikočević <mozgy>
 *
 * MIT Licence
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
JsonArray& WiFiNetworksArray  = wifiDataJson.createArray();

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
  Serial.print(F("Flash Size: ")); Serial.println(ESP.getFlashChipRealSize());
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
  delay(300000);

}

void do_wifiscan( void ) {

  Serial.println(F("scan start"));

  // WiFi.scanNetworks will return the number of networks found
  net_count = WiFi.scanNetworks();
  Serial.println("scan done");
  if ( net_count == 0 ) {
    Serial.println("no networks found");
  } else {
    Serial.print(net_count);
    Serial.println(" networks found");
    parse_networks( net_count );
    if ( net_count > net_max ) {
      net_max = net_count;
    }
  }
  Serial.println("");

}

void parse_networks( int net_num ) {

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

  if ( !update_netdata( net_num ) ) {
    Serial.println(F("Something went WRONG!"));
  }

}

bool find_ssid( String ssid ) {
/*

Serial.print("Find loop - ");
Serial.print( ssid );
Serial.print(", ");
  for ( JsonArray::iterator it = WiFiNetworksArray.begin(); it != WiFiNetworksArray.end(); ++it ) {
    String str = (*it)["ssid"];
Serial.print(", ");
Serial.print( str );
Serial.print(", ");
    if ( ssid == str ) {
Serial.println();
      return true;
    }
  }
Serial.println();
  return false;

 */
}

bool update_netdata( int net_num ) {

  StaticJsonBuffer<1024> jsonBuffer;

// create new data from network list
  JsonObject& WiFiData = jsonBuffer.createObject();
  WiFiData["count"] = net_num;
  WiFiData["max"] = net_max;

  JsonArray& WiFiDataArray  = WiFiData.createNestedArray("networks");

  for ( int i = 0; i < net_num; ++i ) {

    JsonObject& tmp = jsonBuffer.createObject();

    tmp["ssid"] = WiFi.SSID(i);
    tmp["rssi"] = WiFi.RSSI(i);
    tmp["enc"] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");

    WiFiDataArray.add( tmp );
  }

  Serial.println("Scanned wifi data ->");
//  WiFiData.printTo( Serial );
  WiFiData.prettyPrintTo( Serial );
  Serial.println("");

  fh_netdata = SPIFFS.open("/netdata.txt", "r");

  if ( !fh_netdata ) {
// no last data
    Serial.println(F("Data file doesn't exist yet."));

    fh_netdata = SPIFFS.open("/netdata.txt", "w");
    if ( !fh_netdata ) {
      Serial.println(F("Data file creation failed"));
      return false;
    }

// WiFiData is wifi scan snapshot
    WiFiData.printTo( fh_netdata );
    fh_netdata.println( "\n" );

    JsonArray& tmp = WiFiData["networks"];
    for ( int i = 0; i < net_num; i++ ) {
      String strTmp;
      tmp[i].printTo( strTmp );
      JsonObject& tmp2 = wifiDataJson.parseObject( strTmp );
      WiFiNetworksArray.add( tmp2 );
    }

  } else {
// read last WiFi data from file
    Serial.println(F("Reading saved wifi data .."));

    line = fh_netdata.readStringUntil('\n');
    Serial.print(F("Line (read) "));Serial.println( line );

    JsonObject& WiFiDataFile = jsonBuffer.parseObject( line );

    if ( !WiFiDataFile.success() ) {
      Serial.println(F("parsing failed"));
      // maybe to remove data file now?
      // SPIFFS.remove( "/netdata.txt" );
      return false;
    }
    int n = WiFiDataFile["count"];
//    Serial.print(F("Last count read "));Serial.println( n );
    net_max = WiFiDataFile["max"];
//    Serial.print(F("Last max read "));Serial.println( net_max );

    WiFiDataFile.prettyPrintTo( Serial );
    Serial.println("");

// find duplicates
    JsonArray& tmp = WiFiDataFile["networks"];

    for ( int i = 0; i < net_num; i++ ) {
      for ( int j = 0; j < n; j++ ) {
        String ssid1 = tmp[j]["ssid"];
        String ssid2 = WiFiDataArray[i]["ssid"];
        if ( ssid1 != ssid2 || ssid1 == ssid2 ) {
          String strTmp;
          WiFiDataArray[i].printTo( strTmp );
          JsonObject& tmp2 = wifiDataJson.parseObject( strTmp );
          WiFiNetworksArray.add( tmp2 );
        }
      }
    }
    Serial.println("");
  }

  fh_netdata.close();

  Serial.println("Computed wifi data ->");
  WiFiNetworksArray.prettyPrintTo( Serial );

  return true;

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
    JsonObject& WiFiData = jsonBuffer.parseObject( netsJson );

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

    JsonArray& tmp = WiFiData["networks"];
    for ( int i = 0; i < n; i++ ) {
      Serial.print("-- ");
      tmp[i]["ssid"].prettyPrintTo( Serial );
      Serial.print(" ");
      const char *ssid = tmp[i]["ssid"].asString();
      if ( !find_ssid( ssid ) ) {
        Serial.print(" - not found before");
        JsonObject& tmpAdd = wifiDataJson.createObject();
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
