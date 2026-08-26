#include "time_c.h"
#include "Time.h"

void Time_Set(int hour, int min, int sec, int day, int month, int year)
{
    setTime(hour, min, sec, day, month, year);
}

void Time_Adjust(long adjustment)
{
    adjustTime(adjustment);
}

int Time_Year(void)
{
    return year();
}

int Time_Month(void)
{
    return month();
}

int Time_Day(void)
{
    return day();
}

int Time_Hour(void)
{
    return hour();
}

int Time_Minute(void)
{
    return minute();
}

int Time_Second(void)
{
    return second();
}
