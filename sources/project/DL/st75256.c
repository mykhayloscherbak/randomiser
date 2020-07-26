/**
 * @file st75256.c
 * @brief contains st75256 256x160 screen driver
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 11-07-2020
 */
#if 1 //Project switched back to uc1701 (JLX12864G)
#include "st75256.h"

#include <string.h>
#include "Spi.h"
#include "Gpio.h"
#include "Clock.h"
#include "i2c.h"

#define XSIZE 256
#define YSIZE 160

/* Vop = 16V */
/* Bias = 1/14 */

//uint8_t FrameBuf[FRAME_BUF_SIZE];

enum
{
	C = 0x80,
	D = 0xC0,
	LC = 0x00,
	LD = 0x40
};

static const uint8_t Init_Array[]=
{
		 C, 0x30,// 0,    /* select 00 commands */
		 C, 0x94,// 0,    /* sleep out */
		 C, 0xae,// 0,    /* display off */
		 C, 0x23,// 0,
		 C, 0x31,// 0,    /* select 01 commands */
		 C, 0xd7,// 0,  /* disable auto read, 2 byte command */
		 D, 0x9f,// 1,
		 C, 0x32,// 0, /* analog circuit set, 4 bytes */
		 D, 0x00,// 1,
		 D, 0x01,// 1, /* Frequency on booster capacitors 1 = 6KHz? */
		 D, 0x00,// 1, /* Bias: 0: 1/14 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
		 C, 0x30,// 0,
		 C, 0x75,// 0,
		 D, 0x00,// 1,
		 D, 0x4f,// 1,
		 C, 0x15,// 0,
		 D, 0x00,// 1,
		 D, 0xff,// 1,
		 C, 0xbc,// 0, /* scan direction */
		 D, 0x00,// 1,
		 C, 0x0c,// 0,
		 C, 0xca,// 0, /* Display control */
		 D, 0x00,// 1,
		 D,  159,// 1,
		 D, 0x20,// 1,
		 C, 0xf0,// 0, /* no GS */
		 D, 0x10,// 1,
		 C, 0x81,// 0, /* Vop = 16V */
		 D, 0x36,// 1,
		 D, 0x00,// 1,
		 C, 0x20,// 0,
		 D, 0x0b, // 1
		 C, 0xaf, // 0
		 LC, 0x43 // 0
//
//		{.cmd = 0xaf, .a0 = 0},
//		{.cmd = 0x43, .a0 = 0}
};

void st75256_init(void)
{
	Gpio_Clear_Bit(GPIO_RESET);
	DelayMsTimer(10);
	Gpio_Set_Bit(GPIO_RESET);
	DelayMsTimer(10);
	i2cSend(0x78, (uint8_t *) Init_Array, sizeof(Init_Array));

}
#if 0
void uc1701x_set_coordinates(const uint8_t Column,const uint8_t Page)
{	Gpio_Set_Bit(GPIO_D4);
uint32_t Timer;

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
