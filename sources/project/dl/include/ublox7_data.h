#ifndef SOURCES_PROJECT_DL_INCLUDE_UBLOX7_DATA_H_
#define SOURCES_PROJECT_DL_INCLUDE_UBLOX7_DATA_H_

#include <stdint.h>
#include "project_config.h"
#pragma pack(push,1)

typedef enum
{
	cfgChars_5 = 0b00,
	cfgChars_6 = 0b01,
	cfgChars_7 = 0b10,
	cfgChars_8 = 0b11
}cfgChars_t;

typedef enum
{
	cfgParity_odd = 0b000,
	cfgParity_even = 0b001,
	cfgParity_none = 0b101
}cfgParity_t;

typedef enum
{
	cfgStop_1 = 0b00,
	cfgStop_1_5 = 0b01,
	cfgStop_2 = 0b10,
	cfgStop_0_5 = 0b11
}cfgStop_t;

struct ublox_CFG_PRT_s
{
	uint8_t portID;
	uint8_t reserved0;

	uint8_t txReady_en:1;
	uint8_t txReady_pol :1;
	uint8_t txReady_pin :5;
	uint16_t txReady_threshold :9;

	uint8_t :6;
	uint8_t bfMode_charLen :2;
	uint8_t :1;
	uint8_t bfMode_parity :3;
	uint8_t bfMode_nstop :2;
	uint32_t :18;

	uint32_t baudRate;

	uint8_t  InProto_UBX :1;
	uint8_t  InProto_NMEA :1;
	uint8_t  InProto_RTCM :1;
	uint16_t :13;

	uint8_t  OutProto_UBX :1;
	uint8_t  OutProto_NMEA :1;
	uint16_t  :14;

	uint8_t :1;
	uint8_t Flags_extendedTimeout :1;
	uint16_t :14;
	uint16_t reserved1;
};

struct ublox_CFG_MSGRATE_s
{
	uint8_t Class;
	uint8_t Id;
	uint8_t Rate;
};

struct ublox_NAV_CLOCK_s
{
	uint32_t TOW;
	int32_t Bias;
	int32_t Drift;
	uint32_t Accuracy;
	uint32_t FreqEstimate;
};

struct ublox_NAV_TIMEUTC_s
{
	uint32_t iTOW;
	uint32_t AccuracyEstimate;
	int32_t fracrionNs;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t validTow :1;
	uint8_t validWKn :1;
	uint8_t validUTC :1;
	uint8_t :5;
};

struct ublox_CFG_TP5_s
{
	uint8_t tpIdx;
	uint8_t :8;
	uint16_t :16;
	int16_t antennaCableDelay_ns;
	int16_t rfGroupDelay_ns;
	uint32_t freqPeriod;
	uint32_t freqPeriodLocked;
	uint32_t pulseLenRatio;
	uint32_t pulseLenRatioLocked;
	int32_t userConfigDelay;
	uint8_t active :1;
	uint8_t lockGpsFreq :1;
	uint8_t lockOtherSet :1;
	uint8_t isFreq :1;
	uint8_t isLength :1;
	uint8_t alignToTow :1;
	uint8_t polarity :1;
	uint8_t gridUtcGps :1;
	uint32_t :24;

};
#pragma pack(pop)

#endif /* SOURCES_PROJECT_DL_INCLUDE_UBLOX7_DATA_H_ */
