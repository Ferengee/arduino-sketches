#include "SampledButton.h"

SampledButton::SampledButton(int buttonPin)
{
  _buttonPin = buttonPin;
  down = LOW;
  _sampleCount=0;
  pressedTriggerInterval=10;
}


void SampledButton::sample()
{
  int i;
  _sampleCount++;
  int currentValue = digitalRead(_buttonPin);
  for(i = 1; i < SDB_BUFFERLENGTH; i++){
    _sampleBuffer[i-1] = _sampleBuffer[i];
  }
  _sampleBuffer[SDB_BUFFERLENGTH - 1] = currentValue;
  bool complete = true;
  for(i = 0; i < SDB_BUFFERLENGTH; i++){
    if (_sampleBuffer[i] != currentValue){
      
      complete = false;
      break;
    }
  }
  unsigned long int now = millis();
  if (complete){
    unsigned long int deltaT = now - _lastChange;
    if(_lastState != currentValue){
      _lastState = currentValue;
      
      if(_clicked != NULL)
	_clicked(currentValue, deltaT);
      _lastChange = now ;
    }else{
      if(currentValue == down && (_sampleCount % pressedTriggerInterval == 0) &&  (_pressed != NULL))
        _pressed(currentValue, deltaT);
    }
  }
  
}

void SampledButton::setClickEventHandler(ButtonEventHandler eh)
{
  _clicked = eh;
}
void SampledButton::setPressEventHandler(ButtonEventHandler eh)
{
  _pressed = eh;
}
 