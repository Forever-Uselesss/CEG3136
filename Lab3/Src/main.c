// Standard library headers
#include <stdio.h>
// Device driver headers
#include "gpio.h"
#include "i2c.h"
#include "systick.h"
#include "touchpad.h"
// App headers
#include "alarm.h"
#include "display.h"
#include "game.h"
#include "calc.h"


int main(void) {
  // Initialize apps
  Init_Alarm();
  Init_Game();
  Init_Calc();
  // Enable services
  StartSysTick();
  while (1) {
    // Run apps
    Task_Alarm();
    // UpdateDisplay();
    Task_Game();
    Task_Calc();
    // Housekeeping
    UpdateIOExpanders();
    UpdateDisplay();
    ScanTouchpad();
    ServiceI2CRequests();
    WaitForSysTick();
  }
}
