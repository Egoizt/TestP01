#include "Arduino.h"
#include "SPI.h"

#define PIN_CS 10 //Chip-select pin for hardware SPI
#define PIN_ACCEL A0

#define POTENTIOMETER_SLOT_0_ADDRESS 0b00010001
#define POTENTIOMETER_SLOT_1_ADDRESS 0b00010010
//#define POTENTIOMETER_SLOT_ALL_ADDRESS 0b00010011

int accelValue;
byte speedValue;

///
/// This method writes data to two Dais-Chain connected MCP4xxxx via SPI.
///     1. Setting LOW on CS taking control over MCP4xxxx and force them listen
///     2. We send data for second MCP4xxxx, it will be stored in first MCP4xxxx
///     3. We send data for first MCP4xxxx, previous data will be pushed to second MCP4xxxx
///     and first one will get it's own data
///     4. Setting HIGH on CS releases MCP4xxxx so both of them can act
/// \param address1
/// \param address2
/// \param val1
/// \param val2
///
void MCP4xxxxDaisyChainWrite(byte address1, byte address2, byte val1, byte val2) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(address2); // NOLINT
  SPI.transfer(val2); // NOLINT
  SPI.transfer(address1); // NOLINT
  SPI.transfer(val1); // NOLINT
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
}

void loop() {
  accelValue = analogRead(PIN_ACCEL);
  speedValue = getSpeed(accelValue);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_SLOT_0_ADDRESS, POTENTIOMETER_SLOT_0_ADDRESS, speedValue, speedValue);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_SLOT_1_ADDRESS, POTENTIOMETER_SLOT_1_ADDRESS, speedValue, speedValue);
  delay(50);
}