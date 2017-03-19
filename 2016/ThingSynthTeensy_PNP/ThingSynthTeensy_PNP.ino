/* Print Incoming Message USB MIDI Example
   Use the Arduino Serial Monitor to view the messages
   as Teensy receives them by USB MIDI

   You must select MIDI from the "Tools > USB Type" menu

   This example code is in the public domain.
*/
boolean debug= false;

#define pot_1 0
#define numStrikeVals 64
#define numOuts 3

int chan = 2;

int firstPin=3; //lowest pin out the outputs  probably 2
byte incomingByte;
byte note;
byte pNote;
byte velocity;
byte currentNote;

float scaleCalc; 
float potValue;

float dutyCycle=0.7;
float dutyHigh = 50;
float dutyLow =20; 

int statusLed = 13;   // select the pin for the LED
int action=2; //0 =note off ; 1=note on ; 2= nada

#define numNotes 92 //how many outputs
//int firstPin = 22; //what's the first pin
int lowNote = 0;

int solenoid[numOuts];
long strikeMark[numOuts];
boolean strikeState[numOuts];
boolean pulseState[numOuts];
int lastNote[numOuts];
long restMark[numOuts];
long restTime[numOuts];
long strikeTime[numOuts];
long lastNoteOff[numOuts];
long lastNoteOn[numOuts];

float baseTime = 61162.07951;
//float baseTime = 20000;

float cycleVal[numNotes];
float restVals[numNotes];
float strikeVals[numNotes];


void setup() {
  Serial.begin(115200);
  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleVelocityChange(OnVelocityChange);
  usbMIDI.setHandleControlChange(OnControlChange);
  usbMIDI.setHandleProgramChange(OnProgramChange);
  usbMIDI.setHandleAfterTouch(OnAfterTouch);
  usbMIDI.setHandlePitchChange(OnPitchChange);


  for(int i=0; i<numOuts; i++){
  solenoid[i] = i+firstPin;
  
  pinMode(solenoid[i],OUTPUT);
  digitalWrite(solenoid[i],HIGH);

restMark[i] = 0;
restTime[i] =0;
strikeState[i] = false;
pulseState[i]=false;
 
}

//pinMode(statusLed,OUTPUT);   // declare the LED's pin as output
//pinMode(solenoid_1,OUTPUT);
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
   usbMIDI.read(chan); // USB MIDI receive
   
   for(int i=0; i<numOuts;i++){
pulse(i);
}
  
for(int i=0; i<numOuts; i++){  
timeoutCheck(i);

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
playNote(note, velocity);

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
playNote(note, 0);
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


  int value=HIGH;
  if (velocity >10){
      value=LOW;
  }else{
   value=HIGH;
  } 


 //since we don't want to "play" all notes we wait for a note in range
 if(note>=lowNote && note<lowNote+numNotes){
   if(value==LOW){
  
   for(int i = 0; i<numOuts; i++){ 
  if(pulseState[i]==false){
   pulseState[i]=true;
   lastNoteOn[i] = millis();
 
    dutyCycle=map(velocity,10.0,127.0,dutyLow,dutyHigh)/100.0; 
   strikeTime[i]=cycleVal[note-lowNote]*dutyCycle;
   restTime[i]=cycleVal[note-lowNote]-strikeTime[i];

   
   lastNote[i]=note;
   break;  
}
   }
   
    }
   if(value==HIGH){
      for(int i = 0; i<numOuts; i++){ 
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
digitalWrite(solenoid[p], HIGH);
digitalWrite(solenoid[p], LOW);
strikeMark[p]=micros();

}
// if the on cycle has passed
if(micros()-strikeMark[p]>=strikeTime[p] && strikeState[p]==true){
strikeState[p]=false;
digitalWrite(solenoid[p], HIGH);
restMark[p]=micros();
lastNoteOff[p] = millis();

}
}

else digitalWrite(solenoid[p], LOW);
}


void timeoutCheck(int p){
//  if(millis()-lastNoteOff>1000){
if(millis()-lastNoteOff[p]>2000){
  incomingByte=127+chan;
  incomingByte=lastNote[p];
  
  // incomingByte=0;
  //playNote(lastNote[p], 0);
  //pulseState[p] = false;
  // strikeState = false;
  digitalWrite(solenoid[p], HIGH);   
 
 }

if(millis() - lastNoteOn[p]> 4000 && pulseState[p]){
playNote(lastNote[p], 0);
pulseState[p]=false;

}

}

void debugger(){
Serial.println(analogRead(pot_1));
//Serial.println(strikeTime);
delay(1000);
}



