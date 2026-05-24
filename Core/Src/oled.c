/*
 * oled.c
 *
 *  STM32L432KC + 0.96寸 IIC OLED 驱动
 *  写法参考课程工程 I2C_OLED：先写显存，再统一刷新到屏幕。
 */
#include "oled.h"
#include <stdlib.h>

/* IIC控制字节：0x00表示后面是命令，0x40表示后面是显示数据 */
#define OLED_CMD_CONTROL 0x00
#define OLED_DATA_CONTROL 0x40

/* OLED显存：8页，每页128列；每1字节对应竖向8个像素 */
static uint8_t OLED_GRAM[OLED_PAGE][OLED_WIDTH];

/* ========================== 底层IIC通信函数 ========================== */

/**
 * @brief 向OLED发送一段数据
 * @param data 数据首地址
 * @param len 数据长度
 * @note 当前工程使用STM32L432KC的I2C1，SCL=D1(PA9)，SDA=D0(PA10)。
 */
static void OLED_Send(uint8_t *data, uint16_t len)
{
  HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, data, len, HAL_MAX_DELAY);
}

/**
 * @brief 向OLED发送一条命令
 * @param cmd OLED命令字节
 */
static void OLED_SendCmd(uint8_t cmd)
{
  uint8_t sendBuffer[2] = {OLED_CMD_CONTROL, cmd};
  OLED_Send(sendBuffer, 2);
}

/* ========================== OLED驱动函数 ========================== */

/**
 * @brief 初始化OLED
 * @note 适配中景园0.96寸IIC OLED，驱动芯片按SSD1306兼容初始化。
 */
void OLED_Init(void)
{
  HAL_Delay(100);              /* 等待OLED上电稳定 */
  OLED_SendCmd(0xAE);          /* 关闭显示 */
  OLED_SendCmd(0x20);          /* 设置内存寻址模式 */
  OLED_SendCmd(0x02);          /* 页寻址模式 */
  OLED_SendCmd(0xB0);          /* 设置页地址起始为第0页 */
  OLED_SendCmd(0xC8);          /* COM扫描方向：从COM[N-1]到COM[0] */
  OLED_SendCmd(0x00);          /* 列地址低4位 */
  OLED_SendCmd(0x10);          /* 列地址高4位 */
  OLED_SendCmd(0x40);          /* 显示起始行为0 */
  OLED_SendCmd(0x81);          /* 设置对比度 */
  OLED_SendCmd(0xDF);          /* 对比度数值 */
  OLED_SendCmd(0xA1);          /* 段重映射：左右方向 */
  OLED_SendCmd(0xA6);          /* 正常显示，不反色 */
  OLED_SendCmd(0xA8);          /* 设置多路复用率 */
  OLED_SendCmd(0x3F);          /* 1/64 duty，对应64行 */
  OLED_SendCmd(0xA4);          /* 显示内容来自显存 */
  OLED_SendCmd(0xD3);          /* 设置显示偏移 */
  OLED_SendCmd(0x00);          /* 无偏移 */
  OLED_SendCmd(0xD5);          /* 设置显示时钟分频/振荡频率 */
  OLED_SendCmd(0xF0);
  OLED_SendCmd(0xD9);          /* 设置预充电周期 */
  OLED_SendCmd(0x22);
  OLED_SendCmd(0xDA);          /* 设置COM引脚硬件配置 */
  OLED_SendCmd(0x12);
  OLED_SendCmd(0xDB);          /* 设置VCOMH电平 */
  OLED_SendCmd(0x20);
  OLED_SendCmd(0x8D);          /* 设置电荷泵 */
  OLED_SendCmd(0x14);          /* 开启电荷泵 */

  OLED_NewFrame();             /* 清空显存 */
  OLED_ShowFrame();            /* 把空显存刷到屏幕，避免随机亮点 */
  OLED_SendCmd(0xAF);          /* 开启显示 */
}

/**
 * @brief 开启OLED显示
 */
void OLED_DisPlay_On(void)
{
  OLED_SendCmd(0x8D);
  OLED_SendCmd(0x14);
  OLED_SendCmd(0xAF);
}

