/*
 * alarm.c
 *
 *  Created on: Sep 28, 2025
 *      Author: joe31
 */

#include "alarm.h"
#include "display.h"
#include "gpio.h"
#include "systick.h"
#include <stdbool.h>
#include <stdio.h>

static volatile AlarmState_t state = DISARMED;

static Time_t button_press_start = 0;
static Time_t toggle_led = 0;

static const uint16_t SHORT_PRESS_MS = 2000; // 2s
static const uint16_t LONG_PRESS_MS = 3000;  // 3s
static const uint16_t TOGGLE_TIME_MS = 1000; // 1 Hz

static const Pin_t BLUE_LED = {GPIOB, 7};
static const Pin_t GREEN_LED = {GPIOC, 7};
static const Pin_t RED_LED = {GPIOA, 9};
static const Pin_t BUTTON = {GPIOB, 2};
static const Pin_t MOTION = {GPIOB, 9};
volatile bool arm = false;
volatile bool trigger = false;
volatile bool disarm = false;

void Init_Alarm(void) {
  // clocks
  GPIO_Enable(BLUE_LED);
  GPIO_Enable(GREEN_LED);
  GPIO_Enable(RED_LED);
  GPIO_Enable(BUTTON);
  GPIO_Enable(MOTION);
  GPIO_Enable(BUZZER);

  // LED outputs
  GPIO_Mode(BLUE_LED, OUTPUT);
  GPIO_Mode(GREEN_LED, OUTPUT);
  GPIO_Mode(RED_LED, OUTPUT);
  GPIO_Mode(BUZZER, OUTPUT);

  // Input
  GPIO_Mode(BUTTON, INPUT);
  GPIO_Mode(MOTION, INPUT);

  printf("%u ms : System initialized\n", TimeNow());

  state = DISARMED;

  GPIO_Output(BLUE_LED, 0);
  GPIO_Output(GREEN_LED, 0);
  GPIO_Output(RED_LED, 0);

  printf("%u ms : System DISARMED\n", TimeNow());

  GPIO_Callback(BUTTON, CallbackButtonPress, FALL);   // press = falling edge
  GPIO_Callback(BUTTON, CallbackButtonRelease, RISE); // release = rising edge
  GPIO_Callback(MOTION, CallbackMotionDetect, RISE);  // motion = rising edge

  DisplayEnable();
  DisplayColor(ALARM, WHITE);
  DisplayPrint(ALARM, 0, "DISARMED");
}

static void turnOffAllLEDs(void) {
  GPIO_Output(BLUE_LED, 0);
  GPIO_Output(GREEN_LED, 0);
  GPIO_Output(RED_LED, 0);
}

static void setArmedLEDs(void) {
  GPIO_Output(BLUE_LED, 0);
  GPIO_Output(GREEN_LED, 1);
  GPIO_Output(RED_LED, 0);
}

static void setTriggeredLEDs(void) {
  GPIO_Output(BLUE_LED, 0);
  GPIO_Output(GREEN_LED, 0);
  GPIO_Output(RED_LED, 1);
}

static void transitionToDisarmed(void) {
  state = DISARMED;
  turnOffAllLEDs();
  printf("%u ms : System DISARMED\n", TimeNow());
  DisplayColor(ALARM, WHITE);
  DisplayPrint(ALARM, 0, "DISARMED");
  DisplayPrint(ALARM, 1, "");
  UpdateDisplay();
}

static void transitionToArmed(void) {
  state = ARMED;
  setArmedLEDs();
  toggle_led = TimeNow();
  printf("%u ms : System ARMED\n", toggle_led);
  DisplayColor(ALARM, GREEN);
  DisplayPrint(ALARM, 0, "ARMED");
  DisplayPrint(ALARM, 1, "");
  UpdateDisplay();
}

static void transitionToTriggered(void) {
  state = TRIGGERED;
  setTriggeredLEDs();
  toggle_led = TimeNow();
  printf("%u ms : System TRIGGERED\n", toggle_led);
  DisplayColor(ALARM, RED);
  DisplayPrint(ALARM, 0, "TRIGGERED");
  DisplayPrint(ALARM, 1, "");
  UpdateDisplay();
}

void Task_Alarm(void) {

  if (GPIO_Input(BUTTON) && TimePassed(button_press_start) >= LONG_PRESS_MS) {
    disarm = true;
    button_press_start = TimeNow(); // prevent multiple disarms
    transitionToDisarmed();
  }

  switch (state) {
  case DISARMED:
    GPIO_Output(BUZZER, LOW);
    break;

  case ARMED:
    GPIO_Output(BUZZER, LOW);
    if (arm) {
      arm = false;
      transitionToArmed();
    }
    if (TimePassed(toggle_led) >= TOGGLE_TIME_MS) {
      GPIO_Toggle(BLUE_LED);
      GPIO_Toggle(GREEN_LED);
      toggle_led = TimeNow();
    }
    break;

  case TRIGGERED:
    if (trigger) {
      trigger = false;
      transitionToTriggered();
    }
    if (TimePassed(toggle_led) >= TOGGLE_TIME_MS) {
      // GPIO_Toggle(BUZZER);
      toggle_led = TimeNow();
    }
    break;
  }
}

void CallbackButtonPress(void) { button_press_start = TimeNow(); }

void CallbackButtonRelease(void) {
  Time_t duration = TimePassed(button_press_start);

  // short press => arm (if appropriate)
  if (duration > 50 && duration <= SHORT_PRESS_MS) {
    state = ARMED;
    arm = true;
  }
}

void CallbackMotionDetect(void) {
  if (state == ARMED) {
    state = TRIGGERED;
    trigger = true;
  }
}
