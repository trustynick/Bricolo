/* Print Incoming Message USB MIDI Example
   Use the Arduino Serial Monitor to view the messages
   as Teensy receives them by USB MIDI

   You must select MIDI from the "Tools > USB Type" menu

   This example code is in the public domain.
*/
boolean debug= false;

#include <MIDI.h> // Add Midi Library
#define numOutputs 12
#define tNumOuts 4
#define numStrikeVals 64


//define serial pin for midi (Serial1 on TeensyLC is pin 0)
#define HWSERIAL Serial1

boolean synthActive = false;

byte incomingByte;
byte note;
byte pNote;
byte velocity;

int chan = 1;
int tChan = 2;

int numOuts = numOutputs; //how many outputs
#define firstPin 2 //what's the first pin
int lowNote = 36; //what's the first note?

long noteStart[numOutputs+firstPin];
int timeout = 500;
int pwmTimeout = timeout;
boolean noteIsOn[numOutputs+firstPin];

//for handling velocity
byte pwmVal;
long pwmStart[numOutputs+firstPin];
long pwmStop[numOutputs+firstPin];
long pwmOnCycle[numOutputs+firstPin];
long pwmOffCycle[numOutputs+firstPin];
boolean pwmHigh[numOutputs+firstPin];
boolean pwm[numOutputs+firstPin];

long pwmOnMax = 10000;
long pwmOnMin = 0;
long pwmCycleTime=pwmOnMax;


#define tFirstPin 10
int tFirstPinMIDI = lowNote+tFirstPin-firstPin;
#define numNotes 92
int tLowNote = 0;



float scaleCalc; 

float dutyCycle=0.7;
float dutyHigh = 50;
float dutyLow =20; 
int solenoid[tNumOuts];
long strikeMark[tNumOuts];
boolean strikeState[tNumOuts];
boolean pulseState[tNumOuts];
int lastNote[tNumOuts];
long restMark[tNumOuts];
long restTime[tNumOuts];
long strikeTime[tNumOuts];
long lastNoteOff[tNumOuts];
long lastNoteOn[tNumOuts];

float baseTime = 61162.07951;
//float baseTime = 20000;

float cycleVal[numNotes];
float restVals[numNotes];
float strikeVals[numNotes];



void setup() {
  Serial.begin(115200);
  HWSERIAL.begin(31250); //initialize HWSerial pin for MIDI jack
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleVelocityChange(OnVelocityChange);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleProgramChange(OnProgramChange);
  usbMIDI.setHandleAfterTouch(OnAfterTouch);
  usbMIDI.setHandlePitchChange(OnPitchChange);
  
  MIDI.setHandleNoteOn(OnNoteOn);  // Put only the name of the function
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandleProgramChange(OnProgramChange);
  MIDI.setHandleAfterTouchChannel(OnAfterTouch);
  MIDI.setHandlePitchBend(OnPitchChange);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  //MIDI.begin(chan);
  //MIDI.begin(tChan);
  
  
  for(int i=firstPin; i<firstPin+numOuts; i++){
  pinMode(i, OUTPUT);
  digitalWrite(i, LOW);
  }

//init thing synth outputs
 for(int i=0; i<tNumOuts; i++){

solenoid[i] = i+tFirstPin;
restMark[i] = 0;
restTime[i] =0;
strikeState[i] = false;
pulseState[i]=false;
}

scaleCalc=196.0/185.0;
//Serial.println(scaleCalc, 5);

//populate rest time values
for(int i=0; i<numNotes; i++){
float x = pow(scaleCalc,i);

cycleVal[i]=baseTime/x;
//restVals[i] =baseTime/x-strikeTime;
strikeVals[i] = cycleVal[i]*dutyCycle;
restVals[i] = cycleVal[i]- strikeVals[i];
}
  
}

void loop() {
   usbMIDI.read(); // USB MIDI receive
   MIDI.read();
  
for(int i=0; i<numOuts-tNumOuts;i++){
pulsePWM(i);
notesTimeout(i);
}


for(int i=0; i<tNumOuts;i++){
  if(synthActive){
timeoutCheck(i);
pulse(i);
}
else{
pulsePWM(i);
notesTimeout(i);
  }
}

  
}


