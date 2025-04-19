#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "hotspot";     
const char* password = "wlane8b31f";  

// MQTT credentials
const char* mqtt_server = "cf9992ec1ad04333b51cbe2850b55ee0.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // TLS port
const char* mqtt_user = "akmalhfzh";
const char* mqtt_password = "Wlane8b31f";
const char* mqtt_topic = "home";

// Inisialisasi objek
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Variabel untuk menyimpan data sensor (dummy)
volatile int dummy_data = 0;

// Mutex untuk sinkronisasi data antar-core
SemaphoreHandle_t mutexData;

// Task handles
TaskHandle_t TaskReadSensor;
TaskHandle_t TaskUploadMQTT;

// Fungsi untuk membaca sensor (Core 0)
void TaskReadSensorCode(void *pvParameters) {
  while (1) {
    int new_data = random(0, 100);  // Simulasi pembacaan sensor
    xSemaphoreTake(mutexData, portMAX_DELAY);
    dummy_data = new_data;  // Update variabel global dengan proteksi mutex
    xSemaphoreGive(mutexData);

    Serial.print("Sensor Read: ");
    Serial.println(dummy_data);
    vTaskDelay(pdMS_TO_TICKS(2000));  // Delay 2 detik
  }
}

// Fungsi untuk mengupload ke MQTT (Core 1)
void TaskUploadMQTTCode(void *pvParameters) {
  while (1) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    xSemaphoreTake(mutexData, portMAX_DELAY);
    int dataToSend = dummy_data;
    xSemaphoreGive(mutexData);

    // Kirim data ke MQTT
    String payload = "{ \"value\": " + String(dataToSend) + " }";
    Serial.print("Publishing: ");
    Serial.println(payload);
    client.publish(mqtt_topic, payload.c_str());

    vTaskDelay(pdMS_TO_TICKS(5000));  // Delay 5 detik
  }
}

// Fungsi koneksi WiFi
void setup_wifi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

// Fungsi koneksi ke MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  espClient.setInsecure();  // Gunakan ini jika tidak memiliki sertifikat root
  client.setServer(mqtt_server, mqtt_port);

  mutexData = xSemaphoreCreateMutex();  // Buat mutex

  // Buat task RTOS
  xTaskCreatePinnedToCore(TaskReadSensorCode, "TaskReadSensor", 10000, NULL, 1, &TaskReadSensor, 0);
  xTaskCreatePinnedToCore(TaskUploadMQTTCode, "TaskUploadMQTT", 10000, NULL, 1, &TaskUploadMQTT, 1);
}

void loop() {
  // Tidak digunakan karena RTOS menangani semua tugas
}
