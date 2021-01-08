#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHT_GPIO 14     // DHT22 on GPIO 14==D5
#define ONE_WIRE_BUS 12 // DS18B20 ON GPIO 12==D6

DHTesp dht;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20Sensors(&oneWire);

long lastMsg = 0;
int ds18b20DeviceCount = 0;

void setup() {
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize logging to console:
  Serial.begin(115200);

  // Setup DHT22:
  dht.setup(DHT_GPIO, DHTesp::DHT22);
  char samplingperiod[40];
  sprintf(samplingperiod, "Minimum sampling period: %u", dht.getMinimumSamplingPeriod());
  Serial.println(samplingperiod);

  // Setup DS18B20:
  ds18b20Sensors.begin();
  ds18b20DeviceCount = ds18b20Sensors.getDeviceCount();
  Serial.println("Found " + (String)ds18b20DeviceCount + " DS18B20 temperature sensors");
}

void publishData(float p_temperatures[], int p_num_temperatures, float p_humidity) {
  for (int i = 0; i < p_num_temperatures; i++) {
    Serial.println("Temperature " + (String)i + ": " + (String)p_temperatures[i]);
  }
  Serial.println("DHT22 Humidity: " + (String)p_humidity);
  
  // Signal that data has been published by blinking the built-in led on the board:
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  long now = millis();
  if (now - lastMsg > 10000) {
    float temperatures[ds18b20DeviceCount + 1];
    float humidity = dht.getHumidity();
    temperatures[0] = dht.getTemperature();

    // Get temperatures from all DS18B20 devices:
    ds18b20Sensors.requestTemperatures();
    for (int i = 0; i < ds18b20DeviceCount; i++) {
      temperatures[i + 1] = ds18b20Sensors.getTempCByIndex(i);
    }
    
    lastMsg = now;
    publishData(temperatures, ds18b20DeviceCount + 1, humidity);
  }
}