/**
 * @brief 关闭OLED显示
 */
void OLED_DisPlay_Off(void)
{
  OLED_SendCmd(0x8D);
  OLED_SendCmd(0x10);
  OLED_SendCmd(0xAE);
}

/**
 * @brief 设置屏幕颜色模式
 * @param mode OLED_COLOR_NORMAL正常显示，OLED_COLOR_REVERSED反色显示
 */
void OLED_SetColorMode(OLED_ColorMode mode)
{
  if (mode == OLED_COLOR_NORMAL)
  {
    OLED_SendCmd(0xA6);
  }
  else
  {
    OLED_SendCmd(0xA7);
  }
}

/* ========================== 显存操作函数 ========================== */

/**
 * @brief 清空显存，准备绘制新的一帧
 */
void OLED_NewFrame(void)
{
  memset(OLED_GRAM, 0, sizeof(OLED_GRAM));
}

/**
 * @brief 将显存内容刷新到OLED
 * @note 课程代码也是这个思路：绘制函数只改显存，最后统一调用本函数显示。
 */
void OLED_ShowFrame(void)
{
  uint8_t sendBuffer[OLED_WIDTH + 1];
  sendBuffer[0] = OLED_DATA_CONTROL;

  for (uint8_t page = 0; page < OLED_PAGE; page++)
  {
    OLED_SendCmd(0xB0 + page);       /* 设置页地址 */
    OLED_SendCmd(0x00);              /* SSD1306列地址低4位从0开始 */
    OLED_SendCmd(0x10);              /* SSD1306列地址高4位从0开始 */
    memcpy(sendBuffer + 1, OLED_GRAM[page], OLED_WIDTH);
    OLED_Send(sendBuffer, OLED_WIDTH + 1);
  }
}

/**
 * @brief 设置一个像素点
 * @param x 横坐标，范围0~127
 * @param y 纵坐标，范围0~63
 * @param color 颜色模式
 */
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color)
{
  if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
  {
    return;
  }

  if (color == OLED_COLOR_NORMAL)
  {
    OLED_GRAM[y / 8][x] |= (uint8_t)(1 << (y % 8));
  }
  else
  {
    OLED_GRAM[y / 8][x] &= (uint8_t)~(1 << (y % 8));
  }
}

/**
 * @brief 设置显存中的一个真实字节
 * @param page 页地址，范围0~7
 * @param column 列地址，范围0~127
 * @param data 要写入的字节
 * @param color 颜色模式
 */
static void OLED_SetByte(uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color)
{
  if (page >= OLED_PAGE || column >= OLED_WIDTH)
  {
    return;
  }

  if (color == OLED_COLOR_REVERSED)
  {
    data = (uint8_t)~data;
  }

  OLED_GRAM[page][column] = data;
}

/**
 * @brief 设置一块显存区域
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param data 图像或字模数据
 * @param w 宽度
 * @param h 高度
 * @param color 颜色模式
 * @note 数据按“列行式”排列，和课程代码/常见OLED取模方式一致。
 */
static void OLED_SetBlock(uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color)
{
  uint8_t pages = (h + 7) / 8;

  for (uint8_t page = 0; page < pages; page++)
  {
    for (uint8_t column = 0; column < w; column++)
    {
      uint8_t targetPage = (y / 8) + page;
      uint8_t targetColumn = x + column;
      if (targetPage < OLED_PAGE && targetColumn < OLED_WIDTH)
      {
        OLED_SetByte(targetPage, targetColumn, data[page * w + column], color);
      }
    }
  }
}

/* ========================== 图形绘制函数 ========================== */

/**
 * @brief 绘制一条线段
 */
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color)
{
  int16_t dx = abs((int16_t)x2 - (int16_t)x1);
  int16_t sx = x1 < x2 ? 1 : -1;
  int16_t dy = -abs((int16_t)y2 - (int16_t)y1);
  int16_t sy = y1 < y2 ? 1 : -1;
  int16_t err = dx + dy;

  while (1)
  {
    OLED_SetPixel(x1, y1, color);
    if (x1 == x2 && y1 == y2)
    {
      break;
    }

    int16_t e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x1 = (uint8_t)((int16_t)x1 + sx);
    }
    if (e2 <= dx)
    {
      err += dx;
      y1 = (uint8_t)((int16_t)y1 + sy);
    }
  }
}

