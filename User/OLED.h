#pragma once

#include <stdint.h>

/*
 * SSD1306 128x64 OLED on I2C2 (PB10 SCL, PB11 SDA).
 * HAL uses the 7-bit slave address shifted left by one bit.
 * Change this macro to (0x3DU << 1) if the module uses address 0x3D.
 */
#ifndef OLED_I2C_ADDRESS
#define OLED_I2C_ADDRESS (0x3CU << 1)
#endif

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t line, uint8_t column, char character);
void OLED_ShowString(uint8_t line, uint8_t column, const char *string);
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length);
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);
