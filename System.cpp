/*
 * Sysyem.h - Model of the system's state and configuration
 * Create by Styppen, March 2017
 */

#include <Arduino.h>
#include "System.h"

System::System(int offPin, int idlePin, int pumpingPin, int consumePin)
{
  _offPin = offPin;
  _idlePin = idlePin;
  _pumpingPin = pumpingPin;
  _consumePin = consumePin;
  SetState(System::READY);

  /*pinMode(_offPin, OUTPUT);
  pinMode(_idlePin, OUTPUT);
  pinMode(_pumpingPin, OUTPUT);
  pinMode(_consumePin, OUTPUT);*/
}

void System::SetState(int state)
{
  _systemState = state;
  /*digitalWrite(_offPin, LOW);
  digitalWrite(_idlePin, LOW);
  digitalWrite(_pumpingPin, LOW);
  digitalWrite(_consumePin, LOW);
  if(_systemState == System::OFF)
  {
    digitalWrite(_offPin, HIGH);
  }
  else if(_systemState == System::READY)
  {
    digitalWrite(_idlePin, HIGH);
  }
  else if(_systemState == System::PUMPING)
  {
    digitalWrite(_pumpingPin, HIGH);
  }
  else if(_systemState == System::CONSUME)
  {
    digitalWrite(_consumePin, HIGH);
  }*/
}

int System::GetState()
{
  return _systemState;
}
