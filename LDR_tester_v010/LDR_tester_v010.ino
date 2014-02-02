/*
--------------
LED-LDR tester
--------------
This sketch will vary the brightness of an LED coupled with an LDR to vary
the resistance across the LDR.


*/

const int pinPotInput = 1;   //  pin - Voltage signal (0-5V) from pot
const int pinLEDout = 10;    //  pin - PWM out to LED
const int pinLEDsys = 13;    //  pin - Onboard LED

int PotInput;
int LEDout;
int LED = 1;

long tSys;
long tSysPrev;
int tDelta;


void setup()
{
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  tSysPrev = millis();
  
}

void loop()
{
  PotInput = analogRead(pinPotInput);
  LEDout = map(PotInput, 0, 1023, 0, 255);  //  Reverse map pot input to throttle output
  analogWrite(pinLEDout, LEDout);

  
  
  //  Toggle LED output
  if(LED == 1) {
    LED = 0;
  }
  else {
    LED = 1;
  }
  digitalWrite(pinLEDsys, LED);
  
  //  Introduce delay to get approx 100ms loops (10Hz)  
  tSys = millis();
  tDelta = tSys - tSysPrev;
  delay(100-tDelta);
  tSysPrev = millis();

  
  //  Output to serial monitor
  Serial.print("Pot = ");
  Serial.print(PotInput);
  Serial.print("   LED = ");
  Serial.print(LEDout);
  Serial.print("   tDelta = ");
  Serial.println(tDelta);
  
}