/**
 * @brief 绘制空心矩形
 */
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color)
{
  OLED_DrawLine(x, y, x + w, y, color);
  OLED_DrawLine(x, y + h, x + w, y + h, color);
  OLED_DrawLine(x, y, x, y + h, color);
  OLED_DrawLine(x + w, y, x + w, y + h, color);
}

/**
 * @brief 绘制实心矩形
 */
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color)
{
  for (uint8_t i = 0; i <= h; i++)
  {
    OLED_DrawLine(x, y + i, x + w, y + i, color);
  }
}

/**
 * @brief 绘制空心三角形
 */
void OLED_DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color)
{
  OLED_DrawLine(x1, y1, x2, y2, color);
  OLED_DrawLine(x2, y2, x3, y3, color);
  OLED_DrawLine(x3, y3, x1, y1, color);
}

/**
 * @brief 绘制实心三角形
 * @note 采用扫描线填充，先按y坐标排序，再逐行画水平线。
 */
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color)
{
  int16_t tx;
  int16_t ty;
  int16_t xa;
  int16_t xb;

  if (y1 > y2)
  {
    ty = y1; y1 = y2; y2 = (uint8_t)ty;
    tx = x1; x1 = x2; x2 = (uint8_t)tx;
  }
  if (y1 > y3)
  {
    ty = y1; y1 = y3; y3 = (uint8_t)ty;
    tx = x1; x1 = x3; x3 = (uint8_t)tx;
  }
  if (y2 > y3)
  {
    ty = y2; y2 = y3; y3 = (uint8_t)ty;
    tx = x2; x2 = x3; x3 = (uint8_t)tx;
  }

  for (int16_t y = y1; y <= y3; y++)
  {
    if (y3 == y1)
    {
      xa = x1;
    }
    else
    {
      xa = x1 + ((int16_t)(x3 - x1) * (y - y1)) / (y3 - y1);
    }

    if (y < y2)
    {
      xb = (y2 == y1) ? x2 : x1 + ((int16_t)(x2 - x1) * (y - y1)) / (y2 - y1);
    }
    else
    {
      xb = (y3 == y2) ? x3 : x2 + ((int16_t)(x3 - x2) * (y - y2)) / (y3 - y2);
    }

    if (xa > xb)
    {
      tx = xa;
      xa = xb;
      xb = tx;
    }
    OLED_DrawLine((uint8_t)xa, (uint8_t)y, (uint8_t)xb, (uint8_t)y, color);
  }
}

/**
 * @brief 绘制空心圆
 */
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color)
{
  int16_t a = 0;
  int16_t b = r;
  int16_t di = 3 - (r << 1);

  while (a <= b)
  {
    OLED_SetPixel(x - b, y - a, color);
    OLED_SetPixel(x + b, y - a, color);
    OLED_SetPixel(x - a, y - b, color);
    OLED_SetPixel(x + a, y - b, color);
    OLED_SetPixel(x - b, y + a, color);
    OLED_SetPixel(x + b, y + a, color);
    OLED_SetPixel(x - a, y + b, color);
    OLED_SetPixel(x + a, y + b, color);

    a++;
    if (di < 0)
    {
      di += 4 * a + 6;
    }
    else
    {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}

/**
 * @brief 绘制实心圆
 */
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color)
{
  int16_t a = 0;
  int16_t b = r;
  int16_t di = 3 - (r << 1);

  while (a <= b)
  {
    OLED_DrawLine(x - b, y - a, x + b, y - a, color);
    OLED_DrawLine(x - b, y + a, x + b, y + a, color);
    OLED_DrawLine(x - a, y - b, x + a, y - b, color);
    OLED_DrawLine(x - a, y + b, x + a, y + b, color);

    a++;
    if (di < 0)
    {
      di += 4 * a + 6;
    }
    else
    {
      di += 10 + 4 * (a - b);
      b--;
    }
  }
}

/**
 * @brief 绘制空心椭圆
 */
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color)
{
  int32_t xpos = 0;
  int32_t ypos = b;
  int32_t a2 = (int32_t)a * a;
  int32_t b2 = (int32_t)b * b;
  int32_t d = b2 - a2 * b + a2 / 4;

  while (a2 * ypos > b2 * xpos)
  {
    OLED_SetPixel(x + xpos, y + ypos, color);
    OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color);
    OLED_SetPixel(x - xpos, y - ypos, color);

    if (d < 0)
    {
      d += b2 * (2 * xpos + 3);
    }
    else
    {
      d += b2 * (2 * xpos + 3) + a2 * (-2 * ypos + 2);
      ypos--;
    }
    xpos++;
  }

  d = b2 * (xpos * xpos + xpos) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
  while (ypos >= 0)
  {
    OLED_SetPixel(x + xpos, y + ypos, color);
    OLED_SetPixel(x - xpos, y + ypos, color);
    OLED_SetPixel(x + xpos, y - ypos, color);
    OLED_SetPixel(x - xpos, y - ypos, color);

    if (d < 0)
    {
      d += b2 * (2 * xpos + 2) + a2 * (-2 * ypos + 3);
      xpos++;
    }
    else
    {
      d += a2 * (-2 * ypos + 3);
    }
    ypos--;
  }
}

