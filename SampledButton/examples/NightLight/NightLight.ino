#include <SampledButton.h>

#include "Arduino.h"

int count = 0;
int leds[] = {0,1,4}; // pins 0,1 and 4 have pwm
int intensities[] = {0,0,0};
SampledButton button = SampledButton(2);
int threshold = 400;
int lightThreshold = -1;
int lightSensor = 3; // pin 3 has an adc
int cursor = 0;
unsigned long int pressedSince=0;
unsigned long int switchedStateSince=0;

bool isOn = true;

void nextColor(){
  cursor++;
  cursor = cursor % 3; 
}

void increaseIntensity(){
  intensities[cursor] = ((intensities[cursor] + 51) % (256 + 50))-50;
  if(intensities[cursor] > 0){
    analogWrite(leds[cursor], intensities[cursor]);
  }else{
    digitalWrite(leds[cursor], LOW);
  }
}

bool doSwitch(){
  unsigned long int now = millis();
  bool result = (now - switchedStateSince) > 120000;
  if(result)
    switchedStateSince = now;
  return result;
}

void allOff(){
  if(!doSwitch())
    return;
  int i;
  for(i = 0; i < 3; i++){
    digitalWrite(leds[i], LOW); 
  }
 
  isOn = false;
}
void restoreIntensities(){
  if(!doSwitch())
    return;
  int i;
  for(i = 0; i < 3; i++){
    analogWrite(leds[i], intensities[i]); 
  }
  isOn = true;
}

void clicked(bool up, int deltaT){
  if(up && deltaT < threshold)
    nextColor();
}

void pressed(bool up, int deltaT){
  if (lightThreshold == -1)
    lightThreshold = analogRead(lightSensor);
  if(deltaT > threshold)
    increaseIntensity();
}
void setup(){
int i;
  for(i = 0; i < 3; i++){
    pinMode(leds[i], OUTPUT); 
  }
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  
  pinMode(lightSensor, OUTPUT);
  digitalWrite(lightSensor, HIGH);
  
  button.setClickEventHandler(clicked);
  button.setPressEventHandler(pressed);

}

void loop(){
  count++;
  button.sample();
  if(count % 2000 == 0){
    if(lightThreshold > analogRead(lightSensor)){
      if(isOn)
        allOff();
    }else{
      if(!isOn)
        restoreIntensities();
    }
  }
  delay(1);
}
