/*
 * all the other stuff
 */

/*
static void SendCommand(unsigned char cmd) {
  Wire.beginTransmission(OLED_address);
  Wire.write(0x80);
  Wire.write(cmd);
  Wire.endTransmission();
}

static void SendChar(unsigned char data) {
  Wire.beginTransmission(OLED_address);
  Wire.write(0x40);
  Wire.write(data);
  Wire.endTransmission();
}
 */

void I2CStart(void) {
  digitalWrite(OLED_SCL, HIGH);
  digitalWrite(OLED_SDA, HIGH);
  digitalWrite(OLED_SDA, LOW);
  digitalWrite(OLED_SCL, LOW);
}

void I2CStop(void) {
  digitalWrite(OLED_SCL, LOW);
  digitalWrite(OLED_SDA, LOW);
  digitalWrite(OLED_SCL, HIGH);
  digitalWrite(OLED_SDA, HIGH);
}

void WriteI2CByte(unsigned char I2CByte) {
  unsigned char i;
  for( i=0; i<8; i++ )   {
    if((I2CByte << i) & 0x80)
      digitalWrite(OLED_SDA, HIGH);
    else
      digitalWrite(OLED_SDA, LOW);
      digitalWrite(OLED_SCL, HIGH);
      digitalWrite(OLED_SCL, LOW);
  }
  digitalWrite(OLED_SDA, HIGH);
  digitalWrite(OLED_SCL, HIGH);
  digitalWrite(OLED_SCL, LOW);
}

void StartI2CData(void) {
   I2CStart();
   WriteI2CByte(0x78);
   WriteI2CByte(0x40);
}

void SendCommand(unsigned char Command) {
  I2CStart();
  WriteI2CByte(0x78); // OLED_address ??
  WriteI2CByte(0x00);
  WriteI2CByte(Command);
  I2CStop();
}

void SendChar(unsigned char data) {
  StartI2CData();
  WriteI2CByte(data);
  I2CStop();
}

void displayON(void) {
  SendCommand(0xAE);    // Set Display On
}

void displayOFF(void) {
  SendCommand(0xAE);    // Set Display Off
}

static void Init_OLED(void) {

  SendCommand(0xAE);    // Set Display Off

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

  SendCommand(0xAE);    // Set Display On

}

static void setCursorXY(unsigned char row, unsigned char col) {
  SendCommand( 0xB0 + row );                        // set page address
  SendCommand( 0x00 + ( 8 * col & 0x0F ) );         // set low col address
  SendCommand( 0x10 + ( ( 8 * col>>4 ) & 0x0F ) );  // set high col address
}

void Draw_Waves(void) {
  displayOFF();
  // clear_display();
  setCursorXY( 0, 0 );

  // Display Logo here :)
  for( int i=0; i < 128*8; i++ ) {
    SendChar( pgm_read_byte( rfwaves + i ) );
  }

  displayON();
}

void Draw_WiFi(void) {
  displayOFF();
  // clear_display();
  setCursorXY( 0, 0 );

  // Display Logo here :)
  for( int i=0; i < 128*8; i++ ) {
    SendChar( pgm_read_byte( WIFI1 + i ) );
  }

  displayON();
}


