/**
 * @file uc1701x.c
 * @brief contains uc1701x 128x64 screen driver
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 23-06-2018
 */

#include <string.h>
#include "Spi.h"
#include "uc1701x.h"
#include "Gpio.h"
#include "Clock.h"
#include "../fonts/arial36.h"
#include "../fonts/arial8.h"
#include "../fonts/fonts.h"

#define ZERO_OFFSET 4
#define XSIZE 64
#define YSIZE 128
#define DELIM_WIDTH 1
#define CMD_SetColumn_l 		 0x00 /* 4 */
#define CMD_SetColumn_h 		 0x10 /* 4 */
#define CMD_SetPowerCtl 		 0x28 /* 5 */
#define CMD_SetScrollLine 		 0x40 /* 6 */
#define CMD_SetPageAddr   		 0xB0 /* 7 */
#define CMD_SetVlcd				 0x20 /* 8 */
#define CMD_SetElVolume		     0x81 /* 9 Double byte */
#define CMD_SetInvDisplay		 0xA6 /* 11 */
#define CMD_SetDispEnable 		 0xAE /* 12 */
#define CMD_SetSEGDirection		 0xA0 /* 13 */
#define CMD_SetCOMDirection		 0xC0 /* 14 */
#define CMD_Reset				 0xE2 /* 15 */
#define CMD_SetBiasRatio		 0xA2 /* 17 */
#define CMD_SetCursorUpdate		 0xE0 /* 18 */
#define CMD_ResetCursorUpdate    0xEE /* 19 */
#define CMD_SetAPC_0			 0xFA /* 25 Double byte */


uint8_t FrameBuf[FRAME_BUF_SIZE];


static const uint8_t Init_Array[]=
{
		CMD_Reset,
		CMD_SetScrollLine, /* 0 */
		CMD_SetSEGDirection,
		CMD_SetCOMDirection,
		CMD_SetBiasRatio,
		CMD_SetPowerCtl | 0x7,
		CMD_SetVlcd | 0x3,
		CMD_SetElVolume,0x28, /* Double byte */
		CMD_ResetCursorUpdate,
		CMD_SetAPC_0, 0x93, /* (TC and wrappers) */
		CMD_SetInvDisplay,  /* No inv */
		CMD_SetDispEnable | 0x1
};

void uc1701x_init(void)
{
	Gpio_Set_Bit(GPIO_RESET);
	uint32_t Timer;
	SetTimer(&Timer,10);
	while (IsTimerPassed(Timer))
	{

	}
	Gpio_Clear_Bit(GPIO_DC);
	Gpio_Clear_Bit(GPIO_NSS);
	for (uint8_t i = 0; i < sizeof(Init_Array) / sizeof(Init_Array[0]); i++)
	{
		SPI_Send_One(Init_Array[i]);
	}
	Gpio_Set_Bit(GPIO_NSS);
}

void uc1701x_setMirror(const UC1701X_MIRROR_t mirror)
{
	Gpio_Clear_Bit(GPIO_DC);
	Gpio_Clear_Bit(GPIO_NSS);
	const uint8_t x_cmd = ((mirror & UC1701X_MIRROR_x) != 0) ? CMD_SetSEGDirection | 0x1 : CMD_SetSEGDirection;
	const uint8_t y_cmd = ((mirror & UC1701X_MIRROR_y) != 0) ? CMD_SetCOMDirection | 0x8 : CMD_SetCOMDirection;
	SPI_Send_One(x_cmd);
	SPI_Send_One(y_cmd);
	Gpio_Set_Bit(GPIO_NSS);
}

void uc1701x_set_coordinates(const uint8_t Column,const uint8_t Page)
{
	Gpio_Clear_Bit(GPIO_DC);
	Gpio_Clear_Bit(GPIO_NSS);
	SPI_Send_One(CMD_SetColumn_l | (Column & 0xF));
	SPI_Send_One(CMD_SetColumn_h | ((Column >> 4 ) & 0xF));
	SPI_Send_One(CMD_SetPageAddr | (Page & 0xF));
	Gpio_Set_Bit(GPIO_NSS);
}

