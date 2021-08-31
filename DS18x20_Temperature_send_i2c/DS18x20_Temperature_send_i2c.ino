#include <Wire.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
byte AInput = 6;
byte BInput = 7;
 
byte lastState = 0;
byte steps = 0;
int  cw = 0;
int cw_ant = 0;
byte AState = 0;
byte BState = 0;
byte State = 0;
int LDR=0;
int Humedad=0;
float celsius, fahrenheit;

void setup() {
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent);
 sensors.begin(); 
 pinMode(AInput, INPUT);
  pinMode(BInput, INPUT);
}

void loop() { 
  LDR = map(analogRead(A0),0,1024,100, 200);
  Humedad = map(analogRead(A1),0,1024,100, 200);
  AState = digitalRead(AInput);
  BState = digitalRead(BInput) << 1;
  State = AState | BState;
 
  if (lastState != State){
    switch (State) {
      case 0:
        if (lastState == 2){
          steps++;
          cw = 1;
        }
        else if(lastState == 1){
          steps--;
          cw = 2;
        }
        break;
      case 1:
        if (lastState == 0){
          steps++;
          cw = 1;
        }
        else if(lastState == 3){
          steps--;
          cw = 2;
        }
        break;
      case 2:
        if (lastState == 3){
          steps++;
          cw = 1;
        }
        else if(lastState == 0){
          steps--;
          cw = 2;
        }
        break;
      case 3:
        if (lastState == 1){
          steps++;
          cw = 1;
        }
        else if(lastState == 2){
          steps--;
          cw = 2;
        }
        break;
    }
  }
  lastState = State;
  delay(1);
}

void requestEvent() {
  Wire.print(cw);
  Wire.print(LDR);
  Wire.print(Humedad);
}
