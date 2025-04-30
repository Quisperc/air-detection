#include "sgp30.h"
#include "string.h"

#define SGP30_I2C_ADDR (0x58 << 1) // 7位地址左移1位
#define SGP30_CMD_INIT 0x2003
#define SGP30_CMD_MEASURE 0x2008
#define SGP30_CMD_SELFTEST 0x2032

static I2C_HandleTypeDef *hi2c_sgp30 = NULL;

// 发送命令（16位大端格式）
static HAL_StatusTypeDef send_command(uint16_t cmd)
{
	uint8_t buf[2] = {(uint8_t)(cmd >> 8), (uint8_t)cmd};
	return HAL_I2C_Master_Transmit(hi2c_sgp30, SGP30_I2C_ADDR, buf, 2, 100);
}

// 读取数据带CRC校验
static HAL_StatusTypeDef read_data(uint8_t *data, uint8_t len)
{
	if (HAL_I2C_Master_Receive(hi2c_sgp30, SGP30_I2C_ADDR, data, len * 3, 100) != HAL_OK)
		return HAL_ERROR;

	// CRC校验（多项式0x31）
	for (int i = 0; i < len; i++)
	{
		uint8_t crc = 0xFF;
		crc ^= data[i * 3];
		for (uint8_t b = 0; b < 8; b++)
			crc = (crc << 1) ^ ((crc & 0x80) ? 0x31 : 0);

		crc ^= data[i * 3 + 1];
		for (uint8_t b = 0; b < 8; b++)
			crc = (crc << 1) ^ ((crc & 0x80) ? 0x31 : 0);

		if (crc != data[i * 3 + 2])
			return HAL_ERROR;
	}
	return HAL_OK;
}

HAL_StatusTypeDef SGP30_Init(I2C_HandleTypeDef *hi2c)
{
	hi2c_sgp30 = hi2c;

	// 发送初始化命令
	if (send_command(SGP30_CMD_INIT) != HAL_OK)
		return HAL_ERROR;

	HAL_Delay(500); // 初始化需要时间
	return HAL_OK;
}

HAL_StatusTypeDef SGP30_ReadMeasurement(SGP30_Data *data)
{
	static uint8_t raw_data[6];

	if (send_command(SGP30_CMD_MEASURE) != HAL_OK)
		return HAL_ERROR;

	HAL_Delay(25); // 等待测量完成

	if (read_data(raw_data, 2) != HAL_OK)
		return HAL_ERROR;

	data->co2_eq_ppm = (raw_data[0] << 8) | raw_data[1];
	data->tvoc_ppb = (raw_data[3] << 8) | raw_data[4];
	return HAL_OK;
}

HAL_StatusTypeDef SGP30_RunSelfTest(void)
{
	uint8_t response[3];

	if (send_command(SGP30_CMD_SELFTEST) != HAL_OK)
		return HAL_ERROR;

	HAL_Delay(220);

	if (read_data(response, 1) != HAL_OK)
		return HAL_ERROR;

	// 自检成功返回0xD400
	uint16_t result = (response[0] << 8) | response[1];
	return (result == 0xD400) ? HAL_OK : HAL_ERROR;
}