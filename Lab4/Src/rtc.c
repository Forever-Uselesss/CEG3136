// Real-time clock (RTC) driver

#include "rtc.h"
#include "stm32l5xx.h"
#include <stdio.h>

// Async/sync pre-scalers (recommended values)
#define PDA (128 - 1)
#define PDS (256 - 1)

// --------------------------------------------------------
// Initialization
// --------------------------------------------------------

void RTC_Enable(void) {
  // Enable clock to PWR block and disable backup domain write protection
  // Refer to MCU RM 9.8.19 and 8.6.1 and device header file
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
  PWR->CR1 |= PWR_CR1_DBP;

  // Enable LSE 32.768kHz, select LSE for RTC clock, and enable RTC clock
  // Refer to MCU RM 9.8.29 and device header file
  RCC->BDCR |= RCC_BDCR_LSEON;
  while (!(RCC->BDCR & RCC_BDCR_LSERDY)) {
  } // Wait for LSE ready
  RCC->BDCR |= RCC_BDCR_RTCSEL_0; // LSE selected as RTC clock
  RCC->BDCR |= RCC_BDCR_RTCEN;    // Enable RTC clock

  // Enable APB clock to RTC peripheral
  // Refer to MCU RM 9.8.19 and device header file
  RCC->APB1ENR1 |= RCC_APB1ENR1_RTCAPBEN;
  // Disable RTC write protection (2 writes)
  // Refer to MCU RM 41.6.10 and 41.3.11, 2nd subsection
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  // Asynchronous and synchronous pre-scalers
  // Refer to MCU RM 41.3.6 and 41.6.5. Use the PDA and PDS constants.
  RTC->PRER = (PDA << RTC_PRER_PREDIV_A_Pos) | (PDS << RTC_PRER_PREDIV_S_Pos);
}

// --------------------------------------------------------
// Get time/date from RTC
// --------------------------------------------------------

// Generate time string from RTC, formatted HH:MM:SS (24-hour clock)
// Refer to MCU RM 41.6.1
void RTC_GetTime(char *time) {
  uint32_t tr;
  char ht, hu, mnt, mnu, st, su;

  RTC->TR &= ~(RTC_TR_PM_Msk);
  tr = RTC->TR;
  ht = '0' + ((tr & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos); // Hour
  hu = '0' + ((tr & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos);
  mnt = '0' + ((tr & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos); // Minute
  mnu = '0' + ((tr & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
  st = '0' + ((tr & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos); // Second
  su = '0' + ((tr & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos);

  snprintf(time, 9, "%c%c:%c%c:%c%c", ht, hu, mnt, mnu, st, su);
}

// Generate date string from RTC, formatted YYYY-MM-DD
// Refer to MCU RM 41.6.2
void RTC_GetDate(char *date) {
  uint32_t dr;
  int ym, yc, yt, yu, mt, mu, dt, du;

  dr = RTC->DR;
  ym = '2';
  yc = '0';
  yt = '0' + ((dr & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos); // Year
  yu = '0' + ((dr & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos);
  mt = '0' + ((dr & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos); // Month
  mu = '0' + ((dr & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos);
  dt = '0' + ((dr & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos); // Day
  du = '0' + ((dr & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos);

  snprintf(date, 11, "%c%c%c%c-%c%c-%c%c", ym, yc, yt, yu, mt, mu, dt, du);
}

// --------------------------------------------------------
// Set RTC time/date
// --------------------------------------------------------

// Set RTC time from string, formatted HH:MM:SS
void RTC_SetTime(const char *time) {
  uint32_t tr;

  // Enter initialization mode
  // Refer to MCU RM 41.6.4 and 41.3.11, 3rd subsection, items 1-2
  RTC->ICSR |= RTC_ICSR_INIT;
  while (!(RTC->ICSR & RTC_ICSR_INITF)) {
  } // Wait for INITF

  tr = 0;
  tr |= ((time[0] - '0') << RTC_TR_HT_Pos) & RTC_TR_HT_Msk;   // Hour tens
  tr |= ((time[1] - '0') << RTC_TR_HU_Pos) & RTC_TR_HU_Msk;   // Hour units
  tr |= ((time[3] - '0') << RTC_TR_MNT_Pos) & RTC_TR_MNT_Msk; // Minute tens
  tr |= ((time[4] - '0') << RTC_TR_MNU_Pos) & RTC_TR_MNU_Msk; // Minute units
  tr |= ((time[6] - '0') << RTC_TR_ST_Pos) & RTC_TR_ST_Msk;   // Second tens
  tr |= ((time[7] - '0') << RTC_TR_SU_Pos) & RTC_TR_SU_Msk;   // Second units
  tr |= RTC->TR = tr;
  RTC->CR &= ~RTC_CR_FMT; // FMT=0 (24-hour)

  // Return to free-running mode
  // Refer to MCU RM 41.6.4 and 41.3.11, 3rd subsection, item 5
  RTC->ICSR &= ~RTC_ICSR_INIT;
}

// Set RTC date from string, formatted YYYY-MM-DD
void RTC_SetDate(const char *date) {
  uint32_t dr;

  // Enter initialization mode
  // Refer to MCU RM 41.6.4 and 41.3.11, 3rd subsection, items 1-2
  RTC->ICSR |= RTC_ICSR_INIT;
  while (!(RTC->ICSR & RTC_ICSR_INITF)) {
  } // Wait for INITF

  dr = 0;
  dr |= ((date[2] - '0') << RTC_DR_YT_Pos) & RTC_DR_YT_Msk; // Year tens
  dr |= ((date[3] - '0') << RTC_DR_YU_Pos) & RTC_DR_YU_Msk; // Year units
  dr |= ((date[5] - '0') << RTC_DR_MT_Pos) & RTC_DR_MT_Msk; // Month tens
  dr |= ((date[6] - '0') << RTC_DR_MU_Pos) & RTC_DR_MU_Msk; // Month units
  dr |= ((date[8] - '0') << RTC_DR_DT_Pos) & RTC_DR_DT_Msk; // Day tens
  dr |= ((date[9] - '0') << RTC_DR_DU_Pos) & RTC_DR_DU_Msk; // Day units
  RTC->DR = dr;

  // Return to free-running mode
  // Refer to MCU RM 41.6.4 and 41.3.11, 3rd subsection, item 5
  RTC->ICSR &= ~RTC_ICSR_INIT;
}