/**
 * @brief 绘制图片
 */
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color)
{
  if (img == NULL)
  {
    return;
  }

  OLED_SetBlock(x, y, img->data, img->w, img->h, color);
}

/* ========================== 文字绘制函数 ========================== */

/**
 * @brief 显示一个ASCII字符
 */
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color)
{
  if (font == NULL || ch < ' ' || ch > '~')
  {
    return;
  }

  uint16_t bytesPerChar = (uint16_t)(((font->h + 7) / 8) * font->w);
  OLED_SetBlock(x, y, font->chars + (ch - ' ') * bytesPerChar, font->w, font->h, color);
}

/**
 * @brief 显示ASCII字符串
 */
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color)
{
  while (str != NULL && *str != '\0')
  {
    OLED_PrintASCIIChar(x, y, *str, font, color);
    x += font->w;
    str++;
  }
}

/**
 * @brief 获取UTF-8字符占用字节数
 */
static uint8_t OLED_GetUTF8Len(char *string)
{
  if ((string[0] & 0x80) == 0x00)
  {
    return 1;
  }
  if ((string[0] & 0xE0) == 0xC0)
  {
    return 2;
  }
  if ((string[0] & 0xF0) == 0xE0)
  {
    return 3;
  }
  if ((string[0] & 0xF8) == 0xF0)
  {
    return 4;
  }

  return 0;
}

/**
 * @brief 显示字符串，支持课程字库格式的中文/英文混合显示
 * @note 如果中文没有对应字模，则显示为空格。
 */
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color)
{
  uint16_t i = 0;
  uint16_t bytesPerChar;
  uint8_t found;
  uint8_t utf8Len;
  uint8_t *head;

  if (str == NULL || font == NULL)
  {
    return;
  }

  bytesPerChar = (uint16_t)((((font->h + 7) / 8) * font->w) + 4);

  while (str[i] != '\0')
  {
    found = 0;
    utf8Len = OLED_GetUTF8Len(str + i);
    if (utf8Len == 0)
    {
      break;
    }

    for (uint8_t j = 0; j < font->len; j++)
    {
      head = (uint8_t *)(font->chars) + (j * bytesPerChar);
      if (memcmp(str + i, head, utf8Len) == 0)
      {
        OLED_SetBlock(x, y, head + 4, font->w, font->h, color);
        x += font->w;
        i += utf8Len;
        found = 1;
        break;
      }
    }

    if (found == 0)
    {
      if (utf8Len == 1 && font->ascii != NULL)
      {
        OLED_PrintASCIIChar(x, y, str[i], font->ascii, color);
        x += font->ascii->w;
      }
      else if (font->ascii != NULL)
      {
        OLED_PrintASCIIChar(x, y, ' ', font->ascii, color);
        x += font->ascii->w;
      }
      i += utf8Len;
    }
  }
}
