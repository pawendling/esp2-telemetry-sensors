#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DHT.h>
#include <Adafruit_AHTX0.h>
#include <SparkFun_ENS160.h>
#include <SensirionI2CScd4x.h>

#include "config.h"
// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- DHT22 ----------------
#define DHTPIN1 4
#define DHTPIN2 15
#define DHTTYPE DHT22

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

// ---------------- AHT22 ----------------
Adafruit_AHTX0 aht;

// ---------------- ENS161 ----------------
SparkFun_ENS160 ens161;

// ---------------- SCD40 ----------------
SensirionI2cScd4x scd40;

// --- WiFi credentials ---
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// --- MQTT Broker ---
const char* mqtt_server = MQTT_SERVER;
const int   mqtt_port   = MQTT_PORT;
const char* mqtt_topic  = MQTT_TOPIC;

WiFiClient espClient;
PubSubClient client(espClient);

// ---------------- MQTT Reconnect ----------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32-2")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

// ---------------- WiFi ----------------
void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ----------------------------
// ESP32 Internal Temperature
// ----------------------------
extern "C" {
  uint8_t temprature_sens_read();
}

float readEsp32InternalTempC() {
  return (temprature_sens_read() - 32) / 1.8;
}

void setup() 
{

  Serial.begin(115200);
  delay(300);

  Wire.begin(21, 22);

   Serial.println("Scanning...");
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at 0x");
      Serial.println(address, HEX);
    }
  }

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 init failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // DHT sensors
  dht1.begin();
  dht2.begin();

  // AHT22
  if (!aht.begin()) {
    Serial.println("AHT22 init failed");
  }

  // ENS161 init
  if (!ens161.begin(Wire, 0x53)) {
    Serial.println("ENS161 not detected");
  }
  ens161.setOperatingMode(SFE_ENS160_STANDARD);

  // SCD40 init
  scd40.begin(Wire, 0x62);
  scd40.stopPeriodicMeasurement();
  delay(20);
  scd40.startPeriodicMeasurement();
  Serial.println("SCD40 started");

  // WiFi + MQTT
  setup_wifi();

   // ---- MQTT ----
  client.setServer(mqtt_server, mqtt_port);


  Serial.println("End of init");
  delay(1000);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // ESP32 internal temperature
  float espC = readEsp32InternalTempC();

  // -------- DHT22 #1 --------
  float t1, t1_raw, h1, h1_raw; 
  h1_raw = dht1.readHumidity();
  t1_raw = dht1.readTemperature(true);

  if (isnan(t1_raw) || isnan(h1_raw)) {
    Serial.println("DHT22-1 NaN — toggling data pin");

    pinMode(DHTPIN1, OUTPUT);
    digitalWrite(DHTPIN1, LOW);
    delay(20);   // 20ms reset pulse
    pinMode(DHTPIN1, INPUT_PULLUP);
    delay(5000);

    dht1.begin();
    delay(1000);
    h1_raw = dht1.readHumidity();
    t1_raw = dht1.readTemperature(true);

  }
  else 
  {  t1 = t1_raw; 
     h1 = h1_raw; 
  }


  // -------- DHT22 #2 --------
  float t2, t2_raw, h2, h2_raw; 
  h2_raw = dht2.readHumidity();
  t2_raw = dht2.readTemperature(true);

  if (isnan(t2_raw) || isnan(h2_raw)) {
    Serial.println("DHT22-2 NaN — toggling data pin");

    pinMode(DHTPIN2, OUTPUT);
    digitalWrite(DHTPIN2, LOW);
    delay(20);   // 20ms reset pulse
    pinMode(DHTPIN2, INPUT_PULLUP);
    delay(5000);
    
    dht2.begin();
    delay(1000);
    h2_raw = dht2.readHumidity();
    t2_raw = dht2.readTemperature(true);
  }
  else 
  {  t2 = t2_raw; 
     h2 = h2_raw; 
  }
  
  // -------- AHT22 --------
  sensors_event_t humEvent, tempEvent;
  aht.getEvent(&humEvent, &tempEvent);

  float ahtTempC = tempEvent.temperature;
  float ahtTempF = ahtTempC * 9.0 / 5.0 + 32.0;
  float ahtHum   = humEvent.relative_humidity;

  // ENS161 compensation
  ens161.setTempCompensationCelsius(ahtTempC);
  ens161.setRHCompensationFloat(ahtHum);

  // ENS161 readings
  uint8_t aqi   = ens161.getAQI();
  uint16_t tvoc = ens161.getTVOC();
  uint16_t eco2 = ens161.getECO2();

  // -------- SCD40 --------
  uint16_t co2 = 0;
  float scdTemp = 0;
  float scdHum = 0;

  bool dataReady = false;
  scd40.getDataReadyStatus(dataReady);

if (dataReady) {

    uint16_t co2_raw;
    float temp_raw;
    float hum_raw;

    uint16_t error = scd40.readMeasurement(co2_raw, temp_raw, hum_raw);

    // Only accept the reading if it's valid
    if (!error && co2_raw != 0) {
        scdTemp = temp_raw * 9.0 / 5.0 + 32.0;  // convert to F
        scdHum  = hum_raw;
        co2     = co2_raw;
    }
}

  // -------- OLED Output --------
  display.clearDisplay();
  display.setCursor(0, 0);

  display.printf("DTH-1: %.1fF %.0f%%\nDTH-2: %.1fF %.0f%%\n", t1, h1, t2, h2);
  display.printf("AHT21: %.1fF %.0f%%  %d\n", ahtTempF, ahtHum, co2);
  display.printf("AQI %d VOC %d CO2 %d\n", aqi, tvoc, eco2);
  

  display.display();


  // -------- MQTT JSON Payload --------
  char payload[500];
  char *p = payload;

  p += sprintf(p, "{");

  bool first = true;

  #define ADD_FLOAT(key, val, fmt) \
    if (!isnan(val)) { \
      if (!first) p += sprintf(p, ", "); \
      p += sprintf(p, "\"" key "\": " fmt, val); \
      first = false; \
    }

  #define ADD_UINT(key, val) \
    if (!isnan((float)val)) { \
      if (!first) p += sprintf(p, ", "); \
      p += sprintf(p, "\"" key "\": %u", val); \
      first = false; \
    }

  // Add fields conditionally
  ADD_FLOAT("DTH1_Temp", t1, "%.1f");
  ADD_FLOAT("DTH1_RH",   h1, "%.0f");

  ADD_FLOAT("DTH2_Temp", t2, "%.1f");
  ADD_FLOAT("DTH2_RH",   h2, "%.0f");

  ADD_FLOAT("AHT_TempF", ahtTempF, "%.1f");
  ADD_FLOAT("AHT_RH",    ahtHum, "%.0f");

  ADD_UINT("AQI",  aqi);
  ADD_UINT("VOC",  tvoc);
  ADD_UINT("CO2",  eco2);

  ADD_UINT("SCD40_CO2", co2);
  ADD_FLOAT("SCD40_Temp", scdTemp, "%.2f");
  ADD_FLOAT("SCD40_RH",   scdHum, "%.2f");

  ADD_FLOAT("esp32_internal",espC , "%.2f");

  p += sprintf(p, "}");



  Serial.print("Publishing: ");
  Serial.println(payload);

  bool ok = client.publish(mqtt_topic, payload);
  if (!ok) {
    Serial.println("MQTT Publish FAILED");
  }

  delay(5000);
}