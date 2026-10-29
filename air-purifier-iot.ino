#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// // ===== WiFi & MQTT =====
const char* WIFI_SSID   = "OPPO Reno3";   // di alat asli: ganti ke WiFi/hotspot HP
const char* WIFI_PASS   = "12345677";
const char* MQTT_SERVER = "10.31.38.212";
const int   MQTT_PORT   = 1883;

#define TOPIC_DATA   "uas-iot-kelompokX/air/data"      // ESP32 kirim JSON ke sini
#define TOPIC_KIPAS  "uas-iot-kelompokX/air/kipas/set"  // HP kirim: ON / OFF / AUTO
#define TOPIC_STATUS "uas-iot-kelompokX/air/status"     // online / offline
#define TOPIC_LED    "uas-iot-kelompokX/air/led/set"    // HP kirim: AUTO / MERAH / HIJAU / OFF

// ===== Pin =====
const int DHTPIN = 21;
#define DHTTYPE DHT22
const int PIN_GAS = 34;
const int PIN_RELAY_KIPAS = 19;
#define PIN_BUZZER 27
const int PIN_LED_MERAH = 4;
const int PIN_LED_HIJAU = 23;

// ===== Pengaturan =====
#define RELAY_ACTIVE_LOW false
const int PPM_ON = 500;
const int PPM_OFF = 450;
const float SUHU_MAX = 35.0;
const float HUM_MAX = 80.0;
const unsigned long INTERVAL_BACA = 2000;  // DHT22 minimal 2 detik

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

bool statusBahaya = false;
int kipasMode = 0;                  // 0 = AUTO, 1 = ON manual, 2 = OFF manual
int ledMode = 0;                    // 0 = AUTO (keduanya nyala), 1 = MERAH, 2 = HIJAU, 3 = OFF
unsigned long waktuBaca = 0;
unsigned long waktuReconnect = 0;

void setRelay(bool on) {
  bool level = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(PIN_RELAY_KIPAS, level ? HIGH : LOW);
}

void connectWiFi() {
  Serial.print("Menyambung WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" tersambung!");
}


void setLed() {
  bool merah = ledMode != 2 && ledMode != 3;
  bool hijau = ledMode != 1 && ledMode != 3;
  digitalWrite(PIN_LED_MERAH, merah ? HIGH : LOW);
  digitalWrite(PIN_LED_HIJAU, hijau ? HIGH : LOW);
}

void perintahLed(String msg, const char* sumber) {
  msg.trim();
  msg.toUpperCase();

  if (msg == "AUTO") ledMode = 0;
  else if (msg == "MERAH") ledMode = 1;
  else if (msg == "HIJAU") ledMode = 2;
  else if (msg == "OFF") ledMode = 3;
  else return;

  setLed();

  Serial.print("Perintah LED dari ");
  Serial.print(sumber);
  Serial.print(": ");
  Serial.println(msg);
}

// Dipakai oleh MQTT (dashboard) dan Serial Monitor (tes manual)
void perintahKipas(String msg, const char* sumber) {
  msg.trim();
  msg.toUpperCase();

  if (msg == "ON") kipasMode = 1;
  else if (msg == "OFF") kipasMode = 2;
  else if (msg == "AUTO") kipasMode = 0;
  else return;

  Serial.print("Perintah kipas dari ");
  Serial.print(sumber);
  Serial.print(": ");
  Serial.println(msg);
}

// Dipanggil saat ada pesan masuk dari HP
void onMessage(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == TOPIC_KIPAS) perintahKipas(msg, "dashboard");
  else if (String(topic) == TOPIC_LED) perintahLed(msg, "MQTT");
}

// Ketik ON / OFF / AUTO di Serial Monitor untuk tes kipas
void cekSerial() {
  if (!Serial.available()) return;
  String msg = Serial.readStringUntil('\n');
  perintahKipas(msg, "serial");
}

void connectMQTT() {
  if (client.connected()) return;
  if (millis() - waktuReconnect < 5000) return;   // coba lagi tiap 5 detik
  waktuReconnect = millis();

  Serial.print("Menyambung MQTT... ");
  String clientId = "esp32-kelompokX-" + String(random(0xffff), HEX);
  // Last Will: otomatis kirim "offline" kalau ESP32 putus tiba-tiba
  if (client.connect(clientId.c_str(), NULL, NULL, TOPIC_STATUS, 0, true, "offline")) {
    Serial.println("berhasil");
    client.publish(TOPIC_STATUS, "online", true);
    client.subscribe(TOPIC_KIPAS);
    client.subscribe(TOPIC_LED);
  } else {
    Serial.print("gagal, rc=");
    Serial.println(client.state());
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(PIN_RELAY_KIPAS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_MERAH, OUTPUT);
  pinMode(PIN_LED_HIJAU, OUTPUT);
  pinMode(PIN_GAS, INPUT);

  setRelay(false);
  noTone(PIN_BUZZER);
  setLed();

  connectWiFi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(onMessage);

  Serial.println("=== SMART AIR QUALITY MONITOR - ESP32 + MQTT ===");
}

void loop() {
  connectMQTT();
  client.loop();   // wajib dipanggil terus agar pesan masuk terbaca
  cekSerial();

  if (millis() - waktuBaca < INTERVAL_BACA) return;
  waktuBaca = millis();

  // Baca sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int ppm = map(analogRead(PIN_GAS), 0, 4095, 0, 1000);

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error: Sensor DHT22 tid adwwak terbaca!");
    return;
  }

  // Logika polusi dengan hysteresis
  if (!statusBahaya && ppm > PPM_ON) statusBahaya = true;
  else if (statusBahaya && ppm < PPM_OFF) statusBahaya = false;

  bool tidakNyaman = (temp > SUHU_MAX) || (hum > HUM_MAX);

  // Kipas: ikut mode (AUTO / ON manual / OFF manual dari HP)
  bool kipasNyala;
  if (kipasMode == 1) kipasNyala = true;
  else if (kipasMode == 2) kipasNyala = false;
  else kipasNyala = statusBahaya || tidakNyaman;
  setRelay(kipasNyala);

  // LED & buzzer
  setLed();
  if (statusBahaya) tone(PIN_BUZZER, 1000, 150);
  else noTone(PIN_BUZZER);

  // Kirim data ke dashboard (JSON)
  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"suhu\":%.1f,\"kelembapan\":%.1f,\"ppm\":%d,\"status\":\"%s\",\"kipas\":\"%s\",\"mode\":\"%s\"}",
           temp, hum, ppm,
           statusBahaya ? "BAHAYA" : (tidakNyaman ? "WASPADA" : "AMAN"),
           kipasNyala ? "ON" : "OFF",
           kipasMode == 0 ? "AUTO" : (kipasMode == 1 ? "MANUAL_ON" : "MANUAL_OFF"));

  if (client.connected()) client.publish(TOPIC_DATA, payload);

  Serial.println(payload);
}
