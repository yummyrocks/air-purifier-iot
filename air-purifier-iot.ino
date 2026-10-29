// ===== Pin =====
#define PIN_RELAY_KIPAS 19

#define RELAY_ACTIVE_LOW false

bool statusRelay = false;
unsigned long waktuRelay = 0;

void setRelay(bool on) {
  bool level = RELAY_ACTIVE_LOW ? !on : on;
  digitalWrite(PIN_RELAY_KIPAS, level ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELAY_KIPAS, OUTPUT);
  setRelay(false);
}

void loop() {
  if (millis() - waktuRelay >= 1000) {
    waktuRelay = millis();
    statusRelay = !statusRelay;
    setRelay(statusRelay);
    Serial.println(statusRelay ? "Relay ON" : "Relay OFF");
  }
}
