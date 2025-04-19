#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Basecamp Envera";       
const char* password = "25102019"; 

// MQTT credentials
const char* mqtt_server = "cf9992ec1ad04333b51cbe2850b55ee0.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // TLS port
const char* mqtt_user = "akmalhfzh";
const char* mqtt_password = "Wlane8b31f";
const char* mqtt_topic = "home";

// Inisialisasi objek
WiFiClientSecure espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 1 second...");
      delay(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  // Set sertifikat root untuk TLS
  espClient.setInsecure();  // Gunakan ini jika tidak memiliki sertifikat root

  // Setup MQTT server
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Generate 7 angka random (0-100)
  String payload = "";
  for (int i = 0; i < 7; i++) {
    payload += String(random(0, 100));
    if (i < 6) payload += ",";  // Tambahkan koma di antara angka, kecuali di akhir
  }

  Serial.print("Publishing: ");
  Serial.println(payload);
  client.publish(mqtt_topic, payload.c_str());

  delay(250);
}
