#define PIN_BUZZER 27

void setup() {
  pinMode(PIN_BUZZER, OUTPUT);
}

void loop() {
  tone(PIN_BUZZER, 5000);
  delay(1000);
  noTone(PIN_BUZZER);
  delay(1000);
}
