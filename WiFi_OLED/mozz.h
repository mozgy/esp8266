/*
 * all the other stuff
 */

static void SendCommand(unsigned char cmd) {
  Wire.beginTransmission( OLED_address );
  Wire.write( 0x80 );
  Wire.write( cmd );
  Wire.endTransmission();
}

static void SendChar(unsigned char data) {
  Wire.beginTransmission( OLED_address );
  Wire.write( 0x40 );
  Wire.write( data );
  Wire.endTransmission();
}

void displayON(void) {
  SendCommand( 0xAF );    // Set Display On
}

void displayOFF(void) {
  SendCommand( 0xAE );    // Set Display Off
}

static void init_OLED(void) {

  SendCommand( 0xAE );  // Set Display Off

  SendCommand(0xD5);    // Set Display Clock Divide Ratio\Oscilator Frequency
  SendCommand(0x80);    // the suggested ratio 0x80

  SendCommand(0xA8);    // Set Multiplex Ratio
  SendCommand(0x3F);

  SendCommand(0xD3);    // Set Display Offset
  SendCommand(0x00);    // no offset

  SendCommand(0x40);    // Set Display Start Line

  SendCommand(0x8D);    // Set Charge Pump
  SendCommand(0x14);    // Vcc internal

  SendCommand(0xA1);    // Set Segment Re-Map

  SendCommand(0xC8);    // Set COM Output Scan Direction

  SendCommand(0xDA);    // Set COM Pins Hardware Configuration
  SendCommand(0x12);

  SendCommand(0x81);    // Set Contrast Control
  SendCommand(0xCF);    // internal

  SendCommand(0xD9);    // Set Pre-Charge Period
  SendCommand(0xF1);    // internal

  SendCommand(0xDB);    // Set VCOMH Deselect Level
  SendCommand(0x40);

  SendCommand(0xA4);    // Set Entire Display On/Off

  SendCommand(0xA6);    // Set Normal/Inverse Display

  // clearscreen();

  SendCommand( 0x2E );  // Deactivate Scroll

  SendCommand( 0x20 );  // Set Memory Addressing Mode
  SendCommand( 0x00 );  // Horizontal Addressing Mode

  SendCommand( 0xAF );  // Set Display On

}

static void setCursorXY(unsigned char row, unsigned char col) {
  SendCommand( 0xB0 + row );                        // set page address
  SendCommand( 0x00 + ( 8 * col & 0x0F ) );         // set low col address
  SendCommand( 0x10 + ( ( 8 * col>>4 ) & 0x0F ) );  // set high col address
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY( const char *string, int X, int Y) {
  unsigned char i;

  setCursorXY( X, Y );
  while( *string ) {
    for( i=0; i<8; i++ ) {
      SendChar( pgm_read_byte( myFont[*string-0x20] + i ) );
    }
    *string++;
  }
}

void clear_display(void) {
  unsigned char i,j;

  displayOFF();
  for( int i=0; i < 8; i++ ) {
    setCursorXY( i, 0 );
    for( int j=0; j < 128; j++ ) {
      SendChar( 0x00 );
    }
  }
  displayON();
}

void Draw_Waves(void) {
  unsigned char i,j;

  // displayOFF();
  clear_display();
  setCursorXY( 0, 0 );

 for( int i=0; i < 128*8; i++ ) {
    SendChar( pgm_read_byte( rfwaves + i ) );
  }

  displayON();
}

void Draw_WiFi(void) {
  unsigned char i,j;

  displayOFF();
  // clear_display();
  for( int i=0; i<8; i++ ) {
    setCursorXY( i, 0 );
    for( int j=0; j<16*8; j++ ) {
        SendChar( pgm_read_byte( WIFI1 + j + i*16*8 ) );
    }
  }
  displayON();
}

/*
void Draw_WAVES(void) {
  clear_display();
  // Display Logo here :)
  for( int i=0; i<8; i++ ) {
    setXY( i, 0 );
    for( int j=0; j<16*8; j++ ) {
      SendChar( pgm_read_byte( rfwaves + j + i*16*8 ) );
    }
  }
  displayOn();
}
 */
