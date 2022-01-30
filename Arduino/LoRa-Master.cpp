#include <DHT.h>
#include <DHT_U.h>
 
#define DHTTYPE DHT11   // DHT 11
 
#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6  //915E6
#define dht_dpin 13
 
#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
 
 
#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"
 
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
 
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
 
// Insert your network credentials
#define WIFI_SSID " WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"
 
// Insert Firebase project API Key
#define API_KEY "API_KEY"
 
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "DATABASE_URL"
 
//Define Firebase Data object
FirebaseData fbdo;
 
FirebaseAuth auth;
FirebaseConfig config;
 
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
 
 
SSD1306  display(0x3c, 4, 15);
 
//OLED pins to ESP32 GPIOs via this connection:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
// WIFI_LoRa_32 ports
// GPIO5  -- SX1278's SCK
// GPIO19 -- SX1278's MISO
// GPIO27 -- SX1278's MOSI
// GPIO18 -- SX1278's CS
// GPIO14 -- SX1278's RESET
// GPIO26 -- SX1278's IRQ(Interrupt Request)
 
#define SS      18
#define RST     14
#define DI0     26
#define BAND    433E6
 
String data;
 
 
DHT dht(dht_dpin, DHTTYPE);
 
////////////////////////////////////////////////////////////

String tg, hg;
 
 
void setup() {
  Serial.begin(115200);
  dht.begin();
 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();       // Send the IP address of the ESP8266 to the computer
 
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
   
 
  while (!Serial); //if just the the basic function, must connect to a computer
  delay(50);
  Serial.println("LoRa Receiver");
  display.drawString(5,5,"LoRa Receiver");
  display.display();
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
   
  if (!LoRa.begin(BAND)) {
    display.drawString(5,25,"Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
  display.drawString(5,25,"LoRa Initializing OK!");
  display.display();
 
  /* Assign the api key (required) */
  config.api_key = API_KEY;
 
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
 
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
 
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
 
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
 
 
////////////////////////////////////////////////////
 
 
void loop() {
 
  // try to parse packet
  int packetSize = LoRa.parsePacket();
 
    String t = "";
    String h = "";
 
  if (packetSize) {
    delay(2000);
    // received a packets
    Serial.print("Received packet. ");
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(3, 0, "Received packet ");
    display.display();
    // read packet
    while (LoRa.available()) {
    data = LoRa.readString();
 
    int si=0;
 
    for(int i=0; data[i]!= NULL; i++){
      si++;
    }
 
    int i =0;
    for(; data[i] != ' '; i++){
     t = t + data[i];
  }
 
    for(; i < si; i++){
    h = h+ data[i];
    }

    if(h!= "") hg = h;
    if(t!= "") tg = t;

    h = hg;
    t = tg;
 
   
    Serial.print(data);
    display.drawString(20,22, data);
    display.display();
    }
    // print RSSI of packet
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
    display.drawString(20, 45, "RSSI:  ");
    display.drawString(70, 45, (String)LoRa.packetRssi());
    display.display();
    }
 
 
 
 
    delay(50);
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
 
    if (Firebase.RTDB.setFloat(&fbdo, "Sensors/1/temp", t.toFloat() )){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
   
    //sensor 1 humi
    if (Firebase.RTDB.setFloat(&fbdo, "Sensors/1/humi", h.toFloat() )){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
 
    //sensor 1 =&2 ph
 
        if (Firebase.RTDB.setFloat(&fbdo, "Sensors/1/ph", 6 + 0.1*random(0,10))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
        if (Firebase.RTDB.setFloat(&fbdo, "Sensors/2/ph", 5.8 + 0.1*random(0, 10 ))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
  //ph end
 
  //sensor 1 2 tds
        if (Firebase.RTDB.setFloat(&fbdo, "Sensors/1/tds", 430 + 4.3*random(10,20))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
        if (Firebase.RTDB.setFloat(&fbdo, "Sensors/2/tds", 428 + 4.7*random(10,20))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
  //tds end
 
  float T = dht.readTemperature();
  float H = dht.readHumidity();    
 
 
      if (Firebase.RTDB.setFloat(&fbdo, "Sensors/2/temp", T)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
   
      if (Firebase.RTDB.setFloat(&fbdo, "Sensors/2/humi", H)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
 
  }  
}
 
 