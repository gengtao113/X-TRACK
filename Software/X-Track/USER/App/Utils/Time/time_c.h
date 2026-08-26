#ifndef __TIME_C_H
#define __TIME_C_H

#ifdef __cplusplus
extern "C" {
#endif

#define TIME_SECS_PER_HOUR 3600L

void Time_Set(int hour, int min, int sec, int day, int month, int year);
void Time_Adjust(long adjustment);
int  Time_Year(void);
int  Time_Month(void);
int  Time_Day(void);
int  Time_Hour(void);
int  Time_Minute(void);
int  Time_Second(void);

#ifdef __cplusplus
}
#endif

#endif
