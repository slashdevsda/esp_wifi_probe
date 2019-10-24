/**
 * Blink
 *
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "ESPAsyncUDP.h"

// data pin, connected to DHT sensor
// see  https://learn.adafruit.com/dht/overview
#define DHTPIN            0 // label "D3" on the 'Weemos esp mini d1'
#define DHTTYPE           DHT11


/** GLOBALS **/
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
AsyncUDP udp;


/** CONSTANTS **/

// wifi credentials
const char* ssid = CONFIG_WIFI_SSID;
const char* password =  CONFIG_WIFI_PASSWORD;

// the probe name, as it is stored into influxdb (and appears in grafana)
const char *PROBE_NAME = "d1-mini";

// the IP address of your InfluxDB host
const byte host[] = {192, 168, 4, 1};

// the port that the InfluxDB UDP plugin is listening on
const int port = 8089;


void setup_wifi() {

  Serial.println("\n[->)] Connecting to WiFi\n");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.printf("[OK] Connected to ssid `%s`\n", ssid);
}


void probe() {
  sensors_event_t event;
  String line;


  if(udp.connect(host, port)) {
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println("Error reading temperature!");
    }
    else {
      line = String("temperature,host=") + String(PROBE_NAME) + String(" value=") + String(event.temperature);
      Serial.println(line);
      udp.print(line);
    }
    delay(100);
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println("Error reading humidity!");
    }
    else {
      line = String("relative_humidity,host=") + String(PROBE_NAME) + String(" value=") + String(event.relative_humidity);
      Serial.println(line);
      udp.print(line);
    }
  }
  else
    Serial.println("UDP failed");    
}


void setup() {
  delay(1000);
  Serial.begin(9600);
}


void loop() {
  dht.begin();
  setup_wifi();
  delay(200);
  probe();
  WiFi.disconnect();
  // sleep a little less than one minute
  delay(59000);

}
