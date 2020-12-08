 //MAX86141 HEG Biofeedback Firmware. By Joshua Brewster - MIT License
//People who contributed: Diego Schmaedech, Michael Lyons, Ben Hale

#include "WiFi_API.h"
#include "BLE_API.h"


bool BLEtoggle = false;
bool toggleSleep = false;
bool WIFItoggle = false;
unsigned long bootMicros = 0;
unsigned long inputMicros;


void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  currentMicros = esp_timer_get_time();
  pinMode(5,OUTPUT);
  
  EEPROM.begin(512);
  int sleep = EEPROM.read(510);
  if(sleep == 1){
    EEPROM.write(510,0); //Disarm sleep toggle for next reset
    EEPROM.end();
    Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
    delay(300);
    esp_deep_sleep_start(); //Ends the loop() until device is reset.
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
  EEPROM.write(510,1); //Arm the sleep toggle (first in order of checks)
  EEPROM.write(509,0); // Disarm the wifi login reset
  EEPROM.end();
  
  if(wifiReset == 1){
    saveWiFiLogin(true,false,false,false); //Reset WiFi credentials
  }

  setupHEG();

  delay(100);
  commandESP32('f');

  //Now set up the communication protocols (Only 1 active at a time for best results!)
  if(ComMode == 1) {
     setupBLE();
     commandESP32('t');
     delay(100);
     commandESP32('f');
     delay(100);
     commandESP32('t');
     delay(100);
     commandESP32('f');
  }
  else if(ComMode == 2) {
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
  }
  else if(ComMode == 3) {
    Serial.println("USB Only configuration.");
    Serial.println("Sample commands: 't': Toggle HEG program ON, 'f': Toggle HEG program OFF, 'u': Toggle WiFi mode, 'b': Toggle BLE mode, 'B': Toggle Bluetooth Serial mode");
  }
  else {
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
  }
  bootMicros = esp_timer_get_time();
}


void toggleCheck(){ //Checks toggles on initialization
  if(currentMicros - bootMicros < 3500000){
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
  
  toggleCheck();
  HEG_core_loop();
  if(newOutputFlag){
      outputSerial();
      outputBT();
      newOutputFlag = false;
  }
  if(currentMicros - inputMicros >= 300000){ // Check input every N microseconds
    inputMicros = currentMicros;
    delayMicroseconds(1800);
    checkInput();
  }
}
