#include "8channel.h"

#include "usart.h"
#include <string.h>

#define LINEFOLLOW_PACKET_MAX_LENGTH 48U
#define LINEFOLLOW_LINE_VALUE 0U

static const float sensor_weight[LINEFOLLOW_SENSOR_NUM] = {
    -7.0f, -5.0f, -3.0f, -1.0f,
    1.0f, 3.0f, 5.0f, 7.0f};

static uint8_t uart_rx_byte;
static uint8_t rx_work_buffer[LINEFOLLOW_PACKET_MAX_LENGTH];
static uint8_t rx_ready_buffer[LINEFOLLOW_PACKET_MAX_LENGTH];
static uint8_t rx_work_length;
static volatile uint8_t rx_ready_length;
static volatile uint8_t rx_receiving;
static volatile uint8_t rx_packet_ready;

static LineFollow_SensorData_t current_sensor;
static LineFollow_TargetRPM_t current_target;
static uint8_t digital_output_command[] = "$0,0,1#";
static float last_error;

static void LineFollow_InputByte(uint8_t byte)
{
    if (byte == '$')
    {
        /* A new frame header restarts packet collection. */
        rx_work_length = 0U;
        rx_receiving = 1U;
        return;
    }

    if (rx_receiving == 0U)
    {
        return;
    }

    if (byte == '#')
    {
        /* Keep the previous complete frame until the main loop consumes it. */
        if ((rx_work_length > 0U) && (rx_packet_ready == 0U))
        {
            memcpy(rx_ready_buffer, rx_work_buffer, rx_work_length);
            rx_ready_length = rx_work_length;
            rx_packet_ready = 1U;
        }

        rx_receiving = 0U;
        rx_work_length = 0U;
        return;
    }

    if (rx_work_length >= LINEFOLLOW_PACKET_MAX_LENGTH)
    {
        /* Discard an overlength frame and wait for the next '$'. */
        rx_receiving = 0U;
        rx_work_length = 0U;
        return;
    }

    rx_work_buffer[rx_work_length] = byte;
    rx_work_length++;
}

/*
 * The packet passed here excludes '$' and '#'.
 * Expected format:
 * D,x1:0,x2:0,x3:0,x4:0,x5:0,x6:0,x7:0,x8:0
 */
static uint8_t LineFollow_ParseDigitalPacket(
    const uint8_t *packet,
    uint8_t length,
    uint8_t sensor[LINEFOLLOW_SENSOR_NUM])
{
    uint8_t sensor_index;
    uint8_t position = 0U;
    uint8_t value;

    if ((packet == NULL) || (sensor == NULL))
    {
        return 0U;
    }

    if ((length == 0U) || (packet[position] != 'D'))
    {
        return 0U;
    }

    position++;

    for (sensor_index = 0U;
         sensor_index < LINEFOLLOW_SENSOR_NUM;
         sensor_index++)
    {
        if ((uint8_t)(position + 5U) > length)
        {
            return 0U;
        }

        if (packet[position++] != ',')
        {
            return 0U;
        }

        if (packet[position++] != 'x')
        {
            return 0U;
        }

        if (packet[position++] != (uint8_t)('1' + sensor_index))
        {
            return 0U;
        }

        if (packet[position++] != ':')
        {
            return 0U;
        }

        value = packet[position++];

        if ((value != '0') && (value != '1'))
        {
            return 0U;
        }

        sensor[sensor_index] = (uint8_t)(value - '0');
    }

    /* Reject trailing characters after x8. */
    if (position != length)
    {
        return 0U;
    }

    return 1U;
}

static void LineFollow_ProcessSensorData(
    const uint8_t sensor[LINEFOLLOW_SENSOR_NUM],
    LineFollow_SensorData_t *result)
{
    float weight_sum = 0.0f;
    uint8_t sensor_index;

    memset(result, 0, sizeof(*result));
    memcpy(result->value, sensor, sizeof(result->value));

    for (sensor_index = 0U;
         sensor_index < LINEFOLLOW_SENSOR_NUM;
         sensor_index++)
    {
        if (sensor[sensor_index] == LINEFOLLOW_LINE_VALUE)
        {
            result->active_mask |= (uint8_t)(1U << sensor_index);
            result->active_count++;
            weight_sum += sensor_weight[sensor_index];
        }
    }

    if (result->active_count > 0U)
    {
        result->has_line = 1U;
        result->error = weight_sum / (float)result->active_count;
        last_error = result->error;
    }
    else
    {
        result->has_line = 0U;
        result->error = last_error;
    }

    result->valid = 1U;
}

