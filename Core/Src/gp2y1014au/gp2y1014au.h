#ifndef GP2Y1014AU_H
#define GP2Y1014AU_H

#include "stm32f1xx_hal.h"

/**
 * @brief 初始化GP2Y1014AU灰尘传感器
 * @param hadc ADC句柄指针，用于读取传感器输出
 * @param htim 定时器句柄指针，用于控制LED
 */
void GP2Y1014AU_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim);

/**
 * @brief 读取PM2.5粉尘浓度
 * @return 粉尘浓度值，单位μg/m³
 */
float GP2Y1014AU_ReadDustDensity(void);

/**
 * @brief 读取传感器原始ADC值，用于校准
 * @return ADC原始值(0-4095)
 */
uint16_t GP2Y1014AU_ReadRawValue(void);

/**
 * @brief 读取传感器输出电压值
 * @return 电压值，单位V
 */
float GP2Y1014AU_ReadVoltage(void);

#endif