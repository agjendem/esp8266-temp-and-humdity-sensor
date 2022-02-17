# Setup:
* Install [Arduino Sketch](https://docs.arduino.cc/learn/programming/sketches)
* File -> Preferences -> Additional board manager urls -> Add https://arduino.esp8266.com/stable/package_esp8266com_index.json
* Tools -> Board -> Board manager -> Search esp8266 and install package by ESP8266 Community
* Sketch -> Include library -> Manage libraries -> Add (may be inaccurate!):
  * DHTesp by beegee_tokyo
  * OneWire by Paul Stoffregen
  * DallasTemperature by Miles Burton
  * PubSubClient by Nick O'Leary
  * EspMQTTClient by Patrick Lapointe
  * ArduinoJson by Benoit Blanchon
* Change to correct board: Tools -> Board -> ESP8266 boards -> Lolin Wemos D1 Mini Pro

# Hardware:
* [D1 Mini Pro](https://www.banggood.com/5Pcs-Mini-D1-Pro-Upgraded-Version-of-NodeMcu-Lua-Wifi-Development-Board-Based-on-ESP8266-p-1715409.html)

# Gotchas and findings:
* Does NOT seem to support WLAN networks that require PMF, works with PMF set to optional.
* Does support WLANs with WPA3 support enabled and WPA2 on same SSID
* Does support "hidden" (not broadcasted) WLANs
