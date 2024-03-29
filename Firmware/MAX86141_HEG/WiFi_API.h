#ifndef WIFI_API_H
#define WIFI_API_H
//TODO:
/*
 * EventSource for device errors and updating
 * Threading for events when involving multiple sensors?
 * Fix some weird WiFi crashes.
 */
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
//#include <WiFiClient.h>
//#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <EEPROM.h>
//#include "SPIFFS.h"

#include "webDemo/api.h" // Uninitialized API page for external apps.
#include "webDemo/index.h"  //Index/intro page
#include "webDemo/update.h" //Update page
#include "webDemo/ws.h" // Websocket client demo page
#include "webDemo/connect.h" // Wifi connect page
#include "webDemo/HEGwebAPI.h" //HEG web javascript
#include "webDemo/initWebapp.h" //Web app custom javascript
#include "webDemo/webDemo.h" // Web app page
#include "webDemo/webDemoCSS.h" // Web app CSS page
#include "webDemo/help.h" // Help page
#include "webDemo/threeApp.h" // ThreeJS Module

#include "MAX86141_HEG.h"


AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient * globalWSClient = NULL;
AsyncEventSource events("/events");

bool defaultConnectWifi = false; //Set whether to attempt to connect to a router by default.
const char* ssid = "--------";
const char* password = "--------";
//const char* static_ip = "192.168.2.2";

//Enter your WiFi SSID and PASSWORD
const char* host = "esp32";
const char* softAPName = "My_HEG";

char received;
unsigned long eventMicros = 0;

char eventarr[64];
int eventDelayMS = 50;

size_t content_len;
unsigned long t_start,t_stop;
bool connectionSuccess = false;

String setSSID = "";
String setPass = "";
String myLocalIP = "";
String staticIP = "";
String gateway = "";
String subnetM = "";
String primaryDNS = "";
String secondaryDNS = "";

void saveWiFiLogin(bool ap_only, bool use_static, bool use_dns, bool reset){
  //512 usable address bytes [default values: 0x00], each char uses a byte.
  EEPROM.begin(512);
  int address = 2;
  //Serial.print("Previous saved ssid: ");
  //Serial.println(EEPROM.readString(address));
  Serial.print("New saved SSID: ");
  Serial.println(setSSID);
  EEPROM.writeString(address, setSSID);
  address = 128;
  //Serial.print("Previous saved password: ");
  //Serial.println(EEPROM.readString(address));
  Serial.print("New saved password: ");
  Serial.println(setPass);
  EEPROM.writeString(address, setPass);
  if((use_static == true) && (use_dns == false)){
    EEPROM.write(255, 1);
    EEPROM.writeString(256, staticIP);
    EEPROM.writeString(320, gateway);
    EEPROM.writeString(384, subnetM);
  }
  else if((use_static == true) && (use_dns == false)){
    EEPROM.write(255, 2);
    EEPROM.writeString(256, staticIP);
    EEPROM.writeString(320, gateway);
    EEPROM.writeString(384, subnetM);
    EEPROM.writeString(400, primaryDNS);
    EEPROM.writeString(416, secondaryDNS);
  }
  else { EEPROM.write(255,0); EEPROM.writeString(256, ""); EEPROM.writeString(320, ""); EEPROM.writeString(384, ""); EEPROM.writeString(400, ""); EEPROM.writeString(416, "");}
  if(ap_only == true){ EEPROM.write(1, 0); } //Address of whether to attempt a connection by default.
  else { EEPROM.write(1,1); }
  Serial.println("Committing to flash...");
  EEPROM.commit();
  delay(100);
  EEPROM.end();
  
  if(reset == true){ Serial.println("Resetting ESP32..."); ESP.restart();} //Reset to trigger auto-connect
}

void connectAP(){
  //ESP32 As access point IP: 192.168.4.1
  Serial.println("Starting local access point, scan for "+String(softAPName)+" in available WiFi connections");
  Serial.println("Log in at 192.168.4.1 or try http://"+String(host)+".local after connecting to the access point successfully");
  //WiFi.mode(WIFI_AP); //Access Point mode, creates a local access point
  WiFi.softAP(softAPName, "12345678");    //Password length minimum 8 char 
  //myLocalIP = "192.168.4.1";
}

