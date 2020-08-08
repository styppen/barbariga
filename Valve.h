/*
 * Valve.h - Library for handling solenoid valves
 * Create by Styppen, March 2017
 */

#ifndef Valve_h
#define Valve_h

#include <Arduino.h>

class Valve
{
  public:

    // system state constants
    static const int STATE_IDLE = 0;
    static const int STATE_ACTIVE = 1;
    static const int STATE_FORCESTOP = 2;

    // valve types constants
    static const int TYPE_NC = 0;
    static const int TYPE_NO = 1;

    // constructor
    Valve(int valvePin, int valveType);

    // member functions

    void Open();
    void Close();
    void Engage();
    void Disengage();
    String GetState();

  private:
    int _valvePin;
    int _valveType;

    int _valveState;
    int _actionState;

    unsigned long _OnTime;
    unsigned long _previousMillis;

    int _open;
    int _closed;

    void Init();
};

#endif
