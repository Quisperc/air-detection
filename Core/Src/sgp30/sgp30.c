#include "sgp30.h"
#include "i2c.h"
#include <stdint.h> // 添加标准整数类型头文件
// extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;

// 定义SGP30初始化函数，发送初始化命令
void sgp30_init(I2C_HandleTypeDef *hi2c2)
{
	uint8_t cmd[2] = {0x20, 0x03}; // 初始化命令为0x2003
	HAL_StatusTypeDef status;

	status = HAL_I2C_Master_Transmit(hi2c2, SGP30_ADDR << 1, cmd, 2, 100); // 发送初始化命令
	if (status != HAL_OK)
	{
		// 初始化失败，发送错误信息
		char *err_msg = "SGP30 Initialize Error!\r\n";
		HAL_UART_Transmit(&huart1, (uint8_t *)err_msg, 26, 100);
		return;
	}

	HAL_Delay(20); // 等待初始化完成（数据手册建议）
}

// 定义SGP30读取数据函数，返回CO2和TVOC的值
uint8_t sgp30_read(SGP30_DATA *result)
{
	uint8_t cmd[2] = {0x20, 0x08}; // 读取命令为0x2008
	uint8_t data[6];			   // 存储返回的6个字节数据
	uint8_t crc;				   // 存储CRC校验值
	HAL_StatusTypeDef status;

	// 初始化结果为0，防止读取失败时返回随机值
	// result->co2_eq_ppm = 0;
	// result->tvoc_ppb = 0;

	// 发送读取命令
	status = HAL_I2C_Master_Transmit(&hi2c2, SGP30_ADDR << 1, cmd, 2, 100);
	if (status != HAL_OK)
	{
		return HAL_ERROR;
	}

	HAL_Delay(25); // 等待测量完成（数据手册建议）

	// 接收6个字节数据
	status = HAL_I2C_Master_Receive(&hi2c2, SGP30_ADDR << 1 | 0x01, data, 6, 100);
	if (status != HAL_OK)
	{
		return HAL_ERROR;
	}

	// 验证CO2数据的CRC校验
	crc = sgp30_crc(data, 2); // 计算前两个字节的CRC校验值
	if (crc == data[2])
	{												   // 如果和第三个字节相同，说明CO2数据有效
		result->co2_eq_ppm = (data[0] << 8) | data[1]; // 将前两个字节合并为CO2值
	}
	else
	{
		// CRC校验失败，返回错误
		return HAL_ERROR;
	}

	// 验证TVOC数据的CRC校验
	crc = sgp30_crc(data + 3, 2); // 计算第四和第五个字节的CRC校验值
	if (crc == data[5])
	{												 // 如果和第六个字节相同，说明TVOC数据有效
		result->tvoc_ppb = (data[3] << 8) | data[4]; // 将第四和第五个字节合并为TVOC值
	}
	else
	{
		// CRC校验失败，返回错误
		return HAL_ERROR;
	}

	return HAL_OK; // 返回结果
}

// 定义SGP30计算CRC校验值的函数，使用多项式x8 + x5 + x4 + x0
uint8_t sgp30_crc(uint8_t *data, uint8_t len)
{
	uint8_t crc = 0xFF; // 初始值为0xFF
	uint8_t bit;		// 存储位掩码

	for (uint8_t i = 0; i < len; i++)
	{					// 遍历每个字节
		crc ^= data[i]; // 异或当前字节
		for (bit = 8; bit > 0; bit--)
		{ // 遍历每个位
			if (crc & 0x80)
			{ // 如果最高位为1，就左移并异或0x31
				crc = (crc << 1) ^ 0x31;
			}
			else
			{ // 否则，就左移
				crc = (crc << 1);
			}
		}
	}

	return crc; // 返回CRC校验值
}