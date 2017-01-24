/*
  arduino-weather-station
  Arduino sketch for regularly logging temperature and humidity data to a private cloud.

  License:    MIT
  Repository: https://github.com/drasive/arduino-weather-station
  Author:     Dimitri Vranken <me@dimitrivranken.com>
*/

#include <SPI.h>
#include <WiFi101.h>
#include <DHT.h>
#include <UbidotsArduino.h>
#include "LED.h"


// Configuration
const uint8_t LED_PIN = LED_BUILTIN;      // Pin of the status LED

const uint8_t DHT_PIN = 5;                // Data pin of the DHT sensor
const uint8_t DHT_TYPE = DHT22;           // Type of the DTH sensor
const uint8_t PHOTORESISTOR_PIN = PIN_A0; // Pin of the photoresistor
const uint32_t UPDATE_INTERVAL = 5 * 60;  // Update interval in seconds (not guaranteed to be achieved)

const bool LOG_DATA = false;              // Log the recorded data to ThingSpeak
const char* WLAN_SSID = "";               // WLAN SSID
const char* WLAN_PASSWORD = "";           // WLAN Password (secret)
const char* UBIDOTS_TOKEN = "";           // Ubidots token (secret)
const char* UBIDOTS_ID_TEMPERATURE = "";  // Ubidots temperature source id
const char* UBIDOTS_ID_HUMIDITY = "";     // Ubidots humidity source id
const char* UBIDOTS_ID_BRIGHTNESS = "";   // Ubidots brightness source id


// Initialization
LED statusLed = LED(LED_PIN);
DHT weatherSensor = DHT(DHT_PIN, DHT_TYPE);


// Main
void setup() {
    statusLed.turnOn(); // Device initializing

    // Initialize serial port
    Serial.begin(9600);

    // Initialize weather sensor
    weatherSensor.begin();

    blinkLedSynchronous(statusLed, 3); // Device initialized
    Serial.println("arduino-weather-station v1.4.2 (https://github.com/drasive/arduino-weather-station)");
    Serial.println("Initialization successful\n");
    delay(1 * 1000);
}

void loop() {
    statusLed.turnOn(); // Device active
    uint32_t updateStart = millis();

    // Read sensor data
    float temperature = NAN;
    float humidity = NAN;
    uint16_t brightness = NAN;
    bool sensorReadingSuccessful = readSensorData(&temperature, &humidity, &brightness);

    // Log sensor data
    bool dataLoggingSuccessful = false;
    if (LOG_DATA && sensorReadingSuccessful) {
        Serial.println("Logging data");

        if (WiFi.status() != WL_CONNECTED) {
            connectToWirelessNetwork();
        }

        if (WiFi.status() == WL_CONNECTED) {
            dataLoggingSuccessful = writeToUbidots(temperature, humidity, brightness);
        }

        if (dataLoggingSuccessful) {
            Serial.println("Logging data successful");
        }
        else {
            Serial.println("Logging data failed");
        }
    }

    Serial.println();

    // Wait for next update
    if (!sensorReadingSuccessful || (LOG_DATA && !dataLoggingSuccessful)) {
        statusLed.blink(1000); // Device failing
    }
    else {
        statusLed.turnOff(); // Device idle
    }

    uint32_t timeSinceLastUpdate = NULL;
    do {
        uint32_t now = millis();
        if (now > updateStart) {
            timeSinceLastUpdate = now - updateStart;
        }
        else {
            // millis() overflow occured
            // Code assumes that a single update takes less than UINT32_MAX milliseconds
            // Warning: This code path has never been tested
            timeSinceLastUpdate = (UINT32_MAX - updateStart) + now;
        }

        statusLed.update();

        delay(50);
    } while (timeSinceLastUpdate <= UPDATE_INTERVAL * 1000 - (50 / 2));
}


