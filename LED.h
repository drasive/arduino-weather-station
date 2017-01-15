// LED.h

#ifndef _LED_h
#define _LED_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class LED
{
 protected:
	 uint8_t pin;

	 uint32_t interval;
	 uint32_t lastUpdate;

 public:
	LED(uint8_t pin);

	/**
	 Turns the LED on. Stops blinking.
	*/
	void turnOn();

	/**
	 Turns the LED off. Stops blinking.
	*/
	void turnOff();

	/**
	 Starts blinking asynchronously in a constant interval.
	 LED::update() needs to be called in an interval smaller or equal to the interval parameter while blinking.

	 param duration: Blink and pause durations in miliseconds. Must be > 0.
	*/
	void blink(uint16_t interval);

	/**
	 Updates the LED state.
	 Only required while blinking.
	*/
	void update();
};

#endif
