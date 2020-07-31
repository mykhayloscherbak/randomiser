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
#include "../fonts/arial72.h"


#define XSIZE 256
#define YSIZE 160
#define FRAME_BUF_SIZE (XSIZE * YSIZE / 8)
#define DELIM_WIDTH 2
#define LCD_ADDR 0x78

/* Vop = 16V */
/* Bias = 1/14 */
/* Duty = 1/160 */

static uint8_t FrameBufWithPrefix[FRAME_BUF_SIZE + 5];
static uint8_t * const FrameBuf = FrameBufWithPrefix + 5;

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
		 D, 0x28,// 1,
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
		 LC, 0xa6

//
//		{.cmd = 0xaf, .a0 = 0},
//		{.cmd = 0x43, .a0 = 0}
};

uint8_t st75256_init(LCD_Desc_t * const lcdDesc)
{
	Gpio_Clear_Bit(GPIO_RESET);
	DelayMsTimer(10);
	Gpio_Set_Bit(GPIO_RESET);
	DelayMsTimer(10);
	lcdDesc->xsize = XSIZE;
	lcdDesc->ysize = YSIZE;
	return i2cSend(LCD_ADDR, (uint8_t *) Init_Array, sizeof(Init_Array));

}



void st75256_test(void)	//		MainLoop_Iteration();
{
	memset(FrameBuf, 0x00, FRAME_BUF_SIZE);
	st75256_puts(60,40,&arial_72ptFontInfo,L"123");
//	st75256_setCoordinates(0,0);
	st75256_sendBuffer();
}

void st75256_sendBuffer(void)
{
	FrameBufWithPrefix[0] = C;
	FrameBufWithPrefix[1] = 0x30;
	FrameBufWithPrefix[2] = C;
	FrameBufWithPrefix[3] = 0x5C;
	FrameBufWithPrefix[4] = LD;
	i2cSend(LCD_ADDR, FrameBufWithPrefix, sizeof(FrameBufWithPrefix));
}

void st75256_setCoordinates(const uint8_t Column,const uint8_t Page)
{
	uint8_t conf[]=
	{
			C,0x30,
			C,0x75,
			D,Page / 8,
			D,0x14,
			C,0x15,
			D,Column,
			LD,0xFF
	};
	i2cSend(LCD_ADDR, conf,sizeof(conf));
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
