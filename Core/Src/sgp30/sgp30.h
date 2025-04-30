#ifndef SGP30_H
#define SGP30_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// 定义SGP30的IIC地址
#define SGP30_ADDR 0x58

// 定义SGP30数据结构体，存储CO2和TVOC浓度
typedef struct
{
	uint16_t co2_eq_ppm; // CO2当量浓度，单位ppm
	uint16_t tvoc_ppb;	 // TVOC浓度，单位ppb
} SGP30_DATA;

/**
 * @函数名      : sgp30_init
 * @描述        : 初始化SGP30气体传感器
 * @参数        : hi2c2 - I2C句柄指针
 * @返回值      : 无
 */
void sgp30_init(I2C_HandleTypeDef *hi2c2);

/**
 * @函数名      : sgp30_read
 * @描述        : 读取SGP30气体传感器数据
 * @参数        : result - 存储气体浓度的结构体指针
 * @返回值      : HAL_OK - 读取成功; HAL_ERROR - 读取失败
 */
uint8_t sgp30_read(SGP30_DATA *result);

/**
 * @函数名      : sgp30_crc
 * @描述        : 计算SGP30数据的CRC-8校验值
 * @参数        : data - 需要计算校验的数据
 *                len - 数据长度
 * @返回值      : uint8_t - CRC校验值
 */
uint8_t sgp30_crc(uint8_t *data, uint8_t len);

#endif