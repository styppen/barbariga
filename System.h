/*
 * Sysyem.h - Model of the system's state and configuration
 * Create by Styppen, March 2017
 */

#ifndef System_h
#define System_h

#include <Arduino.h>

class System
{
  public:
    // constructor
    System(int offPin, int idlePin, int pumpingPin, int consumePin);

    // member functions
    void SetState(int state);
    int  GetState();

    // system state constants
    static const int OFF = 0;
    static const int READY = 1;
    static const int PUMPING = 2;
    static const int CONSUME = 3;
    static const int PREHEAT = 4;

  private:
    int _systemState;
    int _offPin;
    int _idlePin;
    int _pumpingPin;
    int _consumePin;
};

#endif
