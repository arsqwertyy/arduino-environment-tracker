#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <stdio.h>

// Sensor and pin configuration
constexpr uint8_t DHT_PIN = 2;
constexpr uint8_t DHT_TYPE = DHT11;
constexpr unsigned long UPDATE_INTERVAL_MS = 2000;

// Peripheral instances
LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);

struct ClimateData {
    float temperature = 0.0f;
    float humidity = 0.0f;
    float minTemp = 1000.0f;
    float maxTemp = -1000.0f;
    float minHumidity = 1000.0f;
    float maxHumidity = -1000.0f;
    bool isFirstRead = true;

    void update(float currentTemp, float currentHum) {
        temperature = currentTemp;
        humidity = currentHum;

        if (isFirstRead) {
            minTemp = maxTemp = currentTemp;
            minHumidity = maxHumidity = currentHum;
            isFirstRead = false;
            return;
        }

        if (currentTemp < minTemp) minTemp = currentTemp;
        if (currentTemp > maxTemp) maxTemp = currentTemp;
        if (currentHum < minHumidity) minHumidity = currentHum;
        if (currentHum > maxHumidity) maxHumidity = currentHum;
    }
};

ClimateData climate;
unsigned long lastUpdateTimestamp = 0;

void displayMetrics(const ClimateData& data) {
    char rowBuffer[17];

    // Row 1: Current readings (e.g., "H:45.0% T:23.5C")
    snprintf(rowBuffer, sizeof(rowBuffer), "H:%.1f%% T:%.1fC", data.humidity, data.temperature);
    lcd.setCursor(0, 0);
    lcd.print(rowBuffer);

    // Row 2: Session Min/Max ranges (e.g., "H40-55% T20-25C")
    snprintf(rowBuffer, sizeof(rowBuffer), "H%d-%d%% T%d-%dC", 
             static_cast<int>(data.minHumidity), static_cast<int>(data.maxHumidity),
             static_cast<int>(data.minTemp), static_cast<int>(data.maxTemp));
    lcd.setCursor(0, 1);
    lcd.print(rowBuffer);
}

void displaySensorError() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Read Err");
    lcd.setCursor(0, 1);
    lcd.print("Check Wiring");
}

void setup() {
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("System Booting...");

    dht.begin();
    delay(1000);
    lcd.clear();
}

void loop() {
    unsigned long currentTimestamp = millis();

    // Non-blocking telemetry acquisition
    if (currentTimestamp - lastUpdateTimestamp >= UPDATE_INTERVAL_MS) {
        lastUpdateTimestamp = currentTimestamp;

        float hum = dht.readHumidity();
        float temp = dht.readTemperature();

        if (isnan(hum) || isnan(temp)) {
            displaySensorError();
        } else {
            climate.update(temp, hum);
            displayMetrics(climate);
        }
    }
}
