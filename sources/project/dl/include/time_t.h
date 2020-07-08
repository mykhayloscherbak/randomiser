#ifndef SOURCES_PROJECT_DL_INCLUDE_TIME_T_H_
#define SOURCES_PROJECT_DL_INCLUDE_TIME_T_H_

typedef struct {
	uint8_t year; /* from y2k */
	uint8_t month;
	uint8_t day;
	uint8_t weekdays;
	uint8_t hours;
	uint8_t mins;
	uint8_t secs;
	uint8_t hundredth;
} time_t;


#endif /* SOURCES_PROJECT_DL_INCLUDE_TIME_T_H_ */
