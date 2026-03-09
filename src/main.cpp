#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Adafruit_AHTX0.h>
#include <SparkFun_ENS160.h>

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

// ---------------- AHT21 ----------------
Adafruit_AHTX0 aht;

// ---------------- ENS160 ----------------
SparkFun_ENS160 ens160;

// --- WiFi credentials ---
const char* ssid     = "Saddleback-2.4G";
const char* password = "";

// --- MQTT Broker ---
const char* mqtt_server = "192.168.1.100";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "sensors/esp32-2";

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

// ----------------------------
// Connect to WiFi
// ----------------------------
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

  // AHT21
  if (!aht.begin()) {
    Serial.println("AHT21 init failed");
  }

  // ENS160 init
  if (!ens160.begin(Wire, 0x53)) {
    Serial.println("ENS160 not detected");
  }
  ens160.setOperatingMode(SFE_ENS160_STANDARD);

   // ---- WiFi ----
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


  // -------- DHT22 #1 --------
  float h1 = dht1.readHumidity();
  float t1 = dht1.readTemperature(true);

  // -------- DHT22 #2 --------
  float h2 = dht2.readHumidity();
  float t2 = dht2.readTemperature(true);

  // -------- AHT21 --------
  sensors_event_t humEvent, tempEvent;
  aht.getEvent(&humEvent, &tempEvent);

  float ahtTempC = tempEvent.temperature;
  float ahtTempF = ahtTempC * 9.0 / 5.0 + 32.0;
  float ahtHum   = humEvent.relative_humidity;

  // Send compensation data to ENS160
  ens160.setTempCompensationCelsius(ahtTempC);
  ens160.setRHCompensationFloat(ahtHum);

  // Read ENS160 values
  uint8_t aqi = ens160.getAQI();
  uint16_t tvoc = ens160.getTVOC();
  uint16_t eco2 = ens160.getECO2();

  // -------- OLED Output --------
  display.clearDisplay();
  display.setCursor(0, 0);

  display.printf("DTH-1: %.1fF %.0f%%\nDTH-2: %.1fF %.0f%%\n", t1, h1, t2, h2);
  display.printf("AHT21: %.1fF %.0f%%\n", ahtTempF, ahtHum);
  display.printf("AQI %d VOC %d CO2 %d\n", aqi, tvoc, eco2);

  display.display();


    // Build JSON payload
  char payload[500];
snprintf(payload, sizeof(payload),
    "{"
      "\"DTH1_Temp\": %.1f, \"DTH1_RH\": %.0f, "
      "\"DTH2_Temp\": %.1f, \"DTH2_RH\": %.0f, "
      "\"AQI\": %d, \"VOC\": %d, \"CO2\": %d"
    "}",
    t1, h1,
    t2, h2,
    aqi, tvoc, eco2
);

  Serial.print("Publishing: ");
  Serial.println(payload);

  client.publish(mqtt_topic, payload);


  delay(5000);
}