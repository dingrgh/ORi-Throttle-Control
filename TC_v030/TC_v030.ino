/*
  -----------------------------------
  Throttle control Logic for ORi Bike
  -----------------------------------
  Motor: Keyde rear hub motor with built-in controller.
  Control: Single momentary pushbutton switch and brake cutout switch.
*/ 
//  Control Modes
//  -------------
    const int modeOff = 0;      // 0 = Off
    const int modeRampUp = 1;   // 1 = Ramp up
    const int modeHold = 2;     // 2 = Hold
    const int modeRampDn = 3;   // 3 = Ramp down
    int modeCntrl = modeOff;    // Control Mode status. Initially set to Off

 
//  Control Variables
//  -----------------
    const int tThreshld = 200;           // Defines threshold for a throttle 'blip' (msec)
    int tElapsed = 2000;           // Time (msec) since last Throttle Switch state change. Init to large value.
    const int tLoop = 25;                // Loop time (msec)

    int stateSwThrot = HIGH;       // Throttle switch state (HIGH = open)
    int stateSwThrotOld = HIGH;    // Previous Throttle switch state
    boolean transSwThrot = false;  // Throttle switch in transition    int levelThrot = 0;            // Throttle setting level
    int stateSwBrake = HIGH;       // Brake switch state (HIGH = open)
    
    int throtTgt = 255;            // Target throttle setting (0 ~ 255) - Larger value is lower throttle setting
    int throtAct = 255;            // Actual throttle setting (0 ~ 255) - Larger value is lower throttle setting
    const int throtKRampUp = 15;         // Gain used in RampUp function (larger = faster ramp-up)
    const int throtKRampDn = 25;         // Gain used in RampDn function (larger = faster ramp-down)
    
    const int throtLevelVal[10] = {255,180,170,155,130,110,85,60,35,11};  // Throttle setting levels array
    int throtLevel = 0;                                             // Throttle setting level (0 - 9)
    
    const int freq9 = 3000;  // Beep tone for Level 9 (Hz)
    const int freq0 = 2000;  // Beep tone for Level 0 (Hz)
    const int tBeep = 50;   // Beep length (msec)
    
//  Other Variables
//  ---------------
    unsigned long tSysNow = millis();  // Current system time
    unsigned long tSysOld = millis();  // System time from previous loop
    int tDelta;                        // Time elapsed since last loop (long)
    int tDelay;                        // Variable delay pad
    int LED = 1;

//  Pin Assignments
//  ---------------
    const int pinSwThrot = 12;   // Throttle switch
    const int pinSwBrake = 11;   // Brake switch
    const int pinThrotOut = 10;  // This PWM output pin is connected the Assist Level input to the Keyde motor controller
                                 // and the signal is smoothed with an R-C circuit to ground
    const int pinLED = 13;       // Onboard LED
    const int pinBeep = 9;       // Connected to piezo transducer
                                 
//  Debugging Variables
//  -------------------
 
  

//  Setup
//  =====

void setup() 
{
  // Initialize serial communications:
  Serial.begin(9600);
  
  // Initialize hardware:
  pinMode(pinSwThrot, INPUT_PULLUP);  // Enable internal pullup resistor on Throttle Switch input
  pinMode(pinSwBrake, INPUT_PULLUP);  // Enable internal pullup resistor on Brake Switch input
  analogWrite(pinThrotOut, 255);      // Initialize throttle output to 5.0V (Throttle OFF)
                                      // Note: min throttle is ~3.85V; max throttle is ~1.20V
  
}

/*  Main Loop  */

