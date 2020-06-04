//https://github.com/TMRh20/TMRpcm/wiki/Advanced-Features#wiki-recording-audio for

#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>

#define SD_ChipSelectPin 4

TMRpcm audio;
File myFile;

char fname[20];
int cnt = 0;
int start_p = 2;
int stop_p = 3;
int rec_state = 0;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(13, LOW);
  digitalWrite(8, LOW);
  pinMode(start_p, INPUT_PULLUP);
  pinMode(stop_p, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(start_p), rec_start, FALLING);
  attachInterrupt(digitalPinToInterrupt(stop_p), rec_stop, FALLING);
  delay(200);

  if (!SD.begin(SD_ChipSelectPin)) {  
    return;
  }else{
  }
  audio.CSPin = SD_ChipSelectPin;

  myFile = SD.open ("cnt.txt");

  while (myFile.available()) {
    myFile.read();
    cnt++;
  }

  myFile.close();

  sprintf(fname, "data%04d.wav", cnt+1);

  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13, LOW);
  delay(200);
  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13, LOW);

  digitalWrite(8, HIGH);
}

void loop() {
  
}

void rec_start() {
  if (rec_state == 0) {
    rec_state = 1;
    digitalWrite(13, HIGH);
    audio.startRecording(fname, 16000, A0);
  }
}

void rec_stop() {
  if (rec_state == 1) {
    audio.stopRecording(fname);
    myFile = SD.open ("cnt.txt", FILE_WRITE);
    myFile.print("1");
    myFile.close();
    digitalWrite(13, LOW);
    rec_state = 0;
    cnt++;
    sprintf(fname, "data%04d.wav", cnt+1);
  }
}

