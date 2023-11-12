#define INT0 2
#define INT1 3
#define LED_TPSBOARD 4

volatile byte state0 = LOW;
volatile byte state1 = LOW;
int ledState = LOW; // État de la LED à envoyer   

void setup() {
  Serial.begin(9600);
  pinMode(INT0, INPUT_PULLUP);
  pinMode(INT1, INPUT_PULLUP);
  pinMode(LED_TPSBOARD, OUTPUT);

  EICRA |= 0b00001100; // Any logical change on TN0 generates an interrupt request.
  EICRA |= 0b00000011; // Any logical change on INT1 generates an interrupt request.
  EIMSK |= 0b00000001; // Configure INT1
  EIMSK |= 0b00000010; // Configure INT0
  
  interrupts();

  digitalWrite(LED_TPSBOARD, 0);
}

ISR(INT0_vect) {
  state0 = !state0;
  Serial.write('0');
  delay(100);
}

ISR(INT1_vect) {
  state1 = !state1;
  Serial.write('1');
}

void loop() {
  if (Serial.available() > 0) {
   
    String receivedMessage = Serial.readStringUntil('\n');
    
    
    Serial.print("Message reçu : ");
    Serial.println(receivedMessage);

       
    receivedMessage = "";
  }
}
