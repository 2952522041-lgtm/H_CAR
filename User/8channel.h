#pragma once

#include "main.h"

#define LINEFOLLOW_SENSOR_NUM 8U
#define LINEFOLLOW_TURN_GAIN  5.0f
#define LINEFOLLOW_BASE_RPM   160.0f
#define LINEFOLLOW_MIN_RPM    0.0f
#define LINEFOLLOW_MAX_RPM    330.0f

typedef struct
{
    uint8_t value[LINEFOLLOW_SENSOR_NUM];
    uint8_t active_mask;
    uint8_t active_count;
    uint8_t has_line;
    float error;
    uint8_t valid;
} LineFollow_SensorData_t;

typedef struct
{
    float left_target_rpm;
    float right_target_rpm;
    uint8_t valid;
} LineFollow_TargetRPM_t;

void LineFollow_Init(void);

HAL_StatusTypeDef LineFollow_StartUartReceive(void);
HAL_StatusTypeDef LineFollow_EnableDigitalOutput(void);

uint8_t LineFollow_Update(void);
uint8_t LineFollow_GetSensorData(LineFollow_SensorData_t *data);
uint8_t LineFollow_GetTargetRPM(LineFollow_TargetRPM_t *target);

void LineFollow_UartRxCpltCallback(UART_HandleTypeDef *huart);
void LineFollow_UartErrorCallback(UART_HandleTypeDef *huart);
