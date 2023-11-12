#include "DHT.h"
#include "Adafruit_VEML7700.h"
#include "Mhz19.h"
#include "HP20x_dev.h"

#define PIN_POLLU A0
#define PIN_DHT 2
#define TYPE_DHT 22

DHT captTemp_Humid(PIN_DHT, TYPE_DHT);
Adafruit_VEML7700 captLum;
HP20x_dev captPressure;

void setup() {
  captTemp_Humid.begin();
  captLum.begin();
  captPressure.begin();
  pinMode(PIN_POLLU, INPUT); // Configure la broche 0 en tant qu'entrée
  Serial.begin(9600);
}

void loop() {
   float T=captTemp_Humid.readTemperature();
  float H=captTemp_Humid.readHumidity();
  float L=captLum.readLux();
  float Q= analogRead(PIN_POLLU);
  float P = captPressure.ReadPressure();
  int q = Q * 100/1024;
  P /=100;
  Serial.println("T" + String(T));
  Serial.println("H" + String(H));
  Serial.println("P" + String(P));
  Serial.println("L" + String(L));
  Serial.println("Q" + String(q));
  delay(500);
}
