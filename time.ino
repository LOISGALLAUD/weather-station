#include <SPI.h>
#define INT0 2
#define INT1 3
#define LED_TPSBOARD 4

volatile byte state0 = LOW;
volatile byte state1 = LOW;
volatile byte postscaler=LOW;
volatile long int seconds = 0;
volatile bool oscillatingVar = false;
static int var_state1;

// -------------------- Prototypes --------------------------

reversedBits(uint8_t value);
void sendLCDCommand(uint8_t command);
void sendLCDData(uint8_t command);
void setupLCD();
void writeString(const char* string);
void setLCDposition(uint8_t line, uint8_t column);
void cleanDisplay();

void time(int h, int m, int s);
void oscillate();
void displayTime(int h, int m, int s);

// ------------------------ SPI -----------------------------

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

  SPI.transfer(oct1);
  SPI.transfer(oct2);
  SPI.transfer(oct3);
}
void sendLCDData(uint8_t command) {
  byte oct1 = 0b11111010; // send command 0xF8
  byte oct2 = reverseBits(command & 0x0F); // 4 right bits isolation reversed
  byte oct3 = reverseBits((command >> 4) & 0x0F); // 4 left bits isolation reversed

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
  sendLCDCommand(0b00001100); // Turn on display, cursor and blink
  sendLCDCommand(0b00000001); // Clear display
  delay(250);
}
void writeString(const char* string) {
  while (*string) {
    sendLCDData(*string);
    string++;
  }
}
void setLCDposition(uint8_t line, uint8_t column) {
  uint8_t x = column - 0x1;
  uint8_t y = 0x20*(line);
  uint8_t ddgram = x+y;

  sendLCDCommand(ddgram | 0b10000000);
}
void cleanDisplay(){
  delay(100);
  sendLCDCommand(0b00000001);
}

// ------------------------ FLAGS -----------------------------

ISR(INT0_vect) {
  state0 = !state0;
  Serial.println("pressed 0.");
  delay(100);
  switch(var_state1) {
      case 1:
        seconds++;
        break;
      case 2:
        seconds += 60;
        break;
      case 3:
        seconds += 3600;
        break;
      default:
        break;
  }
}
ISR(INT1_vect) {
  state1 = !state1;
  Serial.println("pressed 1.");
  var_state1++;
  var_state1 %= 4;
}
ISR(TIMER2_OVF_vect){
  TCNT2 = 131;
  if (postscaler<125) postscaler++;
  else {
    postscaler = 0;
    oscillatingVar = !oscillatingVar;
    seconds++;
  }
  if (postscaler==62) 
  {
    oscillatingVar = !oscillatingVar;
  }
}

// ------------------------ TIME -----------------------------

void time(int h, int m, int s) {
  char hourStr[3];
  char minStr[3];
  char secStr[3];
  sprintf(hourStr, "%02d", h);
  sprintf(minStr, "%02d", m);
  sprintf(secStr, "%02d", s);

  char timeStr[9];
  sprintf(timeStr, "%s:%s:%s", hourStr, minStr, secStr);

  setLCDposition(1, 7);
  writeString(timeStr);
}

void oscillate() {
  if (oscillatingVar) {
    digitalWrite(LED_TPSBOARD, HIGH);
  } else {
    digitalWrite(LED_TPSBOARD, LOW); // Éteindre la LED
  }
}

void displayTime(int h, int m, int s) {
  Serial.println(var_state1);
  switch (var_state1) {
    case 1:
      if (oscillatingVar) {
        time(h, m, s);
      } else {
        char hourStr[3];
        char minStr[3];
        sprintf(hourStr, "%02d", h);
        sprintf(minStr, "%02d", m);

        char timeStr[9];
        sprintf(timeStr, "%s:%s:  ", hourStr, minStr);

        setLCDposition(1, 7);
        writeString(timeStr);
      }
      break;
    case 2:
      if (oscillatingVar) {
        time(h, m, s);
      } else {
        char hourStr[3];
        char secStr[3];
        sprintf(hourStr, "%02d", h);
        sprintf(secStr, "%02d", s);

        char timeStr[9];
        sprintf(timeStr, "%s:  :%s", hourStr, secStr);

        setLCDposition(1, 7);
        writeString(timeStr);
      }
      break;
    case 3:
      if (oscillatingVar) {
        time(h, m, s);
      } else {
        char minStr[3];
        char secStr[3];
        sprintf(minStr, "%02d", m);
        sprintf(secStr, "%02d", s);

        char timeStr[9];
        sprintf(timeStr, "  :%s:%s", minStr, secStr);

        setLCDposition(1, 7);
        writeString(timeStr);
      }
      break;
    default:
      time(h, m, s);
      break;
  }
}

void setup() {
  pinMode(INT0, INPUT_PULLUP);
  pinMode(INT1, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  SPI.begin();

  TCCR2A &= 0b11111100;
  TCCR2B &= 0b11110111;
  TCCR2B |= 0b00000111;
  TIMSK2 |= 0b000000001;

  EICRA |= 0b00001100; // Any logical change on TN0 generates an interrupt request.
  EICRA |= 0b00000011; // Any logical change on INT1 generates an interrupt request.
  EIMSK |= 0b00000001; // Configure INT1
  EIMSK |= 0b00000010; // Configure INT0
  
  interrupts();
  setupLCD();
}

void loop() {
  int h = seconds / 3600 % 100;
  int m = (seconds / 60) % 60;
  int s = seconds % 60;
  
  oscillate();
  displayTime(h, m, s);
  cleanDisplay();
}
