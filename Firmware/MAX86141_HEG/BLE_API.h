#ifndef BLE_API_H
#define BLE_API_H

//https://learn.sparkfun.com/tutorials/esp32-ota-updates-over-ble-from-a-react-web-application/all
//^^ Implement Bluetooth OTA ^^

/*
 *  pOtaCharacteristic = pService->createCharacteristic(
                     CHARACTERISTIC_UUID_FW,
                     BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE
                     );
    pOtaCharacteristic->addDescriptor(new BLE2902());
    pOtaCharacteristic->setCallbacks(new otaCallback(this));
        
    void otaCallback::onWrite(BLECharacteristic *pCharacteristic)
    {
      std::string rxData = pCharacteristic->getValue();
      if (!updateFlag) { //If it's the first packet of OTA since bootup, begin OTA
        Serial.println("BeginOTA");
        esp_ota_begin(esp_ota_get_next_update_partition(NULL), OTA_SIZE_UNKNOWN, &otaHandler);
        updateFlag = true;
      }
      if (_p_ble != NULL)
      {
        if (rxData.length() > 0)
        {
          esp_ota_write(otaHandler, rxData.c_str(), rxData.length());
          if (rxData.length() != FULL_PACKET)
          {
            esp_ota_end(otaHandler);
            Serial.println("EndOTA");
            if (ESP_OK == esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL))) {
              esp_restart();
            }
            else {
              Serial.println("Upload Error");
            }
          }
        }
      }

  uint8_t txData[5] = {1, 2, 3, 4, 5};
  //delay(1000);
  pCharacteristic->setValue((uint8_t*)txData, 5);
  pCharacteristic->notify();
} 
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "BluetoothSerial.h"

#include "MAX86141_HEG.h"

BluetoothSerial SerialBT;

BLECharacteristic *pCharacteristic;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

bool USE_BT = false;          
bool USE_BLE = false;         
bool BLE_ON, BLE_SETUP = false;
bool BLEdeviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks
{

  void onConnect(BLEServer *pServer)
  {
    Serial.println("Device connected to BLE");
    BLEdeviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("Device disconnected from BLE");
    BLEdeviceConnected = false;
  }

};

//Setup BLE callbacks here.
class MyCallbacks : public BLECharacteristicCallbacks //We need to set up the BLE callback commands here.
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {

      Serial.println("HEG RECEIVE: ");

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }
       
      //commandESP32(rxValue.c_str());

      if (rxValue.find("t") != -1)
        { //Enable Sensor
          //
          // exit shutdown mode.
          //
          HEG1.write_reg(REG_MODE_CONFIG, 0x00);
        
          //
          // Clear interrupts.
          //
          HEG1.read_reg(REG_INT_STAT_1);
          HEG1.read_reg(REG_INT_STAT_2);
       
          coreProgramEnabled = true;
        }
        if (rxValue.find("f") != -1)
        { //Disable sensor
            HEG1.write_reg(REG_MODE_CONFIG,0b00000011);
            coreProgramEnabled = false;
        }
        if (rxValue.find("o") != -1) {
          if(MODE != "FAST") {
            MODE = "FAST";
          }
          else {
            MODE = "";
          }
        }
        if (rxValue.find("h") != -1) {
          if(MODE != "SPO2") {
            MODE = "SPO2";
          }
          else {
            MODE = "";
          }
        }
        if (rxValue.find("F") != -1){
          if(USE_FILTERS == true){
            USE_FILTERS = false;
          }
          else {
            USE_FILTERS = true;
          }
        }
        if (rxValue.find("X") != -1){
          if(USE_DC_FILTER == true){
            USE_DC_FILTER = false;
          }
          else {
            USE_DC_FILTER = true;
          }
        }
        if (rxValue.find("L") != -1) {}
        if (rxValue.find("l") != -1) {}
        if (rxValue.find("c") != -1) {}
        if (rxValue.find("r") != -1) {}
        if (rxValue.find("R") != -1) {
            HEG1.write_reg(REG_MODE_CONFIG,0b00000011);
            coreProgramEnabled = false;
            if (USE_USB == true) {
            Serial.println("Restarting ESP32...");
            }
            delay(300);
            ESP.restart();
        }
        if (rxValue.find("S") != -1) 
        {
          HEG1.write_reg(REG_MODE_CONFIG,0b00000011);
            coreProgramEnabled = false;
            Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
            delay(1000);
            esp_deep_sleep_start(); //Ends the loop() until device is reset.
        }
        if (rxValue.find("s") != -1)
        { //Standard LED mode
            HEG1.write_reg(REG_LED_SEQ_1, 0b00100001); //0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
            HEG1.write_reg(REG_LED_SEQ_2, 0b00001001);
        }
        if (rxValue.find("p") != -1)
        { //pIR Ambient-only toggle
            HEG1.write_reg(REG_LED_SEQ_1, 0b10011001); //0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
            HEG1.write_reg(REG_LED_SEQ_2, 0b00001001);
        }
        if (rxValue.find("u") != -1)
        { //USB Toggle
            EEPROM.begin(512);
            int ComMode = EEPROM.read(0);
            if(ComMode != 3){
            EEPROM.write(0,3);
            EEPROM.commit();
            EEPROM.end();
            delay(100);
            ESP.restart();
            }
            else //Default back to WiFi mode
            {
            EEPROM.write(0,0);
            EEPROM.commit();
            EEPROM.end();
            delay(100);
            ESP.restart();
            }
        }
        if (rxValue.find("B") != - 1)
        { //Bluetooth Serial Toggle
            EEPROM.begin(512);
            int ComMode = EEPROM.read(0);
            if (ComMode != 2)
            {
            EEPROM.write(0,2);
            EEPROM.commit();
            EEPROM.end();
            delay(100);
            ESP.restart();
            }
            else //Default back to WiFi mode
            {
            EEPROM.write(0,0);
            EEPROM.commit();
            EEPROM.end();
            delay(100);
            ESP.restart();
            }
        }
        if (rxValue.find("b") != -1)
        { //Bluetooth LE Toggle
            EEPROM.begin(512);
            if (EEPROM.read(0) != 1)
            {
            EEPROM.write(0,1);
            EEPROM.commit();
            EEPROM.end();
            delay(100);
            ESP.restart();
            }
            else //Default back to WiFi mode
            {
            EEPROM.write(0,0);
            EEPROM.commit();
            EEPROM.end();
            delay(100);
            ESP.restart();
            }
        }
        delay(100);
      }
    }
};

void setupBTSerial() {

  SerialBT.begin("HEGduino BT");
  Serial.println("HEG booted in Bluetooth Serial Mode! Pair it with your device and access the stream via standard serial monitor.");
  USE_BT = true;
  delay(100);

}

void outputBT(){
  if (USE_BT == true) //BTSerial
    {
      if (SerialBT.hasClient() == true)
      {
          SerialBT.flush();
          SerialBT.print(outputarr);
          delay(10); // 10ms delay min required for BT output only due to bottleneck in library.
      }
    }
  if (BLEdeviceConnected == true) // BLE
  {
      pCharacteristic->setValue(outputarr);
      pCharacteristic->notify();
      delay(10); // bluetooth stack will go into congestion if too many packets are sent
  }
}

void setupBLE()
  {
      // Create the BLE Device
      BLEDevice::init("HEG"); // Give it a name
      BLEDevice::setMTU(512);

      // Create the BLE Server
      BLEServer *pServer = BLEDevice::createServer();
      pServer->setCallbacks(new MyServerCallbacks());

      // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID);

      // Create a BLE Characteristic
      pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

      BLE2902 *desc = new BLE2902();
      desc->setNotifications(true);
      pCharacteristic->addDescriptor(desc);

      BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
      pCharacteristic->setReadProperty(true);
      pCharacteristic->setCallbacks(new MyCallbacks());

      // Start the service
      pService->start();

      // Start advertising
      pServer->getAdvertising()->start();
      BLE_SETUP = true;
      BLE_ON = true;
      USE_BLE = true;
      Serial.println("BLE service started, scan for HEG.");
}


#endif
