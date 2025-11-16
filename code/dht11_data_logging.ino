#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"

/************************* DHT Setup *********************************/
#define DHTPIN 2        // GPIO2 (D4 on NodeMCU)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/************************* WiFi ***************************************/
#define WLAN_SSID   "**********" //Replace with your wifi name
#define WLAN_PASS   "**********" //Replace with your wifi password

/************************* Adafruit IO *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "*****************" //Replace with your adafruit io user name
#define AIO_KEY         "*****************"  //Replace with your adfruit io dashboard key

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/************************* Feeds **************************************/
Adafruit_MQTT_Publish tempFeed(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish humFeed(&mqtt, AIO_USERNAME "/feeds/humidity");

/************************* Setup **************************************/
void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println("Connecting to WiFi...");
  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  dht.begin();
}

/************************* Main Loop ***********************************/
void loop() {

  // Maintain MQTT connection
  MQTT_connect();

  // Read Sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read failed.");
    delay(1000);
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("% | Temperature: ");
  Serial.print(t);
  Serial.println("°C");

  // Publish to Adafruit IO
  humFeed.publish(h);
  tempFeed.publish(t);

  // Keep MQTT alive
  mqtt.ping();
  
  delay(5000);    // Log every 5 second
}

/************************* MQTT Connect *********************************/
void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) return;

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying...");
    mqtt.disconnect();
    delay(3000);

    if (--retries == 0) {
      while (1);  // reset by watchdog
    }
  }

  Serial.println("MQTT Connected!");
}