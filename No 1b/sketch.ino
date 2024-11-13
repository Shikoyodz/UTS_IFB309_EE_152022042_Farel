#include "WiFi.h"
#include "PubSubClient.h"
#include "DHT.h"

// Konfigurasi WiFi
const char* ssid = "Wokwi-GUEST";
const char* pass = "";

// Konfigurasi broker MQTT
const char* mqtt_server = "broker.hivemq.com"; // Ganti dengan alamat broker Anda
const int mqtt_port = 1883;
const char* mqtt_client_id = "UTS_PemIoT_Farel_042";
const char* topic_suhu = "UTS_PemIoT_Farel_Suhu_042";
const char* topic_kelembaban = "UTS_PemIoT_Farel_Kelembaban_042";
const char* topic_relay = "UTS_PemIoT_Farel_Relay_042";

// Konfigurasi sensor DHT
#define DHTPIN 4         // Pin DHT22 ke pin 4 di ESP32
#define DHTTYPE DHT22    // Jenis sensor DHT22

// Konfigurasi pin
#define LED_RED_PIN 13    // LED Merah ke pin 13
#define LED_YELLOW_PIN 5  // LED Kuning ke pin 5
#define LED_GREEN_PIN 12  // LED Hijau ke pin 12
#define BUZZER_PIN 18     // Buzzer ke pin 18
#define RELAY_PIN 2       // Relay ke pin 2

DHT dht(DHTPIN, DHTTYPE); // Inisialisasi objek DHT
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println("Menghubungkan ke WiFi...");
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nTerhubung ke WiFi");
  Serial.print("Alamat IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("connected");
      client.publish("UTS_PemIoT_Farel_042", "Berhasil Masuk Pak Haji");
      client.subscribe("UTS_PemIoT_Farel_042");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // Setup Serial Monitor
  Serial.begin(115200);
  
  setup_wifi();  // Menghubungkan ke WiFi
  client.setServer(mqtt_server, mqtt_port);

  // Setup sensor DHT22
  dht.begin();

  // Setup pin
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Membaca suhu dan kelembaban
  float suhu = dht.readTemperature();
  float kelembaban = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembaban)) {
    Serial.println("Gagal membaca sensor!");
    return;
  }

  // Menampilkan suhu dan kelembaban di Serial Monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" C\t");
  Serial.print("Kelembaban: ");
  Serial.print(kelembaban);
  Serial.println(" %");

  // Debugging pengiriman data ke MQTT
  Serial.println("Mengirim data ke MQTT...");
  bool suhuSent = client.publish(topic_suhu, String(suhu).c_str());
  bool kelembabanSent = client.publish(topic_kelembaban, String(kelembaban).c_str());
  Serial.print("Status pengiriman suhu: ");
  Serial.println(suhuSent ? "Berhasil" : "Gagal");
  Serial.print("Status pengiriman kelembaban: ");
  Serial.println(kelembabanSent ? "Berhasil" : "Gagal");

  // Logika pengendalian LED, buzzer, dan relay
  if (suhu > 35) {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Buzzer Nyala Os");
    client.publish(topic_relay, "ON");
  } else if (suhu >= 30 && suhu <= 35) {
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    client.publish(topic_relay, "OFF");
  } else {
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    client.publish(topic_relay, "OFF");
  }

  delay(2000);
}
