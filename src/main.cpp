#include "Arduino.h"
#include "SPI.h"
#include "SmoothAnalogReader.h"

#define PIN_CS 10 //Chip-select pin for hardware SPI
#define PIN_ACCELERATION A0
#define PIN_BRAKE A1
#define PIN_STEERING_WHEEL_ANGLE A2
#define PIN_STEERING_RACK_ANGLE A3
#define PIN_STEERING_STEPPER_ENABLE 8
#define PIN_STEERING_STEPPER_DIRECTION 7
#define PIN_STEERING_STEPPER_STEP 6
#define PIN_STEERING_STEPPER_SLEEP 5
#define PIN_STEERING_STEPPER_RESET 4

#define POTENTIOMETER_LEFT_ADDRESS 0b00010001
#define POTENTIOMETER_RIGHT_ADDRESS 0b00010010
//#define POTENTIOMETER_SLOT_ALL_ADDRESS 0b00010011

const byte MAX_BYTE = 255;
const double MIN_BRAKE = 0.02f;
const double TRUESPEED_DEADZONE_HIGH = 253.0f;
const double TRUESPEED_DEADZONE_HIGH_VALUE = 255.0f;
const double TRUESPEED_DEADZONE_LOW = 3.0f;
const double TRUESPEED_DEADZONE_LOW_VALUE = 0.0f;
const double MIN_SPEED_DELTA = 0.01f;
const double FRAME = 30.0f;

unsigned long up_time = 0;
unsigned long fresh_up_time = 0;
unsigned long tick = 0;
SmoothAnalogReader accelerator(PIN_ACCELERATION);
SmoothAnalogReader brakes(PIN_BRAKE);
SmoothAnalogReader steering_wheel(PIN_STEERING_WHEEL_ANGLE);
SmoothAnalogReader steering_rack(PIN_STEERING_RACK_ANGLE);
int acceleration_rate = 0;
int braking_rate = 0;
int steering_wheel_value = 0;
int steering_rack_value = 0;
int steering_delta = 0;
byte current_set_resistance = 0;
double current_true_set_speed = 0.0f;
double acceleration_intensity = 0.5f;
double natural_slowdown_intensity = 0.001f;
double braking_slowdown_efficiency = 0.1f;

///
/// This method writes data to two Daisy-Chain connected MCP4xxxx via SPI.
///     1. Setting LOW on CS to take control over MCP4xxxx and force both of them to listen
///     2. Sending data for second MCP4xxxx (it will be stored in first MCP4xxxx for now)
///     3. Sending data for first MCP4xxxx (data stored in first MCP4xxxx will be pushed to the second MCP4xxxx
///     and first one will get new data)
///     4. Setting HIGH on CS to release MCP4xxxx's and let them act
///
void MCP4xxxxDaisyChainWrite(byte address_front, byte address_rear, byte value_front, byte value_rear) {
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(address_rear); // NOLINT
  SPI.transfer(value_rear); // NOLINT
  SPI.transfer(address_front); // NOLINT
  SPI.transfer(value_front); // NOLINT
  digitalWrite(PIN_CS, HIGH);
}

///
/// Takes in acceleration rate, which is inverted analog pin PIN_ACCELERATION value
/// It is inverted because we need rate of pedal pressed
/// Higher pedal is pressed, lower the input current and vice versa
/// This one returns Target Speed that we want to achieve pressing gas pedal
///
inline __attribute__((always_inline)) double GetTargetSpeedFromAccellerationRate(int acceleration_rate) {
  return ((acceleration_rate)/1023.0f)*255.0f;
}

///
/// Takes in braking rate, which is inverted analog pin PIN_BRAKE value
/// It is inverted because we need rate of pedal pressed
///
inline __attribute__((always_inline)) double GetBrakingIntensityFromBrakingRate(int braking_rate) {
  return (braking_rate/1023.0f);
}

///
/// Takes in speed and returns speed limited by deadzones
///
inline __attribute__((always_inline)) double GetTrueSetSpeedDeadzoned(double speed, double deadzone_min, double deadzone_min_value,
    double deadzone_max, double deadzone_max_value) {
  if (speed > deadzone_max) {
    return deadzone_max_value;
  }
  else if (speed < deadzone_min) {
    return deadzone_min_value;
  } else
    return speed;
}

