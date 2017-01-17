# arduino-weather-station

Arduino sketch for regularly logging temperature and humidity data to a private cloud.

## Example Output (Serial)
```
arduino-weather-station v1.0.0 (https://github.com/drasive/arduino-weather-station)
Initialization successful

Reading sensor data
Reading sensor data successful (temperature: 22.30°C, humidity: 49.80%)
Logging data
Connecting to WLAN "Arduino (2.4 GHz)"
Connecting to WLAN "Arduino (2.4 GHz)" successful (192.168.0.228, -69 dBm)
Writing to Ubidots
Writing to Ubidots successful
Logging data successful
```

## Configuration
The configuration values are found at the top of [arduino-weather-station.ino](/arduino-weather-station/arduino-weather-station.ino).  
A value in angled brackets ([]) indicates an external constant, a slash (-) indicates no value.

Name                   | Type     | Default Value   | Description
-----------------------|----------|-----------------|------------
LED_PIN                | uint8_t  | [LED_BUILTIN]   | Pin of the status LED
DHT_PIN                | uint8_t  | 5               | Data pin of the DHT sensor
DHT_TYPE               | uint8_t  | [DHT22]         | Type of the DTH sensor
UPDATE_INTERVAL        | uint32_t | 5 * 60          | Update interval in seconds (not guaranteed to be achieved)
LOG_DATA               | bool     | false           | Log the recorded data to Ubidots
WLAN_SSID              | char*    | -               | WLAN SSID
WLAN_PASSWORD          | char*    | -               | WLAN Password (secret)
UBIDOTS_TOKEN          | char*    | -               | Ubidots token (secret)
UBIDOTS_ID_TEMPERATURE | char*    | -               | Ubidots temperature source id
UBIDOTS_ID_HUMIDITY    | char*    | -               | Ubidots humidity source id

## Hardware
- 1x Arduino MKR1000 (WiFi101 Firmware v19.4.4)
- 1x DHT22 sensor
- 1x Resistor 10 kR

[Breadboard Version](/circuit/Breadboard.png)

![Circuit](/circuit/Circuit.png)

## Documentation
### Status LED
The following states are communicated using the onboard LED:
- Initializing: LED is off (after turning the device on)
- Initialized:  LED blinks three times (after initializing)
- Active:       LED is on (reading or logging sensor values)
- Idle:         LED is off (waiting for next update)
- Failing:      LED blinks (last update failed)

### Data Logging (Ubidots)
If turned on (`LOG_DATA`), the temperature and humidity readings will be send to your [Ubidots](https://ubidots.com/) account.  
This requires an active network connection (`WLAN_SSID` and `WLAN_PASSWORD`), a Ubidots token (`UBIDOTS_TOKEN`) and the data source IDs (`UBIDOTS_ID_TEMPERATURE` and `UBIDOTS_ID_HUMIDITY`) to be configured.

### Serial Bus
All actions are communicated through the serial interface. This can be used for wired operation or debugging.

### Failing to read/ log data
If reading any sensor, connecting to the network or logging data fails, it is retried for up to a total of three times.

## Libraries
- SPI v1.0.0 by Arduino
- WiFi101 v0.12.0 by Arduino
- [DHT v1.3.0 by Adafruit](https://github.com/adafruit/DHT-sensor-library) (external)
- [UbidotsArduino #2803d20 by Ubidots](https://github.com/ubidots/ubidots-arduino-wifi) (external)

## License
[MIT](/LICENSE)
