# arduino-weather-station

Arduino sketch for regularly logging temperature and humidity data to a private cloud.

## Example Output
```
arduino-weather-station v1.0.0 (https://github.com/drasive/arduino-weather-station)
Initialization successful

Reading sensor data
Reading sensor data successful (temperature: 22.30°C, humidity: 49.80%)
Logging data
Connecting to WLAN "Arduino (2.4 GHz)"
Connecting to WLAN "Arduino (2.4 GHz)" successful (192.168.0.228, -69 dBm)
Writing to ThingSpeak #12345
Writing to ThingSpeak #12345 successful
Logging data successful
```

## Configuration
The configuration values are found at the top of [arduino-weather-station.ino](/src/arduino-weather-station.ino).  
A value in angled brackets ([]) indicates an external constant, a slash (-) indicates no value.

Name                  | Type     | Default Value   | Description
----------------------|----------|-----------------|------------
LED_PIN               | uint8_t  | [LED_BUILTIN]   | Pin of the status LED
DHT_PIN               | uint8_t  | 5               | Data pin of the DHT sensor
DHT_TYPE              | uint8_t  | [DHT22]         | Type of the DTH sensor
UPDATE_INTERVAL       | uint32_t | 5 * 60          | Update interval in seconds (not guaranteed to be achieved)
LOG_DATA              | bool     | false           | Log the recorded data to ThingSpeak
WLAN_SSID             | char*    | -               | WLAN SSID
WLAN_PASSWORD         | char*    | -               | WLAN Password (secret)
THINGSPEAK_CHANNEL_ID | uint32_t | -               | ThingSpeak channel ID
THINGSPEAK_API_KEY    | char*    | -               | ThingSpeak write API key (secret)

## Hardware
- 1x Arduino MKR1000 (WiFi101 Firmware v19.4.4)
- 1x DHT22 sensor
- 1x Resistor 10 kR

![Circuit](/circuit/Circuit.png)

[Breadboard Version](/circuit/Breadboard.png)

## Documentation
### Status LED
The following states are communicated using the onboard LED:
- Initializing: LED is off (after turning the device on)
- Initialized:  LED blinks three times (after initializing)
- Active:       LED is on (reading or logging sensor values)
- Idle:         LED is off (between updates)

### Data Logging (ThingSpeak )
If turned on (`LOG_DATA`), the temperature and humidity readings will be send to your [ThingSpeak](https://thingspeak.com/) account.  
This requires an active network connection (`WLAN_SSID` and `WLAN_PASSWORD`) and a ThingSpeak channel (`THINGSPEAK_CHANNEL_ID` and `THINGSPEAK_API_KEY`) to be configured with the temparature as field 1 and the humidity as field 2.

### Serial Bus
All actions are communicated through the serial interface. This can be used for wired operation or debugging.

### Failing to read/ log data
If reading any sensor, connecting to the network or logging data fails, it is retried for up to a total of three times.

## Libraries
- SPI v1.0.0 by Arduino
- WiFi101 v0.12.0 by Arduino
- [DHT v1.3.0 by Adafruit](https://github.com/adafruit/DHT-sensor-library) (external)
- [ThingSpeak v1.2.1 by MathWorks](https://github.com/mathworks/thingspeak-arduino) (external)

## License
[MIT](/LICENSE)
