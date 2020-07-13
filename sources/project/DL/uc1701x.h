/**
 * @file uc1701x.h
 * @brief contains uc1701x 128x64 screen driver
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 25-06-2018
 */

#ifndef SOURCE_DL_UC1701X_H_
#define SOURCE_DL_UC1701X_H_

#include <wchar.h>
#include "../fonts/fonts.h"
#define FRAME_BUF_SIZE (132 * 64 / 8)

extern uint8_t FrameBuf[FRAME_BUF_SIZE];

typedef enum
{
	UC1701X_MIRROR_none = 0,
	UC1701X_MIRROR_x = 1,
	UC1701X_MIRROR_y = 2,
	UC1701X_MIRROR_xy = 3
} UC1701X_MIRROR_t;

/**
 * @brief inits uc1701x with inited SPI
 */
void uc1701x_init(void);
/**
 * @brief Sets cursor to position
 * @param Column Column
 * @param Page Row * 8
 */
void uc1701x_set_coordinates(const uint8_t Column,const uint8_t Page);

/**
 * @brief Sets contrast of lcd
 * @param Contrast 0..63
 */
void uc1701x_set_contrast(const uint8_t Contrast);

/**
 * @brief Puts a pixel
 * @param x Coordinate
 * @param y Coordinate
 * @param color 0/1
 */
void uc1701x_pixel(const uint8_t x,const uint8_t y,const uint8_t color);

/**
 * @brief Prints a character
 * @param x Horizontal coord
 * @param y Vertical coord
 * @param font pointer to font descriptor
 * @param c character to print (maybe more than char size)
 * @return Next horisontal position
 */
uint8_t uc1701x_putchar(const uint8_t x,const uint8_t y,const FONT_INFO * const font,const uint16_t c);

/**
 * @brief Displays a line without wrapping
 * @param x start x coordinate
 * @param y start y coordinate
 * @param font font reference
 * @param string zero-terminated string
 * @return
 */
uint8_t uc1701x_puts(const uint8_t x, const uint8_t y, const FONT_INFO * const font, const wchar_t * const string);

/**
 * @brief Returns height of the font
 * @param Font font id
 * @return height in pixels
 */
uint8_t uc1701x_get_font_height(const FONT_INFO * const Font);
/**
 * @brief Returns max width in pixels of the font
 * @param Font font id
 * @return maximal width
 */
uint8_t uc1701x_get_font_width(const FONT_INFO * const Font);

void uc1701x_setMirror(const UC1701X_MIRROR_t mirror);
void uc1701x_cls(void);

#endif /* SOURCE_DL_UC1701X_H_ */
