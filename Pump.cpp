/*
 * Pump.cpp - Library for handling solenoid Pumps
 * Create by Styppen, March 2017
 */

#include <Arduino.h>
#include "Pump.h"

#define OFF HIGH
#define ON LOW

Pump::Pump(int pumpPin)
{
  _pumpPin = pumpPin;
  pinMode(_pumpPin, OUTPUT);
  Init();
}

void Pump::Init()
{
  // we want to init the Pump default state
  _pumpState = OFF;
  _actionState = Pump::STATE_IDLE;
  _previousMillis = 0;
  digitalWrite(_pumpPin, _pumpState);
}

void Pump::Enable()
{
  _pumpState = ON;
  digitalWrite(_pumpPin, _pumpState);
  _actionState = Pump::STATE_ACTIVE;
}

void Pump::EnableFor(long period)
{
  if (_actionState == Pump::STATE_IDLE)
  {
    _OnTime = period * 1000;
    _previousMillis = millis();
    Enable();
    _actionState = Pump::STATE_TIMER;
  }
}

void Pump::Disable()
{
  _pumpState = OFF;
  digitalWrite(_pumpPin, _pumpState);
  _actionState = Pump::STATE_IDLE;
}

void Pump::Update()
{
  if (_actionState == Pump::STATE_TIMER)
  {
    // check to see if it's time to change the state of the Pump
    unsigned long currentMillis = millis();
    if ((currentMillis - _previousMillis) >= _OnTime)
    {
      if (_actionState == Pump::STATE_FORCESTOP)
      {
        // do force action
      }
      else
      {
        if (_actionState == Pump::STATE_ACTIVE)
        {
          // time is up, time to change state
          Disable();
        }
      }
    }
  }
}

String Pump::GetState()
{
  return _pumpState == ON ? "VKLOPLJENA " : "IZKLOPLJENA";
}
