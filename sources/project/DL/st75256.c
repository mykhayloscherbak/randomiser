/**
 * @file st75256.c
 * @brief contains st75256 256x160 screen driver
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 11-07-2020
 */

#include "st75256.h"

#include <string.h>
#include "Spi.h"
#include "Gpio.h"
#include "Clock.h"
#include "i2c.h"
#include <wchar.h>
#include <stdio.h>
#include "../fonts/arial144.h"


#define XSIZE 256
#define YSIZE 160
#define FRAME_BUF_SIZE (XSIZE * YSIZE / 8)
#define DELIM_WIDTH 2


/* Vop = 16V */
/* Bias = 1/14 */
/* Duty = 1/160 */


enum
{
	C = 0x0,
	D = 0x1
};

static const uint8_t prefix[] =
	{
			C,0x30,
			C,0x15,
			D,0,
			D,0xFF,
			C,0x75,
			D,0x00,
			D,0x28,
			C,0x5c
	};


static uint8_t FrameBuf[FRAME_BUF_SIZE];


void st75256_init(LCD_Desc_t * const lcdDesc)
{
	static const uint8_t Init_Array[]=
	{
			 C, 0x30,// 0,    /* select 00 commands */
			 C, 0x94,// 0,    /* sleep out */
			 C, 0xae,// 0,    /* display off */
			 C, 0x31,// 0,    /* select 01 commands */

			 C, 0xd7,// 0,  /* disable auto read, 2 byte command */
			 D, 0x9f,// 1,

			 C, 0x32,// 0, /* analog circuit set, 4 bytes */
			 D, 0x00,// 1,
			 D, 0x01,// 1, /* Frequency on booster capacitors 1 = 6KHz? */
			 D, 0x03,// 1, /* Bias: 0: 1/14 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */

			 C, 0x30,// 0,

			 C, 0x75,// 0,
			 D, 0x00,// 1,
			 D, 0x13,// 1,
			 C, 0x15,// 0,
			 D, 0x00,// 1,
			 D, 0xff,// 1,
			 C, 0xbc,// 0, /* scan direction */
			 D, 0x00,// 1,
			 C, 0x0c,// 0,
			 C, 0xca,// 0, /* Display control */
			 D, 0x00,// 1,
			 D, 0x9F,// 1,
			 D, 0x20,// 1,
			 C, 0xf0,// 0, /* no GS */
			 D, 0x10,// 1,
			 C, 0x81,// 0, /* Vop = 16V */
			 D, 0x36,// 1,
			 D, 0x04,// 1,
			 C, 0x20,// 0,
			 D, 0x0b, // 1
			 C, 0x40, // 0
			 C, 0xaf, // 0
			 C, 0xa6

	};

	Gpio_Clear_Bit(GPIO_RESET);
	DelayMsTimer(10);
	Gpio_Set_Bit(GPIO_RESET);
	DelayMsTimer(10);
	lcdDesc->xsize = XSIZE;
	lcdDesc->ysize = YSIZE;
	Gpio_Clear_Bit(GPIO_NSS);
	for (uint16_t i = 0; i < sizeof(Init_Array) / 2; i++)
	{
		if (Init_Array[i * 2] == C)
		{
			Gpio_Clear_Bit(GPIO_DC);
		}
		else
		{
			Gpio_Set_Bit(GPIO_DC);
		}
		SPI_Send_One(Init_Array[i * 2 + 1]);
	}
	Gpio_Set_Bit(GPIO_NSS);
}



void st75256_test(void)	//		MainLoop_Iteration();
{
	WaitTransfer();
	static uint8_t phase = 0;
	memset(FrameBuf, 0x00, FRAME_BUF_SIZE);
	wchar_t buf[20];
	swprintf(buf,20, L"%3u",phase++);
	st75256_puts(10,8,&arial_144ptFontInfo,buf);
	st75256_sendBuffer();
}

void st75256_sendBuffer(void)
{

	Gpio_Clear_Bit(GPIO_NSS);
	for (uint16_t i = 0; i < sizeof(prefix) / 2; i++)
	{
		if (prefix[i * 2] == C)
		{
			Gpio_Clear_Bit(GPIO_DC);
		}
		else
		{
			Gpio_Set_Bit(GPIO_DC);
		}
		SPI_Send_One(prefix[i * 2 + 1]);
	}
	SPI_Transfer(FrameBuf,sizeof(FrameBuf));
}



//void uc1701x_set_contrast(const uint8_t Contrast)
//{
//	Gpio_Clear_Bit(GPIO_DC);
//	Gpio_Clear_Bit(GPIO_NSS);
//	SPI_Send_One(CMD_SetElVolume);
//	SPI_Send_One(Contrast <=  63 ? Contrast : 0x10); /* Default if incorrect */
//	Gpio_Set_Bit(GPIO_NSS);
//}


void st75256_pixel(const uint8_t x, const uint8_t y, const uint8_t color)
{
	if ( /* (x < XSIZE) && */ (y < YSIZE))
	{
		const uint8_t yrow = y >> 3;
		const uint8_t ymask = 1 << (y & 7);
		const uint16_t addr = yrow * 256 + x;
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

uint8_t st75256_putchar(const uint8_t x,const uint8_t y,const FONT_INFO * const font,const uint16_t c)
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
					st75256_pixel(x + xx, y + yy, Color);
				}
			}
			RetVal = x + bitWidth;
		}

	}
	return RetVal;
}

uint8_t st75256_puts(const uint8_t x, const uint8_t y, const FONT_INFO * const font, const wchar_t * const string)
{
	uint16_t x_curr = x;
	uint8_t pos = 0;
	const uint8_t Height = st75256_get_font_height(font);
	while ((x_curr < XSIZE) && (string[pos] != 0))
	{
		x_curr = st75256_putchar(x_curr,y,font,string[pos++]);
		for (uint8_t xx = x_curr; xx < x_curr + DELIM_WIDTH; xx++ )
		{
			for (uint8_t yy = y; yy < y + Height; yy++ )
			{
				st75256_pixel(xx,yy,0);
			}
		}
		x_curr += DELIM_WIDTH;
	}
	return x_curr;
}

uint8_t st75256_get_font_height(const FONT_INFO * const Font)
{
	return Font->height;
}

uint8_t st75256_get_font_width(const FONT_INFO * const Font)
{
	uint8_t Width = 0;
	for (uint16_t i = Font->StartChar; i <= Font->EndChar; i++)
	{
		const uint8_t CurrWidth = Font->Info[i - Font->StartChar].width;
		Width = (CurrWidth > Width) ? CurrWidth : Width;
	}
	return Width + DELIM_WIDTH;
}
