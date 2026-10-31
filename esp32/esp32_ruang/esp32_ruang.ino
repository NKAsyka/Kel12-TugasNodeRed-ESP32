#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ==================================================
// 1. KONFIGURASI RUANGAN
// ==================================================

const char* ROOM = "pens/klp12/ruang2/";
// const char* ROOM = "pens/klp12/ruang1/";
// const char* ROOM = "pens/klp12/ruang3/";
// const char* ROOM = "pens/klp12/ruang4/";

// ==================================================
// 2. KONFIGURASI WIFI
// ==================================================

const char* WIFI_SSID = "SSID_LAB";
const char* WIFI_PASS = "PASSWORD_LAB";

// ==================================================
// 3. KONFIGURASI MQTT EMQX PUBLIC
// ==================================================

const char* MQTT_HOST = "broker.emqx.io";
const int MQTT_PORT = 1883;

// ==================================================
// 4. KONFIGURASI PIN ESP32
// ==================================================

const int DHT_PIN = 16;
#define DHT_TYPE DHT11

const int L1_PIN = 25;
const int L2_PIN = 26;

// Asumsi relay aktif-HIGH:
// HIGH = ON
// LOW  = OFF
//
// Jika relay aktif-LOW, balik logika output.

// ==================================================
// 5. OBJEK WIFI, MQTT, DAN SENSOR
// ==================================================

WiFiClient net;
PubSubClient mqtt(net);

DHT dht(DHT_PIN, DHT_TYPE);

String clientId;

// ==================================================
// 6. VARIABEL TOPIK MQTT
// ==================================================

char tTemp[100];
char tHumidity[100];

char tL1Set[100];
char tL2Set[100];

char tL1State[100];
char tL2State[100];

char tStatus[100];

// ==================================================
// 7. TIMER
// ==================================================

unsigned long lastConnectAttempt = 0;
unsigned long lastPublish = 0;
unsigned long lastWiFiAttempt = 0;

const unsigned long CONNECT_INTERVAL = 5000;
const unsigned long PUBLISH_INTERVAL = 5000;
const unsigned long WIFI_INTERVAL = 5000;

// ==================================================
// 8. MEMBUAT TOPIK MQTT
// ==================================================

void setupTopics() {
    snprintf(tTemp, sizeof(tTemp),
             "%stemperature", ROOM);

    snprintf(tHumidity, sizeof(tHumidity),
             "%shumidity", ROOM);

    snprintf(tL1Set, sizeof(tL1Set),
             "%slamp1/set", ROOM);

    snprintf(tL2Set, sizeof(tL2Set),
             "%slamp2/set", ROOM);

    snprintf(tL1State, sizeof(tL1State),
             "%slamp1/state", ROOM);

    snprintf(tL2State, sizeof(tL2State),
             "%slamp2/state", ROOM);

    snprintf(tStatus, sizeof(tStatus),
             "%spstatus", ROOM);

    Serial.println();
    Serial.println("========== TOPIK MQTT ==========");

    Serial.print("Suhu             : ");
    Serial.println(tTemp);

    Serial.print("Kelembapan       : ");
    Serial.println(tHumidity);

    Serial.print("Perintah lampu 1 : ");
    Serial.println(tL1Set);

    Serial.print("Status lampu 1   : ");
    Serial.println(tL1State);

    Serial.print("Perintah lampu 2 : ");
    Serial.println(tL2Set);

    Serial.print("Status lampu 2   : ");
    Serial.println(tL2State);

    Serial.print("Status ESP32     : ");
    Serial.println(tStatus);
}

// ==================================================
// 9. PUBLIKASI STATUS LAMPU
// ==================================================

void publishLampState() {
    if (!mqtt.connected()) {
        return;
    }

    const char* state1 =
        (digitalRead(L1_PIN) == HIGH) ? "1" : "0";

    const char* state2 =
        (digitalRead(L2_PIN) == HIGH) ? "1" : "0";

    bool ok1 = mqtt.publish(tL1State, state1, true);
    bool ok2 = mqtt.publish(tL2State, state2, true);

    Serial.println();
    Serial.println("========== STATUS LAMPU ==========");

    Serial.print("Lampu 1: ");
    Serial.print(state1);
    Serial.print(" | ");
    Serial.println(ok1 ? "TERKIRIM" : "GAGAL");

    Serial.print("Lampu 2: ");
    Serial.print(state2);
    Serial.print(" | ");
    Serial.println(ok2 ? "TERKIRIM" : "GAGAL");
}

// ==================================================
// 10. CALLBACK PESAN MQTT
// Payload 1 = ON, 0 = OFF
// ==================================================

void callback(char* topic, byte* data, unsigned int len) {
    String payload;
    payload.reserve(len);

    for (unsigned int i = 0; i < len; i++) {
        if (data[i] != '\n' &&
            data[i] != '\r' &&
            data[i] != ' '  &&
            data[i] != '\t') {
            payload += (char)data[i];
        }
    }

    Serial.println();
    Serial.println("========== PESAN MQTT ==========");

    Serial.print("Topic   : ");
    Serial.println(topic);

    Serial.print("Payload : [");
    Serial.print(payload);
    Serial.println("]");

    if (payload != "1" && payload != "0") {
        Serial.println("Perintah ditolak. Gunakan payload 1 atau 0.");
        return;
    }

    bool nyala = (payload == "1");

    if (strcmp(topic, tL1Set) == 0) {
        digitalWrite(L1_PIN, nyala ? HIGH : LOW);

        Serial.println(
            nyala ? "Perintah lampu 1: ON"
                  : "Perintah lampu 1: OFF"
        );

        publishLampState();
    }
    else if (strcmp(topic, tL2Set) == 0) {
        digitalWrite(L2_PIN, nyala ? HIGH : LOW);

        Serial.println(
            nyala ? "Perintah lampu 2: ON"
                  : "Perintah lampu 2: OFF"
        );

        publishLampState();
    }
    else {
        Serial.println("Topic tidak dikenali.");
    }
}

