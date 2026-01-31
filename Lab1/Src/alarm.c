/*
 * alarm.c
 *
 *  Created on: Sep 28, 2025
 *      Author: joe31
 */

#include <stdio.h>
#include "gpio.h"
#include "systick.h"
#include "alarm.h"

static AlarmState_t state = DISARMED;

static Time_t button_press_start = 0;
static Time_t toggle_led = 0;
static uint8_t button_pressed = 0;

static const uint16_t SHORT_PRESS_MS = 2000;  // <2s
static const uint16_t LONG_PRESS_MS = 3000;  // ≥3s
static const uint16_t TOGGLE_TIME_MS = 1000;  // 1 Hz

static const Pin_t BLUE_LED = { GPIOB, 7 };
static const Pin_t GREEN_LED = { GPIOC, 7 };
static const Pin_t RED_LED = { GPIOA, 9 };
static const Pin_t BUTTON = { GPIOC, 13 };
static const Pin_t MOTION = { GPIOB, 9 };

void Init_Alarm(void) {
	// clocks
	GPIO_Enable(BLUE_LED);
	GPIO_Enable(GREEN_LED);
	GPIO_Enable(RED_LED);
	GPIO_Enable(BUTTON);
	GPIO_Enable(MOTION);

	// LED outputs
	GPIO_Mode(BLUE_LED, OUTPUT);
	GPIO_Mode(GREEN_LED, OUTPUT);
	GPIO_Mode(RED_LED, OUTPUT);

	// Input
	GPIO_Mode(BUTTON, INPUT);
	GPIO_Mode(MOTION, INPUT);

	printf("%u ms : System initialized\n", TimeNow());

	state = DISARMED;

	GPIO_Output(BLUE_LED, 0);
	GPIO_Output(GREEN_LED, 0);
	GPIO_Output(RED_LED, 0);

	printf("%u ms : System DISARMED\n", TimeNow());

	GPIO_Callback(BUTTON, CallbackButtonPress, FALL);  // press = falling edge
	GPIO_Callback(BUTTON, CallbackButtonRelease, RISE); // release = rising edge
	GPIO_Callback(MOTION, CallbackMotionDetect, RISE); // motion = rising edge
}

void Task_Alarm(void) {

	if (button_pressed) {
		Time_t duration = TimePassed(button_press_start);

		if (duration >= LONG_PRESS_MS) {
			state = DISARMED;
			GPIO_Output(BLUE_LED, 0);
			GPIO_Output(GREEN_LED, 0);
			GPIO_Output(RED_LED, 0);
			printf("%u ms : System DISARMED\n", TimeNow());
		}
	}

	if (state == ARMED) {
		if (TimePassed(toggle_led) >= TOGGLE_TIME_MS) {
			GPIO_Toggle(BLUE_LED);
			GPIO_Toggle(GREEN_LED);
			toggle_led = TimeNow();
		}
	}
}

void CallbackButtonPress(void) {
	button_press_start = TimeNow();
	button_pressed = 1;
}

void CallbackButtonRelease(void) {
	if (!button_pressed)
		return;
	button_pressed = 0;

	Time_t duration = TimePassed(button_press_start);

	if (duration <= SHORT_PRESS_MS) {
		state = ARMED;
		GPIO_Output(BLUE_LED, 0);
		GPIO_Output(GREEN_LED, 1);
		GPIO_Output(RED_LED, 0);
		printf("%u ms : System ARMED\n", TimeNow());
	}
}

void CallbackMotionDetect(void) {
	if (state == ARMED) {
		state = TRIGGERED;
		GPIO_Output(BLUE_LED, 0);
		GPIO_Output(GREEN_LED, 0);
		GPIO_Output(RED_LED, 1);
		toggle_led = TimeNow();
		printf("%u ms : System TRIGGERED\n", toggle_led);
	}
}

