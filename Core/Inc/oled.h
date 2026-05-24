/*
 * oled.h
 *
 *  Created on: May 22, 2026
 *      Author: ZhuanZ（无密码）
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "i2c.h"
#include "main.h"
#include "string.h"

/* OLED基本参数：0.96寸屏，128列 x 64行 */
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGE 8
#define OLED_I2C_ADDR 0x78

/* 图片取模位序开关：1表示翻转每个字节的bit顺序，用来适配“高位在前”的图片取模数据 */
#define OLED_IMAGE_REVERSE_BITS 1

typedef enum {
  OLED_COLOR_NORMAL = 0, /* 正常模式：黑底白字 */
  OLED_COLOR_REVERSED    /* 反色模式：白底黑字 */
} OLED_ColorMode;

typedef struct {
  uint8_t h;
  uint8_t w;
  uint8_t *chars;
} ASCIIFont;

typedef struct {
  uint8_t h;
  uint8_t w;
  const uint8_t *chars;
  uint16_t len;
  const ASCIIFont *ascii;
} Font;

typedef struct {
  uint8_t w;
  uint8_t h;
  const uint8_t *data;
} Image;

void OLED_Init(void);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);
void OLED_SetColorMode(OLED_ColorMode mode);

void OLED_NewFrame(void);
void OLED_ShowFrame(void);
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color);

void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color);
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color);
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color);

void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color);
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color);
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color);

#endif /* INC_OLED_H_ */
