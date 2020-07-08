#ifndef SOURCE_FONTS_FONTS_H_
#define SOURCE_FONTS_FONTS_H_
#include <stdint.h>

#pragma pack(push,1)

typedef struct
{
		uint8_t width;
		uint32_t offset;
} FONT_CHAR_INFO;


typedef struct
{
	uint8_t height;
	uint16_t StartChar;
	uint16_t EndChar;
	const FONT_CHAR_INFO * Info;
	const uint8_t * bitmaps;
} FONT_INFO ;

#pragma pack(pop)

#endif /* SOURCE_FONTS_FONTS_H_ */
