#ifndef SOURCES_PROJECT_BL_INCLUDE_TIMEZONE_H_
#define SOURCES_PROJECT_BL_INCLUDE_TIMEZONE_H_
#include <stdint.h>
#include "time_t.h"
typedef enum  {
    On_exact = 0,
    On_less,
    On_more,
    On_last,
    On_first,
    On_total
} OnConditions_t ;

typedef enum {
    At_wall = 0,
    At_std,
    At_gmt,
    At_total
} AtTime_t ;


typedef struct {
    uint8_t date;
    OnConditions_t  condition;
    uint8_t dayOfTheWeek;
} On_t ;

typedef struct {
    AtTime_t flags;
    uint8_t hours;
    uint8_t minutes;
} At_t;

typedef struct {
    On_t on;
    At_t at;
    uint8_t month;
    uint16_t save;
} DstEvent_t;

typedef struct {
    const char * const name;
    int16_t offset;
    DstEvent_t to;
    DstEvent_t from;
} TZUnpacked_t;
extern const uint16_t numberOfTimezones;
extern const  TZUnpacked_t tzData[];

void gmt2local(time_t * const datetime, const TZUnpacked_t * const tz);

#endif /* SOURCES_PROJECT_BL_INCLUDE_TIMEZONE_H_ */
