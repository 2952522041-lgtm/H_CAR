#include "tb6612.h"

#include "PWM.h"
#include "main.h"

typedef struct
{
    GPIO_TypeDef *IN1_GPIO_Port;
    uint16_t IN1_Pin;
    GPIO_TypeDef *IN2_GPIO_Port;
    uint16_t IN2_Pin;
    PWM_CH_ID pwm_channel;
} tb6612_Config_t;

static const tb6612_Config_t tb6612s[tb6612_CH_NUM] = {
    [tb6612_CH_LEFT] = {
        AIN1_GPIO_Port,
        AIN1_Pin,
        AIN2_GPIO_Port,
        AIN2_Pin,
        PWM_CH_A,
    },
    [tb6612_CH_RIGHT] = {
        BIN1_GPIO_Port,
        BIN1_Pin,
        BIN2_GPIO_Port,
        BIN2_Pin,
        PWM_CH_B,
    },
};

void tb6612_Init(void)
{
    PWM_Init();
    tb6612_Stop(tb6612_CH_LEFT);
    tb6612_Stop(tb6612_CH_RIGHT);
}

void tb6612_SetDuty(tb6612_Channel_t tb6612_Channel, uint32_t duty)
{
    if ((uint32_t)tb6612_Channel >= (uint32_t)tb6612_CH_NUM)
    {
        return;
    }

    if (duty > tb6612_PWM_MAX_DUTY)
    {
        duty = tb6612_PWM_MAX_DUTY;
    }

    PWM_SetDuty(tb6612s[tb6612_Channel].pwm_channel, (uint8_t)duty);
}

void tb6612_SetDirection(tb6612_Channel_t tb6612_Channel, tb6612_Direction_t direction)
{
    const tb6612_Config_t *config;

    if ((uint32_t)tb6612_Channel >= (uint32_t)tb6612_CH_NUM)
    {
        return;
    }

    config = &tb6612s[tb6612_Channel];

    switch (direction)
    {
        case tb6612_DIR_FORWARD:
            HAL_GPIO_WritePin(config->IN1_GPIO_Port, config->IN1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(config->IN2_GPIO_Port, config->IN2_Pin, GPIO_PIN_SET);
            break;

        case tb6612_DIR_BACKWARD:
            HAL_GPIO_WritePin(config->IN1_GPIO_Port, config->IN1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(config->IN2_GPIO_Port, config->IN2_Pin, GPIO_PIN_RESET);
            break;

        case tb6612_DIR_BRAKE:
            HAL_GPIO_WritePin(config->IN1_GPIO_Port, config->IN1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(config->IN2_GPIO_Port, config->IN2_Pin, GPIO_PIN_SET);
            break;

        case tb6612_DIR_STOP:
        default:
            HAL_GPIO_WritePin(config->IN1_GPIO_Port, config->IN1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(config->IN2_GPIO_Port, config->IN2_Pin, GPIO_PIN_RESET);
            break;
    }
}

void tb6612_Stop(tb6612_Channel_t tb6612_Channel)
{
    tb6612_SetDuty(tb6612_Channel, 0U);
    tb6612_SetDirection(tb6612_Channel, tb6612_DIR_STOP);
}

void tb6612_Brake(tb6612_Channel_t tb6612_Channel)
{
    tb6612_SetDuty(tb6612_Channel, 0U);
    tb6612_SetDirection(tb6612_Channel, tb6612_DIR_BRAKE);
}
