#pragma once

typedef struct
{
    /* PID 参数 */
    float kp;
    float ki;
    float kd;

    /* 积分项及其限幅 */
    float integral;
    float integral_min;
    float integral_max;

    /* 上一次误差 */
    float prev_error;

    /* 输出限幅 */
    float output_min;
    float output_max;

} PID_Config_t;

/**
 * @brief 初始化 PID
 *
 * @param pid           PID 对象
 * @param kp            比例系数
 * @param ki            积分系数
 * @param kd            微分系数
 * @param integral_min  积分项下限
 * @param integral_max  积分项上限
 * @param output_min    输出下限
 * @param output_max    输出上限
 */
void PID_Init(PID_Config_t *pid,
              float kp,
              float ki,
              float kd,
              float integral_min,
              float integral_max,
              float output_min,
              float output_max);

/**
 * @brief 清除 PID 内部状态
 */
void PID_Reset(PID_Config_t *pid);

/**
 * @brief 执行一次 PID 计算
 *
 * @param pid    PID 对象
 * @param error  当前误差，通常为 target - measurement
 * @param dt     控制周期，单位为秒
 *
 * @return PID 输出
 */
float PID_Update(PID_Config_t *pid,
                 float error,
                 float dt);