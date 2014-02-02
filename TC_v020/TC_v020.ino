/*
  -----------------------------------
  Throttle control Logic for ORi Bike
  -----------------------------------
  Motor: Keyde rear hub motor with built-in controller.
  Control: Single momentary pushbutton switch and brake cutout switch.
*/ 
//  Control Modes
//  -------------
    int modeCntrl = 0;          // Control Mode status
    const int modeOff = 0;      // 0 = Off
    const int modeRampUp = 1;   // 1 = Ramp up
    const int modeHold = 2;     // 2 = Hold
    const int modeRampDn = 3;   // 3 = Ramp down

//  Throttle Modes
//  --------------
    int modeThrot = 5;           // Throttle Mode: Initialized to OFFsteady
    const int modeOFFblip = 1;   // Momentary ON-OFF-ON detected
    const int modeONblip = 2;    // Momentary OFF-ON-OFF detected
    const int modeOFFwait = 3;   // Button has been OFF for less than tThreshold time
    const int modeONwait = 4;    //Button has been ON for less than tThreshold time
    const int modeOFFsteady = 5; // Switch has been OFF for longer than tThreshold time
    const int modeONsteady = 6;  // Switch has been ON for longer than tThreshold time
/*  

  Brake Switch States
  -------------------
  1. Normal: Brake Switch is open (input internally pulled up to HIGH)
  2. Cutout: Brake Switch is closed (input grounded to LOW)
  
  Transition between Control States
  ----------------------------------
  +===============+============+============+============+============+======================+==================+
  | Current State |   OFF>ON   | OFF>ON>OFF |   ON>OFF   | ON>OFF>ON  | Throttle Tgt Reached | Brk Sw in Cutout |
  +===============+============+============+============+============+======================+==================+
  |    Off        |  >RampUp   |   Ignore   |     NA     |     NA     |          NA          |       >Off       |
  +---------------+------------+------------+------------+------------+----------------------+------------------+
  |    RampUp     |     NA     |     NA     | >RampDown  |  >RampUp   |        >Hold         |       >Off       |
  +---------------+------------+------------+------------+------------+----------------------+------------------+
  |    Hold       |     NA     |     NA     | >RampDown  |  >RampUp   |          NA          |       >Off       |
  +---------------+------------+------------+------------+------------+----------------------+------------------+
  |    RampDown   |   >Hold    |   Ignore   |     NA     |     NA     |        >Off          |       >Off       |
  +---------------+------------+------------+------------+------------+----------------------+------------------+
*/  
 
//  Control Variables
//  -----------------
    int tThreshld = 300;         // Defines threshold for a throttle 'blip' (msec)
    int tElapsed = 3000;         // Time (msec) since last Throttle Switch state change. Init to large value.
    int throtTgt = 0;            // Target throttle setting (0 ~ 255)
    int throtAct = 0;            // Current throttle setting (0 ~ 255)
    int stateSwThrot = HIGH;     // Throttle switch state (HIGH = open)
    int stateSwThrotOld = HIGH;  // Previous Throttle switch state
    int stateSwBrake = HIGH;     // Brake switch state (HIGH = open)
    
//  Other Variables
//  ---------------
    unsigned long tSysNow = millis();  // Current system time
    unsigned long tSysOld = millis();  // System time from previous loop
    int tDelta;                        // Time elapsed since last loop (long)

//  Pin Assignments
//  ---------------
    const int pinSwThrot = 12;   // Throttle switch
    const int pinSwBrake = 11;   // Brake switch
    const int pinThrotOut = 10;  // This PWM output pin is connected the Assist Level input to the Keyde motor controller
                                 // and the signal is smoothed with an R-C circuit to ground
  

//  Setup
//  =====

