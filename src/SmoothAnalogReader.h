//
// Created by Alexey Likhachev on 2019-01-08.
//

#ifndef TESTP01_SMOOTHANALOGREADER_H
#define TESTP01_SMOOTHANALOGREADER_H

#define READINGS_COUNT 10

class SmoothAnalogReader {
 private:
  uint8_t _pin = 0;
  int _readings[READINGS_COUNT] = {};
  int _total = 0;
  int _index = 0;
  int _value = 0;
 public:
  explicit SmoothAnalogReader(uint8_t pin);
  int ReadValue();
};

#endif //TESTP01_SMOOTHANALOGREADER_H