void uc1701x_set_contrast(const uint8_t Contrast)
{
	Gpio_Clear_Bit(GPIO_DC);
	Gpio_Clear_Bit(GPIO_NSS);
	SPI_Send_One(CMD_SetElVolume);
	SPI_Send_One(Contrast <=  63 ? Contrast : 0x10); /* Default if incorrect */
	Gpio_Set_Bit(GPIO_NSS);
}


void uc1701x_pixel(const uint8_t x, const uint8_t y, const uint8_t color)
{
	if ((x < XSIZE) && (y < YSIZE))
	{
		const uint8_t xcol = x >> 3;
		const uint8_t xmask = 1 << (x & 7);
		const uint16_t addr = xcol * 132 + y + ZERO_OFFSET;
		if (color == 0)
		{
			FrameBuf[addr] &= ~xmask;
		}
		else
		{
			FrameBuf[addr] |= xmask;
		}
	}
}

uint8_t uc1701x_putchar(const uint8_t x,const uint8_t y,const FONT_INFO * const font,const uint16_t c)
{
	uint8_t RetVal = x;
	if ((c >= font->StartChar) && (c <= font->EndChar))
	{
		const uint8_t * const bitmap = font->Info[c - font->StartChar].offset + font->bitmaps;
		const uint8_t bitWidth = font->Info[c - font->StartChar].width;
		const uint8_t byteWidth = ((bitWidth & 0x7) == 0) ? bitWidth / 8 : bitWidth / 8 + 1;
		const uint8_t bitHeight = font->height;
		if ((x+bitWidth < XSIZE) && ( y + bitHeight < YSIZE))
		{
			for (uint8_t xx = 0; xx < bitWidth; xx++)
			{
				for (uint8_t yy = 0; yy < bitHeight; yy++)
				{
					const uint8_t * const Row = bitmap + yy * byteWidth;
					const uint8_t Color = Row[xx >> 3] & ( 1 << ( 7 - (xx & 0x7)));
					uc1701x_pixel(x + xx, y + yy, Color);
				}
			}
			RetVal = x + bitWidth;
		}

	}
	return RetVal;
}

uint8_t uc1701x_puts(const uint8_t x, const uint8_t y, const FONT_INFO * const font, const wchar_t * const string)
{
	uint8_t x_curr = x;
	uint8_t pos = 0;
	const uint8_t Height = uc1701x_get_font_height(font);
	while ((x_curr < XSIZE) && (string[pos] != 0))
	{
		x_curr = uc1701x_putchar(x_curr,y,font,string[pos++]);
		for (uint8_t xx = x_curr; xx < x_curr + DELIM_WIDTH; xx++ )
		{
			for (uint8_t yy = y; yy < y + Height; yy++ )
			{
				uc1701x_pixel(xx,yy,0);
			}
		}
		x_curr += DELIM_WIDTH;
	}
	return x_curr;
}

uint8_t uc1701x_get_font_height(const FONT_INFO * const Font)
{
	return Font->height;
}

uint8_t uc1701x_get_symbol_width(const FONT_INFO * const FONT, const wchar_t c)
{
	return FONT->Info[c - FONT->StartChar].width;
}

uint8_t uc1701x_get_font_width(const FONT_INFO * const Font)
{
	uint8_t Width = 0;
	for (uint16_t i = Font->StartChar; i <= Font->EndChar; i++)
	{
		const uint8_t CurrWidth = Font->Info[i - Font->StartChar].width;
		Width = (CurrWidth > Width) ? CurrWidth : Width;
	}
	return Width + DELIM_WIDTH;
}

void uc1701x_cls(void)
{
	memset(FrameBuf,0,FRAME_BUF_SIZE);
}