IPAddress parseIP(String ipString){ //Parse IP from string
  int i = ipString.indexOf('.');
  String temp1 = ipString.substring(0,i);
  int j = ipString.indexOf('.', i+1);
  String temp2 = ipString.substring(i+1,j);
  int k = ipString.indexOf('.', j+1);
  String temp3 = ipString.substring(j+1,k);
  String temp4 = ipString.substring(k+1);
  return IPAddress(temp1.toInt(),temp2.toInt(),temp3.toInt(),temp4.toInt());
}

void setupStation(bool use_static, bool use_dns){
  Serial.println("Setting up WiFi Connection...");

  //WiFi.mode(WIFI_STA);
  if(use_static == true) {
    //str.split('.');
    if(staticIP.indexOf('.') != -1) {
      IPAddress staticIPAddress = parseIP(staticIP);
      IPAddress gateWay = parseIP(gateway);
      IPAddress subnet= parseIP(subnetM);
      Serial.print("Static IP: ");
      Serial.println(staticIPAddress);
      Serial.print("Gateway IP: ");
      Serial.println(gateWay);
      Serial.print("Subnet Mask: ");
      Serial.println(subnet);
      if(use_dns == false){
        WiFi.config(staticIPAddress, gateWay, subnet);
      }
      else {
        IPAddress primary = parseIP(primaryDNS);
        if(secondaryDNS != ""){
          IPAddress secondary = parseIP(secondaryDNS);
          WiFi.config(staticIPAddress, gateWay, subnet, primary, secondary);
        }
        WiFi.config(staticIPAddress, gateWay, subnet, primary);
      }
    }  
    else { 
      Serial.println("No saved Static IP.");
    }
  }
  Serial.println("Connecting to SSID: ");
  Serial.print(setSSID);
  WiFi.begin(setSSID.c_str(),setPass.c_str());
  int wait = 0;
  while((WiFi.waitForConnectResult() != WL_CONNECTED)){
        if(wait >= 0){ break; }     
        Serial.print("...");
        wait++;
        delay(100);
      }
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    //If connection successful show IP address in serial monitor
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(setSSID);
    Serial.print("IP address: ");
    myLocalIP = WiFi.localIP().toString();
    gateway = WiFi.gatewayIP().toString();
    subnetM = WiFi.subnetMask().toString();
    Serial.println(myLocalIP);  //IP address assigned to your ESP
    Serial.println("Connect to host and access via the new local IP assigned to the ESP32, or try http://" + String(host) +".local");
    connectionSuccess = true;
  }
  else{
    WiFi.disconnect();
    Serial.println("");
    Serial.println("Connection Failed.");
    Serial.print("Attempted at SSID: ");
    Serial.println(setSSID);
    delay(100);
    connectionSuccess = false;
    connectAP();
  }
}

// HTTP server GET/POST handles.

