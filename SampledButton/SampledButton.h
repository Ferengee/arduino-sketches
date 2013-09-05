#ifndef SampledButton_h
#define SampledButton_h
#include "Arduino.h"

#define SDB_BUFFERLENGTH 3
/*
 *  callback function
 *  is called if the button changes state
 * 
 */
typedef void (* ButtonEventHandler) (bool pressed, int deltaT);

class SampledButton
{
public:
  SampledButton( int buttonPin);
  void sample();
  void setClickEventHandler(ButtonEventHandler eh);
  void setPressEventHandler(ButtonEventHandler eh);

  void setLedBrightness(int brightness); //0-255
  int pressedTriggerInterval;
  bool down;
private:
  int _buttonPin;
  ButtonEventHandler _pressed;
  ButtonEventHandler _clicked;

  int _sampleBuffer[SDB_BUFFERLENGTH];
  int _lastState;
  int _sampleCount;
 
  unsigned long int _lastChange;
};
#endif