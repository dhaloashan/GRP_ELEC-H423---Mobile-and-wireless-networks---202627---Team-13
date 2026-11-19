#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* MQTT_SERVER = "YOUR_COMPUTER_IP";

const int BUTTON_PIN = 27;
const int RED_LED_PIN = 32;
const int WHITE_LED_PIN = 33;
const int DHT_PIN = 26;

const char* COMMAND_TOPIC = "elech423/esp32-b/white/set";
const char* STATE_TOPIC = "elech423/esp32-b/white/state";
const char* SENSOR_TOPIC = "elech423/esp32-b/sensor";

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
DHT dht(DHT_PIN, DHT11);

unsigned long lastReading = 0;
unsigned long lastAttempt = 0;

void onMessage(char* topic, byte* payload, unsigned int length) {
  String command;

  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }

  command.trim();

  Serial.print("Received topic: ");
  Serial.println(topic);

  Serial.print("Received command: [");
  Serial.print(command);
  Serial.println("]");

  if (command == "ON") {
    digitalWrite(WHITE_LED_PIN, HIGH);
    Serial.println("GPIO33 set HIGH");
  } else if (command == "OFF") {
    digitalWrite(WHITE_LED_PIN, LOW);
    Serial.println("GPIO33 set LOW");
  } else {
    Serial.println("Unknown command");
    return;
  }

  mqtt.publish(STATE_TOPIC, command.c_str(), true);
}

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT); // Harici pull-down
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(WHITE_LED_PIN, LOW);

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  mqtt.setServer(MQTT_SERVER, 1883);
  mqtt.setCallback(onMessage);
}

void loop() {
  // Buton kırmızı LED'i kontrol eder.
  bool pressed = (digitalRead(BUTTON_PIN) == HIGH);
  digitalWrite(RED_LED_PIN, pressed ? HIGH : LOW);

  unsigned long now = millis();

  // MQTT bağlantısını belirli aralıklarla dene.
  if (WiFi.status() == WL_CONNECTED) {
    if (!mqtt.connected()) {
      if (now - lastAttempt >= 5000) {
        lastAttempt = now;

        if (mqtt.connect("elech423-esp32-b")) {
          Serial.println("MQTT connected!");
          mqtt.subscribe(COMMAND_TOPIC);

          mqtt.publish(
            STATE_TOPIC,
            digitalRead(WHITE_LED_PIN) == HIGH ? "ON" : "OFF",
            true
          );
        } else {
          Serial.print("MQTT failed, state = ");
          Serial.println(mqtt.state());
        }
      }
    } else {
      mqtt.loop();
    }
  }

  // Yaklaşık 2 saniyede bir sensörü oku.
  if (now - lastReading >= 2000) {
    lastReading = now;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("DHT11 read failed!");
    } else {
      char message[100];

      snprintf(
        message,
        sizeof(message),
        "{\"temperature\":%.1f,\"humidity\":%.1f}",
        temperature,
        humidity
      );

      Serial.println(message);

      if (mqtt.connected()) {
        bool sent = mqtt.publish(SENSOR_TOPIC, message);
        if (!sent) {
          Serial.println("Sensor publish failed!");
        }
      }
    }
  }
}