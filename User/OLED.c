#include "OLED.h"

#include "OLED_Font.h"
#include "i2c.h"

#define OLED_WIDTH            128U
#define OLED_PAGE_COUNT       8U
#define OLED_TEXT_LINE_COUNT  4U
#define OLED_TEXT_COLUMN_COUNT 16U
#define OLED_I2C_TIMEOUT_MS   100U

#define OLED_CONTROL_COMMAND  0x00U
#define OLED_CONTROL_DATA     0x40U

static void OLED_WriteCommandBuffer(const uint8_t *commands, uint8_t count)
{
    uint8_t packet[32];
    uint8_t i;

    if ((commands == NULL) || (count == 0U) || (count > (sizeof(packet) - 1U)))
    {
        return;
    }

    packet[0] = OLED_CONTROL_COMMAND;
    for (i = 0U; i < count; i++)
    {
        packet[i + 1U] = commands[i];
    }

    (void)HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, packet, (uint16_t)count + 1U, OLED_I2C_TIMEOUT_MS);
}

static void OLED_WriteDataBuffer(const uint8_t *data, uint8_t count)
{
    uint8_t packet[OLED_WIDTH + 1U];
    uint8_t i;

    if ((data == NULL) || (count == 0U) || (count > OLED_WIDTH))
    {
        return;
    }

    packet[0] = OLED_CONTROL_DATA;
    for (i = 0U; i < count; i++)
    {
        packet[i + 1U] = data[i];
    }

    (void)HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, packet, (uint16_t)count + 1U, OLED_I2C_TIMEOUT_MS);
}

static void OLED_SetCursor(uint8_t page, uint8_t x)
{
    uint8_t commands[3];

    if ((page >= OLED_PAGE_COUNT) || (x >= OLED_WIDTH))
    {
        return;
    }

    commands[0] = (uint8_t)(0xB0U | page);
    commands[1] = (uint8_t)(0x10U | ((x & 0xF0U) >> 4U));
    commands[2] = (uint8_t)(x & 0x0FU);
    OLED_WriteCommandBuffer(commands, sizeof(commands));
}

static uint32_t OLED_Pow(uint32_t base, uint8_t exponent)
{
    uint32_t result = 1U;

    while (exponent > 0U)
    {
        result *= base;
        exponent--;
    }

    return result;
}

void OLED_Init(void)
{
    static const uint8_t init_commands[] = {
        0xAEU,       /* Display off. */
        0xD5U, 0x80U, /* Display clock divide ratio and oscillator frequency. */
        0xA8U, 0x3FU, /* Multiplex ratio: 1/64 duty. */
        0xD3U, 0x00U, /* Display offset. */
        0x40U,       /* Display start line. */
        0xA1U,       /* Segment remap. */
        0xC8U,       /* COM output scan direction. */
        0xDAU, 0x12U, /* COM pins hardware configuration. */
        0x81U, 0xCFU, /* Contrast. */
        0xD9U, 0xF1U, /* Pre-charge period. */
        0xDBU, 0x30U, /* VCOMH deselect level. */
        0xA4U,       /* Resume RAM content display. */
        0xA6U,       /* Normal display. */
        0x8DU, 0x14U, /* Enable charge pump. */
        0xAFU        /* Display on. */
    };

    /*
     * MX_I2C1_Init() must be called before this function. A short delay gives
     * the OLED controller time to finish its power-on reset.
     */
    HAL_Delay(100U);
    OLED_WriteCommandBuffer(init_commands, sizeof(init_commands));
    OLED_Clear();
}

void OLED_Clear(void)
{
    uint8_t blank_line[OLED_WIDTH] = {0};
    uint8_t page;

    for (page = 0U; page < OLED_PAGE_COUNT; page++)
    {
        OLED_SetCursor(page, 0U);
        OLED_WriteDataBuffer(blank_line, sizeof(blank_line));
    }
}

void OLED_ShowChar(uint8_t line, uint8_t column, char character)
{
    uint8_t font_index;
    uint8_t x;

    if ((line < 1U) || (line > OLED_TEXT_LINE_COUNT) ||
        (column < 1U) || (column > OLED_TEXT_COLUMN_COUNT))
    {
        return;
    }

    if ((character < ' ') || (character > '~'))
    {
        character = '?';
    }

    font_index = (uint8_t)(character - ' ');
    x = (uint8_t)((column - 1U) * 8U);

    OLED_SetCursor((uint8_t)((line - 1U) * 2U), x);
    OLED_WriteDataBuffer(&OLED_F8x16[(uint16_t)font_index * 16U], 8U);
    OLED_SetCursor((uint8_t)(((line - 1U) * 2U) + 1U), x);
    OLED_WriteDataBuffer(&OLED_F8x16[((uint16_t)font_index * 16U) + 8U], 8U);
}

void OLED_ShowString(uint8_t line, uint8_t column, const char *string)
{
    if (string == NULL)
    {
        return;
    }

    while ((*string != '\0') && (column <= OLED_TEXT_COLUMN_COUNT))
    {
        OLED_ShowChar(line, column, *string);
        string++;
        column++;
    }
}

void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    uint8_t i;

    if ((length == 0U) || (length > 10U))
    {
        return;
    }

    for (i = 0U; i < length; i++)
    {
        OLED_ShowChar(line, (uint8_t)(column + i),
                      (char)((number / OLED_Pow(10U, (uint8_t)(length - i - 1U))) % 10U) + '0');
    }
}

void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length)
{
    uint32_t magnitude;

    if ((length == 0U) || (length > 10U))
    {
        return;
    }

    if (number >= 0)
    {
        OLED_ShowChar(line, column, '+');
        magnitude = (uint32_t)number;
    }
    else
    {
        OLED_ShowChar(line, column, '-');
        magnitude = (uint32_t)(-(int64_t)number);
    }

    OLED_ShowNum(line, (uint8_t)(column + 1U), magnitude, length);
}

void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    uint8_t i;
    uint8_t digit;

    if ((length == 0U) || (length > 8U))
    {
        return;
    }

    for (i = 0U; i < length; i++)
    {
        digit = (uint8_t)((number / OLED_Pow(16U, (uint8_t)(length - i - 1U))) % 16U);
        OLED_ShowChar(line, (uint8_t)(column + i), (char)(digit < 10U ? digit + '0' : digit - 10U + 'A'));
    }
}

void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    uint8_t i;

    if ((length == 0U) || (length > 32U))
    {
        return;
    }

    for (i = 0U; i < length; i++)
    {
        OLED_ShowChar(line, (uint8_t)(column + i),
                      (char)(((number >> (length - i - 1U)) & 0x01U) + '0'));
    }
}
