//
// Created by Alexey Likhachev on 2019-01-08.
//

#include "Arduino.h"
#include "SmoothAnalogReader.h"

SmoothAnalogReader::SmoothAnalogReader(uint8_t pin) {
  _pin = pin;
}

int SmoothAnalogReader::ReadValue() {
  _total = _total - _readings[_index];
  _readings[_index] = analogRead(_pin);
  _total += _readings[_index];
  _index++;
  if (_index >= READINGS_COUNT)
    _index = 0;
  _value = _total / READINGS_COUNT;
  return _value;
}