//MAX86141 HEG Biofeedback Firmware. By Joshua Brewster - MIT License
//People who contributed: Diego Schmaedech, Michael Lyons, Ben Hale

#include "WiFi_API.h"
#include "BLE_API.h"


bool BLEtoggle = false;
bool toggleSleep = false;
bool WIFItoggle = false;
unsigned long bootMicros = 0;
unsigned long inputMicros;
uint8_t sleepTick = 0; //

/*
  Pins:
  SS:          P5
  SCLK:        P18
  MOSI:        P23
  MISO:        P19
  INT:         P17
  VLED1:       P22
  VLED2:       P21
  SWITCH OUT:  P13
  SWITCH BT:   P12
  SWITCH WIFI: P14
  SWITCH BLE:  P27 
  BTN IN:      P26
*/


void goToSleep() {
    EEPROM.write(510,0); //Disarm sleep toggle for next reset
    EEPROM.end();
    Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
    delay(300);
    esp_deep_sleep_start(); //Ends the loop() until device is reset.
}



void toggleCheck(){ //Checks toggles on initialization
  while(currentMicros - bootMicros < 3500000){
    currentMicros = esp_timer_get_time();
    if((toggleSleep == false) && (currentMicros - bootMicros > 1000000)){
      EEPROM.begin(512);
      EEPROM.write(510,0); // Disarm the sleep toggle
      EEPROM.write(511,1); // Now arm the BLE/WiFi toggle
      EEPROM.end();
      toggleSleep = true;
      commandESP32('t');
      delay(100);
      commandESP32('f');
      Serial.println("Reset now to change the connection mode");
    }
    if((toggleSleep == true) && (BLEtoggle == false) && (currentMicros - bootMicros > 2000000) ){
      EEPROM.begin(512);
      EEPROM.write(511,0); //Disarm BLE/WiFi toggle
      EEPROM.write(509,1); // Now arm the Wifi reset toggle
      EEPROM.end();
      BLEtoggle = true;
      commandESP32('t');
      delay(100);
      commandESP32('f');
      Serial.println("Reset now to reset the WiFi credentials");
    }
    if((BLEtoggle == true) && (WIFItoggle == false) && (currentMicros - bootMicros > 3000000) ){
      EEPROM.begin(512);
      EEPROM.write(509,0); //Disarm BLE/WiFi toggle
      EEPROM.end();
      WIFItoggle = true;
      commandESP32('t');
      Serial.println("Running...");
    }
  }
}


void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  currentMicros = esp_timer_get_time();
  pinMode(5,OUTPUT);
  
  EEPROM.begin(512);
  int sleep = EEPROM.read(510);
  if(sleep == 1){
    goToSleep();
  }
  int wifiReset = EEPROM.read(509);
  int toggle = EEPROM.read(511);
  int ComMode = EEPROM.read(0); //Communication mode (WiFi, BLE, or BT Serial)
  if(toggle == 1){
    if(ComMode == 1){
      ComMode = 0;
      EEPROM.write(0,0);
    }
    else if (ComMode == 0){
      ComMode = 2;
      EEPROM.write(0,2);
    } 
    else {
      ComMode = 1;
      EEPROM.write(0,1);
    }
  }
  //EEPROM.write(510,1); //Arm the sleep toggle (first in order of checks)
  //EEPROM.write(509,0); // Disarm the wifi login reset
  EEPROM.end();
  
  if(wifiReset == 1){
    saveWiFiLogin(true,false,false,false); //Reset WiFi credentials
  }

  setupHEG();

  commandESP32('f');

  pinMode(12, INPUT);  //Switch OUT
  pinMode(13, OUTPUT); //Switch BT
  pinMode(14, OUTPUT);  //Switch WIFI
  pinMode(27, OUTPUT);  //Switch BLE
  pinMode(26, INPUT);  //Button In

  Serial.println(ComMode);
  if(ComMode == 3) { 
    Serial.println("USB Only configuration.");
    Serial.println("Sample commands: 't': Toggle HEG program ON, 'f': Toggle HEG program OFF, 'u': Toggle WiFi mode, 'b': Toggle BLE mode, 'B': Toggle Bluetooth Serial mode");
  }
  digitalWrite(13, HIGH);
  delay(50);
  bool cset = false;
  if((digitalRead(12)) || ComMode == 2) { 
     setupBTSerial();
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
     cset = true;
  } 
  digitalWrite(13,LOW);
  digitalWrite(14, HIGH);
  delay(50);
 if((digitalRead(12) && cset == false) || ComMode == 1) {
     setupWiFi();
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
     cset = true;
  } 
  digitalWrite(14, LOW);
  digitalWrite(27, HIGH);
  delay(50);
 if(digitalRead(12) || cset == false) {
     setupBLE();
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
     cset = true;
  }
  digitalWrite(27, LOW);
  //Now set up the communication protocols (Only 1 active at a time for best results!)
  bootMicros = esp_timer_get_time();
  //toggleCheck();
  commandESP32('t');
  Serial.println("Running...");
}



void checkInput()
{
  if (USE_BT == true)
  {
    while (SerialBT.available())
    {
      received = SerialBT.read();
      SerialBT.println(received);
      commandESP32(received); //Currently defined in WiFi_API.h due to some awkward coding
      SerialBT.read(); //Flush endline for single char response
    }
  }
  if (USE_USB == true)
  {
    while (Serial.available())
    {
      received = Serial.read();
      Serial.println(received);
      commandESP32(received);
      Serial.read();
    }
  }
}

void loop(){
  currentMicros = esp_timer_get_time();
  //Serial.print("Time (ms): ");
  //Serial.println(currentMillis);
  
  HEG_core_loop();
  if(newOutputFlag){
      outputSerial();
      outputBT();
      newOutputFlag = false;
  }
  if(currentMicros - inputMicros >= 330000){ // Check input every N microseconds
//    int SLP = digitalRead(26);
//    if(SLP == 1) {
//      sleepTick++;
//      if(sleepTick > 9) {
//        commandESP32('S');
//      }
//    } else { sleepTick = 0; }
    
    inputMicros = currentMicros;
    delayMicroseconds(1800);
    checkInput();
  }
}
