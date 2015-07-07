/*
 *   OLED_SSD1306.h - I2C 128x64 OLED Driver Library
 *     
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   Copyright (c) 2015. Mario Mikočević <mozgy>
 *
 */

#ifndef OLED_SSD1306_H
#define OLED_SSD1306_H

class OLED_SSD1306 {

  private:

    int localI2CAddress;

  public:

    OLED_SSD1306( int i2caddr );

    void Init(void);

    void SendCommand( unsigned char Command );
    void SendChar( unsigned char Data );

    void SendStrXY( const char *string, int X, int Y );
    void SetCursorXY( unsigned char row, unsigned char col );
    void DisplayON( void );
    void DisplayOFF( void );
    void DisplayInverse( void );
    void DisplayNormal( void );
    void BlinkON( void );
    void BlinkOFF( void );
    void ClearDisplay( void );

    void DisplayFlipON( void );
    void DisplayFlipOFF( void );

    void ScrollRight( unsigned char start, unsigned char end, unsigned char speed );
    void ScrollStop( void );

};

#endif
