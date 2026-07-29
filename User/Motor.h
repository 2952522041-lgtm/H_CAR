#pragma once

#define Motor_Max_RPM         (330.0f)
#define Motor_LEFT_direction  (1.0f)
#define Motor_RIGHT_direction (-1.0f)

typedef enum
{
    MOTOR_LEFT = 0,
    MOTOR_RIGHT,
    MOTOR_NUM
} Motor_ID_t;

void Motor_Init(void);

void Motor_SetTargetRPM(Motor_ID_t motor, float rpm);
void Motor_SetBothTargetRPM(float left_rpm, float right_rpm);
float Motor_GetTargetRPM(Motor_ID_t motor);

void Motor_DriveRPM(Motor_ID_t motor, float rpm);
void Motor_DriveAllRPM(float left_rpm, float right_rpm);

void Motor_Brake(Motor_ID_t motor);
void Motor_Stop(Motor_ID_t motor);
