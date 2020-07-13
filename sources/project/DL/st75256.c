/**
 * @file st75256.c
 * @brief contains st75256 256x160 screen driver
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 11-07-2020
 */
#if 0 //Project switched back to uc1701 (JLX12864G)
#include "st75256.h"

#include <string.h>
#include "Spi.h"
#include "Gpio.h"
#include "Clock.h"
#include "../fonts/arial36.h"
#include "../fonts/arial8.h"
#include "../fonts/fonts.h"

#define ZERO_OFFSET 4
#define XSIZE 256
#define YSIZE 160

/* Vop = 16V */
/* Bias = 1/14 */

uint8_t FrameBuf[FRAME_BUF_SIZE];
typedef struct
{
	uint8_t cmd;
	uint8_t a0;
} Lcd_init_t;

static const Lcd_init_t Init_Array[]=
{
		{.cmd = 0x30, .a0 = 0},    /* select 00 commands */
		{.cmd = 0x94, .a0 = 0},    /* sleep out */
		{.cmd = 0xae, .a0 = 0},    /* display off */
		{.cmd = 0x23, .a0 = 0},
		{.cmd = 0x31, .a0 = 0},    /* select 01 commands */
		{.cmd = 0xd7, .a0 = 0},  /* disable auto read, 2 byte command */
		{.cmd = 0x9f, .a0 = 1},
		{.cmd = 0x32, .a0 = 0}, /* analog circuit set, 4 bytes */
		{.cmd = 0x00, .a0 = 1},
		{.cmd = 0x01, .a0 = 1}, /* Frequency on booster capacitors 1 = 6KHz? */
		{.cmd = 0x00, .a0 = 1}, /* Bias: 0: 1/14 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
		{.cmd = 0x30, .a0 = 0},
		{.cmd = 0x75, .a0 = 0},
		{.cmd = 0x00, .a0 = 1},
		{.cmd = 0x4f, .a0 = 1},
		{.cmd = 0x15, .a0 = 0},
		{.cmd = 0x00, .a0 = 1},
		{.cmd = 0xff, .a0 = 1},
		{.cmd = 0xbc, .a0 = 0}, /* scan direction */
		{.cmd = 0x00, .a0 = 1},
		{.cmd = 0x0c, .a0 = 0},
		{.cmd = 0xca, .a0 = 0}, /* Display control */
		{.cmd = 0x00, .a0 = 1},
		{.cmd = 159, .a0 = 1},
		{.cmd = 0x20, .a0 = 1},
		{.cmd = 0xf0, .a0 = 0}, /* no GS */
		{.cmd = 0x10, .a0 = 1},
		{.cmd = 0x81, .a0 = 0}, /* Vop = 16V */
		{.cmd = 0x36, .a0 = 1},
		{.cmd = 0x00, .a0 = 1},
		{.cmd = 0x20, .a0 = 0},
		{.cmd = 0x0b, .a0 = 1}
//
//		{.cmd = 0xaf, .a0 = 0},
//		{.cmd = 0x43, .a0 = 0}
};

void st75256_init(void)
{
	Gpio_Set_Bit(GPIO_D4);
	uint32_t Timer;
	Gpio_Clear_Bit(GPIO_RESET);
	SetTimer(&Timer,10);
	while (IsTimerPassed(Timer))
	{

	}
	Gpio_Set_Bit(GPIO_RESET);
	SetTimer(&Timer,10);
	while (IsTimerPassed(Timer))
	{

	}

	Gpio_Clear_Bit(GPIO_NSS);
	for (uint8_t i = 0; i < sizeof(Init_Array) / sizeof(Init_Array[0]); i++)
	{
		if (Init_Array[i].a0 == 0)
		{
			Gpio_Clear_Bit(GPIO_DC);
		}
		else
		{
			Gpio_Set_Bit(GPIO_DC);
		}
		SPI_Send_One(Init_Array[i].cmd);
	}
	Gpio_Set_Bit(GPIO_NSS);
	DelayMsTimer(100);
	Gpio_Clear_Bit(GPIO_NSS);
	Gpio_Clear_Bit(GPIO_DC);
	SPI_Send_One(0xaf);
	SPI_Send_One(0x23);
	SPI_Send_One(0x31);
	SPI_Send_One(0x41);
	Gpio_Set_Bit(GPIO_NSS);
}
#if 0
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
		const uint8_t yrow = y >> 3;
		const uint8_t ymask = 1 << (y & 7);
		const uint16_t addr = yrow * 132 + x + ZERO_OFFSET;
		if (color == 0)
		{
			FrameBuf[addr] &= ~ymask;
		}
		else
		{
			FrameBuf[addr] |= ymask;
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
#endif
#endif
