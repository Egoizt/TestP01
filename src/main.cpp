#include "Arduino.h"
#include "SPI.h"

#define PIN_CS 10 //Chip-select pin for hardware SPI
#define PIN_ACCELERATION A0
#define PIN_STEERING A1

#define POTENTIOMETER_LEFT_ADDRESS 0b00010001
#define POTENTIOMETER_RIGHT_ADDRESS 0b00010010
//#define POTENTIOMETER_SLOT_ALL_ADDRESS 0b00010011

const byte MAX_BYTE = 255;

int accelerationRate;
int steeringValue;
byte currentSetSpeed = 0;
double currentTrueSetSpeed = 0.0f;
double accelerationSmoothness = 0.05f;
double naturalSlowdownSmoothness = 0.5f;

///
/// This method writes data to two Dais-Chain connected MCP4xxxx via SPI.
///     1. Setting LOW on CS taking control over MCP4xxxx and force them listen
///     2. We send data for second MCP4xxxx, it will be stored in first MCP4xxxx
///     3. We send data for first MCP4xxxx, previous data will be pushed to second MCP4xxxx
///     and first one will get it's own data
///     4. Setting HIGH on CS releases MCP4xxxx so both of them can act
/// \param address_front
/// \param address_rear
/// \param value_front
/// \param value_rear
///
void MCP4xxxxDaisyChainWrite(byte address_front, byte address_rear, byte value_front, byte value_rear) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(address_rear); // NOLINT
  SPI.transfer(value_rear); // NOLINT
  SPI.transfer(address_front); // NOLINT
  SPI.transfer(value_front); // NOLINT
  digitalWrite(PIN_CS, HIGH);
}

double GetTargetSpeedFromAccellerationRate(int acceleration_rate) {
  return ((acceleration_rate)/1023.0f)*255.0f;
}

double GetNewCurrentSpeed(int acceleration_rate, double current_speed, double acceleration_smoothness,
    double natural_slowdown_smoothness) {
  double target = GetTargetSpeedFromAccellerationRate(acceleration_rate);
  double new_current;
  if (target < current_speed) {
    new_current = current_speed - (current_speed - target) * natural_slowdown_smoothness;
  }
  else if (target > current_speed) {
    new_current = current_speed + (target - current_speed) * acceleration_smoothness;
  }
  else
    new_current = current_speed;
  return new_current;
}

int InvertAnalogPinValue(int value) {
  return 1023 - value;
}

byte InvertByte(byte value) {
  return MAX_BYTE - value;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CS, OUTPUT);
  SPI.begin(); // NOLINT
  pinMode(PIN_ACCELERATION, INPUT);
  pinMode(PIN_STEERING, INPUT);
}

void loop() {
  accelerationRate = InvertAnalogPinValue(analogRead(PIN_ACCELERATION));
  steeringValue = analogRead(PIN_STEERING);
  currentTrueSetSpeed = GetNewCurrentSpeed(accelerationRate, currentTrueSetSpeed, accelerationSmoothness,
      naturalSlowdownSmoothness);
  currentSetSpeed = InvertByte((byte)currentTrueSetSpeed);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_LEFT_ADDRESS, POTENTIOMETER_LEFT_ADDRESS, currentSetSpeed, currentSetSpeed);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_RIGHT_ADDRESS, POTENTIOMETER_RIGHT_ADDRESS, currentSetSpeed, currentSetSpeed);
  delay(50);
}