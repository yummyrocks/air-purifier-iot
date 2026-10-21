#include <PubSubClient.h>
#include <DHT.h>


const int DHTPIN = 21;
const int PIN_GAS = 34;
const int PIN_LED= 4;


void setup() {
  Serial.begin(115200);

  pinMode(PIN_GAS, INPUT);
  pinMode(PIN_LED,OUTPUT);


  Serial.println("=== SMART AIR QUALITY MONITOR - ESP32 + MQTT ===");
}

void loop() {
  int ppm = map(analogRead(PIN_GAS), 0, 4095, 0, 1000);

  Serial.println(ppm);

  if (ppm > 390)
  {
    digitalWrite(PIN_LED,HIGH);
  }else {digitalWrite(PIN_LED,LOW);}
  
}
