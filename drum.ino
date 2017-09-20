#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();   //start MIDI channels
#define SERIALPRT 115200 //this is the needed serial baud rate for the MIDI
#define MIDI_CHANNEL 2 //define where the signals should be sent to
static const int NUM_DRUM = 1;

//Time measurements in milliseconds
static const int SIGNAL_BUFFER_SIZE = 100;
static const int PEAK_BUFFER_SIZE = 30;
static const int MAX_TIME_BETWEEN_PEAKS = 20;
static const int MIN_TIME_BETWEEN_NOTES = 50;
//Analog signal storage
unsigned short currentSignalIndex[NUM_DRUM];
unsigned short currentPeakIndex[NUM_DRUM];
unsigned short signalBuffer[NUM_DRUM][SIGNAL_BUFFER_SIZE];
unsigned short peakBuffer[NUM_DRUM][PEAK_BUFFER_SIZE];

boolean noteReady[NUM_DRUM];
unsigned short noteReadyVelocity[NUM_DRUM];
boolean isLastPeakZeroed[NUM_DRUM];

unsigned long lastPeakTime[NUM_DRUM];
unsigned long lastNoteTime[NUM_DRUM];

//Maps for analog storage settings and tweaking
unsigned short drumMap[NUM_DRUM];     //define total number of piezos in this list and their respective analog input pins
unsigned short thresholdMap[NUM_DRUM];
unsigned short noteMap[NUM_DRUM];
//Define thresholds and note values for each drum
static const int SNARE_TRESH = 20;
static const int SNARE_NOTE = 38;
static const int SNARERIM_TRESH = 25;
static const int SNARERIM_NOTE = 37;
static const int FLOOR_NOTE = 43;
static const int FLOOR_TRESH = 20;
static const int KICK_TRESH = 35;
static const int KICK_NOTE = 36;
static const int HATC_TRESH = 20;
static const int HATC_NOTE = 65;
//add more when needed and don't forget to increment DRUMS//


void setup() {
  pinMode(A0, INPUT);        // Define all your analog inputs for all currently conected drums
//  pinMode(A1, INPUT);
//  pinMode(A2, INPUT);
//  pinMode(A3, INPUT);
//  pinMode(A4, INPUT);
  MIDI.begin(MIDI_CHANNEL);             // Launch MIDI and listen to channel 2, you can redefine this at the beggining
  Serial.begin(SERIALPRT);

  for(short i=0; i < NUM_DRUM; i++){
    currentSignalIndex[i] = 0;
    currentPeakIndex[i] = 0;
    memset(signalBuffer[i],0,sizeof(signalBuffer[i]));
    memset(peakBuffer[i],0,sizeof(peakBuffer[i]));
    noteReady[i] = false;
    noteReadyVelocity[i] = 0;
    isLastPeakZeroed[i] = true;
    lastPeakTime[i] = 0;
    lastNoteTime[i] = 0;
    drumMap[i] = i;         //initialise drum set

   }
  thresholdMap[0] = SNARE_TRESH;
  noteMap[0] = SNARE_NOTE;
//  thresholdMap[1] = SNARERIM_TRESH;
//  noteMap[1] = SNARERIM_NOTE;
//  thresholdMap[2] = KICK_TRESH;
//  noteMap[2] = KICK_NOTE;
//  thresholdMap[3] = FLOOR_TRESH;
//  noteMap[3] = FLOOR_NOTE;
//  thresholdMap[4] = HATC_TRESH;
//  noteMap[4] = HATC_NOTE;

  
}

void loop() {
  unsigned long currentTime = millis();

  for (short i = 0; i < NUM_DRUM; i++){
    unsigned short signal = analogRead(drumMap[i]);   //constantly poll for signal on all drum piezos
    signalBuffer[i][currentSignalIndex[i]] = signal;  //asign every collected signal to the respective drum position in the buffer


  if(signal < thresholdMap[i]){
    if(!isLastPeakZeroed[i] && (currentTime-lastPeakTime[i])> MAX_TIME_BETWEEN_PEAKS){
          calcNewPeak(i,0);
    }
    else{
      //get previous signal
      short prevSignalIndex = currentSignalIndex[i] - 1;
      if(prevSignalIndex < 0){
        prevSignalIndex = SIGNAL_BUFFER_SIZE-1;
      }
      unsigned short prevSignal = signalBuffer[i][prevSignalIndex];
      unsigned short newPeak = 0;

      while (prevSignal >= thresholdMap[i]){
        if(signalBuffer[i][prevSignalIndex]>newPeak){
          newPeak = signalBuffer[i][prevSignalIndex];
        }
        prevSignalIndex--;
        if(prevSignalIndex<0) {
          prevSignalIndex = SIGNAL_BUFFER_SIZE-1;
        }
        prevSignal = signalBuffer[i][prevSignalIndex];
      }
      if(newPeak > 0){
        calcNewPeak(i,newPeak);
      }
    }
  }
  currentSignalIndex[i]++;
  if(currentSignalIndex[i] == SIGNAL_BUFFER_SIZE){
    currentSignalIndex[i] = 0;
  }
}
}

void calcNewPeak(short drum, short newPeak){

  isLastPeakZeroed[drum] = (newPeak == 0);        //document if the peak we are adding is 0
  
  unsigned long currentTime = millis();
  lastPeakTime[drum] = currentTime;

  peakBuffer[drum][currentPeakIndex[drum]] = newPeak; //record new peak;

  short prevPeakIndex = currentPeakIndex[drum]-1;
  if(prevPeakIndex < 0){
    prevPeakIndex = PEAK_BUFFER_SIZE-1;                           //get previous peak
  } 
  unsigned short prevPeak = peakBuffer[drum][prevPeakIndex];

  //if the new peak is >= the previous peak, then note is ready
  if (newPeak > prevPeak && (currentTime - lastNoteTime[drum])> MIN_TIME_BETWEEN_NOTES){
    noteReady[drum] = true;
    if(newPeak > noteReadyVelocity[drum]){
      noteReadyVelocity[drum] = newPeak;
    }
  }
  //if the new peak is < the previous peak and the previous peak was note ready then fire the note
  else if(newPeak < prevPeak && noteReady[drum]){
    noteFire(noteMap[drum],noteReadyVelocity[drum]);
    noteReady[drum] = false;
    noteReadyVelocity[drum] = 0;
    lastNoteTime[drum] = currentTime;
  }

  currentPeakIndex[drum]++;
  if(currentPeakIndex[drum] == PEAK_BUFFER_SIZE){
    currentPeakIndex[drum] = 0;
  }


}


void noteFire(unsigned short note, unsigned short velocity){
//last line of sensitivity tweeking for the piezo triggers
  unsigned short newVelo;
  newVelo = floor(velocity/2);
  MIDI.sendNoteOn(note,newVelo,MIDI_CHANNEL);
  MIDI.sendNoteOff(note,newVelo,MIDI_CHANNEL);
}
