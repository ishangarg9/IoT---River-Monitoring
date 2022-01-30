#include <DHT.h>
#include <DHT_U.h>

#include <SPI.h>
#include <LoRa.h>
#include "SSD1306.h"
#include<Arduino.h>      
#define DHTTYPE DHT11   // DHT 11

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
 
SSD1306  display(0x3c, 4, 15);
 
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
#define BAND    433E6  //915E6 
#define dht_dpin 13
DHT dht(dht_dpin, DHTTYPE); 
int counter = 0;

//String s[2];
 
void setup() {
  
  dht.begin();
  
  pinMode(25,OUTPUT); //Send success, LED will bright 1 second
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH);
   
  Serial.begin(115200);
  while (!Serial); //If just the the basic function, must connect to a computer
  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(5,5,"LoRa Sender");
  display.display();
   
  SPI.begin(5,19,27,18);
  LoRa.setPins(SS,RST,DI0);
  Serial.println("LoRa Sender");
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
  display.drawString(5,20,"LoRa Initializing OK!");
  display.display();
  delay(200);
}
void loop() {
  
  float t = dht.readTemperature(); 
  float h = dht.readHumidity();    

  String pack = String(t) + " " + String(h); 
  
  Serial.print("Sending packet: ");
  Serial.println(pack);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(3, 5, "Sending packet ");
  display.drawString(50, 30, pack);
  display.display();
  // send packet
  LoRa.beginPacket();
  LoRa.print(pack);
  LoRa.endPacket();
 
  delay(300);
}