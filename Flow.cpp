#include <Arduino.h>
#include "Flow.h"

Flow::Flow(int flowPin, int sampleRate)
{
  _flowPin = flowPin;
  digitalWrite(_flowPin, HIGH);
  _currentTime = millis();
  _cloopTime = _currentTime;
  _sampleRate = sampleRate;
}

void Flow::Sample()
{
  _flowFrequency++;
}

void Flow::Update()
{
  _currentTime = millis();
  if(_currentTime >= (_cloopTime + _sampleRate))
  {
    _cloopTime = _currentTime;
    _pulseRate = _flowFrequency; // Pulses per second  (_flowFrequency * 60 / 7.5);
    _flowFrequency = 0;
  }
}

boolean Flow::isFlowing()
{
  return _pulseRate > 0;
}

int Flow::GetPulseRate()
{
  return _pulseRate;
}
