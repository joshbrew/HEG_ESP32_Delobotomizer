#ifndef MAX86141_HEG_H
#define MAX86141_HEG_H
//MAX86141 HEG SCRIPT

#include "MAX86141.h"
#include "spo2_algorithm.h"
#include <CircularBuffer.h>
#include <esp_timer.h>
#include "IIRfilter.h"

bool USE_FILTERS = true;
bool USE_DC_FILTER = false;

float sps = 2048/3; //Samplerate per site: 

IIRnotch notch50_R(50,sps,0.5);
IIRnotch notch50_I(50,sps,0.5);
IIRnotch notch50_A(50,sps,0.5);

IIRnotch notch50_R2(50,sps,0.5);
IIRnotch notch50_I2(50,sps,0.5);
IIRnotch notch50_A2(50,sps,0.5);

IIRnotch notch50_R3(50,sps,0.5);
IIRnotch notch50_I3(50,sps,0.5);
IIRnotch notch50_A3(50,sps,0.5);

IIRnotch notch60_R(60,sps,0.5);
IIRnotch notch60_I(60,sps,0.5);
IIRnotch notch60_A(60,sps,0.5);

IIRnotch notch60_R2(60,sps,0.5);
IIRnotch notch60_I2(60,sps,0.5);
IIRnotch notch60_A2(60,sps,0.5);

IIRnotch notch60_R3(60,sps,0.5);
IIRnotch notch60_I3(60,sps,0.5);
IIRnotch notch60_A3(60,sps,0.5);

IIRlowpass lp_R(40,sps);
IIRlowpass lp_I(40,sps);
IIRlowpass lp_A(40,sps);

IIRlowpass lp_R2(40,sps);
IIRlowpass lp_I2(40,sps);
IIRlowpass lp_A2(40,sps);

IIRlowpass lp_R3(40,sps);
IIRlowpass lp_I3(40,sps);
IIRlowpass lp_A3(40,sps);

DCBlocker dc_R(0.995);
DCBlocker dc_I(0.995);
DCBlocker dc_A(0.995);

MAX86141 HEG1;
//MAX86141 HEG2;

static int spiClk = 1000000; // 8 MHz Maximum for MAX86141

bool USE_USB = true;

char outputarr[64];
bool newOutputFlag = false;
bool newEvent = false; //WiFi event task

bool RED_ON = false;
bool IR_ON = false;
bool AMBIENT = true;
char * MODE = ""; //SPO2, DEBUG, FAST, EXT_LED (raw ambient mode with GPIO timer based external leds)
char * LEDPA = "FULL"; //FULL, HALF
char * EXPMODE = "DEFAULT"; //Exposure modes, DEFAULT, FAST, SLOW
char * LEDMODE = "DEFAULT"; //DEFAULT, 2IR, 

bool coreProgramEnabled = false;

float RED_AVG, IR_AVG, AMBIENT_AVG, RATIO_AVG;
float v1, drdt, lastRatio;

//pulse Ox library variables
CircularBuffer<uint32_t, 200> redBuffer;
CircularBuffer<uint32_t, 200> irBuffer;
int32_t bufCap = 200;

int32_t spo2;
int8_t validSPO2;
int32_t lastValidSPO2;
int32_t heartRate;
int8_t validHeartRate;
int32_t lastValidHeartRate;
////

unsigned long currentMicros = 0;
unsigned long lastSampleMicros = 0;
unsigned long coreNotEnabledMicros = 0;
unsigned long lastSPO2Micros = 0;

unsigned long SPO2Freq = 25000; //SPO2/HR calculation frequency

bool DEEP_SLEEP_EN = true;
unsigned long sleepTimeout = 600000000; //10 minutes till sleep if no activity on sensor.

//Manual LED sampling controls.
unsigned long LEDMicros = 0;
unsigned long LEDFrequency = 7500; //Time (in Microseconds) between each change in LED state (Red, IR, Ambient);

//
// Pin Definitions.
//
#define MISO_PIN              19
#define MOSI_PIN              23
#define SCK_PIN               18
#define SS_PIN                5

#define VSPI_MISO             MISO
#define VSPI_MOSI             MOSI
#define VSPI_SCLK             SCK
#define VSPI_SS               SS_PIN

#define INT_PIN               17

#define GPIO1_PIN             16
#define GPIO2_PIN             4

#define RED                   12 //External LED gpio
#define IR                    14

void setupHEG() {
  Serial.begin(115200);

  //
  // Configure IO.
  //
  pinMode(SS_PIN, OUTPUT);
  pinMode(INT_PIN, INPUT_PULLUP);
  //pinMode(GPIO1_PIN, OUTPUT);
  //pinMode(GPIO2_PIN, INPUT_PULLUP);

  pinMode(RED, OUTPUT);
  pinMode(IR, OUTPUT);
  digitalWrite(RED, LOW);
  digitalWrite(IR, LOW);

  digitalWrite(SS_PIN, LOW);
  //digitalWrite(GPIO1_PIN, HIGH);

  //initialise SPI
  HEG1.spi = new SPIClass(VSPI);
  HEG1.SS = 5;
  //clock miso mosi ss

  //Serial.println("Init SPI Port...");
  //initialise vspi with default pins
  //SCLK = 18, MISO = 19, MOSI = 23, SS = 5, INT = 17
  HEG1.spi->begin();
  delay(100);
  //Serial.println("Init Pulse Ox 1...");
  HEG1.setDebug(false);
  HEG1.init(spiClk);
  //HEG1.begin();

  //Serial.println(pulseOx1.read_reg(0x0D));

  currentMicros = esp_timer_get_time();
  coreNotEnabledMicros = currentMicros;
  coreProgramEnabled = true;

}