// Methods
bool readSensorData(float* temperature, float* humidity, uint16_t* brightness) {
    const uint8_t SENSOR_READING_ATTEMPTS = 3;
    const uint8_t SENSOR_READING_INTERVAL = 3 * 1000;

    Serial.println("Reading sensor data");

    for (uint8_t sensorReadAttempt = 0; sensorReadAttempt < SENSOR_READING_ATTEMPTS; sensorReadAttempt++) {
        *temperature = weatherSensor.readTemperature(false, false);
        *humidity = weatherSensor.readHumidity(false);
        *brightness = analogRead(PHOTORESISTOR_PIN);

        bool isTemperatureValid = !isnan(*temperature) && *temperature >= -40.0 && *temperature <= 80.0;
        bool isHumidityValid = !isnan(*humidity) && *humidity >= 0.0 && *humidity <= 100.0;
        bool isBrightnessValid = !isnan(*brightness) && *brightness >= 0 && *brightness <= 1023;

        if (isTemperatureValid && isHumidityValid && isBrightnessValid) {
            *brightness = 1023 - *brightness;

            Serial.print("Reading sensor data successful (temperature: ");
            Serial.print(*temperature);
            Serial.print("°C, humidity: ");
            Serial.print(*humidity);
            Serial.print("%, brightness: ");
            Serial.print(*brightness);
            Serial.println(")");

            return true;
        }

        if (!isTemperatureValid) {
            Serial.print("Reading valid temperature failed: ");
            Serial.println(*temperature);
        }
        if (!isHumidityValid) {
            Serial.print("Reading valid humidity failed: ");
            Serial.println(*humidity);
        }
        if (!isBrightnessValid) {
            Serial.print("Reading valid brightness failed: ");
            Serial.println(*brightness);
        }

        // Wait in between attempts
        if (sensorReadAttempt + 1 < SENSOR_READING_ATTEMPTS) {
            delay(SENSOR_READING_INTERVAL);
        }
    }

    *temperature = NAN;
    *humidity = NAN;
    *brightness = NAN;
    Serial.print("Reading sensor data failed");
    return false;
}

bool writeToUbidots(float temperature, float humidity, uint16_t brightness) {
    const uint8_t DATA_LOGGING_ATTEMPTS = 3;
    const uint8_t DATA_LOGGING_INTERVAL = 5 * 1000;

    Serial.println("Writing to Ubidots");

    Ubidots ubidots = Ubidots((char*)UBIDOTS_TOKEN, "things.ubidots.com");
    for (int dataLoggingAttempt = 0; dataLoggingAttempt < DATA_LOGGING_ATTEMPTS; dataLoggingAttempt++) {
        ubidots.add((char*)UBIDOTS_ID_TEMPERATURE, temperature);
        ubidots.add((char*)UBIDOTS_ID_HUMIDITY, humidity);
        ubidots.add((char*)UBIDOTS_ID_BRIGHTNESS, brightness);

        bool writeSuccessful = ubidots.sendAll();

        if (writeSuccessful) {
            Serial.println("Writing to Ubidots successful");
            return true;
        }

        Serial.println("Writing to Ubidots failed");

        // Wait in between attempts
        if (dataLoggingAttempt + 1 < DATA_LOGGING_ATTEMPTS) {
            delay(DATA_LOGGING_INTERVAL);
        }
    }

    return false;
}

void connectToWirelessNetwork() {
    const uint8_t CONNECTION_ATTEMPTS = 3;
    const uint8_t CONNECTION_INTERVAL = 10 * 1000;

    Serial.print("Connecting to WLAN \"");
    Serial.print(WLAN_SSID);
    Serial.println("\"");

    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.print("Connecting to WLAN \"");
        Serial.print(WLAN_SSID);
        Serial.println("\" failed: No WLAN shield on device");

        return;
    }

    uint8_t networkConnectionStatus = WL_IDLE_STATUS;
    for (int connectionAttempt = 0; connectionAttempt < CONNECTION_ATTEMPTS; connectionAttempt++) {
        networkConnectionStatus = WiFi.begin(WLAN_SSID, WLAN_PASSWORD);

        if (networkConnectionStatus == WL_CONNECTED) {
            IPAddress ipAddress = IPAddress(WiFi.localIP());

            Serial.print("Connecting to WLAN \"");
            Serial.print(WLAN_SSID);
            Serial.print("\" successful (");
            Serial.print(ipAddress);
            Serial.print(", ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm)");

            WiFi.maxLowPowerMode();
            return;
        }

        Serial.print("Connecting to WLAN \"");
        Serial.print(WLAN_SSID);
        Serial.print("\" failed: Error ");
        Serial.println(networkConnectionStatus);

        // Wait in between attempts
        if (connectionAttempt + 1 < CONNECTION_ATTEMPTS) {
            delay(CONNECTION_INTERVAL);
        }
    }
}


// Helpers
void blinkLedSynchronous(LED led, uint8_t count) {
    const uint8_t BLINK_INTERVAL = 200;

    for (uint8_t blinkIndex = 0; blinkIndex < count; blinkIndex++) {
        led.turnOn();
        delay(BLINK_INTERVAL);
        led.turnOff();

        // Wait in between blinking
        if (blinkIndex + 1 < count) {
            delay(BLINK_INTERVAL);
        }
    }
}
