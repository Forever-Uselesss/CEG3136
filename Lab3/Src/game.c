/*
 * game.c
 *
 *  Created on: Oct 25, 2025
 *      Author: jthoz081
 */

// App headers
#include "game.h"
#include "alarm.h"
#include "display.h"
#include "gpio.h"
#include "i2c.h"
#include "systick.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define SHIFT_TIME 200
#define OFF_TIME 200
#define ON_TIME 800
#define DEBOUNCE_TIME 50
static Time_t timeShift; // Time LED position was last shifted
static Time_t now;
static int position = 0;  // Current position of illuminated LED
static int direction = 0; // Current LED shift direction: 0=right 1=left
static int reversed = 0;  // Flag to indicate direction has been reversed
static uint32_t HIGH_SCORE = 0;
volatile uint32_t score = 0;
volatile bool displaying = false;
static Sequence_t *start = NULL;
static Sequence_t *tail = NULL;
static Sequence_t *current = NULL;

static volatile GameState_t Gstate = START;

void Init_Game(void) {
	GPIO_PortEnable(GPIOX);
	timeShift = TimeNow();
	// GPIOEna current = start;
	// GPIO_Callback(StartButton, CallbackStart, FALL); // press = falling edge
}
void Task_Gamegarbage(void) {
	if (TimePassed(timeShift) >= SHIFT_TIME) {
		// Move LED position by 1, depending on direction, wrapping at end
		if (position == (direction ? 7 : 0))
			position = direction ? 0 : 7;
		else
			position += direction ? +1 : -1;
		GPIO_PortOutput(GPIOX, 1 << position); // Update LEDs
		timeShift = TimeNow();
		reversed = 0; // Check for button presses once again
	}
	// Reverse direction if button next to illuminated LED is pressed
	if (!reversed && GPIO_PortInput(GPIOX) & 1 << (position + 8)) {
		direction ^= 1;
		reversed = 1; // Ignore button until LED moves to next position
	}
}

void CallbackStart(void) {

	printf("%u ms : Callback Start!", TimeNow());
	if (Gstate == START) {
		resetSequence();
		score = 0;
		nextLevel();
		current = start;
		displaying = false;
		//		DisplayColor(YELLOW);
		DisplayPrint(GAME, 1, "Get Ready!");
		UpdateDisplay();
		timeShift = TimeNow();
		Gstate = DISPLAY;
	}
}

static Sequence_t* generateNext(void) {
	// Allocate new node
	Sequence_t *newNode = (Sequence_t*) malloc(sizeof(Sequence_t));
	if (newNode == NULL) {
		return NULL;
	}

	// Initialize node
	newNode->pinPos = (uint8_t) (rand() % 8);
	newNode->next = NULL;
	return newNode;
}

void nextLevel(void) {
	Sequence_t *newNode = generateNext();
	if (newNode == NULL) {
		return;
	}

	// Add to list
	if (start == NULL) {
		start = newNode;
	} else {
		tail->next = newNode;
	}
	tail = newNode;
}

void resetSequence(void) {
	Sequence_t *curr = start;
	while (curr != NULL) {
		Sequence_t *next = curr->next;
		free(curr);
		curr = next;
	}
	start = NULL;
	tail = NULL;
}

void displaySequence(void) {

	static bool ledOn = false;
	static Time_t now;
	if (current == NULL) {
		nextLevel();
		current = start;
		return;
	}

	if (!displaying) {
		current = start;
		displaying = true;
		now = TimeNow();
		GPIO_PortOutput(GPIOX, 1 << current->pinPos);
		ledOn = true;
		return;
	}

	if (ledOn && TimePassed(now) > ON_TIME) {
		GPIO_PortOutput(GPIOX, 0);
		now = TimeNow();
		ledOn = false;
	} else if (!ledOn && TimePassed(now) > OFF_TIME) {
		current = current->next;
		if (current) {
			GPIO_PortOutput(GPIOX, 1 << current->pinPos);
			now = TimeNow();
			ledOn = true;
		} else {
			displaying = false;
			Gstate = PLAYING;
			current = start;
		}
	}
}