static float LineFollow_LimitRPM(float rpm)
{
    if (rpm > LINEFOLLOW_MAX_RPM)
    {
        return LINEFOLLOW_MAX_RPM;
    }

    if (rpm < LINEFOLLOW_MIN_RPM)
    {
        return LINEFOLLOW_MIN_RPM;
    }

    return rpm;
}

static void LineFollow_UpdateTargetRPM(
    const LineFollow_SensorData_t *sensor)
{
    LineFollow_TargetRPM_t target = {0};
    float turn;

    if (sensor->has_line == 0U)
    {
        target.left_target_rpm = 0.0f;
        target.right_target_rpm = 0.0f;
        target.valid = 0U;
    }
    else
    {
        turn = sensor->error * LINEFOLLOW_TURN_GAIN;

        target.left_target_rpm =
            LineFollow_LimitRPM(LINEFOLLOW_BASE_RPM + turn);
        target.right_target_rpm =
            LineFollow_LimitRPM(LINEFOLLOW_BASE_RPM - turn);
        target.valid = 1U;
    }

    current_target = target;
}

void LineFollow_Init(void)
{
    memset(rx_work_buffer, 0, sizeof(rx_work_buffer));
    memset(rx_ready_buffer, 0, sizeof(rx_ready_buffer));
    memset(&current_sensor, 0, sizeof(current_sensor));
    memset(&current_target, 0, sizeof(current_target));

    uart_rx_byte = 0U;
    rx_work_length = 0U;
    rx_ready_length = 0U;
    rx_receiving = 0U;
    rx_packet_ready = 0U;
    last_error = 0.0f;
}

HAL_StatusTypeDef LineFollow_StartUartReceive(void)
{
    return HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1U);
}

HAL_StatusTypeDef LineFollow_EnableDigitalOutput(void)
{
    return HAL_UART_Transmit_IT(&huart2,
                                digital_output_command,
                                sizeof(digital_output_command) - 1U);
}

void LineFollow_UartRxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        LineFollow_InputByte(uart_rx_byte);
        (void)HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1U);
    }
}

void LineFollow_UartErrorCallback(UART_HandleTypeDef *huart)
{
    if ((huart != NULL) && (huart->Instance == USART2))
    {
        rx_work_length = 0U;
        rx_receiving = 0U;
        (void)HAL_UART_Receive_IT(&huart2, &uart_rx_byte, 1U);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    LineFollow_UartRxCpltCallback(huart);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    LineFollow_UartErrorCallback(huart);
}

uint8_t LineFollow_Update(void)
{
    uint8_t packet[LINEFOLLOW_PACKET_MAX_LENGTH];
    uint8_t packet_length;
    uint8_t sensor[LINEFOLLOW_SENSOR_NUM];
    LineFollow_SensorData_t sensor_result;
    uint32_t primask;

    if (rx_packet_ready == 0U)
    {
        return 0U;
    }

    /* Copy the completed frame atomically with respect to the UART ISR. */
    primask = __get_PRIMASK();
    __disable_irq();

    packet_length = rx_ready_length;
    memcpy(packet, rx_ready_buffer, packet_length);
    rx_packet_ready = 0U;

    if (primask == 0U)
    {
        __enable_irq();
    }

    if (LineFollow_ParseDigitalPacket(packet,
                                      packet_length,
                                      sensor) == 0U)
    {
        return 0U;
    }

    LineFollow_ProcessSensorData(sensor, &sensor_result);
    current_sensor = sensor_result;
    LineFollow_UpdateTargetRPM(&sensor_result);

    return 1U;
}

uint8_t LineFollow_GetSensorData(LineFollow_SensorData_t *data)
{
    if (data == NULL)
    {
        return 0U;
    }

    *data = current_sensor;
    return current_sensor.valid;
}

uint8_t LineFollow_GetTargetRPM(LineFollow_TargetRPM_t *target)
{
    if (target == NULL)
    {
        return 0U;
    }

    *target = current_target;
    return current_target.valid;
}