///
/// Takes a number of parameters and calculated new current speed
///
double GetNewCurrentSpeed(int acceleration_rate, int braking_rate, double current_speed,
    double acceleration_intensity, double natural_slowdown_intensity, double braking_slowdown_efficiency) {
  double target_speed_acl = GetTargetSpeedFromAccellerationRate(acceleration_rate);
  double braking_intensity = GetBrakingIntensityFromBrakingRate(braking_rate);
  double new_current;
  double speed_delta_by_acl = abs(target_speed_acl - current_speed);
  if ((braking_intensity > MIN_BRAKE) && (current_speed > 0.0f)) {
    new_current = current_speed - current_speed * braking_intensity * braking_slowdown_efficiency;
  }
  else if (target_speed_acl < current_speed) {
    if (speed_delta_by_acl > MIN_SPEED_DELTA) {
      new_current = current_speed - speed_delta_by_acl * natural_slowdown_intensity;
    } else {
      new_current = current_speed - speed_delta_by_acl;
    }
  }
  else if (target_speed_acl > current_speed) {
    if (speed_delta_by_acl > MIN_SPEED_DELTA) {
      new_current = current_speed + speed_delta_by_acl * acceleration_intensity;
    } else {
      new_current = current_speed + speed_delta_by_acl;
    }
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

inline __attribute__((always_inline)) void Tick() {
  fresh_up_time = millis();
  tick = fresh_up_time - up_time;
  up_time = fresh_up_time;
}

double GetAccelerationIntensity() {
  return (acceleration_intensity/FRAME)*tick;
}

double GetNaturalSlowdownIntensity() {
  return (natural_slowdown_intensity/FRAME)*tick;
}

double GetBrakingSlowdownEfficiency() {
  return (braking_slowdown_efficiency/FRAME)*tick;
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CS, OUTPUT);
  SPI.begin(); // NOLINT
  pinMode(PIN_ACCELERATION, INPUT);
  pinMode(PIN_BRAKE, INPUT);
  pinMode(PIN_STEERING_WHEEL_ANGLE, INPUT);
  pinMode(PIN_STEERING_RACK_ANGLE, INPUT);
  pinMode(PIN_STEERING_STEPPER_ENABLE, OUTPUT);
  pinMode(PIN_STEERING_STEPPER_DIRECTION, OUTPUT);
  pinMode(PIN_STEERING_STEPPER_STEP, OUTPUT);
  pinMode(PIN_STEERING_STEPPER_SLEEP, OUTPUT);
  pinMode(PIN_STEERING_STEPPER_RESET, OUTPUT);
  digitalWrite(PIN_STEERING_STEPPER_RESET, HIGH);
  digitalWrite(PIN_STEERING_STEPPER_SLEEP, HIGH);
  digitalWrite(PIN_STEERING_STEPPER_ENABLE, LOW);
  Tick();
}

void loop() {
  acceleration_rate = InvertAnalogPinValue(accelerator.ReadValue());
  braking_rate = InvertAnalogPinValue(brakes.ReadValue());
  steering_wheel_value = steering_wheel.ReadValue();
  steering_rack_value = steering_rack.ReadValue();
  current_true_set_speed = GetNewCurrentSpeed(acceleration_rate, braking_rate, current_true_set_speed,
          GetAccelerationIntensity(), GetNaturalSlowdownIntensity(), GetBrakingSlowdownEfficiency());
  current_set_resistance = InvertByte((byte)ceil(GetTrueSetSpeedDeadzoned(current_true_set_speed,
                                                                        TRUESPEED_DEADZONE_LOW,
                                                                        TRUESPEED_DEADZONE_LOW_VALUE,
                                                                        TRUESPEED_DEADZONE_HIGH,
                                                                        TRUESPEED_DEADZONE_HIGH_VALUE)));
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_LEFT_ADDRESS, POTENTIOMETER_LEFT_ADDRESS, current_set_resistance,
      current_set_resistance);
  MCP4xxxxDaisyChainWrite(POTENTIOMETER_RIGHT_ADDRESS, POTENTIOMETER_RIGHT_ADDRESS, current_set_resistance,
      current_set_resistance);
  steering_delta = steering_rack_value - steering_wheel_value;
  if (steering_delta > 20) {
    digitalWrite(PIN_STEERING_STEPPER_SLEEP, HIGH);
    digitalWrite(PIN_STEERING_STEPPER_DIRECTION, LOW);
    digitalWrite(PIN_STEERING_STEPPER_STEP, HIGH);
    delayMicroseconds(500);
    digitalWrite(PIN_STEERING_STEPPER_STEP, LOW);
  }
  else if (steering_delta < -20) {
    digitalWrite(PIN_STEERING_STEPPER_SLEEP, HIGH);
    digitalWrite(PIN_STEERING_STEPPER_DIRECTION, HIGH);
    digitalWrite(PIN_STEERING_STEPPER_STEP, HIGH);
    delayMicroseconds(500);
    digitalWrite(PIN_STEERING_STEPPER_STEP, LOW);
  } else {
    digitalWrite(PIN_STEERING_STEPPER_SLEEP, LOW);
  }
  Tick();
}