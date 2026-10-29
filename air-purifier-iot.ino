#include <PubSubClient.h>
#include <DHT.h>

constexpr int BUTTON_PIN = 21;
constexpr int RELAY_PIN = 19;

// Assumes an active-low relay module.
constexpr int RELAY_ON = LOW;
constexpr int RELAY_OFF = HIGH;
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, RELAY_OFF);
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  bool buttonPressed = digitalRead(BUTTON_PIN) == HIGH;
  if (buttonPressed)
  {
    Serial.println("aosdkowkdowkd");
  }
  
  digitalWrite(RELAY_PIN, buttonPressed ? RELAY_ON : RELAY_OFF);
}