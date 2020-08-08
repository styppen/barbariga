/*
 * Valve.cpp - Library for handling solenoid valves
 * Create by Styppen, March 2017
 */

#include <Arduino.h>
#include "Valve.h"

#define OFF HIGH
#define ON LOW

Valve::Valve(int pin, int valveType)
{
  pinMode(pin, OUTPUT);
  _valvePin = pin;
  _valveType = valveType;
  Init();
}

void Valve::Init()
{
  if (_valveType == Valve::TYPE_NC)
  {
    _open = LOW;
    _closed = HIGH;
  }
  else if (_valveType == Valve::TYPE_NO)
  {
    _open = HIGH;
    _closed = LOW;
  }

  // we want to init the valve default state
  _valveState = _valveType == Valve::TYPE_NC ? _closed : _open;
  _actionState = Valve::STATE_IDLE;
  _previousMillis = 0;
  digitalWrite(_valvePin, _valveState);
}

void Valve::Open()
{
  _valveState = _open;
  digitalWrite(_valvePin, _valveState);
  _actionState = Valve::STATE_ACTIVE;
}

void Valve::Close()
{
  _valveState = _closed;
  digitalWrite(_valvePin, _valveState);
  _actionState = Valve::STATE_IDLE;

}

void Valve::Engage()
{
  _valveState = LOW;
  _actionState = Valve::STATE_ACTIVE;
  digitalWrite(_valvePin, _valveState);
}

void Valve::Disengage()
{
  _valveState = HIGH;
  _actionState = Valve::STATE_IDLE;
  digitalWrite(_valvePin, _valveState);
}

String Valve::GetState()
{
  return _valveState == _open ? "ODPRT" : "ZAPRT";
}
