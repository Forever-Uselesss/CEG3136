/*
 * game.h
 *
 *  Created on: Oct 25, 2025
 *      Author: jthoz081
 */

#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

void Init_Game();
void Task_Game();

typedef enum {
  START,
  DISPLAY,
  PLAYING,
  GAMEOVER,
} GameState_t;

typedef enum {
  CORRECT,
  INCORRECT,
} InputState_t;

typedef struct Sequence_t {
  uint8_t pinPos;
  struct Sequence_t *next;
} Sequence_t;
void nextLevel(void);
void CallbackStart(void);
void resetSequence(void);
#endif /* GAME_H_ */
