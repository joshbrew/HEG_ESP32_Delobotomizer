#include "MAX86141.h"

#include <esp_timer.h>

static int spiClk = 1000000; // 8 MHz Maximum

unsigned long currentMicros;
unsigned long LEDMicros;
unsigned long LEDFrequency = 7500; //Time (in Microseconds) between each change in LED state (Red, IR, Ambient);

bool RED_ON = false;
bool IR_ON = false;
bool AMBIENT = true;

float RED_AVG, IR_AVG, AMBIENT_AVG;

//
// Pin Definitions.
//*6
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

#define RED                   12
#define IR                    14

//uninitalised pointers to SPI objects
MAX86141 pulseOx1;
//MAX86141 pulseOx2;

void setup() {
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
  pulseOx1.spi = new SPIClass(VSPI);

  pulseOx1.SS = 5;
  //clock miso mosi ss

  //Serial.println("Init SPI Port...");
  //initialise vspi with default pins
  //SCLK = 18, MISO = 19, MOSI = 23, SS = 5, INT = 17
  Serial.println("Init Device");
  pulseOx1.spi->begin();
  delay(100);
  //Serial.println("Init Pulse Ox 1...");
  pulseOx1.setDebug(false);
  pulseOx1.init(spiClk);

  //Serial.println(pulseOx1.read_reg(0x0D));

  currentMicros = esp_timer_get_time();
  LEDMicros = currentMicros;

}

void switch_LED(){
  //pulseOx1.device_data_read();
  if(IR_ON){
      digitalWrite(RED,LOW);
      digitalWrite(IR,LOW);
      //Serial.println("AMBIENT");
      RED_ON = false;
      IR_ON = false;
      AMBIENT = true;
      return;
  }
  if(RED_ON){
    digitalWrite(RED,LOW);
    digitalWrite(IR,HIGH);
    //Serial.println("IR");
    RED_ON = false;
    AMBIENT = false;
    IR_ON = true;
    return;
  }
  if(AMBIENT){
    digitalWrite(IR,LOW);
    digitalWrite(RED,HIGH);
    //Serial.println("RED");
    AMBIENT = false;
    IR_ON = false;
    RED_ON = true;
    return;
  }
}

// the loop function runs over and over again until power down or reset
void loop() {
  
  currentMicros = esp_timer_get_time();
/*
  Serial.print("LED1");
  Serial.print(led1A);
  Serial.print("\t");
  Serial.println(led1B);

  Serial.print("LED2");
  Serial.print(led2A);
  Serial.print("\t");
  Serial.println(led2B);

  Serial.print("AMBIENT");
  Serial.print(pulseOx1.ambA[0]);
  Serial.print("\t");
  Serial.println(pulseOx1.ambB[0]);
*/

  //if(currentMicros - LEDMicros > LEDFrequency){  
    if(pulseOx1.read_reg(REG_FIFO_DATA_COUNT) >= 6){
      
      pulseOx1.device_data_read();
      int led1A = pulseOx1.led1A[0];
      int led2A = pulseOx1.led2A[0];
      int led1B = pulseOx1.led1B[0];
      int led2B = pulseOx1.led2B[0];

      /*
      float AVG = 0.16666666*(led1A + led1B + led2A + led2B + pulseOx1.ambA[0] + pulseOx1.ambB[0]);
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
      */

      AMBIENT_AVG = (pulseOx1.led3A[0] + pulseOx1.led3B[0])*0.5;

      IR_AVG = (led2A + led2B)*0.5;
      Serial.print("\t");
      Serial.print("IR: ");
      Serial.print(IR_AVG);

      RED_AVG = (led1A + led1B)*0.5;
      Serial.print("Red: ");
      Serial.print(RED_AVG);

      Serial.print("\t");
      Serial.print("Ambient: ");
      Serial.println(AMBIENT_AVG); 

      float ratio = RED_AVG/IR_AVG;
      //Serial.print("\t");
      //Serial.print("Ratio: ");
      //Serial.println(ratio*4);
      
      //switch_LED();
      LEDMicros = currentMicros;
      //delayMicroseconds(2500);
      //pulseOx1.device_data_read();
    }
  //}
}
