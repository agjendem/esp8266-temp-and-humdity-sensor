#include <DHTesp.h>

DHTesp dht;
long lastMsg = 0;

void setup() {
  // Initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize logging to console:
  Serial.begin(115200);

  dht.setup(14, DHTesp::DHT22); // DHT22 on GPIO 14==D5
  char samplingperiod[40];
  sprintf(samplingperiod, "Minimum sampling period: %ul", dht.getMinimumSamplingPeriod());
  Serial.println(samplingperiod);
}

void publishData(float p_temperature, float p_humidity) {
  Serial.println((String)p_temperature);
  Serial.println((String)p_humidity);

  // Signal that data has been published by blinking the built-in led on the board:
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  //Serial.println("Looping");

  long now = millis();
  if (now - lastMsg > 10000) {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();    

    lastMsg = now;
    publishData(temperature, humidity);
  }
}
