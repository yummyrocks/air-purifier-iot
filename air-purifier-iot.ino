// Uji coba sensor DHT22 (ESP32)

#include <DHT.h>

const int DHTPIN = 21;
#define DHTTYPE DHT22
const unsigned long INTERVAL_UJI = 2000;   // DHT22 minimal 2 detik

DHT dht(DHTPIN, DHTTYPE);
unsigned long waktuUji = 0;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Serial.println("=== UJI DHT22 ===");
}

void loop() {
  if (millis() - waktuUji < INTERVAL_UJI) return;
  waktuUji = millis();

  float suhu = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(suhu) || isnan(hum)) {
    Serial.println("Gagal membaca DHT22. Cek kabel dan pin data.");
    return;
  }

  Serial.print("Suhu: ");
  Serial.print(suhu, 1);
  Serial.print(" C  |  Kelembapan: ");
  Serial.print(hum, 1);
  Serial.println(" %");
}
