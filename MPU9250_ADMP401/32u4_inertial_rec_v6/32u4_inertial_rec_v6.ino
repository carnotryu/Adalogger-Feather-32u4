#include "Wire.h"
#include "I2Cdev.h"
#include "MPU9250.h"

#include <SD.h>
#include <SPI.h>
#include <TimerOne.h>

#define SD_ChipSelectPin 4
#define bufLen 12

File myFile;

char fname[20];
volatile int cnt = 0;
int rec_sec = 10;

MPU9250 accelgyro;
I2Cdev   I2C_M;

void getAccel_Data(void);

unsigned long start_t = 0;
unsigned long end_t = 0;

volatile int rec_state = 0; // 0: idle, 1: recording, 2: stop
volatile uint16_t cnt_buf = 0;
volatile uint16_t buf_boxes = 1;

volatile int16_t ax, ay, az;
volatile int16_t gx, gy, gz;

volatile byte buf[bufLen];
byte toSend[bufLen];

int sample_rate = 100;
int timer_t = (int) (1000000 / sample_rate);

volatile uint16_t measures[6];

int start_p = 1;
int stop_p = 0;

void setup() {
  pinMode(start_p, INPUT_PULLUP);
  pinMode(stop_p, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  digitalWrite(13, LOW);
  digitalWrite(8, LOW);
  attachInterrupt(digitalPinToInterrupt(start_p), rec_start, FALLING);
  attachInterrupt(digitalPinToInterrupt(stop_p), rec_stop, FALLING);

  Wire.begin();
  delay(200);
  if (!SD.begin(SD_ChipSelectPin)) {  
    return;
  }else{
  }  

  accelgyro.setClockSource(MPU9250_CLOCK_PLL_XGYRO);
  accelgyro.setFullScaleGyroRange(MPU9250_GYRO_FS_2000);
  accelgyro.setFullScaleAccelRange(MPU9250_ACCEL_FS_16);
  accelgyro.setSleepEnabled(false);

  myFile = SD.open ("cnt.txt");

  while (myFile.available()) {
    //Serial.println(myFile.read());
    myFile.read();
    cnt++;
  }

  myFile.close();

  sprintf(fname, "data%04d.txt", cnt+1);
  
  Timer1.initialize(timer_t);
  Timer1.attachInterrupt(getAccel_Data);  

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
  if (rec_state == 1) {
    if (cnt_buf == bufLen) {
      memcpy((char*)toSend, (char*)buf, bufLen);
      myFile.write(toSend,bufLen);
      cnt_buf = 0;
      buf_boxes++;
    }
  }
  else if (rec_state == 2) {
    myFile.close();
    myFile = SD.open ("cnt.txt", FILE_WRITE);
    myFile.print("1");
    myFile.close();
    cnt++;
    sprintf(fname, "data%04d.txt", cnt+1);  
    rec_state = 0;
    digitalWrite(13, LOW);      
  }
}

void getAccel_Data(void)
{
  interrupts();
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  noInterrupts();
  if (rec_state == 1) {
    if (cnt_buf == 0 and buf_boxes == 1) {
      myFile = SD.open(fname, FILE_WRITE);      
    }

    measures[0] = (uint16_t) 32768 + (uint16_t) ax;
    measures[1] = (uint16_t) 32768 + (uint16_t) ay;
    measures[2] = (uint16_t) 32768 + (uint16_t) az;
    measures[3] = (uint16_t) 32768 + (uint16_t) gx;
    measures[4] = (uint16_t) 32768 + (uint16_t) gy;
    measures[5] = (uint16_t) 32768 + (uint16_t) gz;

    if (cnt_buf < bufLen) {
      for (int i=0; i<bufLen; i++) {
        if (cnt_buf % 2 == 0) {
          buf[cnt_buf] = measures[cnt_buf] >> 8;
        }
        else {
          buf[cnt_buf] = measures[cnt_buf];  
        }
        cnt_buf++;        
      }
    }
  }
}

void rec_start() {
  if (rec_state == 0) {
    rec_state = 1;
    cnt_buf = 0;
    buf_boxes = 1;
    digitalWrite(13, HIGH);
  }
}

void rec_stop() {
  if (rec_state == 1) {
    rec_state = 2;
  }  
}

