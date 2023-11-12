#include <SPI.h>
#define INT0 2
#define INT1 3
#define LED_TPSBOARD 4
#define BLANK_CHARACTER " "

volatile byte postscaler=LOW;
volatile long int seconds = 0;
volatile bool oscillatingVar = false;
int hour=0;
int min=0;
int sec=0;

static int var_state1, var_state0;

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
  uint8_t x = column;
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
  Serial.print("var_stat0 = ");
  Serial.println(var_state0);
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
      default: // pas de chgt dheure
        var_state0++;
        var_state0 %= 4;
        cleanDisplay();
        break;
  }
}
ISR(INT1_vect) {
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
    hour = seconds / 3600 % 24;
    min = (seconds / 60) % 60;
    sec = seconds % 60;
  }
  if (postscaler==62) 
  {
    oscillatingVar = !oscillatingVar;
  }
}

// ------------------------ TIME -----------------------------
void oscillate() {
  if (oscillatingVar) {
    digitalWrite(LED_TPSBOARD, HIGH);
  } else {
    digitalWrite(LED_TPSBOARD, LOW); // Éteindre la LED
  }
}

void displayTime() {
  char hourStr[3];
  char minStr[3];
  char secStr[3];
  char timeStr[9];

  sprintf(hourStr, "%02d", hour);
  sprintf(minStr, "%02d", min);
  sprintf(secStr, "%02d", sec);
  
  sprintf(timeStr, "%s-%s-%s", hourStr, minStr, secStr);
  
  if (oscillatingVar) {
    switch (var_state1) {
        case 1: 
            sprintf(timeStr, "%s-%s-  ", hourStr, minStr);
          break;
        case 2:
            sprintf(timeStr, "%s-  -%s", hourStr, secStr);
          break;
        case 3:
            sprintf(timeStr, "  -%s-%s", minStr, secStr);
          break;
        default:
          break;
      }
  }
  setLCDposition(0, 6);
  writeString(timeStr);
}

void displayCara(char * c1, char *c2, char *unit1, char *unit2,
                int step1, int step2){
  setLCDposition(2, 0);
  writeString(c1);
  setLCDposition(3, 0);
  writeString(c2);
  setLCDposition(2, step1);
  writeString(unit1);
  setLCDposition(3, step2);
  writeString(unit2);
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

void writeDataLCD(String messagegot, char c1, char c2, int y1, int y2){
  if (messagegot.startsWith(String(c1))) {
      String H = messagegot.substring(1);
      setLCDposition(2, y1);
      writeString(H.c_str());
    }
  if (messagegot.startsWith(String(c2))) {
      String P = messagegot.substring(1);
      setLCDposition(3, y2);
      writeString(P.c_str());
    }
}

void loop() {
  setLCDposition(0, 0);
  writeString("Heure:");
  displayTime();
  setLCDposition(1, 0);
  writeString("Temp:");

  if (Serial.available() > 0) {
    String receivedMessage = Serial.readStringUntil('\n');
    //!\ ne pas toucher
    if (receivedMessage.startsWith("T")) {
      String T = receivedMessage.substring(1);
      setLCDposition(1, 5);
      writeString(T.c_str());
      setLCDposition(1, 11);
      writeString("C");
    }

    switch (var_state0) {
          case 1:
            displayCara("Pression:", "Luminosite:", "phPa", "lux", 15, 15);
            writeDataLCD(receivedMessage, 'P', 'L', 9, 11);
            break;
          case 2:
            displayCara("Luminosite:", "Qualite de l'air:", "lux", " ", 15, 19);
            writeDataLCD(receivedMessage, 'L', 'Q', 11, 17);
            break;
          case 3:
            displayCara("Qualite de l'air:", "Humidite:", " ", "%", 18, 14);
            writeDataLCD(receivedMessage, 'Q', 'H', 17, 9);
            break;
          default:
            displayCara("Humidite:", "Pression:", "%", "phPa", 15, 15);
            writeDataLCD(receivedMessage, 'H', 'P', 9, 9 );
            break;
        }
    
    receivedMessage = "";
  }

}