// ==================================================
// 11. KONEKSI WIFI
// ==================================================

void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print("Menghubungkan Wi-Fi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Wi-Fi terhubung!");

    Serial.print("SSID     : ");
    Serial.println(WIFI_SSID);

    Serial.print("IP ESP32 : ");
    Serial.println(WiFi.localIP());
}

// ==================================================
// 12. KONEKSI MQTT DAN RECONNECT
// ==================================================

void reconnectMQTT() {
    unsigned long now = millis();

    if (now - lastConnectAttempt < CONNECT_INTERVAL) {
        return;
    }

    lastConnectAttempt = now;

    Serial.print("Menghubungkan ke EMQX Public... ");

    // Last Will:
    // Broker mengirim status offline jika koneksi
    // terputus tanpa disconnect normal.

    bool berhasil = mqtt.connect(
        clientId.c_str(),
        tStatus,
        1,
        true,
        "offline"
    );

    if (berhasil) {
        Serial.println("BERHASIL");

        bool sub1 = mqtt.subscribe(tL1Set, 1);
        bool sub2 = mqtt.subscribe(tL2Set, 1);

        Serial.print("Subscribe lampu 1: ");
        Serial.println(sub1 ? "OK" : "GAGAL");

        Serial.print("Subscribe lampu 2: ");
        Serial.println(sub2 ? "OK" : "GAGAL");

        bool statusOK = mqtt.publish(
            tStatus, "online", true
        );

        Serial.print("Status ESP32: ");
        Serial.println(statusOK ? "ONLINE" : "GAGAL");

        // Sinkronisasi status lampu awal.
        publishLampState();

        lastPublish = millis();
    }
    else {
        Serial.print("GAGAL. MQTT state: ");
        Serial.println(mqtt.state());
    }
}

// ==================================================
// 13. SETUP
// ==================================================

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("==================================");
    Serial.println(" ESP32 DHT11 + MQTT EMQX PUBLIC");
    Serial.println("==================================");

    // Inisialisasi output relay.
    pinMode(L1_PIN, OUTPUT);
    pinMode(L2_PIN, OUTPUT);

    digitalWrite(L1_PIN, LOW);
    digitalWrite(L2_PIN, LOW);

    // Inisialisasi sensor DHT11.
    dht.begin();

    // Membuat topik.
    setupTopics();

    // Menghubungkan Wi-Fi.
    setupWiFi();

    // Client ID unik berdasarkan MAC ESP32.
    clientId = "esp32-" + WiFi.macAddress();
    clientId.replace(":", "");

    Serial.print("Client ID: ");
    Serial.println(clientId);

    // Konfigurasi MQTT.
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setCallback(callback);
    mqtt.setKeepAlive(30);
    mqtt.setBufferSize(256);
    mqtt.setSocketTimeout(10);

    Serial.println();
    Serial.println("ESP32 siap.");
    Serial.println("Perintah ON  : payload 1");
    Serial.println("Perintah OFF : payload 0");
    Serial.println("Baud rate    : 115200");
}

// ==================================================
// 14. LOOP
// ==================================================

void loop() {
    // ----------------------------------------------
    // Periksa koneksi Wi-Fi.
    // ----------------------------------------------

    if (WiFi.status() != WL_CONNECTED) {
        unsigned long now = millis();

        if (now - lastWiFiAttempt >= WIFI_INTERVAL) {
            lastWiFiAttempt = now;

            Serial.println(
                "Wi-Fi terputus. Mencoba reconnect..."
            );

            WiFi.reconnect();
        }

        return;
    }

    // ----------------------------------------------
    // Periksa koneksi MQTT.
    // ----------------------------------------------

    if (!mqtt.connected()) {
        reconnectMQTT();
        return;
    }

    // Memproses pesan MQTT masuk.
    mqtt.loop();

    // ----------------------------------------------
    // Baca dan publikasikan DHT11 setiap 5 detik.
    // ----------------------------------------------

    unsigned long now = millis();

    if (now - lastPublish >= PUBLISH_INTERVAL) {
        lastPublish = now;

        float suhu = dht.readTemperature();
        float kelembapan = dht.readHumidity();

        if (isnan(suhu) || isnan(kelembapan)) {
            Serial.println(
                "Gagal membaca DHT11. Periksa sensor dan kabel."
            );
            return;
        }

        char payloadTemp[12];
        char payloadHumidity[12];

        snprintf(
            payloadTemp,
            sizeof(payloadTemp),
            "%.1f",
            suhu
        );

        snprintf(
            payloadHumidity,
            sizeof(payloadHumidity),
            "%.1f",
            kelembapan
        );

        // Publish suhu.
        bool suhuOK = mqtt.publish(
            tTemp,
            payloadTemp,
            true
        );

        // Publish kelembapan.
        bool humidityOK = mqtt.publish(
            tHumidity,
            payloadHumidity,
            true
        );

        Serial.println();
        Serial.println("========== DATA DHT11 ==========");

        Serial.print("Suhu       : ");
        Serial.print(payloadTemp);
        Serial.print(" C | ");
        Serial.println(suhuOK ? "TERKIRIM" : "GAGAL");

        Serial.print("Kelembapan : ");
        Serial.print(payloadHumidity);
        Serial.print(" % | ");
        Serial.println(
            humidityOK ? "TERKIRIM" : "GAGAL"
        );
    }
}
