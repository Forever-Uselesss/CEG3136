// Standard library headers
#include <stdio.h>
// Device driver headers
#include "gpio.h"
#include "i2c.h"
#include "systick.h"
// App headers
#include "alarm.h"
#include "display.h"
#include "game.h"

int main(void) {
  // Initialize apps
  Init_Alarm();
  Init_Game();
  // Enable services
  StartSysTick();
  while (1) {
    // Run apps
    Task_Alarm();
    //UpdateDisplay();
    Task_Game();
    // Housekeeping
    UpdateIOExpanders();
    UpdateDisplay();
    ServiceI2CRequests();
    WaitForSysTick();
  }
}