void sampleExternalLEDPulse(){
    if(currentMicros - LEDMicros > LEDFrequency){
        if(HEG1.read_reg(REG_FIFO_DATA_COUNT) >= 6){ //1 full sequence is 6 samples.

            HEG1.device_data_read();
            int led1A = HEG1.led1A[0];
            int led2A = HEG1.led2A[0];
            int led1B = HEG1.led1B[0];
            int led2B = HEG1.led2B[0];
            int led3A = HEG1.led3A[0];
            int led3B = HEG1.led3B[0];

            float AVG = 0.16666666*(led1A + led1B + led2A + led2B + led3A + led3B);
            //Serial.println(AVG);
            if(RED_ON) RED_AVG = AVG;
            if(IR_ON) IR_AVG = AVG;
            if(AMBIENT) AMBIENT_AVG = AVG;

            if((IR_AVG != 0) && (RED_AVG != 0) && (AMBIENT_AVG != 0)){
                RED_AVG = RED_AVG - AMBIENT_AVG;
                IR_AVG = IR_AVG - AMBIENT_AVG;
                Serial.print("Red: ");
                Serial.print(RED_AVG);
                Serial.print("\t IR:");
                Serial.print(IR_AVG);
                Serial.print("\t Ratio: ");
                Serial.println(RED_AVG/IR_AVG);
                RED_AVG = 0;
                IR_AVG = 0;
                AMBIENT_AVG = 0;
            }
        }

        if(IR_ON) {
            digitalWrite(RED,LOW);
            digitalWrite(IR,LOW);
            //Serial.println("AMBIENT");
            RED_ON = false;
            IR_ON = false;
            AMBIENT = true;
        }
        else if(RED_ON) {
            digitalWrite(RED,LOW);
            digitalWrite(IR,HIGH);
            //Serial.println("IR");
            RED_ON = false;
            AMBIENT = false;
            IR_ON = true;
        }
        else if(AMBIENT) {
            digitalWrite(IR,LOW);
            digitalWrite(RED,HIGH);
            //Serial.println("RED");
            AMBIENT = false;
            IR_ON = false;
            RED_ON = true;
        }

        delayMicroseconds(2500);
        HEG1.device_data_read();//Clear buffer (lets slower LEDs warm up and prevents overlap)

    }
}


void debugPrintLatestValues(){
    HEG1.device_data_read();
    int led1A = HEG1.led1A[0];
    int led2A = HEG1.led2A[0];
    int led1B = HEG1.led1B[0];
    int led2B = HEG1.led2B[0];
    int led3A = HEG1.led3A[0];
    int led3B = HEG1.led3B[0];

    Serial.print("LED1");
    Serial.print(led1A);
    Serial.print("\t");
    Serial.println(led1B);

    Serial.print("LED2");
    Serial.print(led2A);
    Serial.print("\t");
    Serial.println(led2B);

    Serial.print("AMBIENT");
    Serial.print(led3A);
    Serial.print("\t");
    Serial.println(led3B);
}

void outputSerial(){
    if (USE_USB == true)
    {
        //Serial.flush();
        Serial.print(outputarr);
    }
}

