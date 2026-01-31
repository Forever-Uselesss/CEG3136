// Standard library headers
#include <stdio.h>
// Device driver headers

#include "gpio.h"
#include "i2c.h"
#include "spi.h"
#include "systick.h"
#include "touchpad.h"
// App headers
#include "alarm.h"
#include "calc.h"
#include "clock.h"
#include "display.h"
#include "enviro.h"
#include "game.h"
#include "motor.h"

int main(void) {
  // Initialize apps
  Init_Alarm();
  Init_Game();
  Init_Calc();
  Init_Motor();
  Init_Clock();
  Init_Enviro();
  // Enable services
  StartSysTick();
  while (1) {
    // Run apps
    Task_Alarm();
    // UpdateDisplay();
    Task_Game();
    Task_Calc();
    Task_Motor();
    Task_Enviro();
    Task_Clock();
    // Housekeeping
    UpdateIOExpanders();
    UpdateDisplay();
    ScanTouchpad();
    ServiceI2CRequests();
    ServiceSPIRequests();
    WaitForSysTick();
  }
}
