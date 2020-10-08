 //MAX86141 HEG Biofeedback Firmware. By Joshua Brewster - MIT License
//People who contributed: Diego Schmaedech, Michael Lyons, Ben Hale

#include "WiFi_API.h"
#include "BLE_API.h"

unsigned long inputMicros;

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting...");

  currentMicros = esp_timer_get_time();
  setupHEG();
  delay(100);
  
  EEPROM.begin(512);
  int ComMode = EEPROM.read(0); //Communication mode (WiFi, BLE, or BT Serial)
  EEPROM.end();
  if(ComMode == 1) {
    setupBLE();
  }
  else if(ComMode == 2) {
    setupBTSerial();
  }
  else if(ComMode == 3) {
    Serial.println("USB Only configuration.");
    Serial.println("Sample commands: 't': Toggle HEG program ON, 'f': Toggle HEG program OFF, 'u': Toggle WiFi mode, 'b': Toggle BLE mode, 'B': Toggle Bluetooth Serial mode");
  }
  else {
    setupWiFi();
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
