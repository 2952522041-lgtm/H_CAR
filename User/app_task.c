#include "app_task.h"

#include "8channel.h"
#include "Encoder.h"
#include "Motor.h"
#include "PID.h"
#include "cmsis_os2.h"
#include "tim.h"

#define SPEED_CONTROL_PERIOD_SEC 0.01f

#define LEFT_SPEED_PID_KP 0.30f
#define LEFT_SPEED_PID_KI 0.00f
#define LEFT_SPEED_PID_KD 0.00f

#define RIGHT_SPEED_PID_KP 0.30f
#define RIGHT_SPEED_PID_KI 0.00f
#define RIGHT_SPEED_PID_KD 0.00f

#define SPEED_CONTROL_TASK_STACK_SIZE 1024U
#define LINE_FOLLOW_TASK_STACK_SIZE   1024U

#define LINE_FOLLOW_START_DELAY_MS 1000U
#define LINE_FOLLOW_TASK_PERIOD_MS 10U

static void SpeedControlTask(void *argument);
static void LineFollowTask(void *argument);
static void SpeedControl_UpdateMotor(Motor_ID_t motor,
                                     Encoder_ID_t encoder);

static osSemaphoreId_t speed_tick_semaphore;
static osThreadId_t speed_control_task_handle;
static osThreadId_t line_follow_task_handle;

static PID_Config_t speed_pid[MOTOR_NUM];

static const osThreadAttr_t speed_control_task_attributes = {
    .name = "SpeedControl",
    .stack_size = SPEED_CONTROL_TASK_STACK_SIZE,
    .priority = osPriorityHigh,
};

static const osThreadAttr_t line_follow_task_attributes = {
    .name = "LineFollow",
    .stack_size = LINE_FOLLOW_TASK_STACK_SIZE,
    .priority = osPriorityAboveNormal,
};

void User_Init(void)
{
    Motor_Init();
    Encoder_Init();
    LineFollow_Init();

    if (LineFollow_StartUartReceive() != HAL_OK)
    {
        Error_Handler();
    }
}

void APP_FREERTOS_Init(void)
{
    speed_tick_semaphore = osSemaphoreNew(1U, 0U, NULL);
    if (speed_tick_semaphore == NULL)
    {
        Error_Handler();
    }

    PID_Init(&speed_pid[MOTOR_LEFT],
             LEFT_SPEED_PID_KP,
             LEFT_SPEED_PID_KI,
             LEFT_SPEED_PID_KD,
             -Motor_Max_Duty,
             Motor_Max_Duty,
             -Motor_Max_Duty,
             Motor_Max_Duty);

    PID_Init(&speed_pid[MOTOR_RIGHT],
             RIGHT_SPEED_PID_KP,
             RIGHT_SPEED_PID_KI,
             RIGHT_SPEED_PID_KD,
             -Motor_Max_Duty,
             Motor_Max_Duty,
             -Motor_Max_Duty,
             Motor_Max_Duty);

    speed_control_task_handle =
        osThreadNew(SpeedControlTask,
                    NULL,
                    &speed_control_task_attributes);

    line_follow_task_handle =
        osThreadNew(LineFollowTask,
                    NULL,
                    &line_follow_task_attributes);

    if ((speed_control_task_handle == NULL) ||
        (line_follow_task_handle == NULL))
    {
        Error_Handler();
    }
}

void App_Timer100HzISR(void)
{
    if (speed_tick_semaphore != NULL)
    {
        (void)osSemaphoreRelease(speed_tick_semaphore);
    }
}

static void SpeedControl_UpdateMotor(Motor_ID_t motor,
                                     Encoder_ID_t encoder)
{
    float target_rpm = Motor_GetTargetRPM(motor);
    float measured_rpm = Encoder_GetRPM(encoder);
    float output_duty;

    if (target_rpm == 0.0f)
    {
        PID_Reset(&speed_pid[motor]);
        Motor_Stop(motor);
        return;
    }

    output_duty = PID_Update(&speed_pid[motor],
                             target_rpm - measured_rpm,
                             SPEED_CONTROL_PERIOD_SEC);

    Motor_DriveDuty(motor, output_duty);
}

static void SpeedControlTask(void *argument)
{
    (void)argument;

    if (HAL_TIM_Base_Start_IT(&htim4) != HAL_OK)
    {
        Error_Handler();
    }

    for (;;)
    {
        if (osSemaphoreAcquire(speed_tick_semaphore,
                               osWaitForever) == osOK)
        {
            Encoder_Update(SPEED_CONTROL_PERIOD_SEC);
            SpeedControl_UpdateMotor(MOTOR_LEFT, ENCODER_LEFT);
            SpeedControl_UpdateMotor(MOTOR_RIGHT, ENCODER_RIGHT);
        }
    }
}

static void LineFollowTask(void *argument)
{
    LineFollow_TargetRPM_t target;

    (void)argument;

    Motor_SetBothTargetRPM(0.0f, 0.0f);
    osDelay(LINE_FOLLOW_START_DELAY_MS);

    if (LineFollow_EnableDigitalOutput() != HAL_OK)
    {
        Error_Handler();
    }

    for (;;)
    {
        if (LineFollow_Update() != 0U)
        {
            if (LineFollow_GetTargetRPM(&target) != 0U)
            {
                Motor_SetBothTargetRPM(target.left_target_rpm,
                                       target.right_target_rpm);
            }
            else
            {
                Motor_SetBothTargetRPM(0.0f, 0.0f);
            }
        }

        osDelay(LINE_FOLLOW_TASK_PERIOD_MS);
    }
}
