/*
  This sketch reads throttle voltage (0 to 5V) from an analogue input and outputs a PWM
  signal on two outputs.
*/

// Input and output pin assignment
  // Pot input
  const int throttleInPin = A0;      // Connected to pot wiper pin.

  // Voltage smoothing circuit output
  const int throttleOutPin = 3; // PWM output pin connected to RC voltage smoother.
  
  // LED output
  const int ledOutPin = 11; // PWM output pin connected to external LED.

// Variables used
  int throttleValue = 0;
  int throttlePercent = 0;
  float throttleSetting = 0.0;
  float throttleOutVoltage = 0.0;
  int throttleOut = 0;
  int ledOut = 0;

// The setup routine runs once when you press RESET
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
}

// The loop routine runs continuously
void loop() {
  
  // Read throttle input on Analog Pin 0
  throttleValue = analogRead(throttleInPin);
  throttleSetting = throttleValue / 1023.0;
  throttlePercent = throttleSetting * 100;

  // Output PWM to drive RC voltage smoother
  // 3.9V is min throttle and 1.2V is max throttle.
  // Drive outputs between 4.0V and 1.0V.
  throttleOutVoltage = 4.0 - (throttleSetting * 3.0);
  throttleOut = throttleOutVoltage / 5 * 255;
  analogWrite(throttleOutPin, throttleOut);

  //Output PWM to drive LED
  ledOut = throttleSetting * 255;
  analogWrite(ledOutPin, ledOut);

  //Display Analog input value
  Serial.print("Throttle = ");
  Serial.print(throttlePercent);
  Serial.print("%  ");
  Serial.print("Output = ");
  Serial.print(throttleOutVoltage);
  Serial.println("V");
  
  // Short delay before next loop
  delay(100);
 
}
