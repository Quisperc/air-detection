#ifndef GP2Y1014AU_H
#define GP2Y1014AU_H

#include "stm32f1xx_hal.h"

// 传感器参数
#define DUST_SAMPLE_MS 10 // 采样周期10ms
#define DUST_PULSE_US 280 // 有效采样延迟时间（0.28ms）

// 初始化函数
void GP2Y1014AU_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim);

// 读取粉尘浓度（μg/m³）
float GP2Y1014AU_ReadDustDensity(void);

// 

#endif