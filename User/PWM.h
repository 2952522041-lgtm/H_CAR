#pragma once

#include <stdint.h>

typedef enum
{
    PWM_CH_A = 0,
    PWM_CH_B,
    PWM_NUM
} PWM_CH_ID;

void PWM_Init(void);
void PWM_SetDuty(PWM_CH_ID channel, uint8_t duty);
