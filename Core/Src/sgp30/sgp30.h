#ifndef SGP30_H
#define SGP30_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f1xx_hal.h"

	// 传感器数据结构
	typedef struct
	{
		uint16_t tvoc_ppb;	 // TVOC浓度（ppb）
		uint16_t co2_eq_ppm; // CO2等效浓度（ppm）
	} SGP30_Data;

	// 初始化函数
	HAL_StatusTypeDef SGP30_Init(I2C_HandleTypeDef *hi2c);

	// 数据读取（非阻塞）
	HAL_StatusTypeDef SGP30_ReadMeasurement(SGP30_Data *data);

	// 自检函数
	HAL_StatusTypeDef SGP30_RunSelfTest(void);

#ifdef __cplusplus
}
#endif

#endif /* SGP30_H */