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

File fh_netdata;
String line;

extern "C" {
#include "user_interface.h"
}

void setup() {

  Serial.begin(115200);
  delay(10);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();

  bool result = SPIFFS.begin();
  if( !result ) {
    Serial.println( F("SPIFFS open failed!") );
  }

/*
  // comment format section after DEBUGING done
  result = SPIFFS.format();
  if( !result ) {
    Serial.println("SPIFFS format failed!");
  }
 */
//  SPIFFS.remove( "/netdata.txt" );

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println( F("Setup done") );
}

void loop() {

  do_wifiscan();

  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());

  ElapsedStr( tmpstr );
  Serial.println( tmpstr );

  // Wait some time before scanning again
  delay(900000);

}

void do_wifiscan( void ) {
  int netCount;

  Serial.println( F("scan start") );

  // WiFi.scanNetworks will return the number of networks found
  netCount = WiFi.scanNetworks();
//  Serial.println("scan done"); // no need if Serial.setDebugOutput(true)
  if ( netCount == 0 ) {
    Serial.println("no network found");
  } else {
    Serial.print(netCount);
    Serial.println(" network(s) found");
    parse_networks( netCount );
  }
  Serial.println("");

}

void parse_networks( int netNum ) {

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

  if ( !update_netdata( netNum ) ) {
    Serial.println( F("Something went WRONG!") );
  }

}

bool update_netdata( int netNum ) {

  int netId;
  int netFound = 0;

  DynamicJsonBuffer jsonBuffer;

//    fh_netdata.println( "{\"count\":0,\"max\":0}" );
//    fh_netdata.println( "{\"count\":0,\"max\":0,\"networks\":[{\"ssid\":\"ssid\",\"bssid\":\"bssid\",\"rssi\":0,\"ch\":1,\"enc\":\"*\"}]}" );

// create new data from network list
  JsonObject& WiFiData = jsonBuffer.createObject();
  WiFiData["count"] = netNum;
  WiFiData["max"] = netNum;

  JsonArray& WiFiDataArray  = WiFiData.createNestedArray("networks");

  fh_netdata = SPIFFS.open("/netdata.txt", "r");

  if ( !fh_netdata ) {

// no last data
    Serial.println( F("Data file doesn't exist yet.") );

    fh_netdata = SPIFFS.open("/netdata.txt", "w");
    if ( !fh_netdata ) {
      Serial.println( F("Data file creation failed") );
      return false;
    }
    for ( int i = 0; i < netNum; ++i ) {

      JsonObject& tmpObj = jsonBuffer.createObject();

      tmpObj["id"] = i;
      tmpObj["ssid"] = WiFi.SSID(i);
      tmpObj["bssid"] = bssidToString( WiFi.BSSID(i) );
      tmpObj["rssi"] = WiFi.RSSI(i);
      tmpObj["ch"] = WiFi.channel(i);
      tmpObj["enc"] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");

      Serial.print("Add - ");tmpObj.printTo( Serial );Serial.println();
      WiFiDataArray.add( tmpObj );

    }

// WiFiData is wifi scan snapshot
//    Serial.println("Scanned wifi data ->");
//    WiFiData.printTo( Serial );
//    WiFiData.prettyPrintTo( Serial );
//    Serial.println("");

  } else {

// read last WiFi data from file
//    Serial.println( F("Reading saved wifi data ..") );
    line = fh_netdata.readStringUntil('\n');
//    Serial.print( F("Line (read) ") );Serial.println( line );

    JsonObject& WiFiDataFile = jsonBuffer.parseObject( line );
    if ( !WiFiDataFile.success() ) {
      Serial.println( F("parsing failed") );
      // parsing failed, removing old data
      SPIFFS.remove( "/netdata.txt" );
      return false;
    }

    int netNumFile = WiFiDataFile["count"];
    int netMaxFile = WiFiDataFile["max"];
    netId = netMaxFile;

//    WiFiDataFile.prettyPrintTo( Serial );
//    Serial.println("");

    JsonArray& tmpArray = WiFiDataFile["networks"];
    for ( JsonArray::iterator it = tmpArray.begin(); it != tmpArray.end(); ++it ) {
      JsonObject& tmpObj = *it;
      WiFiDataArray.add( tmpObj );
      Serial.print("Copy - ");tmpObj.printTo( Serial );Serial.println();
    }

    for ( int i = 0; i < netNum; i++ ) {
      bool wifiNetFound = false;
      for ( int j = 0; j < netNumFile; j++ ) {
        String ssid1 = WiFi.SSID(i);
        String ssid2 = WiFiDataArray[j]["ssid"];
        if ( ssid1 == ssid2 ) {
          String bssid1 = bssidToString( WiFi.BSSID(i) );
          String bssid2 = WiFiDataArray[j]["bssid"];
          if ( bssid1 == bssid2 ) {
            wifiNetFound = true;
          }
        }
      }
      if ( !wifiNetFound ) {

        JsonObject& tmpObj = jsonBuffer.createObject();

        tmpObj["id"] = netId;
        tmpObj["ssid"] = WiFi.SSID(i);
        tmpObj["bssid"] = bssidToString( WiFi.BSSID(i) );
        tmpObj["rssi"] = WiFi.RSSI(i);
        tmpObj["ch"] = WiFi.channel(i);
        tmpObj["enc"] = ((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");

        WiFiDataArray.add( tmpObj );
        Serial.print("Found new - ");tmpObj.printTo( Serial );Serial.println();

        netFound++;
        netId++;
      }
    }

    WiFiData["count"] = netNumFile + netFound;
    WiFiData["max"] = netId;

//    Serial.println("Computed wifi data ->");
//    WiFiData.prettyPrintTo( Serial );

    fh_netdata.close();
    SPIFFS.remove( "/netdata.txt" );

    fh_netdata = SPIFFS.open("/netdata.txt", "w");
    if ( !fh_netdata ) {
      Serial.println( F("Data file creation failed") );
      return false;
    }

  }
  WiFiData.printTo( fh_netdata );
  fh_netdata.println( "\n" );
  fh_netdata.close();

  return true;

}

String bssidToString( uint8_t *bssid ) {

  char mac[18] = {0};

  sprintf( mac,"%02X:%02X:%02X:%02X:%02X:%02X", bssid[0],  bssid[1],  bssid[2], bssid[3], bssid[4], bssid[5] );
  return String( mac );

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
