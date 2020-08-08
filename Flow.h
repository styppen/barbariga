#ifndef Flow_h
#define Flow_h

#include <Arduino.h>

class Flow
{
  public:
    //constructor
    Flow(int flowPin, int sampleRate);

    //member functions
    void Sample();
    void Update();
    boolean isFlowing();
    int GetPulseRate();

  private:
    int _flowPin;
    int _flowLedPin;
    int _sampleRate;
    volatile int _flowFrequency; // Measures flow sensor pulses
    unsigned int _pulseRate; // Calculated pulse/sampleRate
    unsigned long _currentTime;
    unsigned long _cloopTime;

};

#endif