void OnNoteOn(byte channel, byte note, byte velocity) {
  Serial.print("Note On, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.print(velocity, DEC);
  Serial.println();



if(channel == chan){
synthActive=false;
if(note>=tFirstPinMIDI && note<=tFirstPinMIDI+tNumOuts){
  
  }
  

playNote(note, velocity);
}

if(channel == tChan){

synthActive=true;
tPlayNote(note, velocity);
  }
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  Serial.print("Note Off, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.print(velocity, DEC);
  Serial.println();

if(channel == chan){
playNote(note, 0);
}

if(channel == tChan){
tPlayNote(note, 0);
  }
}

void OnVelocityChange(byte channel, byte note, byte velocity) {
  Serial.print("Velocity Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.print(velocity, DEC);
  Serial.println();
}

void OnControlChange(byte channel, byte control, byte value) {
  Serial.print("Control Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", control=");
  Serial.print(control, DEC);
  Serial.print(", value=");
  Serial.print(value, DEC);
  Serial.println();
}

void OnProgramChange(byte channel, byte program) {
  Serial.print("Program Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", program=");
  Serial.print(program, DEC);
  Serial.println();
}

void OnAfterTouch(byte channel, byte pressure) {
  Serial.print("After Touch, ch=");
  Serial.print(channel, DEC);
  Serial.print(", pressure=");
  Serial.print(pressure, DEC);
  Serial.println();
}

void OnPitchChange(byte channel, int pitch) {
  Serial.print("Pitch Change, ch=");
  Serial.print(channel, DEC);
  Serial.print(", pitch=");
  Serial.print(pitch, DEC);
  Serial.println();
}

void playNote(byte note, byte velocity){
  int value=LOW;
  if (velocity >10){
      value=HIGH;
  }else{
   value=LOW;
  }

 //since we don't want to "play" all notes we wait for a note in range

 if(note>=lowNote && note<lowNote+numOuts){
   byte myPin=note-(lowNote-firstPin); // to get a pinnumber within note range
   //digitalWrite(myPin, value);
  
   if(value==HIGH){
   noteStart[myPin] = millis();
   noteIsOn[myPin] = true;
 
 if(velocity<80){
   setPWM(myPin, velocity);
   pwm[myPin] = true;
   //pwmStart[myPin]=millis();
   pwmStart[myPin]=micros();
   pwmHigh[myPin]=true;
    digitalWrite(myPin, HIGH);
 }
 else{ pwm[myPin] = false;
 digitalWrite(myPin, HIGH);
 }
 
 }
   if(value==LOW){
   noteIsOn[myPin] = false;
   digitalWrite(myPin, LOW);
//________
    digitalWrite(myPin+2, LOW);   //debug
//________
   }
   
 }
}


//12 out functions

void notesTimeout(int _pin){
  if(!pwm[_pin]){
  if(millis() - noteStart[_pin]>timeout &&  noteIsOn[_pin] == true){
  digitalWrite(_pin, LOW);
  noteIsOn[_pin] == false;

}
  }
  else{
  if(millis() - noteStart[_pin]>pwmTimeout &&  noteIsOn[_pin] == true){
  digitalWrite(_pin, LOW);
  pwm[_pin] = false;
  noteIsOn[_pin] == false;
  }
  }

}

void pulsePWM(byte _pin){
  if(pwm[_pin]){
  
  if(noteIsOn[_pin]){
    
  //if solenoid is on and the HIGH duty cyle is surpassed, switch to LOW
    if(micros()-pwmStart[_pin]>pwmOnCycle[_pin] && pwmHigh[_pin]){
  digitalWrite(_pin, LOW);
  //digitalWrite(_pin+2, LOW); //for debug trigger a different output to see if its triggering as expected
  pwmHigh[_pin]=false;
 pwmStop[_pin]=micros();
  }
   else if(micros()-pwmStop[_pin]>pwmOffCycle[_pin] && !pwmHigh[_pin]){
  digitalWrite(_pin, HIGH);
  pwmHigh[_pin]=true;
  pwmStart[_pin]=micros();
}
  } 
  }
}

//receive velocity byte and map it to pwm duty cycle times based on min & max;
void setPWM(byte _pin, byte _velocity){
  
pwmOnCycle[_pin] = 100;
int baseTime = 4000;
//vel mapping
if(_velocity <80 && _velocity >= 10){
 int duty = map(_velocity,10,100,50,99); //if velocity is below 100 map duty cycle
  dutyCalc(duty,baseTime, _pin);

}
}

void dutyCalc(float _duty, int _timeBase,int __pin){
pwmOnCycle[__pin] = long(_timeBase*_duty/100);  
pwmOffCycle[__pin] = _timeBase-pwmOnCycle[__pin];

}


void tPlayNote(byte note, byte velocity){

  int value=LOW;
  if (velocity >10){
      value=HIGH;
  }else{
   value=LOW;
  } 


 //since we don't want to "play" all notes we wait for a note in range
 if(note>=tLowNote && note<tLowNote+numNotes){
   if(value==HIGH){
  
   for(int i = 0; i<tNumOuts; i++){ 
  if(pulseState[i]==false){
   pulseState[i]=true;
   lastNoteOn[i] = millis();
 
    dutyCycle=map(velocity,10.0,127.0,dutyLow,dutyHigh)/100.0; 
   strikeTime[i]=cycleVal[note-tLowNote]*dutyCycle;
   restTime[i]=cycleVal[note-tLowNote]-strikeTime[i];

   
   lastNote[i]=note;
   break;  
}
   }
   
    }
   if(value==LOW){
      for(int i = 0; i<tNumOuts; i++){ 
     if(lastNote[i] == note && pulseState[i]){
     pulseState[i]=false;
     }
      }
    
   } 
 }
}


void pulse(int p){

if(pulseState[p]==true){
  // if the off cycle has passed
if(micros()-restMark[p]>=restTime[p] && strikeState[p]==false){
strikeState[p]=true;
digitalWrite(solenoid[p], LOW);
digitalWrite(solenoid[p], HIGH);
strikeMark[p]=micros();

}
// if the on cycle has passed
if(micros()-strikeMark[p]>=strikeTime[p] && strikeState[p]==true){
strikeState[p]=false;
digitalWrite(solenoid[p], LOW);
restMark[p]=micros();
lastNoteOff[p] = millis();

}
}

else digitalWrite(solenoid[p], LOW);
}


//timeout for thing synth
void timeoutCheck(int p){
if(millis()-lastNoteOff[p]>2000){
  incomingByte=127+chan;
  incomingByte=lastNote[p];

  digitalWrite(solenoid[p], LOW);   
 
 }

if(millis() - lastNoteOn[p]> 4000 && pulseState[p]){
playNote(lastNote[p], 0);
pulseState[p]=false;

}

}


