#include "EspMQTTClient.h"
#include <DHT.h>

// Ganti klpX, SSID, PASSWORD, dan ROOM sebelum upload.
// Jangan commit kredensial Wi-Fi asli ke repository publik.
#define KELOMPOK "klpX"
#define ROOM "ruang1"  // Ubah menjadi ruang1, ruang2, ruang3, atau ruang4
#define WIFI_SSID "SSID_LAB"
#define WIFI_PASS "PASSWORD_LAB"
#define BROKER "broker.mqttdashboard.com"

#define PREFIX "pens/" KELOMPOK "/" ROOM
const char* T_TEMP = PREFIX "/temperature";
const char* T_HUM = PREFIX "/humidity";
const char* T_L1 = PREFIX "/lamp1";
const char* T_L2 = PREFIX "/lamp2";
const char* T_STATUS = PREFIX "/status";

#define DHTPIN 16
#define DHTTYPE DHT11
const int L1_PIN = 25;
const int L2_PIN = 26;

DHT dht(DHTPIN, DHTTYPE);
EspMQTTClient client(
  WIFI_SSID, WIFI_PASS, BROKER, "", "",
  "esp32-" KELOMPOK "-" ROOM, 1883
);

unsigned long lastPublishTime = 0;

void setLamp(int pin, const String& payload) {
  if (payload == "1") digitalWrite(pin, HIGH);
  else if (payload == "0") digitalWrite(pin, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(L1_PIN, OUTPUT);
  pinMode(L2_PIN, OUTPUT);
  digitalWrite(L1_PIN, LOW);
  digitalWrite(L2_PIN, LOW);
  dht.begin();

  client.enableDebuggingMessages();
  client.setKeepAlive(15);
  client.enableLastWillMessage(T_STATUS, "offline", true);
}

void onConnectionEstablished() {
  Serial.println("MQTT Connection established.");
  client.subscribe(T_L1, [](const String& payload) {
    setLamp(L1_PIN, payload);
  }, 1);
  client.subscribe(T_L2, [](const String& payload) {
    setLamp(L2_PIN, payload);
  }, 1);
  client.publish(T_STATUS, "online", true);
}

void loop() {
  client.loop();

  unsigned long now = millis();
  if (now - lastPublishTime >= 5000) {
    lastPublishTime = now;
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp)) client.publish(T_TEMP, String(temp, 1));
    if (!isnan(hum)) client.publish(T_HUM, String(hum, 1));
  }
}
