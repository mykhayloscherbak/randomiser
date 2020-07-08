#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "time_t.h"
#include "timezone.h"

static const uint8_t monthLength[2][12] = {
        {31,28,31,30,31,30,31,31,30,31,30,31},
        {31,29,31,30,31,30,31,31,30,31,30,31},
};

static bool isleap(const uint16_t year) {
    return ((year % 4) == 0) && (((year % 100) != 0) || ((year % 400)) == 0);
}

/* Use Zeller's Congruence to get day-of-week of first day of month. */

static uint8_t weekDayOfMonth(const time_t * const datetime)
{
    uint16_t year = datetime->year + 2000;
    const uint8_t monthTranslated = (datetime->month + 9) % 12 + 1;
    const uint16_t yearTranslated = (datetime->month <= 2) ? (year - 1) : year;
    const uint8_t century =  yearTranslated / 100;
    const uint8_t yearWithoutCentury = yearTranslated % 100;
    int8_t dayOfTheWeek = ((13 * monthTranslated - 1) / 5 + 1 + yearWithoutCentury + yearWithoutCentury / 4 + century / 4 - 2 * century + datetime->day + 5) % 7 + 1;
    return dayOfTheWeek; /* MON = 1 , SUN = 7 */
}

/*
 * returns the date of the last day of the week before or equal.
 * For example if dd-mm-yy is date and weekday is 1 the function will return the date of the last monday before  or equal dd-mm-yy
 */
static uint8_t getLastBefore(const time_t * const datetime)
{
    uint8_t currentWeekDay = weekDayOfMonth(datetime);
    const uint8_t shift = (currentWeekDay >= datetime->weekdays) ? currentWeekDay - datetime->weekdays : 7 - datetime->weekdays + currentWeekDay;
    return datetime->day - shift;
}


/* the same as getLastBefore but returns the last weekday of the month. For example weekdays = 1 month = 5 and year = 19 returns the last Monday of may 2019 */
static uint8_t getLastWeekdayOfTheMonth(const time_t * const datetime)
{
    time_t tmp;
    memcpy(&tmp,datetime,sizeof(tmp));
    tmp.day = isleap(tmp.year + 2000) ? monthLength[1][tmp.month - 1] : monthLength[0][tmp.month - 1];
    return getLastBefore(&tmp);
}

/*
 * returns the date of the first day of the week before or equal.
 * For example if dd-mm-yy is date and weekday is 1 the function will return the date of the first Monday after or equal dd-mm-yy
 */
static uint8_t getFirstAfter(const time_t * const datetime)
{
    uint8_t currentWeekDay = weekDayOfMonth(datetime);
    const uint8_t shift = (currentWeekDay <= datetime->weekdays) ? datetime->weekdays - currentWeekDay : 7 - currentWeekDay + datetime->weekdays;
    return datetime->day + shift;
}


/* the same as getFirstAfter but returns the first weekday of the month. For example weekdays = 1 month = 5 and year = 19 returns the first Monday of may 2019 */
static uint8_t getFirstWeekdayOfTheMonth(const time_t * const datetime)
{
    time_t tmp;
    memcpy(&tmp,datetime,sizeof(tmp));
    tmp.day = 1;
    return getFirstAfter(&tmp);
}


/* datetime->year must be filled before */
static void getDateOfEvent(const DstEvent_t  * const  event, time_t * const datetime)
{
    switch (event->on.condition)
    {
    case On_exact:
        datetime->day = event->on.date;
        datetime->month = event->month;
        break;
    case On_first:
        datetime->weekdays = event->on.dayOfTheWeek;
        datetime->month = event->month;
        datetime->day = getFirstWeekdayOfTheMonth(datetime);
        break;
    case On_last:
        datetime->weekdays = event->on.dayOfTheWeek;
        datetime->month = event->month;
        datetime->day = getLastWeekdayOfTheMonth(datetime);
        break;
    case On_less:
        datetime->weekdays = event->on.dayOfTheWeek;
        datetime->month = event->month;
        datetime->day = event->on.date;
        datetime->day = getLastBefore(datetime);
        break;
    case On_more:
        datetime->weekdays = event->on.dayOfTheWeek;
        datetime->month = event->month;
        datetime->day = event->on.date;
        datetime->day = getFirstAfter(datetime);
        break;
    default:
        memset(datetime,0,sizeof(time_t));
    }
}

static uint32_t getMinutesFromDate(const time_t * const datetime)
{
    uint32_t retVal = 0;
    for (uint8_t year = 0; year < datetime->year; year++)
    {
        retVal += isleap(year + 2000) ? 366 : 365;
    }
    const uint8_t leap = isleap(datetime->year + 2000) ? 1 : 0;
    retVal += datetime->day - 1;
    for (uint8_t month = 1; month < datetime->month; month++)
    {
        retVal += monthLength[leap][month -1];
    }

    retVal = retVal * 24 + datetime->hours;
    retVal = retVal * 60 + datetime->mins;
    return retVal;
}

static void incrementDate(const int32_t minutes, time_t * const datetime)
{
    int32_t gmtMinutes = getMinutesFromDate(datetime) + minutes; /* Never decrement so that it becomes negative */
    datetime->mins = gmtMinutes % 60;
    gmtMinutes /= 60; /* Hours left */
    datetime->hours = gmtMinutes % 24;
    gmtMinutes /= 24; /* Days left */
    uint8_t month = 0;
    while (gmtMinutes >= 0)
    {
        const uint8_t leap = isleap(month / 12 + 2000) ? 1 : 0;
        gmtMinutes -= monthLength[leap][month % 12];
        month++;
    }

    if (month > 0)
    {
        month--;
        datetime->month = month % 12 + 1;
        const uint8_t leap = isleap(month / 12 + 2000) ? 1 : 0;
        datetime->day = gmtMinutes + monthLength[leap][month % 12] + 1;
        datetime->year = month / 12;
    }

    else
    {
        datetime->month = 1;
        datetime->day = 1;
        datetime->year = 0;
    }
}


static bool isGmtLaterThanEvent(const time_t * const gmt, const DstEvent_t * const event,const int16_t gmt2StandartMinutes,const int16_t diffEventSave)
{
    const int32_t gmtMinutes = getMinutesFromDate(gmt);
    int16_t offset = 0;
    switch (event->at.flags)
    {
    case At_wall:
        offset += gmt2StandartMinutes + diffEventSave;
        break;
    case At_std:
        offset += gmt2StandartMinutes;
        break;
    case At_gmt:
        break;
    default:
        break;
    }
    time_t eventTime = {.year = gmt->year};
    getDateOfEvent(event,&eventTime);
    eventTime.hours = event->at.hours;
    eventTime.mins = event->at.minutes;
    eventTime.secs = 0;
    eventTime.hundredth = 0;
    const int32_t eventMinutes = getMinutesFromDate(&eventTime);
    return gmtMinutes >= eventMinutes - offset;
}

void gmt2local(time_t * const datetime, const TZUnpacked_t * const tz)
{
    int16_t offset = tz->offset;
    if (tz->from.month != 0 && tz->to.month != 0) /* DST */
    {
        const bool afterTo = isGmtLaterThanEvent(datetime,&tz->to,tz->offset,tz->from.save);
        const bool afterFrom = isGmtLaterThanEvent(datetime,&tz->from,tz->offset,tz->to.save);
        if ((afterTo || afterFrom) && !(afterTo && afterFrom)) /* summer */
        {
            offset += tz->to.save;
        }
        else
        {
            offset += tz->from.save;
        }
    }
    incrementDate(offset,datetime);
    datetime->weekdays = weekDayOfMonth(datetime);
}
