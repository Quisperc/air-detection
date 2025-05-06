/**
 * @文件        : mq4.c
 * @描述        : MQ4甲烷气体传感器驱动实现
 * @注意事项    : MQ4传感器需要预热和校准后才能提供准确读数
 *                校准需要在干净空气中进行，总时间约300秒
 */

#include "mq4.h"
#include "math.h"
#include "main.h"
#include <stdint.h> // 添加标准整数类型头文件

/**
 * 模块私有变量定义
 */
static ADC_HandleTypeDef *hadc_mq4 = NULL; // ADC句柄
static float R0 = 10.0f;				   // 默认校准电阻值（单位KΩ）
static const float RL = 10.0f;			   // 负载电阻值（单位KΩ）

/**
 * 校准相关变量结构
 */
static struct
{
	uint32_t last_sample_time; // 上次采样时间戳
	uint16_t sample_count;	   // 已采样次数计数
	float sum_adc;			   // ADC采样值累加和
	MQ4_CalibState state;	   // 校准状态
} calibration = {0};

/**
 * @函数名      : MQ4_Init
 * @描述        : 初始化MQ4甲烷传感器
 * @参数        : hadc - MQ4传感器连接的ADC句柄指针
 * @返回值      : 无
 * @实现细节    : 保存ADC句柄并初始化校准状态
 */
void MQ4_Init(ADC_HandleTypeDef *hadc)
{
	hadc_mq4 = hadc;
	calibration.state = MQ4_CALIB_IDLE;
}

/**
 * @函数名      : MQ4_GetCalibStatus
 * @描述        : 获取当前校准状态
 * @参数        : 无
 * @返回值      : MQ4_CalibState - 当前校准状态
 */
MQ4_CalibState MQ4_GetCalibStatus(void)
{
	return calibration.state;
}

/**
 * @函数名      : read_adc
 * @描述        : 读取MQ4传感器ADC值的内部函数
 * @参数        : 无
 * @返回值      : uint32_t - ADC原始读数
 * @注意事项    : 此函数为模块内部使用，不对外暴露
 */
static uint32_t read_adc(void)
{
	HAL_ADC_Start(hadc_mq4);
	HAL_ADC_PollForConversion(hadc_mq4, 100); // 等待转换完成，超时100ms
	return HAL_ADC_GetValue(hadc_mq4);
}

/**
 * @函数名      : MQ4_Calibrate
 * @描述        : 执行MQ4传感器校准（非阻塞方式）
 * @参数        : 无
 * @返回值      : 无
 * @实现细节    :
 *   1. 根据校准状态执行不同操作
 *   2. 在IDLE状态初始化校准参数
 *   3. 在RUNNING状态每6秒采集一次样本，50个样本后完成校准
 *   4. 在DONE状态不执行任何操作
 */
void MQ4_Calibrate(void)
{
	switch (calibration.state)
	{
	case MQ4_CALIB_IDLE:
		// 初始化校准参数
		calibration.last_sample_time = HAL_GetTick();
		calibration.sample_count = 0;
		calibration.sum_adc = 0.0f;
		calibration.state = MQ4_CALIB_RUNNING;
		break;

	case MQ4_CALIB_RUNNING:
		// 每6秒采样一次，共50次采样
		if (HAL_GetTick() - calibration.last_sample_time >= 6000)
		{
			calibration.sum_adc += read_adc();
			calibration.sample_count++;
			calibration.last_sample_time = HAL_GetTick();

			float temp = MQ4_GetCalibrationTotal();

			// 采样完成后计算R0值
			if (calibration.sample_count >= MQ4_GetCalibrationTotal())
			{
				// 计算平均电压和R0电阻值
				float Vrl = (calibration.sum_adc / temp) * 3.3f / 4095.0f;
				R0 = (3.3f - Vrl) * RL / Vrl; // 基于分压电路计算R0
				calibration.state = MQ4_CALIB_DONE;
			}
		}
		break;

	case MQ4_CALIB_DONE:
		// 校准完成，无需操作
		break;
	}
}

/**
 * @函数名      : MQ4_ReadPPM
 * @描述        : 读取甲烷气体浓度
 * @参数        : 无
 * @返回值      : float - 甲烷浓度(PPM)，校准未完成时返回-1.0
 * @实现细节    :
 *   1. 检查校准状态，未完成则返回-1.0
 *   2. 读取ADC值并转换为电压
 *   3. 计算传感器电阻Rs
 *   4. 基于Rs/R0比值和MQ4特性曲线公式计算PPM值
 */
float MQ4_ReadPPM(void)
{
	// 校准未完成则返回错误值
	if (calibration.state != MQ4_CALIB_DONE)
		return -1.0f;

	// 读取ADC值并转换为电压和电阻
	const uint32_t adc_val = read_adc();
	const float Vrl = adc_val * 3.3f / 4095.0f; // 电压值 (基于3.3V参考电压和12位ADC)
	const float Rs = (3.3f - Vrl) * RL / Vrl;	// 传感器电阻值

	// 根据MQ-4特性曲线拟合的公式计算PPM值
	// 公式基于log-log特性曲线: log(ppm) = (log(Rs/R0) - b) / a
	// 其中a = -0.32, b = 0.48 (根据MQ4数据手册曲线拟合得到)
	return pow(10.0f, (log10(Rs / R0) - 0.74f) / (-0.65f));
}

/**
 * @函数名      : MQ4_GetSampleCount
 * @描述        : 获取当前已采集的校准样本数
 * @参数        : 无
 * @返回值      : uint16_t - 已采集的校准样本数
 */
uint16_t MQ4_GetSampleCount(void)
{
	return calibration.sample_count;
}

/**
 * @函数名      : MQ4_GetCalibrationTotal
 * @描述        : 获取校准所需的总样本数
 * @参数        : 无
 * @返回值      : uint16_t - 校准所需的总样本数(50)
 */
uint16_t MQ4_GetCalibrationTotal(void)
{
	return 50; // 总采样次数固定为50
}

/**
 * @函数名      : MQ4_GetRemainingTime
 * @描述        : 获取校准剩余时间(秒)
 * @参数        : 无
 * @返回值      : uint16_t - 校准剩余时间(秒)
 * @实现细节    : 总校准时间300秒，每个样本6秒，计算剩余秒数
 */
uint16_t MQ4_GetRemainingTime(void)
{
	return MQ4_GetCalibrationTotal() * 6 - (calibration.sample_count * 6);
}