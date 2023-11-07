#include <SPI.h>

#define REFRESH_RATE 100
#define CONFIG_DELAY 250

// --------------------------------------------------------------

uint8_t reverseBits(uint8_t value);
void sendLCDCommand(uint8_t command);
void sendLCDData(uint8_t command);
void setupLCD();
void writeString(const char* string);
void setLCDposition(uint8_t line, uint8_t column);
void cleanDisplay();

// --------------------------------------------------------------

uint8_t reverseBits(uint8_t value) {
  uint8_t result = 0;
  for (int i = 0; i < 8; i++) {
    result |= ((value >> i) & 0x01) << (7 - i);
  }
  return result;
}

void sendLCDCommand(uint8_t command) {
  byte oct1 = 0b11111000; // send command 0xF8
  byte oct2 = reverseBits(command & 0x0F); // 4 right bits isolation reversed
  byte oct3 = reverseBits((command >> 4) & 0x0F); // 4 left bits isolation reversed

  // Transfer the 3 bytes
  SPI.transfer(oct1);
  SPI.transfer(oct2);
  SPI.transfer(oct3);
}

void sendLCDData(uint8_t command) {
  byte oct1 = 0b11111010; // send command 0xF8
  byte oct2 = reverseBits(command & 0x0F); // 4 right bits isolation reversed
  byte oct3 = reverseBits((command >> 4) & 0x0F); // 4 left bits isolation reversed
  
  // Transfer the 3 bytes
  SPI.transfer(oct1);
  SPI.transfer(oct2);
  SPI.transfer(oct3);
}

void setupLCD() {
  sendLCDCommand(0b00000001); // Clear display
  sendLCDCommand(0b00100010);  // Enter extension register RE=1
  sendLCDCommand(0b00001001); // Character amount per line NW
  sendLCDCommand(0b00000110); // Common Bidirection Function BDC / Segment Bidirection Function BDS
  sendLCDCommand(0b00101000); // Exit extension register mode RE=0
  sendLCDCommand(0b00001111); // Turn on display, cursor and blink
  sendLCDCommand(0b00000001); // Clear display
  delay(CONFIG_DELAY);
}

void writeString(const char* string) {
  while (*string) {
    sendLCDData(*string);
    string++;
  }
}
void setLCDposition(uint8_t line, uint8_t column) {
  // Lines & columns counted from 1
  uint8_t x = column - 0x1;
  uint8_t y = 0x20 * (line - 0x1);
  sendLCDCommand(x+y);
}

void cleanDisplay(){
  delay(REFRESH_RATE);
  sendLCDCommand(0b00000001);
}
void setup() {
  SPI.begin();
  setupLCD();
}

void loop() {
  setLCDposition(1, 5);
  writeString("May the force");
  setLCDposition(2, 6);
  writeString("be with you ...");
  cleanDisplay();
}
