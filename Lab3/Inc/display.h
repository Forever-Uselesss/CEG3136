/*
 * display.h
 *
 *  Created on: Oct 25, 2025
 *      Author: jthoz081
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

typedef enum {
  RED = 0xFF0000,
  GREEN = 0x00FF00,
  BLUE = 0x0000FF,
  YELLOW = 0xFFFF00,
  ORANGE = 0xFFA500,
  CYAN = 0x00FFFF,
  MAGENTA = 0xFF00FF,
  WHITE = 0xFFFFFF,
  OFF = 0x0
} Color_t;

typedef enum { ALARM = 0, CALC = 1, GAME = 2 } Page_t;
#define PAGES 4

void DisplayEnable(void);
void DisplayPrint(const Page_t page, const int line, const char *msg, ...);
void DisplayColor(const Page_t page, Color_t color);
Page_t GetPage(void);

void UpdateDisplay(void);

#endif /* DISPLAY_H_ */
