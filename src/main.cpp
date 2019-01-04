#include "Arduino.h"
#include "SPI.h"

#define PIN_CS 10 //Chip-select pin for hardware SPI
#define PIN_ACCEL A0

#define POTENTIOMETER_SLOT_0_ADDRESS 0b00010001
#define POTENTIOMETER_SLOT_1_ADDRESS 0b00010010
//#define POTENTIOMETER_SLOT_ALL_ADDRESS 0b00010011

int accelValue;
byte speedValue;



void MCP4xxxxDaisyChainWrite(byte address1, byte address2, byte val1, byte val2) {
  digitalWrite(PIN_CS, LOW);                    // включаем прием данных микросхемой
  SPI.transfer(address2); // NOLINT             // отправляем первый байт в регистр конфигурации
  SPI.transfer(val2); // NOLINT                 // отправляем второй байт в "регистр ползунка"
  SPI.transfer(address1); // NOLINT             // отправляем первый байт в регистр конфигурации
  SPI.transfer(val1); // NOLINT                 // отправляем второй байт в "регистр ползунка"
  digitalWrite(PIN_CS, HIGH);                   // выключаем прием данных микросхемой
}

byte getSpeed(int accel) {
  //1024 – maximal analog input value which is converted to...
  //...256 – maximal potenciometer accepted value
  double speed = ((accel)/1024.0)*256.0;
  return (byte)speed;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CS, OUTPUT);
  SPI.begin(); // NOLINT
  pinMode(PIN_ACCEL, INPUT);
}

void loop() {
  accelValue = analogRead(PIN_ACCEL);
  speedValue = getSpeed(accelValue);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_SLOT_0_ADDRESS, POTENTIOMETER_SLOT_0_ADDRESS, speedValue, speedValue);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_SLOT_1_ADDRESS, POTENTIOMETER_SLOT_1_ADDRESS, speedValue, speedValue);
  delay(50);
}