void loop() 
{
  //  Toggle LED output
  if(LED == 1) {
    LED = 0;
  }
  else {
    LED = 1;
  }
  digitalWrite(pinLED, LED);
  //
  //  **Monitor Brake Switch
  //
  stateSwBrake = digitalRead(pinSwBrake);
  
  if (stateSwBrake = LOW)  {  //  if Brake Switch is in Cutout state, then
    modeCntrl = modeOff;      //  Set Control Mode to Off
  }
      
  //  else do regular loop items
  else
  {
    //
    //    ** Update timing functions
    //
    //    Update tElapsed :
    tSysNow = millis();
    tDelta = tSysNow - tSysOld;
    tDelay = tLoop - tDelta;  //  delay
    delay(tDelay);            //  delay
    tSysOld = millis();
    tElapsed = tElapsed + tLoop;
    tElapsed = min(tElapsed, 2000);  // limit it to 2000 msec to prevent overflow problems
    
    //    ** Update Throttle System state
    //
    //  Poll throttle switch and set transition flag:
        stateSwThrot = digitalRead(pinSwThrot);  // Poll throttle switch
    //  if Throttle Switch state has not changed, then
    if (stateSwThrot == stateSwThrotOld) {
      transSwThrot = false;  // Clear transition flag
    }      
    else {  // Throttle Switch state has just changed
      transSwThrot = true;  // Set transition flag
      tElapsed = 0;         // Reset tElapsed
    }
    stateSwThrotOld = stateSwThrot;
  }
  //
  //    ** Update Control State & Throttle Level
  //
 /*  
  
  Transition between Control States
  ----------------------------------
  +===============+============+============+=====================+
  | Current State |   OFF>ON   |  OFFSteady |Throttle Tgt Reached |
  +===============+============+============+=====================+
  |    Off        |  >RampUp+  |     NA     |         NA          |
  +---------------+------------+------------+---------------------+
  |    RampUp     |   RampUp+  |  >RampDn   |       >Hold         |
  +---------------+------------+------------+---------------------+
  |    Hold       |  >RampUp+  |  >RampDn   |         NA          |
  +---------------+------------+------------+---------------------+
  |    RampDown   |   >Hold    |     NA     |        >Off         |
  +---------------+------------+------------+---------------------+
  
  > prefix = Change mode
  + suffix = Increment throttle target value
   
*/  
  //  Execute this if in Off mode:
  if (modeCntrl == modeOff)  {
    if (transSwThrot == true && stateSwThrot == LOW)  {
       throtLevel = 1;
       modeCntrl = modeRampUp;
    }
    else  {
      throtLevel = 0;
    }
  }
  //  Execute this if in RampUp mode or Hold mode:
  else if (modeCntrl == modeRampUp || modeCntrl == modeHold)  {  
    if (transSwThrot == true && stateSwThrot == LOW)  {
      throtLevel = min(throtLevel + 1, 9);  //  Increment throttle level with a cap at level 9
      modeCntrl = modeRampUp;
      if (throtLevel == 9)  {
        tone(pinBeep, freq9, tBeep);  //  Beep to indicate reaching Level 9
      }
    }
    else if (tElapsed > tThreshld && stateSwThrot == HIGH)  {
      modeCntrl = modeRampDn;
    }
  }
  //  Execute this if in RampDn mode:
  else {
    if (stateSwThrot == LOW)  {
      modeCntrl = modeHold;
    }
  }
  //
  //    ** Update throttle output parameters
  //
  throtTgt = throtLevelVal[throtLevel];
  //
  //  Off mode:
  if (modeCntrl == modeOff)  {
    throtAct = 255;
  }
  //  RampUp mode:
  if (modeCntrl == modeRampUp)  {
    if (throtLevel == 1)  {  //  If throttle level is 1,
      throtAct = throtTgt;   //  then set throttle immediately
    }
    else  {
      throtAct = max((throtAct * 10 - throtKRampUp) / 10, throtTgt);  // else ramp the throttle up
    }
    if (throtAct == throtTgt)  {  //  If throttle target reached,
      modeCntrl = modeHold;       //  go into Hold mode
    }
  }
  //  RampDn mode:
  else if (modeCntrl == modeRampDn)  {
    throtAct = (throtAct * 10 + throtKRampDn) / 10;
    if (throtAct >= throtLevelVal[1])  {
      throtAct = 255;
      throtTgt = 255;
      modeCntrl = modeOff;
      throtLevel = 0;
      tone(pinBeep, freq0, tBeep);  //  Beep to indicate reaching Level 0 
    }
    else if (throtAct >= throtTgt)  {
      throtLevel = throtLevel - 1;
    }
  }
  //  Hold mode:
  else if (modeCntrl == modeHold)  {
    throtAct = throtLevelVal[throtLevel];
  }
  
  //
  //    ** Output to hardware
  //
  analogWrite(pinThrotOut, throtAct);
  //
  //    ** Display monitored variables for debugging.
  //
  Serial.print("tDelta = ");
  Serial.print(tDelta);
  /*
  Serial.print("  tDelay = ");
  Serial.print(tDelay);
  */
  Serial.print("  tElapsed = ");
  Serial.print(tElapsed);
  Serial.print("  Sw = ");
  Serial.print(stateSwThrot);
  Serial.print("  Mode = ");
  Serial.print(modeCntrl);
  Serial.print("  ThrLvl = ");
  Serial.print(throtLevel);
  Serial.print("  ThrAct = ");
  Serial.print(throtAct);
  Serial.println(" ");
}


