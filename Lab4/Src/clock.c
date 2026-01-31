// Clock app

#include "clock.h"
#include "display.h"
#include "rtc.h"
#include "touchpad.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static char time[9] = "HH:MM:SS";
static char date[11] = "YYYY-MM-DD";

static uint32_t entry; // Touchpad numeric entry

static enum { SHOW, SETDATE, SETTIME } state;

// Facilitate numeric entry of new time or date
bool TimeDateEntry(char *td) {
  bool done;
  char delimiter = (td == &time[0]) ? ':' : '-';

  DisplayPrint(CLOCK, 1, entry ? "%u" : "", entry);

  done = TouchEntry(CLOCK, &entry);
  if (entry > 999999)
    // Too long; remove last entered digit
    entry /= 10;
  else if (done && entry >= 100000) {
    // Convert 6 digits to ASCII and populate string
    // char td[8] = "HH:MM:SS"
    td[7] = '0' + entry % 10;
    entry /= 10;
    td[6] = '0' + entry % 10;
    entry /= 10;
    td[5] = delimiter;
    td[4] = '0' + entry % 10;
    entry /= 10;
    td[3] = '0' + entry % 10;
    entry /= 10;
    td[2] = delimiter;
    td[1] = '0' + entry % 10;
    entry /= 10;
    td[0] = '0' + entry % 10;
    entry = 0;

    return true; // Complete entry
  }
  return false; // Incomplete entry
}

// App initialization
void Init_Clock(void) { RTC_Enable(); DisplayColor(CLOCK, WHITE);}

// App execution
void Task_Clock(void) {
  switch (state) {

  case SHOW:
    // Display time and date from RTC
    RTC_GetTime(time);
    RTC_GetDate(date);
    DisplayPrint(CLOCK, 0, time);
    DisplayPrint(CLOCK, 1, date);

    switch (TouchInput(CLOCK)) {
    case 1:
      state = SETTIME;
      entry = 0;
      break;
    case 2:
      state = SETDATE;
      entry = 0;
      break;
    default:
      break;
    }
    break;

  case SETTIME:
    DisplayPrint(CLOCK, 0, "Set time: HHMMSS");

    if (TimeDateEntry(&time[0])) {
      RTC_SetTime(time);
      printf("Time set to %s\n", time);
      state = SHOW;
    }
    break;

  case SETDATE:
    DisplayPrint(CLOCK, 0, "Set date: YYMMDD");

    if (TimeDateEntry(&date[2])) {
      RTC_SetDate(date);
      printf("Date set to %s\n", date);
      state = SHOW;
    }
    break;
  }
}