void setup() 
{
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // Initialize hardware:
  pinMode(pinSwThrot, INPUT_PULLUP);  // Enable internal pullup resistor on Throttle Switch input
  pinMode(pinSwBrake, INPUT_PULLUP);  // Enable internal pullup resistor on Brake Switch input
  analogWrite(pinThrotOut, 255);      // Initialize throttle output to 5.0V (Throttle OFF)
                                      // Note: min throttle is ~3.85V; max throttle is ~1.20V
  
  // Do not start if the Throttle Switch is closed:
  stateSwThrot = digitalRead(pinSwThrot);
  while (stateSwThrot == LOW) {
    // Loop and do nothing
  }
}

/*  Main Loop  */

void loop() 
{
  
  //  poll Brake Switch state.
  stateSwBrake = digitalRead(pinSwBrake);
  
  //  if Brake Switch is in Cutout state, then
  if (stateSwBrake = LOW)
  {
    //  Set Control Mode to Off.
    modeCntrl = modeOff;
  }
      
  //  else
  else
  {
    //    ** Update Throttle System state
    //    Update tElapsed :
    tSysNow = millis();
    tDelta = tSysNow - tSysOld;
    tSysOld = tSysNow;
    tElapsed = tElapsed + tDelta;
    tElapsed = min(tElapsed, 3000);  // limit it to 3000 msec to prevent overflow problems
    
    stateSwThrot = digitalRead(pinSwThrot);  // Poll throttle switch
    //  if Throttle Switch state has not changed, then
    if (stateSwThrot == stateSwThrotOld) {
      // if it has been longer than tThreshold since the last Throttle Switch transition, then
      if (tElapsed >= tThreshld) {
        if (stateSwThrot = HIGH) {    // If Throttle Switch is open
          modeThrot = modeOFFsteady;  //   Set Throttle Mode to OFFsteady
        }
        else {                        // If Throttle Switch is closed
          modeThrot = modeONsteady;   //   Set Throttle Mode to ONsteady
        }
      }
    }      
    else {  // Throttle Switch state has just changed
      // if it has been less than tThreshld since previous Throttle Switch transition, then
      if (tElapsed < tThreshld) {
        //    Set ONblip or OFFblip state
        if (stateSwThrot == HIGH) {  // If Throttle Switch is open
          modeThrot = modeONblip;    //   Set Throttle Mode to ONblip
        }
        else {                       // If Throttle Switch is closed
          modeThrot = modeOFFblip;   //   Set Throttle Mode to OFFblip
        }
      } 
      else {  // It has been longer than tThreshld
        // Set ONwait or OFFwait state
        if (stateSwThrot == HIGH) {  // If Throttle Switch is open
          modeThrot = modeOFFwait;   //   Set Throttle mode to OFFwait
        }
        else {                       // Throttle Switch is closed
          modeThrot = modeONwait;    //   Set Throttle Mode to ONwait
        }
      // Reset tElapsed
      tElapsed = 0;
      }
    }
    stateSwThrotOld = stateSwThrot;  //  
          
  //
  //    ** Update Control State
  //    if Control State is Off, then
  //      if Throttle System state is ONsteady, then
  //        Set Control state to RampUp.
  //        Increment throttleTarget a step.
  //      endif
  //    else
  //      if Control State is RampUp or Hold, then
  //        if Throttle System state is OFFsteady, then
  //          Set Control state to RampDown.
  //        else
  //          if Throttle System state is OFFblip, then
  //            Increment throttleTarget a step.
  //          endif
  //        endif
  //      else
  //        if Throttle System state is ONsteady, then
  //          Set Control State to Hold.
  //        endif
  //      endif
  //    endif
  //  endif
  //
  //  ** Action depending on Control State.
  //  if Control State is Off, then
  //    Set target and current throttle to zero.
  //    Set Throttle System state to OFFsteady or ONsteady.
  //  else
  //    if Control State is RampUp, then
  //      Increment throttleActual.
  //      if throttleActual has reached throttleTarget, then
  //        Set Control State to Hold.
  //      endif
  //    else
  //      if Control State is RampDown, then
  //        Decrement throttleActual.
  //        if throttleActual has reached zero, then
  //          Set Control State to Off.
  //        endif
  //      endif
  //    endif
  
  //  endif
  }
  
  // Output to hardware

  // Display monitored variables for debugging.
  
}


