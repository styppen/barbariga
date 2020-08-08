#include <Arduino.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Valve.h"
#include "System.h"
#include "Flow.h"
#include "Pump.h"


// tolerances for begin/end pumping
#define PUMP_BEGIN_TOLERANCE 1
#define PUMP_END_TOLERANCE 0

// consume status transitions thresholds
#define CONSUME_FROM_TOP  40
#define CONSUME_FROM_PIPE 35

#define GO_IN_PREHEAT_THRES 40

// device configuration
const int disp  = 2;
const int pinF1 = 3;
const int oneWireBus = 4;
const int pinV1 = 5;
const int pinV2 = 6;
const int pinV3 = 8;
const int pinP1 = 7;
const int redButton = 13;

const int MODE_TEMP = 0;
const int MODE_CONSUME = 1;

const int FLOW_SAMPLE_RATE = 500;

const unsigned long lcdRefreshRate = 60; // seconds

volatile int CONSUME_STATUS = -1;

int DISPLAY_MODE = MODE_TEMP;
boolean dispChange = false;
boolean testFlow = false;

float temp1, temp2, temp3;
const unsigned long TEMP_POLL_RATE = 30; //seconds
unsigned long lastTempCheck = 30*1000;
volatile boolean consumePlus = false;
volatile boolean preheat = false;

unsigned long preheatStart = 0;
const unsigned long PREHEAT_TIME = 240*1000; // 240 seconds = 4 min

// valve control objects
Valve v1(pinV1, Valve::TYPE_NO);
Valve v2(pinV2, Valve::TYPE_NO);
Valve v3(pinV3, Valve::TYPE_NC);

// system control object
System sys(0,0,0,0);

// flow control object
Flow f1(pinF1, FLOW_SAMPLE_RATE);

// pump control object
Pump p1(pinP1);

DeviceAddress Probe01 = { 0x28, 0xFF, 0x99, 0xC7, 0x64, 0x15, 0x02, 0x27 };
DeviceAddress Probe02 = { 0x28, 0xFF, 0xF9, 0xA7, 0x64, 0x15, 0x01, 0x44 };
DeviceAddress Probe03 = { 0x28, 0xFF, 0x9E, 0xAF, 0x64, 0x15, 0x01, 0x18 };
DeviceAddress Probe04 = { 0x28, 0xFF, 0x22, 0xD0, 0x64, 0x15, 0x01, 0x44 };
DeviceAddress Probe05 = { 0x28, 0xFF, 0xD2, 0xAC, 0x64, 0x15, 0x01, 0x58 };

OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
unsigned long lcdLastInit = 0;

// count how many pulses!
volatile uint16_t pulses = 0;
// track the state of the pulse pin
volatile uint8_t lastflowpinstate;
// you can try to keep time of how long it is between pulses
volatile uint32_t lastflowratetimer = 0;
// and use that to calculate a flow rate
volatile float flowrate;
// Interrupt is called once a millisecond, looks for any pulses from the sensor!
SIGNAL(TIMER0_COMPA_vect)
{
  uint8_t x = digitalRead(pinF1);
  if (x == lastflowpinstate)
  {
    lastflowratetimer++;
    return; // nothing changed!
  }

  if (x == HIGH)
  {
    //low to high transition!
    pulses++;
    f1.Sample();
  }

  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;  // in hertz
  lastflowratetimer = 0;
}

