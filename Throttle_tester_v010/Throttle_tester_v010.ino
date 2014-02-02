/*
-------------------------------------
Throttle testing circuit for ORi bike
-------------------------------------


*/

const int pinPotInput = 1;   //  pin - Voltage signal (0-5V) from pot
const int pinThrotOut = 10;  //  pin - PWM out to R-C circuit
const int pinThrotSig = 2;   //  pin - Actual throttle signal to controller
const int pinLED = 13;       //  pin - Onboard LED

int PotInput;
int ThrotOut;
int ThrotSig;
int LED = 1;

float vPotInput;  //  Pot input (Volts)
float vThrotOut;  //  Throttle output (Volts)
float vThrotSig;  //  Actual throttle signal to controller (Volts)

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
  ThrotOut = map(PotInput, 0, 1023, 255, 0);  //  Reverse map pot input to throttle output
  analogWrite(pinThrotOut, ThrotOut);

  
  ThrotSig = analogRead(pinThrotSig);  //  Read actual throttle signal voltage
  vPotInput = 5.0 * PotInput / 1023;   //  Scale to 5 Volts
  vThrotOut = 5.0 * ThrotOut / 255;    //  Scale to 5 Volts
  vThrotSig = 5.0 * ThrotSig / 1023;   //  Scale to 5 Volts
  
  //  Toggle LED output
  if(LED == 1) {
    LED = 0;
  }
  else {
    LED = 1;
  }
  digitalWrite(pinLED, LED);
  
  //  Introduce delay to get approx 50ms loops (20Hz)  
  tSys = millis();
  tDelta = tSys - tSysPrev;
  delay(50-tDelta);
  tSysPrev = millis();

  
  //  Output to serial monitor
  Serial.print("Pot = ");
  Serial.print(vPotInput, 2);
  Serial.print("   Output = ");
  Serial.print(ThrotOut);
  Serial.print(" = ");
  Serial.print(vThrotOut, 2);
  Serial.print("   Actual = ");
  Serial.print(ThrotSig);
  Serial.print(" = ");
  Serial.print(vThrotSig, 2);
  Serial.print("   tDelta = ");
  Serial.println(tDelta);
  
}