void titleScreen(void) {
	static Time_t lastPress = 0;
	if ((GPIO_PortInput(GPIOX) & (1 << StartButton.bit))
			&& TimePassed(lastPress) > DEBOUNCE_TIME) {
		printf("%u ms : Start pressed\n", TimeNow());
		lastPress = TimeNow();
		CallbackStart();
	}

	static Time_t lastFlash;
	static bool ledOn = false;
	static uint8_t pin;
	static bool di = false;

	if (!ledOn && TimePassed(lastFlash) > OFF_TIME) {
		pin = rand() % 8;
		GPIO_PortOutput(GPIOX, 1 << pin);
		lastFlash = TimeNow();
		ledOn = true;
	} else if (ledOn && TimePassed(lastFlash) > ON_TIME) {
		//		DisplayColor(WHITE);
		DisplayPrint(GAME, 0, "MEMORY");
		if (di == true) {
			DisplayPrint(GAME, 1, "HIGH SCORE: %lu", HIGH_SCORE);
		} else {
			DisplayPrint(GAME, 1, "PRESS START");
		}
		di = !di;
		GPIO_PortOutput(GPIOX, 0);
		lastFlash = TimeNow();
		ledOn = false;
	}
}

void play(void) {
	static uint16_t input = 0b0;
	static Time_t lastPressd = 0;
	static bool ledIsOn = false;
	static bool buttonPress = false;

	if (current == NULL)
		current = start;

	uint16_t rawInput = GPIO_PortInput(GPIOX);
	if (!buttonPress && rawInput && TimePassed(lastPressd) > DEBOUNCE_TIME) {
		input = rawInput;
		GPIO_PortOutput(GPIOX, input >> 8);
		ledIsOn = true;
		buttonPress = true;
		lastPressd = TimeNow();
	}
	if (buttonPress && !rawInput) {
		buttonPress = false;
	}
	if (ledIsOn && TimePassed(lastPressd) > ON_TIME) {
		GPIO_PortOutput(GPIOX, 0);
		ledIsOn = false;
	}

	if (input) {
		if (current != NULL && !(input ^ (1 << (current->pinPos + 8)))) {
			// Correct input

			current = current->next;
			if (current == NULL) {
				// Completed sequence
				score++;
				current = start;
				Gstate = DISPLAY;
				nextLevel();
				now = TimeNow();
			}
		} else {
			// Incorrect input
			now = TimeNow();
			displaying = true;
			Gstate = GAMEOVER;
		}
		input = 0b0;
	}
}

void Task_Game(void) {

	switch (Gstate) {
	case START:
		titleScreen();
		break;

	case DISPLAY:
		if (TimePassed(now) > ON_TIME) {
			DisplayPrint(GAME, 0, "Memorize");
			DisplayPrint(GAME, 1, "Level %lu", score + 1);
			displaySequence();
		}
		break;

	case PLAYING:
		DisplayPrint(GAME, 0, "Play");
		play();

		break;

	case GAMEOVER:
		if (displaying) {
			//		DisplayColor(RED);
			DisplayPrint(GAME, 0, "GAME OVER");
			DisplayPrint(GAME, 1, "FINAL SCORE: %lu", score);
			UpdateDisplay();
			displaying = false;
		}
		if (score > HIGH_SCORE) {
			HIGH_SCORE = score;
		}
		// to do display binary...
		GPIO_PortOutput(GPIOX, (uint8_t) (score & 0xFF));
		score = 0;
		resetSequence();
		if (TimePassed(now) > ON_TIME) {
			GPIO_PortOutput(GPIOX, 0);
			Gstate = START;
		}
		break;

	default:
		break;
	}
}
