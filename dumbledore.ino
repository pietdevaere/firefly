#define NONE 0
#define TOGGLE 1
#define ZONE 2
#define ALL 3

const int minPulseWidth = 10;                        // minimum duration for valid inter board signal (in ms)
const int signalPulse = 20;                          // duration of a signal between different boards (in ms)
const int zonePin = 12;                              // the zone off pin
unsigned long zoneDebounce;                                   // debounce timer for reading zoneoff
int zoneInState = 0;                                 // state variable for reading
unsigned long zonePulse;                                      // timer for pulse generation
int zoneState = 0;                                   // state variable for pulse generation
const int allPin = 13;                               // the all off pin
unsigned long allDebounce;                                    // debounce timer for reading alloff
int allInState = 0;                                  // state variable for reading
unsigned long allPulse;                                       // timer for pulse generation
int allState = 0;                                    // state variable for pulse generation
const int numOfLights = 7;                           // the number of lights
const int lightsIn[numOfLights] = {10, 14, 15, 16, 17, 18, 19};   //The input pins of the lights
const int lightsOut[numOfLights] = {9, 8, 7, 6, 5, 4, 3}; //The output pins of the lights
int outState[numOfLights] = {0, 0, 0, 0, 0, 0, 0};         //The current state of the lights
int inState[numOfLights] = {0, 0, 0, 0, 0, 0, 0};          //The current state of the inputs
unsigned long inMillis[numOfLights] = {0, 0, 0, 0, 0, 0, 0};        //Used to debounce and time the inputs
int debounceTime = 20;                               //Threshold for a valid input signal (in ms)
int zoneThreshold = 1000;                            //Threshold to swich over to a zone signal (in ms)
int allThreshold = 3000;                             //Threshold to switch over to an all signal (in ms)
const int numOfFanTriggers = 3;                      //Number of lights that can trigger the fan pin
const int fanTriggers[numOfFanTriggers] = {1, 2, 3}; //The indexed of the lights that can trigger the fan pin
const int fanPin = 9999;                             //The pin number of the fan pin 


int read_light(int lightNr){
  int newVal;       //Stores data read from the pin
  
  newVal = !digitalRead(lightsIn[lightNr]); // read the data from the pin
  
  if (inState[lightNr] == 0){        // if the last read was low
    if (newVal == 1){                // if we have a rising edge
      inMillis[lightNr] = millis();  // set the timer for this pin
      inState[lightNr] = 1;          // and set the last state to high
    }
    return NONE;                     // don't return any activity
  }
  /* Triger an all or zone off whenever the timer has elapsed
     but only trigger the toggle on a faling edge */
  else{
    unsigned long timeDelta;
    if (newVal == 0) inState[lightNr] = 0;     // set the last known state to off
    timeDelta = millis() - inMillis[lightNr];  // calculate the time the pin was high upto now
    if (timeDelta > allThreshold) return ALL;  // if it was more than the all of timer
    else if (timeDelta > zoneThreshold) return ZONE;  // if it was more than the zone timer
    else if (newVal == 0 && timeDelta > debounceTime) return TOGGLE; // faling edge and valid pulse
    return NONE;
  }
  
  
  
//  else if (inState[lightNr] == 1){   // if the last read was high
//    if (newVal == 0){                // if we have a faling edge
//      long timeDelta;
//
//      inState[lightNr] = 0;          // set the last state to low
//      timeDelta = millis() - inMillis[lightNr];              //calculate the time the pin was high
//      
//      if (timeDelta > allThreshold) return ALL;              // if it was more than the all of timer
//      else if (timeDelta > zoneThreshold) return ZONE;       // if it was more than the zone timer
//      else if (timeDelta > debounceTime) return TOGGLE;   // if it was more than the debounce timer
//    }
//    return NONE;  // otherwise dont't return anny acitivtiy
// }
}

int set_all_zero(){
  int i;
  for (i = 0; i < numOfLights; i++){
    outState[i] = 0;
  }
}

int read_all_off(){
  int newVal;

  newVal = digitalRead(allPin);
  
  if (allInState == 0){  // If the last know state was low
    if (newVal == 1){  // and we detect a rising edge
      allDebounce = millis(); // start the timer
      allInState = 1; // last know value is now high
    }
    return 0; // we haven't read a valid pulse
  }

  if (allInState == 1){ // if the last know value was one
    Serial.println("We know all the pin is high");
 //   if (newVal == 0){ // and we have a faling edge
      unsigned long timeDelta;

      if (newVal == 0) allInState = 0; // last known value is now low
      timeDelta = millis()-allDebounce;
      if (timeDelta > minPulseWidth) return 1; // valid pulse detected
 //     }
  }

  return 0; // no valid pulse detected

}

