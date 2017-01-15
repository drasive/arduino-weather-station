#include "LED.h"


LED::LED(uint8_t pin)
{
	LED::pin = pin;
	pinMode(pin, OUTPUT);
}


void LED::turnOn()
{
	LED::interval = 0;
	digitalWrite(LED::pin, HIGH);
}

void LED::turnOff()
{
	LED::interval = 0;
	digitalWrite(LED::pin, LOW);
}

void LED::blink(uint16_t interval)
{
	LED::lastUpdate = millis();
	LED::interval = interval;
}

void LED::update()
{
	uint32_t now = millis();

	// Warning: millis() overflow check has never been tested
	if (LED::interval > 0
		&& (now - LED::lastUpdate >= LED::interval || now < LED::lastUpdate)) {
		if (digitalRead(LED::pin)) {
			digitalWrite(LED::pin, LOW);
		}
		else {
			digitalWrite(LED::pin, HIGH);
		}

		LED::lastUpdate = now;
	}
}