void useInterrupt(boolean v)
{
  if (v)
  {
    // Timer0 is already used for millis() - we'll just interrupt somewhere
    // in the middle and call the "Compare A" function above
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  }
  else
  {
    // do not call the interrupt function COMPA anymore
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void flow () // Interrupt function
{
   f1.Sample();
}

void flowTest()
{
  testFlow = !testFlow;
}

void display()
{
  dispChange = true;
}

void toggleConsumePlus()
{
  consumePlus = !consumePlus;
}

void togglePreheat()
{
  preheat = !preheat;
}

void reset()
{
  v1.Disengage();
  v2.Disengage();
  v3.Disengage();
  p1.Disable();
}

void transitToState(int futureState)
{

  /**** CURRENT_STATE == FUTURE_STATE ****/
  if (sys.GetState() == futureState)
  {
    // we don't want to do anything if current and future states are the same
    return;
  }
  else
  {
    // transition into a new state
    reset();
  }

  /**** READY --> PUMPING ****/
  if (sys.GetState() == System::READY && futureState == System::PUMPING)
  {
    Serial.println("READY -> PUMPING");
    p1.Enable();
  }

  /**** READY -> CONSUME ****/
  else if (sys.GetState() == System::READY && futureState == System::CONSUME)
  {
    Serial.println("READY -> CONSUME");
  }

  /**** PUMPING -> READY ****/
  else if (sys.GetState() == System::PUMPING && futureState == System::READY)
  {
    Serial.println("PUMPING -> READY");
    p1.Disable();
  }

  /**** PUMPING -> CONSUME ****/
  else if (sys.GetState() == System::PUMPING && futureState == System::CONSUME)
  {
    Serial.println("PUMPING -> CONSUME");
    p1.Disable();
  }

  /**** CONSUME -> READY ****/
  else if (sys.GetState() == System::CONSUME && futureState == System::READY)
  {
    Serial.println("CONSUME -> READY");
    reset();
  }

  /**** CONSUME -> PUMPING ****/
  else if (sys.GetState() == System::CONSUME && futureState == System::PUMPING)
  {
    Serial.println("CONSUME -> PUMPING");
    reset();
    p1.Enable();
  }

  /**** READY -> PREHEAT ****/
  else if (sys.GetState() == System:: READY && futureState == System::PREHEAT) {
    Serial.println("READY -> PREHEAT");
    reset();
    p1.Enable();
    v1.Engage();
    v3.Engage();
    preheatStart = millis();
  }

  sys.SetState(futureState);

}

void setup()
{
  Serial.begin(9600);
  sensors.begin();
  sensors.setResolution(Probe01, 10);
  sensors.setResolution(Probe05, 10);
  sensors.setResolution(Probe04, 10);

  pinMode(disp, INPUT);
  pinMode(pinF1, INPUT);
  digitalWrite(pinF1, HIGH);
  lastflowpinstate = digitalRead(pinF1);
  useInterrupt(true);

  lcd.begin(20, 4);
  for(int i = 0; i < 3; i++)
  {
    lcd.backlight();
    delay(100);
    lcd.noBacklight();
    delay(100);
  }
  lcd.backlight(); // finish with backlight on
}

void loop()
{
  // ******* 1. prepare inputs for reading *******
  sensors.requestTemperatures();
  f1.Update();

  // ******* 2. read values from sensors *******
  if ((millis() - lastTempCheck) > (TEMP_POLL_RATE * 1000))
  {
    temp1 = sensors.getTempC(Probe03);
    temp2 = sensors.getTempC(Probe02);
    temp3 = sensors.getTempC(Probe04);
    lastTempCheck = millis();
  }
  int pulseRate = f1.GetPulseRate();

  // read the button state
  int buttonState = digitalRead(disp);
  int redButtonState = digitalRead(redButton);
  if (buttonState == HIGH)
  {
    //display();
    toggleConsumePlus();
  }

  /*** PREHEAT LOGIC ***/
  // when red button is pressed, preheat is started
  // it lasts for 4 mins
  if (redButtonState == HIGH)
  {
    togglePreheat();
  }

  // check if we need to reinit the LCD display
  if((millis() - lcdLastInit) > (lcdRefreshRate * 1000))
  {
    lcd.begin(20, 4);
    lcdLastInit = millis();
    Serial.println("Screen was reinitialised!");
  }

  // ******* 3. process the sensor values and act accordingly *******
  if (pulseRate > 1)
  {
    // State::CONSUME mode has a higher priority and must therefore be handled first
    transitToState(System::CONSUME);
    int previousState = CONSUME_STATUS;

    // determine what do we have to do next
    if (consumePlus)
    {
      CONSUME_STATUS = 4;
    }
    else if (temp2 > CONSUME_FROM_TOP)
    {
      if(temp3 >= CONSUME_FROM_PIPE)
      {
        CONSUME_STATUS = 3;
      }
      else
      {
        CONSUME_STATUS = 2;
      }
    }
    else
    {
      CONSUME_STATUS = 1;
    }

    // handle before transitioning to new state
    if (previousState != CONSUME_STATUS)
    {
      reset();
    }

    if (CONSUME_STATUS == 1)
    {
      v2.Close();
    }
    else if(CONSUME_STATUS == 2)
    {
      v2.Close();
      v3.Open();
      p1.Enable();
    }
    else if (CONSUME_STATUS == 3)
    {
      v1.Close();
    }
    else if (CONSUME_STATUS == 4)
    {
      v1.Engage();
    }
  }
  else
  {
    // no flow is detected
    float tempDiff = temp2 - temp1;
    if (tempDiff >= PUMP_BEGIN_TOLERANCE)
    {
      transitToState(System::PUMPING);
    }
    else if (preheat)
    {
      transitToState(System::PREHEAT);
    }
    else
    {
      transitToState(System::READY);
    }
  }

  // NOTE: Cursor Position: Lines and Characters start at 0
  /*if (dispChange)
  {
    DISPLAY_MODE++;
    DISPLAY_MODE = DISPLAY_MODE%2;
    lcd.clear();
    dispChange = !dispChange;
  }*/


  if (DISPLAY_MODE == MODE_TEMP)
  {
    lcd.setCursor(0, 0);
    lcd.print("Status: ");
    lcd.setCursor(8, 0);

    if (sys.GetState() == System::READY)
    {
        lcd.print("PRIPRAVLJEN");
    }
    else if (sys.GetState() == System::CONSUME)
    {
      lcd.print("PORABA     ");
    }
    else if(sys.GetState() == System::PUMPING)
    {
      lcd.print("CRPANJE    ");
    }

    lcd.setCursor(0,1); //Start at character 4 on line 0
    lcd.print("SPODAJ=");
    lcd.setCursor(8,1);
    lcd.print(temp1);
    lcd.setCursor(13,1);
    lcd.print("C");

    lcd.setCursor(0,2); //Start at character 4 on line 0
    lcd.print("ZGORAJ=");
    lcd.setCursor(8,2);
    lcd.print(temp2);
    lcd.setCursor(13,2);
    lcd.print("C");

    lcd.setCursor(0,3); //Start at character 4 on line 0
    lcd.print("   CEV=");
    lcd.setCursor(8,3);
    lcd.print(temp3);
    lcd.setCursor(13,3);
    lcd.print("C");

    if(consumePlus)
    {
      lcd.setCursor(19, 3);
      lcd.print("+");
    }
    else
    {
      lcd.setCursor(19, 3);
      lcd.print(" ");
    }

  }
  else if (DISPLAY_MODE == MODE_CONSUME)
  {
    lcd.setCursor(0, 0);
    lcd.print("V1=");
    lcd.setCursor(3, 0);
    lcd.print(v1.GetState());

    lcd.setCursor(0, 1);
    lcd.print("V2=");
    lcd.setCursor(3, 1);
    lcd.print(v2.GetState());

    lcd.setCursor(0, 2);
    lcd.print("V3=");
    lcd.setCursor(3, 2);
    lcd.print(v3.GetState());

    lcd.setCursor(0, 3);
    lcd.print("P1=");
    lcd.setCursor(3, 3);
    lcd.print(p1.GetState());

    lcd.setCursor(19, 0);
    lcd.print(CONSUME_STATUS);

    lcd.setCursor(0, 2);
    lcd.print("V");

  }
}
