#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define DHT_GPIO 14     // DHT22 on GPIO 14==D5
#define ONE_WIRE_BUS 12 // DS18B20 ON GPIO 12==D6

long last_report = 0;
int ds18b20_device_count = 0;
String current_ip = "";

// How often should we gather and send the information, in milliseconds:
const int report_interval = 60000;

// Wifi config:
const char* wifi_ssid = "ssid";
const char* wifi_password = "xxx";

// MQTT server config:
const char* mqtt_server = "xxx.xxx.xxx.xxx";
const String mqtt_clientid = "garasje";
const String mqtt_username = "user";
const String mqtt_password = "xxx";
const PROGMEM char* mqtt_sensor_topic = "sensor/garasje";
const PROGMEM char* mqtt_connect_topic = "sensor_status/connect";
const int mqtt_reconnect_retry_interval = 5000;  // In milliseconds

DHTesp dht;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20Sensors(&oneWire);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupWifi() {
  delay(10);

  Serial.println("Connecting to wifi network '" + (String)wifi_ssid + "'");
  WiFi.begin(wifi_ssid, wifi_password);

  while(WiFi.status() != WL_CONNECTED) {
    // Explanation for the codes can be found here: https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html#check-return-codes
    Serial.printf("WIFI connection status: %d\n", WiFi.status());
    //Serial.print(".");
    delay(500);
  }
  Serial.println();

  randomSeed(micros());

  current_ip = WiFi.localIP().toString();
  Serial.println("WiFi connected on IP: " + current_ip);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived on topic " + (String)topic);
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setupMqtt() {
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
}

void setup() {
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize logging to console:
  Serial.begin(115200);
  Serial.println(ARDUINO_BOARD);

  setupWifi();
  setupMqtt();

  // Setup DHT22:
  dht.setup(DHT_GPIO, DHTesp::DHT22);
  char samplingperiod[40];
  sprintf(samplingperiod, "Minimum sampling period: %u", dht.getMinimumSamplingPeriod());
  Serial.println(samplingperiod);

  // Setup DS18B20:
  ds18b20Sensors.begin();
  ds18b20_device_count = ds18b20Sensors.getDeviceCount();
  Serial.println("Found " + (String)ds18b20_device_count + " DS18B20 temperature sensors");
}

void publishData(float p_temperatures[], int p_num_temperatures, float p_humidity) {
  StaticJsonDocument<200> doc;
  
  for (int i = 0; i < p_num_temperatures; i++) {
    Serial.println("Temperature " + (String)i + ": " + (String)p_temperatures[i]);
    doc["temperature" + (String)i] = (String)p_temperatures[i];
  }
  Serial.println("DHT22 Humidity: " + (String)p_humidity);
  doc["humidity"] = (String)p_humidity;

  char data[200];
  serializeJson(doc, data);
  mqttClient.publish(mqtt_sensor_topic, data, true);
  Serial.print("Published: ");
  Serial.println(data);
  yield();
  
  // Signal that data has been published by blinking the built-in led on the board:
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void reconnectMqtt() {
  while(!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");

    if (mqttClient.connect(mqtt_clientid.c_str(), mqtt_username.c_str(), mqtt_password.c_str())) {
      Serial.println("Connected!");
      StaticJsonDocument<200> doc;
      doc["type"] = ARDUINO_BOARD;
      doc["ip"] = current_ip;
      doc["client_id"] = mqtt_clientid;
      doc["mqtt_sensor_topic"] = mqtt_sensor_topic;
      doc["number_of_temperature_sensors"] = ds18b20_device_count + 1;
      doc["number_of_humidity_sensors"] = 1;
      
      char data[200];
      serializeJson(doc, data);
      mqttClient.publish(mqtt_connect_topic, data, true);
      Serial.print("Published: ");
      Serial.println(data);
      yield();
    } else {
      Serial.println("Failed, state: " + mqttClient.state());
      Serial.println("Retrying in " + (String)mqtt_reconnect_retry_interval + " milliseconds");
      delay(mqtt_reconnect_retry_interval);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMqtt();
  }
  mqttClient.loop();
  
  long now = millis();
  if (now - last_report >= report_interval) {
    float temperatures[ds18b20_device_count + 1];
    float humidity = dht.getHumidity();
    temperatures[0] = dht.getTemperature();

    // Get temperatures from all DS18B20 devices:
    ds18b20Sensors.requestTemperatures();
    for (int i = 0; i < ds18b20_device_count; i++) {
      temperatures[i + 1] = ds18b20Sensors.getTempCByIndex(i);
    }
    
    last_report = now;
    publishData(temperatures, ds18b20_device_count + 1, humidity);
  }
}
