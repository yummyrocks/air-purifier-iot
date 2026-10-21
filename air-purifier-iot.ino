#include <PubSubClient.h>
#include <DHT.h>


const int PIN_RELAY = 21;


void setup() {
  Serial.begin(115200);

  // pinMode(PIN_DHT22, INPUT);
  pinMode(PIN_REPLAY,OUTPUT);


  Serial.println("=== SMART AIR QUALITY MONITOR - ESP32 + MQTT ===");
}

void loop() {
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}
