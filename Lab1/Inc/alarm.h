/*
 * alarm.h
 *
 *  Created on: Sep 28, 2025
 *      Author: joe31
 */

#ifndef ALARM_H_
#define ALARM_H_

typedef enum {
	DISARMED, ARMED, TRIGGERED
} AlarmState_t;

void Init_Alarm();
void Task_Alarm();

void CallbackMotionDetect();
void CallbackButtonPress();
void CallbackButtonRelease();

#endif /* ALARM_H_ */
