#include <Arduino.h>
#include "Button.hpp"
#include "Adafruit_NeoPixel.h"

// I/O Pins
#define LED_DATA 2
#define POWER_BUTTON 3
#define PI_GLOBAL_EN 4
#define PI_DETECT 5
#define PI_SHUTDOWN 6

// States
#define PI_OFF 0
#define PI_ON 1
#define PI_SHUTDOWN_WAIT 2
uint8_t state = 0;

// Addressable LED Configuration
#define NUM_LEDS 2
Adafruit_NeoPixel led(NUM_LEDS, LED_DATA, NEO_GRB + NEO_KHZ800);
uint32_t black = led.Color(0, 0, 0);
uint32_t red = led.Color(255, 0, 0);
uint32_t yellow = led.Color(255, 255, 0);
uint32_t green = led.Color(0, 255, 0);

Button power_button(POWER_BUTTON, true);
unsigned long start_time = 0;

// Function Prototypes
void on_sequence();
void off_sequence();

void setup()
{
	Serial.begin(9600);
	pinMode(PI_DETECT, INPUT);
	pinMode(PI_GLOBAL_EN, OUTPUT);
	pinMode(PI_SHUTDOWN, OUTPUT);
	digitalWrite(PI_SHUTDOWN, HIGH);
	led.begin();
	led.show();
	led.setBrightness(100);

	state = digitalRead(PI_DETECT);
	if (state == PI_ON)
	{
		led.setPixelColor(0, green);
		led.setPixelColor(1, green);
	}
	else
	{
		led.setPixelColor(0, red);
		led.setPixelColor(1, red);
	}
	led.show();
}

void loop()
{
	power_button.update_button();
	if (power_button.was_pushed())
	{
		switch (state)
		{
		case PI_ON:
			Serial.println("SHUTDOWN CALLED!");
			off_sequence();
			break;
		case PI_OFF:
			Serial.println("STARTUP CALLED!");
			on_sequence();
			break;
		default:
			break;
		}
	}
	state = digitalRead(PI_DETECT);
	delay(50);
	Serial.println(state ? "ON!" : "OFF!");
}

void off_sequence()
{
	// Confirm Prompt
	start_time = millis();
	Serial.println("Confirm shutdown!");
	while (millis() - start_time < 10000)
	{
		power_button.update_button();
		// Detect confirm shutdown input
		if (power_button.was_pushed() && state == PI_ON)
		{
			Serial.println("Confirmed!");
			// Pull shutdown pin low then reset pin to floating
			digitalWrite(PI_SHUTDOWN, LOW);
			delay(200);
			digitalWrite(PI_SHUTDOWN, HIGH);
			// pinMode(PI_SHUTDOWN, INPUT);
			// pinMode(PI_SHUTDOWN, OUTPUT);
			start_time = millis(); // Restart timer for shutdown detect
			state = PI_SHUTDOWN_WAIT;
		}
		// Detect Pi shutdown done
		if (!digitalRead(PI_DETECT) && state == PI_SHUTDOWN_WAIT)
		{
			led.setPixelColor(0, red);
			led.setPixelColor(1, red);
			led.show();
			state = PI_OFF;
			return;
		}
		// Flash yellow LED
		led.setPixelColor(0, black);
		led.setPixelColor(1, black);
		led.show();
		delay(50);
		led.setPixelColor(0, yellow);
		led.setPixelColor(1, yellow);
		led.show();
		delay(50);
	}
	if (state == PI_ON || state == PI_SHUTDOWN_WAIT)
	{
		// Timer ran out, shutdown wasnt confirmed or something went wrong with shutdown command
		led.setPixelColor(0, green);
		led.setPixelColor(1, green);
		led.show();
		state = PI_ON;
		Serial.println("Shutdown not detected!");
	}
}

void on_sequence()
{
	// digitalWrite(PI_GLOBAL_EN, LOW);
	// delay(50);
	// pinMode(PI_GLOBAL_EN, INPUT);
	// pinMode(PI_GLOBAL_EN, OUTPUT);
	digitalWrite(PI_SHUTDOWN, LOW);
	delay(50);
	pinMode(PI_SHUTDOWN, INPUT);
	pinMode(PI_SHUTDOWN, OUTPUT);
	start_time = millis();
	while (millis() - start_time < 5000)
	{
		if (digitalRead(PI_DETECT))
		{
			// Pi turned on just fine
			led.setPixelColor(0, green);
			led.setPixelColor(1, green);
			led.show();
			state = PI_ON;
			return;
		}
		// Flash yellow LED
		led.setPixelColor(0, black);
		led.setPixelColor(1, black);
		led.show();
		delay(50);
		led.setPixelColor(0, yellow);
		led.setPixelColor(1, yellow);
		led.show();
		delay(50);
	}
	// Pi didnt turn on properly
	led.setPixelColor(0, red);
	led.setPixelColor(1, red);
	led.show();
	state = PI_OFF;
	Serial.println("Startup not detected!");
}