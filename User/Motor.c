#include "Motor.h"

#include "tb6612.h"

static volatile float target_rpms[MOTOR_NUM] = {
    [MOTOR_LEFT] = 0.0f,
    [MOTOR_RIGHT] = 0.0f,
};

typedef struct
{
    tb6612_Channel_t tb6612_CH;
    float direction;
} Motor_Config_t;

static const Motor_Config_t motor_map[MOTOR_NUM] = {
    [MOTOR_RIGHT] = {tb6612_CH_LEFT, Motor_LEFT_direction},
    [MOTOR_LEFT] = {tb6612_CH_RIGHT, Motor_RIGHT_direction},
};

static float Motor_LimitRPM(float rpm)
{
    if (rpm > Motor_Max_RPM)
    {
        return Motor_Max_RPM;
    }

    if (rpm < -Motor_Max_RPM)
    {
        return -Motor_Max_RPM;
    }

    return rpm;
}

static float Motor_LimitDuty(float duty)
{
    if (duty > Motor_Max_Duty)
    {
        return Motor_Max_Duty;
    }

    if (duty < -Motor_Max_Duty)
    {
        return -Motor_Max_Duty;
    }

    return duty;
}

void Motor_Init(void)
{
    tb6612_Init();
    Motor_Stop(MOTOR_LEFT);
    Motor_Stop(MOTOR_RIGHT);
}

void Motor_SetTargetRPM(Motor_ID_t motor, float rpm)
{
    if ((uint32_t)motor >= (uint32_t)MOTOR_NUM)
    {
        return;
    }

    target_rpms[motor] = Motor_LimitRPM(rpm);
}

void Motor_SetBothTargetRPM(float left_rpm, float right_rpm)
{
    Motor_SetTargetRPM(MOTOR_LEFT, left_rpm);
    Motor_SetTargetRPM(MOTOR_RIGHT, right_rpm);
}

float Motor_GetTargetRPM(Motor_ID_t motor)
{
    if ((uint32_t)motor >= (uint32_t)MOTOR_NUM)
    {
        return 0.0f;
    }

    return target_rpms[motor];
}

void Motor_DriveRPM(Motor_ID_t motor, float rpm)
{
    Motor_DriveDuty(motor, Motor_LimitRPM(rpm) / Motor_Max_RPM * Motor_Max_Duty);
}

void Motor_DriveAllRPM(float left_rpm, float right_rpm)
{
    Motor_DriveRPM(MOTOR_LEFT, left_rpm);
    Motor_DriveRPM(MOTOR_RIGHT, right_rpm);
}

void Motor_DriveDuty(Motor_ID_t motor, float duty)
{
    tb6612_Channel_t tb6612_channel;
    float motor_duty;
    uint32_t duty_value;

    if ((uint32_t)motor >= (uint32_t)MOTOR_NUM)
    {
        return;
    }

    tb6612_channel = motor_map[motor].tb6612_CH;
    motor_duty = Motor_LimitDuty(duty) * motor_map[motor].direction;

    if (motor_duty == 0.0f)
    {
        Motor_Stop(motor);
        return;
    }

    if (motor_duty > 0.0f)
    {
        tb6612_SetDirection(tb6612_channel, tb6612_DIR_FORWARD);
    }
    else
    {
        tb6612_SetDirection(tb6612_channel, tb6612_DIR_BACKWARD);
        motor_duty = -motor_duty;
    }

    duty_value = (uint32_t)(motor_duty + 0.5f);
    tb6612_SetDuty(tb6612_channel, duty_value);
}

void Motor_DriveAllDuty(float left_duty, float right_duty)
{
    Motor_DriveDuty(MOTOR_LEFT, left_duty);
    Motor_DriveDuty(MOTOR_RIGHT, right_duty);
}

void Motor_Stop(Motor_ID_t motor)
{
    if ((uint32_t)motor < (uint32_t)MOTOR_NUM)
    {
        tb6612_Stop(motor_map[motor].tb6612_CH);
    }
}

void Motor_Brake(Motor_ID_t motor)
{
    if ((uint32_t)motor < (uint32_t)MOTOR_NUM)
    {
        tb6612_Brake(motor_map[motor].tb6612_CH);
    }
}