//Return JSON string of WiFi scan results
String scanWiFi(){
  String json = "<div>[";
  int n = WiFi.scanComplete();
  if(n == -2){
    WiFi.scanNetworks(true); //WiFi.scanNetworks(true,true); // Exposes hidden networks
  } else if(n){
    for (int i = 0; i < n; ++i){
      if(i) json += ",";
      json += "{";
      json += "\"rssi\":"+String(WiFi.RSSI(i));
      json += ",\"ssid\":\""+WiFi.SSID(i)+"\"";
      json += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
      json += ",\"channel\":"+String(WiFi.channel(i));
      json += ",\"secure\":"+String(WiFi.encryptionType(i));
      //json += ",\"hidden\":"+String(WiFi.isHidden(i)?"true":"false");
      json += "}<br>";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  json += "]</div>";
  //request->send(200, "text/html", json);
  return json;
}

//ESP32 SERIAL COMMAND MODULE
void commandESP32(char received)
{
  if (received == 't')
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
  if (received == 'f')
  { //Disable sensor
    HEG1.write_reg(REG_MODE_CONFIG,0b00000010);
    coreProgramEnabled = false;
  }
  if (received == 'N'){
    if(MODE == "SPO2"){
      MODE = "";
    }
    else {
      MODE = "SPO2";
    }
  }
  if (received == 'T'){
    if(MODE == "TEMP"){
      MODE = "";
    }
    else {
      MODE = "TEMP";
    }
  }
  if (received == 'D'){ // Debug mode (for Arduino plotter)
    if(MODE == "DEBUG"){
      MODE = "";
    }
    else {
      MODE = "DEBUG";
    }
  }
  if (received == 'L'){ // External LED
    if(MODE == "EXT_LED"){
      coreProgramEnabled=false;
      MODE = "";
      HEG1.write_reg(REG_MODE_CONFIG,0b00000010);
      HEG1.write_reg(REG_LED_SEQ_1, 0b00100011); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
      HEG1.write_reg(REG_LED_SEQ_2, 0b00001001); //DATA BUF 4 | DATA BUF 3  //
      HEG1.write_reg(REG_LED1_PA, 255); // 0 = 0mA, 255 = Max mA
      HEG1.write_reg(REG_LED2_PA, 255);
      HEG1.write_reg(REG_LED3_PA, 128);
      HEG1.write_reg(REG_MODE_CONFIG, 0x00);
      HEG1.read_reg(REG_INT_STAT_1);
      HEG1.read_reg(REG_INT_STAT_2);
      coreProgramEnabled=true;
    }
    else{
      coreProgramEnabled=false;
      MODE = "EXT_LED"; 
      HEG1.write_reg(REG_MODE_CONFIG,0b00000010);
      HEG1.write_reg(REG_LED_SEQ_1, 0b10011001); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
      HEG1.write_reg(REG_LED_SEQ_2, 0b00001001); //DATA BUF 4 | DATA BUF 3  //
      HEG1.write_reg(REG_LED1_PA, 0); // 0 = 0mA, 255 = Max mA
      HEG1.write_reg(REG_LED2_PA, 0);
      HEG1.write_reg(REG_LED3_PA, 0);
      MODE = "EXT_LED"; 
      HEG1.write_reg(REG_MODE_CONFIG, 0x00);
      HEG1.read_reg(REG_INT_STAT_1);
      HEG1.read_reg(REG_INT_STAT_2);
      coreProgramEnabled=true;
    }
  }
  if (received == 'i'){ //change intensity
    if(LEDPA == "FULL"){
      HEG1.write_reg(REG_LED1_PA, 128); // 0 = 0mA, 255 = Max mA
      HEG1.write_reg(REG_LED2_PA, 128);
      HEG1.write_reg(REG_LED3_PA, 128);
      LEDPA = "HALF";
    }
    else {
      HEG1.write_reg(REG_LED1_PA, 255); // 0 = 0mA, 255 = Max mA
      HEG1.write_reg(REG_LED2_PA, 255);
      HEG1.write_reg(REG_LED3_PA, 255);
      LEDPA = "FULL";
    }
  }
  if (received == 'l') { //change LED modes
    //
    // Reset part and place into shutdown mode for config.
    //
    HEG1.write_reg(REG_MODE_CONFIG, 0x02);
    HEG1.read_reg(REG_INT_STAT_1);
    HEG1.read_reg(REG_INT_STAT_2);
    if(LEDMODE == "DEFAULT") {
        HEG1.write_reg(REG_LED_SEQ_1, 0b10010010); //write_reg(REG_LED_SEQ_1, 0b00100001); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
        HEG1.write_reg(REG_LED_SEQ_2, 0b00000100); //DATA BUF 4 | DATA BUF 3  //
        LEDMODE = "REDISAMB";
    } else if (LEDMODE == "REDISAMB") {
      HEG1.write_reg(REG_LED_SEQ_1, 0b00100011); //write_reg(REG_LED_SEQ_1, 0b00100001); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
      HEG1.write_reg(REG_LED_SEQ_2, 0b00000100); //DATA BUF 4 | DATA BUF 3  //  
      LEDMODE = "2IRAMB";
    }
    else if (LEDMODE == "2IRAMB") {
      HEG1.write_reg(REG_LED_SEQ_1, 0b00100100); //write_reg(REG_LED_SEQ_1, 0b00100001); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
      HEG1.write_reg(REG_LED_SEQ_2, 0b00001001); //DATA BUF 4 | DATA BUF 3  //  
      LEDMODE = "2IR";
    } else if (LEDMODE == "2IR") {
      HEG1.write_reg(REG_LED_SEQ_1, 0b00100011); //write_reg(REG_LED_SEQ_1, 0b00100001); //DATA BUF 2 | DATA BUF 1  // 0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
      HEG1.write_reg(REG_LED_SEQ_2, 0b00001001); //DATA BUF 4 | DATA BUF 3  //  
      LEDMODE = "DEFAULT";
    } 
    //
    // exit shutdown mode.
    //
    HEG1.write_reg(REG_MODE_CONFIG, 0x00);
    
  }
  if (received == 'e'){
    //
    // Reset part and place into shutdown mode for config.
    //
    HEG1.write_reg(REG_MODE_CONFIG, 0x02);
    HEG1.read_reg(REG_INT_STAT_1);
    HEG1.read_reg(REG_INT_STAT_2);
  
    if(EXPMODE == "FAST") {
      //
      // PPG1 & 2 & 3
      //
      HEG1.write_reg(REG_PPG_SYNC_CTRL, 0b00000000);
      HEG1.write_reg(REG_PPG_CONFIG_1,  0b00000010); //ALC_DIS,ADD_OFF,PPG2_RGE,PPG1_RGE,PPG_TINT
      HEG1.write_reg(REG_PPG_CONFIG_2,  0b10011100); //SPS (0-5), SMP_AVE (6-8)
      HEG1.write_reg(REG_PPG_CONFIG_3,  0b10000110); //LED_SETLNG, DIG_FILT_SEL, BURST_EN
      HEG1.write_reg(REG_PICKET_FENCE,  0b01000010); //PF_ENABLE, PF_ORDER, IIR_TC, IIR_INIT_VALUE, THRESHOLD_SIGMA_MULT
      EXPMODE = "DEFAULT";
    } else if (EXPMODE == "DEFAULT") {
      //
      // PPG1 & 2 & 3
      //
      HEG1.write_reg(REG_PPG_SYNC_CTRL, 0b00000000);
      HEG1.write_reg(REG_PPG_CONFIG_1,  0b00000011); //ALC_DIS,ADD_OFF,PPG2_RGE,PPG1_RGE,PPG_TINT
      HEG1.write_reg(REG_PPG_CONFIG_2,  0b10011011); //SPS (0-5), SMP_AVE (6-8)
      HEG1.write_reg(REG_PPG_CONFIG_3,  0b11000110); //LED_SETLNG, DIG_FILT_SEL, BURST_EN
      HEG1.write_reg(REG_PICKET_FENCE,  0b01000010); //PF_ENABLE, PF_ORDER, IIR_TC, IIR_INIT_VALUE, THRESHOLD_SIGMA_MULT
      EXPMODE = "SLOW";
    } else if (EXPMODE == "SLOW") {
    //
      // PPG1 & 2 & 3
      //
      HEG1.write_reg(REG_PPG_SYNC_CTRL, 0b00000000);
      HEG1.write_reg(REG_PPG_CONFIG_1,  0b00000000); //ALC_DIS,ADD_OFF,PPG2_RGE,PPG1_RGE,PPG_TINT
      HEG1.write_reg(REG_PPG_CONFIG_2,  0b10011100); //SPS (0-5), SMP_AVE (6-8)
      HEG1.write_reg(REG_PPG_CONFIG_3,  0b00000110); //LED_SETLNG, DIG_FILT_SEL, BURST_EN
      HEG1.write_reg(REG_PICKET_FENCE,  0b01000010); //PF_ENABLE, PF_ORDER, IIR_TC, IIR_INIT_VALUE, THRESHOLD_SIGMA_MULT
      EXPMODE = "FAST";
    }
    //
    // exit shutdown mode.
    //
    HEG1.write_reg(REG_MODE_CONFIG, 0x00);

  }
  if (received == 'r'){ //Higher intensity
    }
  if (received == 'c'){}
  if (received == 'F'){
    if(USE_FILTERS == true){
      USE_FILTERS = false;
    }
    else {
      USE_FILTERS = true;
    }
  }
  if (received == 'X'){
    if(USE_DC_FILTER == true){
      USE_DC_FILTER = false;
    }
    else {
      USE_DC_FILTER = true;
    }
  }
  if (received == 'o'){
    if(MODE != "FAST"){
      MODE = "FAST";
    }
    else{
      MODE = "";
    }
  }
  if (received == 'h'){
    if(MODE != "SPO2"){
      MODE = "SPO2";
    }
    else{
      MODE = "";
    }
  }
  if (received == 'W') { //Reset wifi mode.
    saveWiFiLogin(true,false,false,true);
  }
  if (received == 'R') {
    HEG1.write_reg(REG_MODE_CONFIG,0b00000011);
    coreProgramEnabled = false;
    if (USE_USB == true) {
      Serial.println("Restarting ESP32...");
    }
    delay(300);
    ESP.restart();
  }
  if (received == 'S') {
    HEG1.write_reg(REG_MODE_CONFIG,0b00000011);
    coreProgramEnabled = false;
    Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
    delay(1000);
    esp_deep_sleep_start(); //Ends the loop() until device is reset.
  }
  if (received == 's'){ //Standard LED mode
    HEG1.write_reg(REG_LED_SEQ_1, 0b00100001); //0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
	  HEG1.write_reg(REG_LED_SEQ_2, 0b00001001);
  }
  if (received == 'p')
  { //Ambient-only toggle
    HEG1.write_reg(REG_LED_SEQ_1, 0b10011001); //0001 - LED 1, 0010 - LED2, 0011 - LED3, 1001 - AMBIENT
	  HEG1.write_reg(REG_LED_SEQ_2, 0b00001001);
  }
  if (received == 'u')
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
    else //Default back to WIFI mode
    {
      EEPROM.write(0,1);
      EEPROM.commit();
      EEPROM.end();
      delay(100);
      ESP.restart();
    }
  }
  if (received == 'B')
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
    else //Default back to WIFI mode
    {
      EEPROM.write(0,1);
      EEPROM.commit();
      EEPROM.end();
      delay(100);
      ESP.restart();
    }
  }
  if (received == 'b')
  { //Bluetooth LE Toggle
    EEPROM.begin(512);
    if (EEPROM.read(0) != 0)
    {
      EEPROM.write(0,0);
      EEPROM.commit();
      EEPROM.end();
      delay(100);
      ESP.restart();
    }
    else //Default back to WIFI mode
    {
      EEPROM.write(0,1);
      EEPROM.commit();
      EEPROM.end();
      delay(100);
      ESP.restart();
    }
  }
  delay(100);
}


