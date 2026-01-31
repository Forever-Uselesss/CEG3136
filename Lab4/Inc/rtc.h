#ifndef RTC_H_
#define RTC_H_

void RTC_Enable(void);
void RTC_GetTime(char *time); // "HH:MM:SS" ( 9 bytes, including NUL)
void RTC_GetDate(char *date); // "YYYY-MM-DD" (11 bytes, including NUL)
void RTC_SetTime(const char *time);
void RTC_SetDate(const char *date);

#endif /* RTC_H_ */
