
#ifndef SOURCES_PROJECT_DL_ST75256_H_
#define SOURCES_PROJECT_DL_ST75256_H_
#include <stdint.h>
#include "../fonts/fonts.h"
#include <wchar.h>

typedef struct
{
	uint16_t xsize;
	uint16_t ysize;
} LCD_Desc_t;

void st75256_init(LCD_Desc_t * const lcdDesc);
void st75256_test(void);
void st75256_sendBuffer(void);
void st75256_setCoordinates(const uint8_t Column,const uint8_t Page);
void st75256_pixel(const uint8_t x, const uint8_t y, const uint8_t color);
uint8_t st75256_putchar(const uint8_t x,const uint8_t y,const FONT_INFO * const font,const uint16_t c);
uint8_t st75256_puts(const uint8_t x, const uint8_t y, const FONT_INFO * const font, const wchar_t * const string);
uint8_t st75256_get_font_height(const FONT_INFO * const Font);
uint8_t st75256_get_font_width(const FONT_INFO * const Font);




#endif /* SOURCES_PROJECT_DL_ST75256_H_ */