void handleDoCommand(AsyncWebServerRequest *request){
  for(uint8_t i = 0; i< request->args(); i++){
    if(request->argName(i) == "command"){
      commandESP32(char(String(request->arg(i))[0]));
    }
  }
}

void handleWiFiSetup(AsyncWebServerRequest *request){
  String suggestedIP = "";
  if(connectionSuccess == true){suggestedIP = "Suggested IP config for SSID " + setSSID + ":: Static IP: " + myLocalIP + " | Gateway IP: " + gateway + " | Subnet Mask: " + subnetM + "<br>";}
  String scanResult = scanWiFi();
  request->send(200, "text/html", connect_page1 + suggestedIP + scanResult + connect_page2); //Send web page 
}

void handleDoConnect(AsyncWebServerRequest *request) {

  bool save = true;
  bool ap_only = false;
  bool use_static = false;
  bool use_dns = false;
  bool bleSwitch = false;
  bool btserSwitch = false;
  bool usbonlySwitch = false;
  bool suggestIP = false;
  
  for(uint8_t i = 0; i < request->args(); i++){
    if(request->argName(i) == "ssid"){setSSID = String(request->arg(i)); Serial.print("SSID: "); Serial.println(setSSID);}
    if(request->argName(i) == "pw"){setPass = String(request->arg(i)); Serial.print("Password: "); Serial.println(setPass);}
    if(request->argName(i) == "static"){ staticIP = String(request->arg(i)); Serial.print("Static IP: "); Serial.println(staticIP);}
    if(request->argName(i) == "gateway"){ gateway = String(request->arg(i)); Serial.print("Gateway IP: "); Serial.println(gateway);}
    if(request->argName(i) == "subnet"){ subnetM = String(request->arg(i)); Serial.print("Subnet Mask: "); Serial.println(subnetM);  }
    if(request->argName(i) == "primary"){ primaryDNS = String(request->arg(i)); Serial.print("Primary DNS: "); Serial.println(primaryDNS); }
    if(request->argName(i) == "secondary"){ secondaryDNS = String(request->arg(i)); Serial.print("Secondary DNS: "); Serial.println(secondaryDNS); }
    if(request->argName(i) == "choices") { 
      if      (String(request->arg(i)) == "0") {use_static = true; Serial.println("Use Static IP: true");}
      else if (String(request->arg(i)) == "1") {ap_only = true; Serial.println("AP Only: true");}
      else if (String(request->arg(i)) == "2") {bleSwitch = true; Serial.println("Use Bluetooth LE: true");}
      else if (String(request->arg(i)) == "3") {use_dns = true; Serial.println("Use Static IP with DNS: true"); }
      else if (String(request->arg(i)) == "4") {suggestIP = true; Serial.println("Suggesting IP...");}
      else if (String(request->arg(i)) == "5") {btserSwitch = true; Serial.println("Use Bluetooth Serial: true");}
      else if (String(request->arg(i)) == "6") {usbonlySwitch = true; Serial.println("Use USB Only: true");}
    }
    //if(request->argName(i) == "save"){save = bool(request->arg(i)); Serial.println(save);}
  }
  delay(100);
  if ((bleSwitch == false) && (btserSwitch == false) && (usbonlySwitch == false)) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device connects.");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/");
    request->send(response);
    delay(100);
    if(suggestIP==true){
      setupStation(false,false); //IP temp saved in localIP variable
      if(connectionSuccess == true){
        Serial.print("Suggested IP: "); Serial.println(myLocalIP);
        Serial.print("Gateway IP: "); Serial.println(gateway);
        Serial.print("Subnet Mask: "); Serial.println(subnetM);
        delay(1000);
        WiFi.disconnect();
        delay(100);
        connectAP(); //Now go to /connect again and it will suggest the last IP for this SSID.
      }
    }
    else { 
      if(save==true){
        saveWiFiLogin(ap_only,use_static,use_dns,false);
      }
      if(setSSID.length() > 0) {
        delay(300);
        if(use_static == true){
          setupStation(true,false);
        }
        else if (use_dns == true) {
          setupStation(true,true);
        }
        else { 
          setupStation(false,false); 
        }
      }
      else {
        ESP.restart();
      }
    }
  }
  else if (bleSwitch == true) {
    commandESP32('b');
  }
  else if (btserSwitch == true){
    commandESP32('B');
  }
  else if (usbonlySwitch == true){
    commandESP32('u');
  }
  delay(100);
}

