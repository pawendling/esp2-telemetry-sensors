#include <Arduino.h>
#include <Wire.h>
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

  Serial.println("End of init"); 
  delay(1000);
}

void loop()
{
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
  display.printf("AQI %d TVOC %d CO2 %d\n", aqi, tvoc, eco2);

  display.display();

  delay(2000);
}