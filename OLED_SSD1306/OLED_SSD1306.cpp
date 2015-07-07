/*
  OLED_SSD1306.cpp - I2C 128x64 OLED Driver Library
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

*/

#include <Wire.h>
// #include <avr/pgmspace.h>
#include "OLED_SSD1306.h"
#include "font.h"

OLED_SSD1306::OLED_SSD1306( int i2caddr ) {
  localI2CAddress = i2caddr;
}

void OLED_SSD1306::SendCommand( unsigned char cmd ) {
  Wire.beginTransmission( localI2CAddress );
  Wire.write( 0x80 );
  Wire.write( cmd );
  Wire.endTransmission();
}

void OLED_SSD1306::SendChar( unsigned char data ) {
  Wire.beginTransmission( localI2CAddress );
  Wire.write( 0x40 );
  Wire.write( data );
  Wire.endTransmission();
}

void OLED_SSD1306::SetCursorXY( unsigned char row, unsigned char col ) {
  OLED_SSD1306::SendCommand( 0xB0 + row );                        // set page address
  OLED_SSD1306::SendCommand( 0x00 + ( 8 * col & 0x0F ) );         // set low col address
  OLED_SSD1306::SendCommand( 0x10 + ( ( 8 * col>>4 ) & 0x0F ) );  // set high col address
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
void OLED_SSD1306::SendStrXY( const char *string, int X, int Y ) {
  unsigned char i;

  OLED_SSD1306::SetCursorXY( X, Y );
  while( *string ) {
    for( i=0; i<8; i++ ) {
      OLED_SSD1306::SendChar( pgm_read_byte( myFont[*string-0x20] + i ) );
    }
    *string++;
  }
}

void OLED_SSD1306::ClearDisplay(void) {
  unsigned char i,j;

  OLED_SSD1306::DisplayOFF();
  for( int i=0; i < 8; i++ ) {
    OLED_SSD1306::SetCursorXY( i, 0 );
    for( int j=0; j < 128; j++ ) {
      OLED_SSD1306::SendChar( 0x00 );
    }
  }
  OLED_SSD1306::DisplayON();
}

#define Scroll_2Frames			0x07
#define Scroll_3Frames			0x04
#define Scroll_4Frames			0x05
#define Scroll_5Frames			0x00
#define Scroll_25Frames			0x06
#define Scroll_64Frames			0x01
#define Scroll_128Frames		0x02
#define Scroll_256Frames		0x03
// speed is in number of frames
void OLED_SSD1306::ScrollRight( unsigned char start, unsigned char end, unsigned char speed ) {

  SendCommand( 0x26 );  // Right Horizontal Scroll
  SendCommand( 0x00 );	// Dummy byte (Set as 00h)

  SendCommand( start );	// start page address
  SendCommand( speed );	// set time interval between each scroll
  SendCommand( end );	// end page address

  SendCommand( 0x00 );	// Dummy byte (Set as 00h)
  SendCommand( 0xFF );	// Dummy byte (Set as FFh)

  SendCommand( 0x2F );	// Activate Scroll

}

void OLED_SSD1306::ScrollStop( void ) {
  SendCommand( 0x2E );  // Deactivate Scroll
}

void OLED_SSD1306::DisplayFlipON( void ) {
  SendCommand( 0xC0 );  // Set COM Output Scan Direction
  SendCommand( 0xA0 );  // Set Segment Re-Map
}

void OLED_SSD1306::DisplayFlipOFF( void ) {
  SendCommand( 0xC8 );  // Set COM Output Scan Direction
  SendCommand( 0xA1 );  // Set Segment Re-Map
}

void OLED_SSD1306::BlinkOFF(void) {
  SendCommand( 0xA4 );  // Entire Display Normal
}

void OLED_SSD1306::BlinkON(void) {
  SendCommand( 0xA5 );  // Entire Display ON
}

void OLED_SSD1306::DisplayNormal(void) {
  SendCommand( 0xA6 );  // Set Display Normal
}

void OLED_SSD1306::DisplayInverse(void) {
  SendCommand( 0xA7 );  // Set Display Inverse
}

void OLED_SSD1306::DisplayON(void) {
  SendCommand( 0xAF );  // Set Display On
}

void OLED_SSD1306::DisplayOFF(void) {
  SendCommand( 0xAE );  // Set Display Off
}

void OLED_SSD1306::Init(void) {

  SendCommand( 0xAE );  // Set Display Off

  SendCommand( 0xA8 );  // Set Multiplex Ratio
  SendCommand( 0x3F );

  SendCommand( 0xD3 );  // Set Display Offset
  SendCommand( 0x00 );  // no offset

  SendCommand( 0x40 );  // Set Display Start Line

  SendCommand( 0xA1 );  // Set Segment Re-Map

  SendCommand( 0xC8 );  // Set COM Output Scan Direction

  SendCommand( 0xDA );  // Set COM Pins Hardware Configuration
  SendCommand( 0x12 );

  SendCommand( 0x81 );  // Set Contrast Control
  SendCommand( 0x7F );

  SendCommand( 0xA4 );  // Set Entire Display On/Off

  SendCommand( 0xA6 );  // Set Normal/Inverse Display

  SendCommand( 0xD9 );  // Set Pre-Charge Period
  SendCommand( 0xF1 );  // internal

  SendCommand( 0xDB );  // Set VCOMH Deselect Level
  SendCommand( 0x40 );

  SendCommand( 0xD5 );  // Set Display Clock Divide Ratio\Oscilator Frequency
  SendCommand( 0x80 );  // the suggested ratio 0x80

  SendCommand( 0x8D );  // Set Charge Pump
  SendCommand( 0x14 );  // Vcc internal

  SendCommand( 0x2E );  // Deactivate Scroll

  SendCommand( 0x00 );  // Set Lower Column Start Address

  SendCommand( 0x10 );  // Set Higher Column Start Address

  // 00 - Horizontal Addressing Mode
  // 01 - Vertical Addressing Mode
  // 02 - Page Addressing Mode
  SendCommand( 0x20 );  // Set Memory Addressing Mode
  SendCommand( 0x00 );

  SendCommand( 0x21 );  // Set Column Address (only for horizontal or vertical mode)
  SendCommand( 0x00 );
  SendCommand( 0x7F );

  SendCommand( 0x22 );  // Set Page Address
  SendCommand( 0x00 );
  SendCommand( 0x07 );

  SendCommand( 0xB0 );  // Set Page Start Address for Page Addressing Mode

  SendCommand( 0xAF );  // Set Display On

}