void sampleHEG(){
    if(HEG1.read_reg(REG_FIFO_DATA_COUNT) >= 6){ //1 full sequence is 6 samples.
      //Serial.println(HEG1.read_reg(REG_FIFO_DATA_COUNT));
      HEG1.device_data_read();
      int led1A = HEG1.led1A[0];
      int led2A = HEG1.led2A[0];
      int led1B = HEG1.led1B[0];
      int led2B = HEG1.led2B[0];
      int led3A = HEG1.led3A[0];
      int led3B = HEG1.led3B[0];

      AMBIENT_AVG = (led3A + led3B)*0.5;
      IR_AVG = (led2A + led2B)*0.5;
      RED_AVG = (led1A + led1B)*0.5;//ALC circuit automatically cancels Ambient
     
      if(USE_FILTERS == true){
        if(USE_DC_FILTER == true){
          RED_AVG = dc_R.apply(RED_AVG);
          IR_AVG = dc_I.apply(IR_AVG);
          AMBIENT_AVG = dc_A.apply(AMBIENT_AVG);
        }
        RED_AVG = notch50_R.apply(RED_AVG);
        RED_AVG = notch50_R2.apply(RED_AVG);
        RED_AVG = notch50_R3.apply(RED_AVG);
        
        RED_AVG = notch60_R.apply(RED_AVG);
        RED_AVG = notch60_R2.apply(RED_AVG);
        RED_AVG = notch60_R3.apply(RED_AVG);
        
        RED_AVG = lp_R.apply(RED_AVG);
        RED_AVG = lp_R2.apply(RED_AVG);
        RED_AVG = lp_R3.apply(RED_AVG);

        
        IR_AVG = notch50_I.apply(IR_AVG);
        IR_AVG = notch50_I2.apply(IR_AVG);
        IR_AVG = notch50_I3.apply(IR_AVG);
        
        IR_AVG = notch60_I.apply(IR_AVG);
        IR_AVG = notch60_I2.apply(IR_AVG);
        IR_AVG = notch60_I3.apply(IR_AVG);
        
        IR_AVG = lp_I.apply(IR_AVG);
        IR_AVG = lp_I2.apply(IR_AVG);
        IR_AVG = lp_I3.apply(IR_AVG);

        
        AMBIENT_AVG = notch50_A.apply(AMBIENT_AVG);
        AMBIENT_AVG = notch50_A2.apply(AMBIENT_AVG);
        AMBIENT_AVG = notch50_A3.apply(AMBIENT_AVG);
        
        AMBIENT_AVG = notch60_A.apply(AMBIENT_AVG);
        AMBIENT_AVG = notch60_A2.apply(AMBIENT_AVG);
        AMBIENT_AVG = notch60_A3.apply(AMBIENT_AVG);
        
        AMBIENT_AVG = lp_A.apply(AMBIENT_AVG);
        AMBIENT_AVG = lp_A2.apply(AMBIENT_AVG);
        AMBIENT_AVG = lp_A3.apply(AMBIENT_AVG);
      }
      
      RATIO_AVG = RED_AVG/IR_AVG;


      if(MODE == "SPO2"){
        redBuffer.push(RED_AVG);
        irBuffer.push(IR_AVG);

        if(currentMicros - lastSPO2Micros > SPO2Freq){ //
          if(redBuffer.isFull()){
            uint32_t rBufTemp[bufCap];
            uint32_t irBufTemp[bufCap];
            for(int i=0; i<bufCap; i++){
              rBufTemp[i] = redBuffer[i];
              irBufTemp[i] = irBuffer[i];
            }

            maxim_heart_rate_and_oxygen_saturation(rBufTemp, bufCap, irBufTemp, &spo2, &validSPO2, &heartRate, &validHeartRate);

            if(validSPO2 == 1){ lastValidSPO2 = spo2;}
            if(validHeartRate == 1) { lastValidHeartRate = heartRate; }
          }
          lastSPO2Micros = currentMicros;
        }
          sprintf(outputarr, "%lu|%0.0f|%0.0f|%0.4f|%0.0f|%d|%d\r\n",
            currentMicros, RED_AVG, IR_AVG, RATIO_AVG, AMBIENT_AVG, lastValidHeartRate, lastValidSPO2);
      }
      else if (MODE == "DEBUG"){
          sprintf(outputarr, "RED: %0.0f \t IR: %0.0f \t RATIO: %0.4f \t AMBIENT: %0.0f\r\n",
              RED_AVG, IR_AVG, RATIO_AVG, AMBIENT_AVG);
        }
      else if (MODE == "FAST"){
          sprintf(outputarr, "%0.0f|%0.0f|%0.4f\r\n",
              RED_AVG, IR_AVG, RATIO_AVG);
      }
      else { //Default, get 1st and 2nd derivatives
          v1 = drdt;
          drdt = (RATIO_AVG - lastRatio) / ((currentMicros - lastSampleMicros)*.000001); //1st derivative of ratio, "Velocity"
          float ddrdt = (drdt - v1) / ((currentMicros - lastSampleMicros)*0.000001); //2nd derivative of ratio, "Acceleration"

          sprintf(outputarr, "%lu|%0.1f|%0.1f|%0.4f|%0.1f|%0.3f|%0.3f\r\n",
            currentMicros, RED_AVG, IR_AVG, RATIO_AVG, AMBIENT_AVG, drdt, ddrdt);
      }
      //Serial.print("Red: ");
      //Serial.print(RED_AVG);
      //Serial.print("\t");
      //Serial.print("IR: ");
      //Serial.print(IR_AVG);
      //Serial.print("\t");
      //Serial.print("Ambient: ");
      //Serial.println(AMBIENT_AVG);

      //Serial.println(outputarr);

      newOutputFlag = true; //Flags for output functions
      newEvent = true;
      lastSampleMicros = currentMicros;
      lastRatio=RATIO_AVG;
    }
}


// the loop function runs over and over again until power down or reset
void HEG_core_loop() {

  if(currentMicros - coreNotEnabledMicros < sleepTimeout){ //Enter sleep mode after 10 min of inactivity (in microseconds).
    if(coreProgramEnabled == true){
      if(MODE != "EXT_LED"){
        sampleHEG();
      }
      else{
        sampleExternalLEDPulse();
      }
      coreNotEnabledMicros = currentMicros; // Core is enabled, sleep timer resets;
    }
  }
  else if (DEEP_SLEEP_EN == true){
    Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
    delay(1000);
    esp_deep_sleep_start(); //Ends the loop() until device is reset.
  }
}

#endif
