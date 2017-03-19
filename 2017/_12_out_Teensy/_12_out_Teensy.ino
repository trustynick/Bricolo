/* Print Incoming Message USB MIDI Example
   Use the Arduino Serial Monitor to view the messages
   as Teensy receives them by USB MIDI

   You must select MIDI from the "Tools > USB Type" menu

   This example code is in the public domain.
*/
boolean debug= false;

#include <MIDI.h> // Add Midi Library
#define numOutputs 12

//define serial pin for midi (Serial1 on TeensyLC is pin 0)
#define HWSERIAL Serial1

byte incomingByte;
byte note;
byte velocity;

int chan = 1;
int action=2; //0 =note off ; 1=note on ; 2= nada
int numOuts = numOutputs; //how many outputs
#define firstPin 2 //what's the first pin
int lowNote = 36; //what's the first note?

long noteStart[numOutputs+firstPin];
int timeout = 500;
int pwmTimeout = timeout;
boolean noteOn[numOutputs+firstPin];

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
  MIDI.begin(chan);
    MIDI.begin(chan+1);
  
  for(int i=firstPin; i<firstPin+numOuts; i++){
  pinMode(i, OUTPUT);
  digitalWrite(i, LOW);
  }
}

void loop() {
  
   //usbMIDI.read(chan); // USB MIDI receive
   usbMIDI.read(); // USB MIDI receive
   MIDI.read();
   
   for(int i=0; i<numOuts;i++){
pulsePWM(i);
notesTimeout(i);

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
playNote(note, velocity);
  }
//digitalWrite(solenoid[0], HIGH);

}

void OnNoteOff(byte channel, byte note, byte velocity) {
  Serial.print("Note Off, ch=");
  Serial.print(channel, DEC);
  Serial.print(", note=");
  Serial.print(note, DEC);
  Serial.print(", velocity=");
  Serial.print(velocity, DEC);
  Serial.println();
//digitalWrite(solenoid[0], LOW);
if(channel == chan){
playNote(note, 0);
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
   noteOn[myPin] = true;
 
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
   noteOn[myPin] = false;
   digitalWrite(myPin, LOW);
//________
    digitalWrite(myPin+2, LOW);   //debug
//________
   }
   
 }
}


//void playNote(byte note, byte velocity){
//
//
//  int value=LOW;
//  if (velocity >10){
//      value=HIGH;
//  }else{
//   value=LOW;
//  } 
//
//
// //since we don't want to "play" all notes we wait for a note in range
// if(note>=lowNote && note<lowNote+numNotes){
//   if(value==HIGH){
//  
//   for(int i = 0; i<numOuts; i++){ 
//  if(pulseState[i]==false){
//   pulseState[i]=true;
//   lastNoteOn[i] = millis();
// 
//    dutyCycle=map(velocity,10.0,127.0,dutyLow,dutyHigh)/100.0; 
//   strikeTime[i]=cycleVal[note-lowNote]*dutyCycle;
//   restTime[i]=cycleVal[note-lowNote]-strikeTime[i];
//
//   
//   lastNote[i]=note;
//   break;  
//}
//   }
//   
//    }
//   if(value==LOW){
//      for(int i = 0; i<numOuts; i++){ 
//     if(lastNote[i] == note && pulseState[i]){
//     pulseState[i]=false;
//     }
//      }
//    
//   } 
// }
//}

void notesTimeout(int _pin){
  if(!pwm[_pin]){
  if(millis() - noteStart[_pin]>timeout &&  noteOn[_pin] == true){
  digitalWrite(_pin, LOW);
  noteOn[_pin] == false;

}
  }
  else{
  if(millis() - noteStart[_pin]>pwmTimeout &&  noteOn[_pin] == true){
  digitalWrite(_pin, LOW);
  pwm[_pin] = false;
  noteOn[_pin] == false;
    
  }
  }

}

void pulsePWM(byte _pin){
  if(pwm[_pin]){
  
  if(noteOn[_pin]){
    
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



