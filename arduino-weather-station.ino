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
#include <ThingSpeak.h>


// Configuration
// TODO: Find way to remove sensitive data from code file
const uint8_t LED_PIN = LED_BUILTIN;      // Pin of the status LED

const uint8_t DHT_PIN = 5;                // Data pin of the DHT sensor
const uint8_t DHT_TYPE = DHT22;           // Type of the DTH sensor
const uint32_t UPDATE_INTERVAL = 5 * 60;  // Update interval in seconds (not guaranteed to be achieved)

const bool LOG_DATA = false;              // Log the recorded data to ThingSpeak
const char* WLAN_SSID = "";               // WLAN SSID
const char* WLAN_PASSWORD = "";           // WLAN Password (secret)
const uint32_t THINGSPEAK_CHANNEL_ID = 0; // ThingSpeak channel ID
const char* THINGSPEAK_API_KEY = "";      // ThingSpeak write API key (secret)


// Initialization
DHT weatherSensor = DHT(DHT_PIN, DHT_TYPE);


// Main
void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // Device initializing

    // Initialize serial port
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect. Needed for native USB (32u4 based boards)
    }

    // Initialize weather sensor
    weatherSensor.begin();

    blinkLed(LED_PIN, 3); // Device initialized
    Serial.println("arduino-weather-station v1.0.0 (https://github.com/drasive/arduino-weather-station)");
    Serial.println("Initialization successful\n");
    delay(1 * 1000);
}

void loop() {
    digitalWrite(LED_PIN, HIGH); // Device active

    // Read sensor data
    float temperature = NAN;
    float humidity = NAN;
    bool sensorReadingSuccessful = readSensorData(&temperature, &humidity);

    // TODO: Output data to LCD

    // Log sensor data
    if (LOG_DATA && sensorReadingSuccessful) {
        Serial.println("Logging data");

        if (WiFi.status() != WL_CONNECTED) {
            connectToWirelessNetwork();
        }

        bool dataLoggingSuccessful = false;
        if (WiFi.status() == WL_CONNECTED) {
            dataLoggingSuccessful = writeToThingSpeak(temperature, humidity);
        }

        if (dataLoggingSuccessful) {
            Serial.println("Logging data successful");
        }
        else {
            Serial.println("Logging data failed");
        }
    }

    Serial.println();
    // TODO: Introduce "failing" status when last update was not successful
    digitalWrite(LED_PIN, LOW); // Device idle

    // Wait for next update
    // TODO: Subtract runtime of last iteration
    delay(UPDATE_INTERVAL * 1000);
}


// Methods
bool readSensorData(float* temperature, float* humidity) {
    const uint8_t SENSOR_READING_ATTEMPTS = 3;
    const uint8_t SENSOR_READING_INTERVAL = 3 * 1000;

    Serial.println("Reading sensor data");

    for (uint8_t sensorReadAttempt = 0; sensorReadAttempt <= SENSOR_READING_ATTEMPTS; sensorReadAttempt++) {
        *temperature = weatherSensor.readTemperature(false, false);
        *humidity = weatherSensor.readHumidity(false);

        bool isTemperatureValid = !isnan(*temperature) && *temperature >= -40.0 && *temperature <= 80.0;
        if (!isTemperatureValid) {
            Serial.print("Reading valid temperature failed: ");
            Serial.print(*temperature);
            Serial.println(")");
        }
        bool isHumidityValid = !isnan(*humidity) && *humidity >= 0.0 && *humidity <= 100.0;
        if (!isHumidityValid) {
            Serial.print("Reading valid humidity failed: ");
            Serial.print(*humidity);
            Serial.println(")");
        }

        if (isTemperatureValid && isHumidityValid) {
            Serial.print("Reading sensor data successful (temperature: ");
            Serial.print(*temperature);
            Serial.print("°C, humidity: ");
            Serial.print(*humidity);
            Serial.println("%)");

            return true;
        }

        // Wait in between attempts
        if (sensorReadAttempt + 1 < SENSOR_READING_ATTEMPTS) {
            delay(SENSOR_READING_INTERVAL);
        }
    }

    *temperature = NAN;
    *humidity = NAN;
    Serial.print("Reading sensor data failed");
    return false;
}

bool writeToThingSpeak(float temperature, float humidity) {
    const uint8_t DATA_LOGGING_ATTEMPTS = 3;
    const uint8_t DATA_LOGGING_INTERVAL = 5 * 1000;

    Serial.print("Writing to ThingSpeak #");
    Serial.println(THINGSPEAK_CHANNEL_ID);

    uint8_t writeStatus = 0;
    for (int dataLoggingAttempt = 0; dataLoggingAttempt <= DATA_LOGGING_ATTEMPTS; dataLoggingAttempt++) {
        WiFiClient client;
        ThingSpeak.begin(client);

        ThingSpeak.setField(1, temperature);
        ThingSpeak.setField(2, humidity);
        writeStatus = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_ID, THINGSPEAK_API_KEY);

        if (writeStatus == OK_SUCCESS) {
            Serial.print("Writing to ThingSpeak #");
            Serial.print(THINGSPEAK_CHANNEL_ID);
            Serial.println(" successful");

            return true;
        }

        // Wait in between attempts
        if (dataLoggingAttempt + 1 < DATA_LOGGING_ATTEMPTS) {
            delay(DATA_LOGGING_INTERVAL);
        }
    }

    Serial.print("Writing to ThingSpeak #");
    Serial.print(THINGSPEAK_CHANNEL_ID);
    Serial.print(" failed: Error ");
    Serial.println(writeStatus);
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
    for (int connectionAttempt = 0; connectionAttempt <= CONNECTION_ATTEMPTS; connectionAttempt++) {
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

        // Wait in between attempts
        if (connectionAttempt + 1 < CONNECTION_ATTEMPTS) {
            delay(CONNECTION_INTERVAL);
        }
    }

    Serial.print("Connecting to WLAN \"");
    Serial.print(WLAN_SSID);
    Serial.print("\" failed: Error ");
    Serial.println(networkConnectionStatus);
}


// Helpers
void blinkLed(uint8_t ledPin, uint8_t count) {
    const uint8_t BLINK_INTERVAL = 200;

    for (uint8_t blinkIndex = 0; blinkIndex < count; blinkIndex++) {
        digitalWrite(ledPin, HIGH);
        delay(BLINK_INTERVAL);
        digitalWrite(ledPin, LOW);

        // Wait in between blinking
        if (blinkIndex + 1 < count) {
            delay(BLINK_INTERVAL);
        }
    }
}
