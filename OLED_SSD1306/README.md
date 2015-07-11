My take on SSD1306 library,
works with 0.96" and 1.3" OLEDs

example ->

``` c++
#define OLED_ADDRESS 0x3C
OLED_SSD1306 oled( OLED_ADDRESS );
void setup() {
  Serial.println("OLED Init...");
  Wire.begin( SDA_pin, SCL_pin );
  oled.Init();
}
void loop() {
  oled.ClearDisplay();
  oled.SendStrXY( "Hello World!", 3, 0 );
}
```

for SH1106 OLEDs (like some 1.3" ones from ebay) just add
``` c++
#define OLED_SH1106
```

--
Mozz
