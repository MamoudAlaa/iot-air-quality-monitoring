// Fill in your Blynk credentials before uploading
#define BLYNK_TEMPLATE_ID   "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitoring"
#define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN"
#define BLYNK_PRINT         Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// ThingSpeak configuration (free tier requires >= 15 s between updates)
const char* THINGSPEAK_SERVER  = "api.thingspeak.com";
const String THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_API_KEY";

// Hardware pins & thresholds
#define DHTPIN             4
#define DHTTYPE            DHT22
#define GAS_SENSOR_PIN     32
#define GAS_ALERT_THRESH   600

// Timing intervals (ms)
const unsigned long BLYNK_INTERVAL_MS      = 2000UL;
const unsigned long THINGSPEAK_INTERVAL_MS = 20000UL;
const unsigned long LCD_CYCLE_MS           = 2000UL;
const unsigned long ALERT_COOLDOWN_MS      = 300000UL; // 5 minutes

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;
WiFiClient client;

float latestTemp = NAN;
float latestHum  = NAN;
int   latestGas  = 0;
uint8_t lcdPage  = 0;
unsigned long lastAlertTime = 0;

byte degreeSymbol[8] = {
  0b00111, 0b00101, 0b00111, 0b00000,
  0b00000, 0b00000, 0b00000, 0b00000
};

void readSensorsAndUpdateBlynk() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int gas = analogRead(GAS_SENSOR_PIN);

  if (isnan(h) || isnan(t)) {
    Serial.println("Warning: Failed to read from DHT22 sensor.");
    return;
  }

  latestTemp = t;
  latestHum  = h;
  latestGas  = gas;

  Blynk.virtualWrite(V0, latestTemp);
  Blynk.virtualWrite(V1, latestHum);
  Blynk.virtualWrite(V2, latestGas);

  if (latestGas >= GAS_ALERT_THRESH &&
      (lastAlertTime == 0 || millis() - lastAlertTime >= ALERT_COOLDOWN_MS)) {
    Blynk.logEvent("pollution_alert", "Bad Air Detected!");
    lastAlertTime = millis();
  }

  Serial.printf("Temp: %.1f C | Humidity: %.1f %% | Gas ADC: %d\n",
                latestTemp, latestHum, latestGas);
}

void sendToThingSpeak() {
  if (isnan(latestTemp) || isnan(latestHum)) return;

  if (client.connect(THINGSPEAK_SERVER, 80)) {
    String body = "api_key=" + THINGSPEAK_API_KEY
                + "&field1=" + String(latestTemp, 2)
                + "&field2=" + String(latestHum, 2)
                + "&field3=" + String(latestGas);

    client.print("POST /update HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\r\n");
    client.print("Content-Type: application/x-www-form-urlencoded\r\n");
    client.print("Content-Length: " + String(body.length()) + "\r\n\r\n");
    client.print(body);

    Serial.println("ThingSpeak update sent.");
  } else {
    Serial.println("ThingSpeak connection failed.");
  }
  client.stop();
}

void updateLCD() {
  lcd.clear();
  if (isnan(latestTemp) || isnan(latestHum)) {
    lcd.setCursor(0, 0);
    lcd.print("Reading sensors");
    return;
  }

  if (lcdPage == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(latestTemp, 1);
    lcd.write(1);
    lcd.print("C");
  } else if (lcdPage == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Humidity: ");
    lcd.print(latestHum, 1);
    lcd.print("%");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Gas Value: ");
    lcd.print(latestGas);
    lcd.setCursor(0, 1);
    lcd.print(latestGas < GAS_ALERT_THRESH ? "Fresh Air" : "Bad Air");
  }

  lcdPage = (lcdPage + 1) % 3;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, degreeSymbol);

  lcd.setCursor(3, 0);
  lcd.print("Air Quality");
  lcd.setCursor(3, 1);
  lcd.print("Monitoring");

  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  lcd.clear();

  timer.setInterval(BLYNK_INTERVAL_MS, readSensorsAndUpdateBlynk);
  timer.setInterval(THINGSPEAK_INTERVAL_MS, sendToThingSpeak);
  timer.setInterval(LCD_CYCLE_MS, updateLCD);
}

void loop() {
  Blynk.run();
  timer.run();
}
