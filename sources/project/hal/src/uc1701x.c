/**
 * @file uc1701x.c
 * @brief contains uc1701x 128x64 screen driver
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 28-04-2019
 * @version 2.00
 */

#include <string.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "Spi.h"
#include "uc1701x.h"
#include "Gpio.h"
#include "Clock.h"
#include "arial36.h"
#include "arial8.h"
#include "fonts.h"

#define ZERO_OFFSET 4
#define XSIZE 128
#define YSIZE 64
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


static const uint8_t Init_Array[]=
{
		CMD_Reset,
		CMD_SetScrollLine, /* 0 */
		CMD_SetSEGDirection | 0x1,
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
	SPI_lock();
	Gpio_Set_Bit(GPIO_LCD_RESET);
	vTaskDelay(10);
	Gpio_Clear_Bit(GPIO_LCD_RS);
	Gpio_Clear_Bit(GPIO_LCD_CS);
	for (uint8_t i = 0; i < sizeof(Init_Array) / sizeof(Init_Array[0]); i++)
	{
		SPI_Send_One(Init_Array[i]);
	}
	Gpio_Set_Bit(GPIO_LCD_CS);
	SPI_unlock();
}


void uc1701x_set_coordinates(const uint8_t Column,const uint8_t Page)
{
	SPI_lock();
	Gpio_Clear_Bit(GPIO_LCD_RS);
	Gpio_Clear_Bit(GPIO_LCD_CS);
	SPI_Send_One(CMD_SetColumn_l | (Column & 0xF));
	SPI_Send_One(CMD_SetColumn_h | ((Column >> 4 ) & 0xF));
	SPI_Send_One(CMD_SetPageAddr | (Page & 0xF));
	Gpio_Set_Bit(GPIO_LCD_CS);
	SPI_unlock();
}

void uc1701x_set_contrast(const uint8_t Contrast)
{
	SPI_lock();
	Gpio_Clear_Bit(GPIO_LCD_RS);
	Gpio_Clear_Bit(GPIO_LCD_CS);
	SPI_Send_One(CMD_SetElVolume);
	SPI_Send_One(Contrast <=  63 ? Contrast : 0x10); /* Default if incorrect */
	Gpio_Set_Bit(GPIO_LCD_CS);
	SPI_unlock();
}


void uc1701x_pixel(const uint8_t x, const uint8_t y, const uint8_t color)
{
	if ((x < XSIZE) && (y < YSIZE))
	{
		Protected_FB_t * FB = ScreenUpdate_Task_get_FB();
		xSemaphoreTake(FB->semaphore,portMAX_DELAY);
		FB->isChanged = true;
		const uint8_t yrow = y >> 3;
		const uint8_t ymask = 1 << (y & 7);
		const uint16_t addr = yrow * 132 + x + ZERO_OFFSET;
		if (color == 0)
		{
			FB->Buf[addr] &= ~ymask;
		}
		else
		{
			FB->Buf[addr] |= ymask;
		}
		xSemaphoreGive(FB->semaphore);
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