int do_all_off(int localTrigger){
 if(localTrigger) Serial.println("All trigered localy");
 int remoteTrigger;
 
// remoteTrigger = digitalRead(allPin); // replace this with a proper debouncing function
 remoteTrigger = read_all_off();

 
 if (remoteTrigger){
   Serial.println("All trigered externaly");
   set_all_zero();
 }
 
 if (localTrigger){
   set_all_zero();
   allState = 1;
   allPulse = millis();
   digitalWrite(allPin, HIGH);
   pinMode(allPin, OUTPUT);
 }
 
 if (allState == 1 && millis() - allPulse >= signalPulse){
   allState = 0;
   pinMode(allPin, INPUT);
   digitalWrite(allPin, LOW);
 }
 
 return localTrigger || remoteTrigger;
}

int read_zone_off(){
  int newVal;

  newVal = digitalRead(zonePin);
  
  
  if (zoneInState == 0){  // If the last know state was low
    if (newVal == 1){  // and we detect a rising edge
      zoneDebounce = millis(); // start the timer
      zoneInState = 1; // last know value is now high
    }
    return 0; // we haven't read a valid pulse
  }

  if (zoneInState == 1){ // if the last know value was one
    Serial.println("We know the zone pin is high");
 //   if (newVal == 0){ // and we have a faling edge
      unsigned long timeDelta;

      if (newVal == 0) zoneInState = 0; // last known value is now low
      timeDelta = millis()-zoneDebounce;
      if (timeDelta > minPulseWidth) return 1; // valid pulse detected
//      }
  }

  return 0; // no valid pulse detected

}

int do_zone_off(int localTrigger){
// if(localTrigger) Serial.println("Zone trigered localy");
 int remoteTrigger;
 
// remoteTrigger = digitalRead(allPin); // replace this with a proper debouncing function
 remoteTrigger = read_zone_off();

 
 if (remoteTrigger){
   Serial.println("Zone trigered externaly");
   set_all_zero();
   //return 1;
 }
 
 if (localTrigger){
   Serial.println("Zone trigered localy");
   set_all_zero();
   zoneState = 1;
   zonePulse = millis();
   digitalWrite(zonePin, HIGH);
   pinMode(zonePin, OUTPUT);
   //return 1;
 }
 
 if (zoneState == 1 && millis() - zonePulse >= signalPulse){
   Serial.println("----> Moving back to pinmode INPUT");
   zoneState = 0;
   pinMode(zonePin, INPUT);
   digitalWrite(zonePin, LOW); 
 }
 
 return remoteTrigger || localTrigger;
}

int write_states(){
  int i;
  int fanTarget;
  for (i = 0; i < numOfLights; i++){
    digitalWrite(lightsOut[i], outState[i]);
  }

  fanTarget = 0;
  for (i = 0; i < numOfFanTriggers; i++){
      if(outState[fanTriggers[i]]) fanTarget = 1;
  }
  //digitalWrite(fanPin, fanTarget);

  
}

void setup() {
  int i;
  
  for (i = 0; i < numOfLights; i++){
    pinMode(lightsIn[i], INPUT_PULLUP);
    pinMode(lightsOut[i], OUTPUT);
  }
  
  pinMode(zonePin, INPUT);
  pinMode(allPin, INPUT);
  Serial.begin(115200);
  Serial.println("READY");
}

void loop() {
//  Serial.println("------------------------------");
//  for (int i; i<7; i++){
//    Serial.print("light ");
//    Serial.print(i);
//    Serial.print(":\t");
//    Serial.println(!digitalRead(lightsIn[i]));
//  }
//  Serial.print("all pin:\t");
//  Serial.println(digitalRead(allPin));
//  Serial.print("zone pin:\t");
//  Serial.println(digitalRead(zonePin));
//  Serial.print("zoneState:\t");
//  Serial.println(zoneState);  
//  Serial.print("millis():\t");
//  Serial.println(millis());
//  Serial.print("zonePulse:\t");
//  Serial.println(zonePulse);

  
  int i;
  int allOff = 0;  // stores whether an all off should be triggerd
  int zoneOff = 0; // stores wheterh a zone off should be triggerd
  
  for (i = 0; i < numOfLights; i++){
    switch (read_light(i)){ // see what to do, and do it
      case NONE:
        break;
      
      case TOGGLE:
        Serial.print(i);
        Serial.println("\t TOGGLE");
        outState[i] = !outState[i]; // toggle a light
        break;
       
      case ZONE:
        Serial.print(i);
        Serial.println("\t ZONE");
        zoneOff = 1; // trigger a zone off
        break;
      
      case ALL:
        Serial.print(i);
        Serial.println("\t ALL");
        allOff = 1; // trgier an all off
        break;
    } 
  }
  if(!do_all_off(allOff)) // if we did not just do an all of
  do_zone_off(zoneOff); // stile have to make this functio
  write_states();
 
     
}

