#include "PWM.h"

#include "tim.h"

typedef struct
{
    TIM_HandleTypeDef *timer;
    uint32_t channel;
} PWM_Config_t;

static const PWM_Config_t PWM_CHS[PWM_NUM] = {
    [PWM_CH_A] = {&htim1, TIM_CHANNEL_1},
    [PWM_CH_B] = {&htim1, TIM_CHANNEL_2},
};

void PWM_Init(void)
{
    PWM_SetDuty(PWM_CH_A, 0U);
    PWM_SetDuty(PWM_CH_B, 0U);

    if (HAL_TIM_PWM_Start(PWM_CHS[PWM_CH_A].timer, PWM_CHS[PWM_CH_A].channel) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_TIM_PWM_Start(PWM_CHS[PWM_CH_B].timer, PWM_CHS[PWM_CH_B].channel) != HAL_OK)
    {
        Error_Handler();
    }
}

void PWM_SetDuty(PWM_CH_ID channel, uint8_t duty)
{
    TIM_HandleTypeDef *timer;
    uint32_t period_count;
    uint32_t pulse;

    if ((uint32_t)channel >= (uint32_t)PWM_NUM)
    {
        return;
    }

    if (duty > 100U)
    {
        duty = 100U;
    }

    timer = PWM_CHS[channel].timer;
    period_count = __HAL_TIM_GET_AUTORELOAD(timer) + 1U;
    pulse = ((uint32_t)duty * period_count) / 100U;

    __HAL_TIM_SET_COMPARE(timer, PWM_CHS[channel].channel, pulse);
}
