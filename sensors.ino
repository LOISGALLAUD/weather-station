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
float T=captTemp_Humid.readTemperature();
float H=captTemp_Humid.readHumidity();
float L=captLum.readLux();
float Pollu= analogRead(PIN_POLLU);
float Pressure = captPressure.ReadPressure();

void setup() {
  captTemp_Humid.begin();
  captLum.begin();
  captPressure.begin();
  pinMode(PIN_POLLU, INPUT);
  Serial.begin(9600);
}

void loop() {
  Serial.print("Lux : ");
  Serial.print(L);
  Serial.println("lux ");
  
  Serial.print("Pollution : ");
  Serial.print(Pollu);
  Serial.println("g/cm3 ");

  Serial.print("Pression : ");
  Serial.print(Pressure);
  Serial.println(" P ");

  Serial.print("Temperature : ");
  Serial.print(T);
  Serial.println("°C");
  
  Serial.print("Humidity : ");
  Serial.print(H);
  Serial.println("g/m3");
}
