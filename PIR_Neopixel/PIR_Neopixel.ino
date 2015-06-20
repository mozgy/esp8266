
#include <NeoPixelBus.h>

extern "C" {
#include "user_interface.h"
}

#define NeoPixelCount 8
#define NeoPixelPin 0
NeoPixelBus strip = NeoPixelBus( NeoPixelCount, NeoPixelPin );

int led = 13;                // the pin that the LED is atteched to
int sensor = 2;              // the pin that the sensor is atteched to
int state = LOW;             // by default, no motion detected
int val = 0;                 // variable to store the sensor status (value)

uint16_t c;


void setup(void) {

  Serial.begin( 115200 );
  delay(100);
  // print out all system information
  Serial.println();
  Serial.print( "Heap: " );
  Serial.println( system_get_free_heap_size() );
  Serial.print( "Boot Vers: " );
  Serial.println( system_get_boot_version() );
  Serial.print( "CPU: " );
  Serial.println( system_get_cpu_freq() );
  Serial.print( "ChipID: " );
  Serial.println( ESP.getChipId() );
  Serial.println();

  strip.Begin();
  strip.Show();
  c = 0;

  // pinMode( led, OUTPUT );      // initalize LED as an output
  pinMode( sensor, INPUT );    // initialize sensor as an input
  Serial.println( "Setup done" );

}

void loop(void) {

  strip.SetPixelColor( 0, RgbColor( 0, 0, c ) );
  strip.Show();

  val = digitalRead( sensor );   // read sensor value
  delay(100);
  if ( val == HIGH ) {           // check if the sensor is HIGH
    // digitalWrite( led, HIGH );   // turn LED ON
    delay(100);
    
    if ( state == LOW ) {
      Serial.println("Motion detected!"); 
      state = HIGH;              // update variable state to HIGH
      strip.SetPixelColor( 1, RgbColor( 128, 0, 0 ) ); // Red
      strip.SetPixelColor( 2, RgbColor( 128, 0, 0 ) ); // Red
      strip.SetPixelColor( 3, RgbColor( 128, 0, 0 ) ); // Red
      strip.SetPixelColor( 4, RgbColor( 128, 0, 0 ) ); // Red
      strip.SetPixelColor( 5, RgbColor( 128, 0, 0 ) ); // Red
      strip.SetPixelColor( 6, RgbColor( 128, 0, 0 ) ); // Red
      strip.SetPixelColor( 7, RgbColor( 128, 0, 0 ) ); // Red
      strip.Show();
    }
  } else {
    // digitalWrite( led, LOW ); // turn LED OFF
    delay(100);

    if ( state == HIGH ) {
      Serial.println( "Motion stopped!" );
      state = LOW;               // update variable state to LOW
      strip.SetPixelColor( 1, RgbColor( 0, 128, 0 ) ); // Green
      strip.SetPixelColor( 2, RgbColor( 0, 128, 0 ) ); // Green
      strip.SetPixelColor( 3, RgbColor( 0, 128, 0 ) ); // Green
      strip.SetPixelColor( 4, RgbColor( 0, 128, 0 ) ); // Green
      strip.SetPixelColor( 5, RgbColor( 0, 128, 0 ) ); // Green
      strip.SetPixelColor( 6, RgbColor( 0, 128, 0 ) ); // Green
      strip.SetPixelColor( 7, RgbColor( 0, 128, 0 ) ); // Green
      strip.Show();
    }
  }

  c = c + 3;
  c = c % 128;

  // Serial.println( "Loop!" );
  delay(500);

}
