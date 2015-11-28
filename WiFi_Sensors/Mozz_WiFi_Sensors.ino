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

// set only one of these true
// #define SENSOR_DHT
// #undef SENSOR_DS18B20
#undef SENSOR_DHT
#define SENSOR_DS18B20
// set only one of these true

// set only one of these true
#define DEEPSLEEP
#undef OTA
// #undef DEEPSLEEP
// #define OTA
// set only one of these true

// ToDo - OTA as lib and as http get
#ifdef DEEPSLEEP
 #undef OTA_LIB
 #undef OTA_HTTP
#endif
// ToDo

/* END of compile DEFINEs */

#include <ESP8266WiFi.h>
#ifdef OTA
 #include <WiFiUdp.h>
 #include <ArduinoOTA.h>
#endif

ADC_MODE(ADC_VCC);

#include <Ticker.h>
Ticker tickerSensorScan;
#define WAITTIME 300
boolean tickerFired;

const char* ssid     = "x";
const char* pass     = "y";
const char* host     = "foo.bar";
const char* urlHost  = "192.168.256.256";

const char* sensorLocation = "Room";
// const char* sensorLocation = "Attic1";
// const char* sensorLocation = "Attic2";
// const char* sensorLocation = "LiFePO4Test";
// const char* sensorLocation = "LiPO2STest";

#ifdef SENSOR_DHT
#include "DHT.h"
#define DHTPIN 2
// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht( DHTPIN, DHTTYPE );
#endif

#ifdef SENSOR_DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>

// (a 4.7K resistor is necessary)
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire( ONE_WIRE_BUS );
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors( &oneWire );
#endif

char tmpstr[40];

const int sleepTimeSec = 300;

void flagSensorScan( void ) {
  tickerFired = true;
}

void initWiFi(void) {

  Serial.print( "Connecting to " );
  Serial.println( ssid );
  WiFi.mode( WIFI_STA );
  WiFi.begin( );
  while ( WiFi.waitForConnectResult() != WL_CONNECTED ){
     WiFi.begin( ssid, pass );
     Serial.println( "Retrying connection..." );
//    Serial.println("Connection Failed! Rebooting...");
//    delay(5000);
//    ESP.restart();
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

void sendSensorData( const char *id, float hum, float temp ) {

  WiFiClient client;
  int mV = ESP.getVcc();

  initWiFi();

  client.stop();
  uint8_t i = 0;
  while( !client.connect( urlHost, 80 ) && i++ < 50 ) {
    Serial.println( "Cannot connect!" );
    delay(500);
  }

  String WiFiString = "GET /sensors/insertdata.php?hash=test&s=";
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

/*
  char charBuf[200];
  WiFiString.toCharArray( charBuf, 200 );
  wifi.send(1, (const uint8_t*)charBuf, 200);
 */

  client.stop();

}

#ifdef SENSOR_DHT
boolean doSomethingWithSensor( void ) {

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if ( isnan(h) || isnan(t) ) {
    // Serial.println( "Failed to read from DHT sensor!" );
    Serial.print(".");
    delay(10);
    return false;
  }

  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print(" *C ");
  Serial.println();

  sendSensorData( sensorLocation, h, t );

  return true;
}
#endif

#ifdef SENSOR_DS18B20
boolean doSomethingWithSensor( void ) {

  // we are reusing same php on receiving side, hence humidity -1% here ;)
  float hum = -1;

  float temp;

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  Serial.print(" Requesting temperature...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature for Device 1 is: ");
  temp = sensors.getTempCByIndex( 0 );
  Serial.print( temp );
  // Why "byIndex"? 
  // You can have more than one IC on the same bus. 
  // 0 refers to the first IC on the wire
  Serial.println();

  sendSensorData( sensorLocation, hum, temp );

  return true;
}
#endif

#ifdef OTA
void SetupArduinoOTA( void ) {

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("sensorX-esp");

  // ArduinoOTA.setPassword((const char *)"xxx");

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
    else if (error == OTA_RECIEVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

}
#endif

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

#ifdef OTA
  SetupArduinoOTA();
#endif

#ifdef SENSOR_DHT
  dht.begin();
#endif

#ifdef SENSOR_DS18B20
  sensors.begin();
#endif

#ifdef DEEPSLEEP
  while( ! doSomethingWithSensor() );

  Serial.printf( "Gonna ZZzz..\n" );
  ESP.deepSleep( sleepTimeSec * 1000000 );
#endif

  tickerSensorScan.attach( WAITTIME, flagSensorScan );
  tickerFired = true;

}

void loop() {

#ifdef OTA
  ArduinoOTA.handle();

  if( tickerFired ) {
    tickerFired = false;
    while( ! doSomethingWithSensor() );

    ElapsedStr( tmpstr );
    Serial.println( tmpstr );
    Serial.printf( "Heap: %u\n", ESP.getFreeHeap() );
  }
#endif

}