void handleUpdate(AsyncWebServerRequest *request) {
  //char* html = "<h4>Upload compiled sketch .bin file</h4><form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", update_page);
  request->send(response);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  //uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
  if (!index){
    Serial.println("Update");
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)){
      Update.printError(Serial);
    }
  }
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }
  if (final) {
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      String msg = "Update complete, the device will reboot.";
      events.send(msg.c_str(), "message", esp_timer_get_time());
      Serial.flush();
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
      response->addHeader("Refresh", "20");  
      response->addHeader("Location", "/");
      request->send(response);
      delay(100);
      ESP.restart();
    }
  }
}

void printProgress(size_t prg, size_t sz) {
  String progress = "Upload Success! Progress: " + String((prg*100)/content_len);
  Serial.println(progress);
  events.send(progress.c_str(),"message",esp_timer_get_time());
}

void eventTask(void * param) {
  while(true) {
    eventMicros = currentMicros;
    if(newEvent == true) { //Prevents sending the same thing twice if the output is not updated
      events.send(outputarr,"heg",esp_timer_get_time());
      //memcpy(eventarr,outputarr,64);
      newEvent = false;
    }
    //adc0 = ads.readADC_SingleEnded(adcChannel); // test fix for weird data bug
    vTaskDelay(eventDelayMS / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void wsHEGTask(void *p){
  while(globalWSClient != NULL && globalWSClient->status() == WS_CONNECTED){
    globalWSClient->text(outputarr);
    vTaskDelay(75 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

//void evsHEGTask(void *p){
  //events.send(output,"event",millis());
//}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
 
  if(type == WS_EVT_CONNECT){
 
    Serial.println("Websocket client connection received");
    //commandESP32('t');
    globalWSClient = client;
    xTaskCreate(wsHEGTask,"HEG_ws",8196,NULL,2,NULL);
 
  } else if(type == WS_EVT_DISCONNECT){
 
    Serial.println("Websocket client connection finished");
    //commandESP32('f');
    globalWSClient = NULL;
 
  }
}

void setupWiFi(){

  //if(!SPIFFS.begin(true)){
  //  Serial.println("An Error has occurred while mounting SPIFFS");
  //  return;
  //}
  WiFi.setTxPower(WIFI_POWER_13dBm); //19_5Dbm, 19dBm, 18.5dBm, 17dBm, 15 dBm, 13dBm, 11dBm, 8_5dBm, 7dBm, 2dBm, MINUS_1dBm
  EEPROM.begin(512);
  //Serial.println(EEPROM.read(0));
  //Serial.println(EEPROM.readString(2));
  //Serial.println(EEPROM.readString(128));
  if ((defaultConnectWifi == true) || (EEPROM.read(1) == 1)){
  //ESP32 connects to your wifi -----------------------------------
    if(EEPROM.read(1) == 1){ //Read from flash memory
      setSSID = EEPROM.readString(2);
      setPass = EEPROM.readString(128);
      staticIP = EEPROM.readString(256);
      gateway = EEPROM.readString(320);
      subnetM = EEPROM.readString(384);
      primaryDNS = EEPROM.readString(400);
      secondaryDNS = EEPROM.readString(416);
    }
    else{
      setSSID = String(ssid);
      setPass = String(password);
    }
    if(EEPROM.read(255) == 1){
      setupStation(true, false);
    }
    else if(EEPROM.read(255) == 2){
      setupStation(true, true);
    }
    else{
      setupStation(false,false);
    }
  //----------------------------------------------------------------
  }
  else {
    if (WiFi.waitForConnectResult() != WL_CONNECTED){
      connectAP();
    }
  }
  EEPROM.end();
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
    }
    //send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    //client->send("hello!",NULL,millis(),1000);
  });
  
  //HTTP Basic authentication
  //events.setAuthentication("user", "pass");
  server.addHandler(&events);
  xTaskCreate(eventTask, "eventTask", 8192, NULL, 2, NULL);

  WiFi.scanNetworks(true);
  WiFi.scanDelete();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    coreNotEnabledMicros = currentMicros; //Resetting the timer on page access.
    AsyncWebServerResponse *response = request->beginResponse(200,"text/html",MAIN_page);
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });      //This is the default display page
  //server.on("/events", HTTP_GET, [](AsyncWebServerRequest *request) {
  //  Serial.println("GET events");
  //  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
  //  response->addHeader("Access-Control-Allow-Origin", "*");  
  //  request->send(response);
  //});
  	server.on("/help", HTTP_GET,[](AsyncWebServerRequest *request)
	{
    coreNotEnabledMicros = currentMicros;
		request->send_P(200,"text/html", help_page);
	});

  server.on("/stream",HTTP_GET, [](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    //AsyncWebServerResponse *response = request->beginResponse(200,"text/html",ws_page);
    //response->addHeader("Access-Control-Allow-Origin", "*");
    request->send_P(200,"text/html",ws_page);
    delay(1000);
  });
  server.on("/listen",HTTP_GET, [](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    //xTaskCreate(evsHEGTask,"evsHEGTask",8196,NULL,2,NULL);
    //commandESP32('t');
    //AsyncWebServerResponse *response = request->beginResponse(200,"text/html",event_page);
    //response->addHeader("Access-Control-Allow-Origin", "*");
    request->send_P(200,"text/html", event_page);
  });
  server.on("/connect",HTTP_GET,[](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    handleWiFiSetup(request);
  });
  server.on("/doConnect",HTTP_POST, [](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    handleDoConnect(request);
  });
  //First request will return 0 results unless you start scan from somewhere else (loop/setup)
  //Do not request more often than 3-5 seconds
  server.on("/doScan", HTTP_POST, [](AsyncWebServerRequest *request){
    String temp = scanWiFi();
  });
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    request->send(200,"text/html",scanWiFi());
  });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    handleUpdate(request);
  });
  server.on("/doUpdate", HTTP_POST,
    [&](AsyncWebServerRequest *request) {
      DEEP_SLEEP_EN = false; //Disable Deep Sleep mode
      // the request handler is triggered after the upload has finished... 
      // create the response, add header, and send response
      AsyncWebServerResponse *response = request->beginResponse((Update.hasError())?500:200, "text/plain", (Update.hasError())?"FAIL":"OK");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
      },
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
      size_t len, bool final) {
        handleDoUpdate(request, filename, index, data, len, final);
        }
  );
  server.onNotFound([](AsyncWebServerRequest *request){request->send(404);});
  
  server.on("/discovery", HTTP_GET, [](AsyncWebServerRequest *request) {
    coreNotEnabledMicros = currentMicros;
    StaticJsonDocument<1000> sjd; 
    sjd["name"] =  String(softAPName) ; 
    sjd["ipAddress"] = WiFi.localIP().toString();   
    sjd["deviceType"] = "Headset";
    sjd["location"] = setSSID;
    sjd["date"] = "";
    sjd["time"] = "";
    sjd["state"] = coreProgramEnabled ? "ON" : "OFF";
    sjd["image"] = "";
    sjd["idDevice"] = WiFi.macAddress(); 
     
    char response[1000]; 
    serializeJson(sjd, response); 
    request->send(200, "text/javascript", response); 
  });

  //Text-based commands. Send char corresponding to known commands.
  server.on("/command",HTTP_POST,[](AsyncWebServerRequest *request){
    handleDoCommand(request);
  });
  
  //POST-based commands
  server.on("/startHEG",HTTP_POST,[](AsyncWebServerRequest *request){
    commandESP32('t');
  });
  server.on("/stopHEG",HTTP_POST, [](AsyncWebServerRequest *request){
    commandESP32('f');
  });
  server.on("/restart",HTTP_POST, [](AsyncWebServerRequest *request){
    commandESP32('R');
  });
  server.on("/l",HTTP_POST, [](AsyncWebServerRequest *request){
    commandESP32('l');
  });
  server.on("/c",HTTP_POST, [](AsyncWebServerRequest *request){
    commandESP32('c');
  });
  server.on("/r",HTTP_POST, [](AsyncWebServerRequest *request){
    commandESP32('r');
  });
  server.on("/n",HTTP_POST, [](AsyncWebServerRequest *request){
    commandESP32('n');
  });
  server.on("/b",HTTP_POST, [](AsyncWebServerRequest *request){ //BLE trigger
    commandESP32('b');
  });
  server.on("/B",HTTP_POST, [](AsyncWebServerRequest *request){ //BT Serial trigger
    commandESP32('B');
  });
  server.on("/sleep",HTTP_GET, [](AsyncWebServerRequest *request){ //BT Serial trigger
    request->send(200, "text/html", "GOING TO SLEEP, RESET POWER TO REGAIN FUNCTIONALITY");
    Serial.println("HEG going to sleep now... Reset the power to turn device back on!");
    delay(1000);
    esp_deep_sleep_start();
  });

  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request){
    coreNotEnabledMicros = currentMicros;
    request->send_P(200, "text/html", api);
  });
  server.on("/HEGwebAPI.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", HEGwebAPI);
  });
  server.on("/initWebapp.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", initWebapp);
  });
  
  server.on("/webDemoCSS.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/css", webDemoCSS);
  });
  server.on("/threeApp.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/javascript", threeAppJS);
  });
  //server.on("/jquery-3.4.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
  //  request->send(SPIFFS, "/jquery-3.4.1.min.js", "text/javascript");
  //});

  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
  DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");

  Update.onProgress(printProgress);

  
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started. Bonjour service required (default enabled on Apple products)");
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
}



#endif
