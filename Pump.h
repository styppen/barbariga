/*
 * Pump.h - Class for handling water pumps
 * Create by Styppen, March 2017
 */

#ifndef Pump_h
#define Pump_h

#include <Arduino.h>

class Pump
{
  public:

    // system state constants
    static const int STATE_IDLE = 0;
    static const int STATE_ACTIVE = 1;
    static const int STATE_FORCESTOP = 2;
    static const int STATE_TIMER = 3;

    // constructor
    Pump(int pumpPin);

    // member functions
    void Update();
    void EnableFor(long period);
    void Enable();
    void Disable();
    String GetState();
    //void ForceStop();

  private:
    int _pumpPin;

    int _pumpState;
    int _actionState;

    unsigned long _OnTime;
    unsigned long _previousMillis;

    void Init();
};

#endif
