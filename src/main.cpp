#include "Arduino.h"
#include "SPI.h"

#define PIN_CS 10 //Chip-select pin for hardware SPI
#define PIN_ACCEL A0
#define PIN_STEER A1

#define POTENTIOMETER_LEFT_ADDRESS 0b00010001
#define POTENTIOMETER_RIGHT_ADDRESS 0b00010010
//#define POTENTIOMETER_SLOT_ALL_ADDRESS 0b00010011

int accelValue;
int steerValue;
byte speedValue;

///
/// This method writes data to two Dais-Chain connected MCP4xxxx via SPI.
///     1. Setting LOW on CS taking control over MCP4xxxx and force them listen
///     2. We send data for second MCP4xxxx, it will be stored in first MCP4xxxx
///     3. We send data for first MCP4xxxx, previous data will be pushed to second MCP4xxxx
///     and first one will get it's own data
///     4. Setting HIGH on CS releases MCP4xxxx so both of them can act
/// \param addressFront
/// \param addressRear
/// \param valFront
/// \param valRear
///
void MCP4xxxxDaisyChainWrite(byte addressFront, byte addressRear, byte valFront, byte valRear) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(addressRear); // NOLINT
  SPI.transfer(valRear); // NOLINT
  SPI.transfer(addressFront); // NOLINT
  SPI.transfer(valFront); // NOLINT
  digitalWrite(PIN_CS, HIGH);
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
  pinMode(PIN_STEER, INPUT);
}

void loop() {
  accelValue = analogRead(PIN_ACCEL);
  steerValue = analogRead(PIN_STEER);
  speedValue = getSpeed(accelValue);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_LEFT_ADDRESS, POTENTIOMETER_LEFT_ADDRESS, speedValue, speedValue);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_RIGHT_ADDRESS, POTENTIOMETER_RIGHT_ADDRESS, speedValue, speedValue);
  delay(50);
}