#include <Q2HX711.h>
#include "notes.h"
#include <Wire.h>


const byte hx711_data_pin = 2;
const byte hx711_clock_pin = 3;

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);

#define LEDPIN      13
#define CALPIN      9
#define ZEROMAXPIN  8
#define SPEAKER     A0
#define CALDELAY    200

float CalZero = 0.0;
float CalMax = 30.0;
// 5kg == 103.35
// 10kg == 210.36
float Ratio = 21.036; // Figured out by trial and error.

float LowZone = 0.0;
float MidZone = 0.0;
float TopZone = 0.0;


void setup()
{
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  pinMode(CALPIN, INPUT_PULLUP);
  pinMode(ZEROMAXPIN, INPUT_PULLUP);
  pinMode(SPEAKER, OUTPUT);
  digitalWrite(SPEAKER, LOW);

  CalibrateZero(8);

  LowZone = CalMax / 4;
  MidZone = CalMax / 2;
  TopZone = LowZone + MidZone;

  Serial.println(Weight());
}


void loop()
{
  float cWeight;

  digitalWrite(LEDPIN, HIGH);
  while (!digitalRead(CALPIN))
  {
    if (digitalRead(ZEROMAXPIN))
    {
          CalibrateZero(8);
    }
    else
    {
          CalibrateMax(8);
    }
  }

  cWeight = Weight();
  Serial.print("Weight :");
  // Serial.println((hx711.read()/1000.0)-CalZero);
  Serial.println(cWeight);
  Serial.print("\r");

  if (cWeight > TopZone)
  {
    Beep(NOTE_C5);
  }
  else if (cWeight > MidZone)
  {
    Beep(NOTE_C4);
  }
  else if (cWeight > LowZone)
  {
    Beep(NOTE_C3);
  }

  delay(100);
  digitalWrite(LEDPIN, LOW);
  delay(100);
}


void Beep(int Note)
{
    tone(SPEAKER, Note);
    delay(50);
    noTone(SPEAKER);
    delay(50);
    tone(SPEAKER, Note);
    delay(100);
    noTone(SPEAKER);
}


void CalZeroTuneStart(void)
{
  for(int Count = 0; (Count < 8); Count++)
  {
    tone(SPEAKER, NOTE_C3);
    delay(10);
    tone(SPEAKER, NOTE_E3);
    delay(10);
  }
  noTone(SPEAKER);
}


void CalZeroTuneEnd(void)
{
  tone(SPEAKER, NOTE_C3);
  delay(200);
  noTone(SPEAKER);
}


void CalMaxTuneStart(void)
{
  for(int Count = 0; (Count < 8); Count++)
  {
    tone(SPEAKER, NOTE_C5);
    delay(10);
    tone(SPEAKER, NOTE_E5);
    delay(10);
  }
  noTone(SPEAKER);
}


void CalMaxTuneEnd(void)
{
  tone(SPEAKER, NOTE_C5);
  delay(200);
  noTone(SPEAKER);
}


float CalibrateZero(int Loops)
{
  Serial.print("Calibrating zero");
  CalZeroTuneStart();

  float Temp = 0.0;
  for(int Count = 0; (Count < Loops); Count++)
  {
      Serial.print(".");
      //if (hx711.readyToSend())
      //{
      Temp += hx711.read()/1000.0;
      //}
      delay(CALDELAY);
  }
  CalZero = Temp / Loops;
  Serial.println("E");

  Serial.print("Zero point:");
  Serial.print(CalZero);
  Serial.print(" / ");
  Serial.println(Temp);

  CalZeroTuneEnd();

  return(CalZero);
}


float CalibrateMax(int Loops)
{
  // Max can only be calculated once we have a zero point.
  if (CalZero == 0.0)
  {
      return(CalMax);
  }

  Serial.print("Calibrating max");
  CalMaxTuneStart();

  float Temp = 0.0;
  for(int Count = 0; (Count < Loops); Count++)
  {
      Serial.print(".");
      Temp += Weight();
      delay(CALDELAY);
  }
  CalMax = Temp / Loops;
  Serial.println("E");

  LowZone = CalMax / 4;
  MidZone = CalMax / 2;
  TopZone = LowZone + MidZone;

  Serial.print("Max point:");
  Serial.print(CalMax);
  Serial.print(" / ");
  Serial.print(Temp);
  Serial.print(" : ");

  Serial.print(LowZone);
  Serial.print("(low) ");
  Serial.print(MidZone);
  Serial.print("(mid) ");
  Serial.print(TopZone);
  Serial.println("(top)");

  CalMaxTuneEnd();

  return(CalMax);
}


float Weight()
{
  float HXvalue = hx711.read() / 1000.0;
  float HXweight = (CalZero - HXvalue) / Ratio;

/*
  Serial.print("HXvalue: ");
  Serial.print(HXvalue);
  Serial.print("  HXweight: ");
  Serial.print(HXweight);
*/

  return(HXweight);
}


