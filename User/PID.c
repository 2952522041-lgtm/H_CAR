#include "PID.h"

/**
 * @brief 数值限幅
 */
static float PID_Clamp(float value,
                       float min_value,
                       float max_value)
{
    if (value > max_value)
    {
        return max_value;
    }

    if (value < min_value)
    {
        return min_value;
    }

    return value;
}

void PID_Init(PID_Config_t *pid,
              float kp,
              float ki,
              float kd,
              float integral_min,
              float integral_max,
              float output_min,
              float output_max)
{
    if (pid == 0)
    {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->integral_min = integral_min;
    pid->integral_max = integral_max;

    pid->output_min = output_min;
    pid->output_max = output_max;

    PID_Reset(pid);
}

void PID_Reset(PID_Config_t *pid)
{
    if (pid == 0)
    {
        return;
    }

    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

float PID_Update(PID_Config_t *pid,
                 float error,
                 float dt)
{
    if ((pid == 0) || (dt <= 0.0f))
    {
        return 0.0f;
    }

    /*
     * 比例项：
     * P = Kp × error
     */
    float proportional = pid->kp * error;

    /*
     * 积分项：
     * I(k) = I(k-1) + Ki × error × dt
     */
    pid->integral += pid->ki * error * dt;

    /*
     * 积分限幅
     */
    pid->integral = PID_Clamp(
        pid->integral,
        pid->integral_min,
        pid->integral_max
    );

    /*
     * 微分项：
     * D = Kd × (error - prev_error) / dt
     */
    float derivative =
        pid->kd * (error - pid->prev_error) / dt;

    /*
     * PID 总输出
     */
    float output =
        proportional +
        pid->integral +
        derivative;

    /*
     * 输出限幅
     */
    output = PID_Clamp(
        output,
        pid->output_min,
        pid->output_max
    );

    /*
     * 保存本次误差
     */
    pid->prev_error = error;

    return output;
}
