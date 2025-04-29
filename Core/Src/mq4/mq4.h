#ifndef MQ4_H
#define MQ4_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f1xx_hal.h"

	/**
	 * @枚举名      : MQ4_CalibState
	 * @描述        : MQ4传感器校准状态枚举
	 * @成员        : MQ4_CALIB_IDLE - 空闲状态，未开始校准
	 *                MQ4_CALIB_RUNNING - 校准进行中
	 *                MQ4_CALIB_DONE - 校准完成
	 */
	typedef enum
	{
		MQ4_CALIB_IDLE,	   // 空闲状态，未开始校准
		MQ4_CALIB_RUNNING, // 校准进行中
		MQ4_CALIB_DONE	   // 校准完成
	} MQ4_CalibState;

	/**
	 * @函数名      : MQ4_Init
	 * @描述        : 初始化MQ4甲烷传感器
	 * @参数        : hadc - MQ4传感器连接的ADC句柄指针
	 * @返回值      : 无
	 * @注意事项    : 使用前必须调用此函数进行初始化
	 */
	void MQ4_Init(ADC_HandleTypeDef *hadc);

	/**
	 * @函数名      : MQ4_Calibrate
	 * @描述        : 执行MQ4传感器校准（非阻塞方式）
	 * @参数        : 无
	 * @返回值      : 无
	 * @注意事项    : 需要在主循环中重复调用，直至校准完成
	 *                总校准时间约为300秒(5分钟)
	 */
	void MQ4_Calibrate(void);

	/**
	 * @函数名      : MQ4_ReadPPM
	 * @描述        : 读取甲烷气体浓度
	 * @参数        : 无
	 * @返回值      : float - 甲烷浓度(PPM)，校准未完成时返回-1.0
	 * @注意事项    : 只有在校准完成后才能获得有效读数
	 */
	float MQ4_ReadPPM(void);

	/**
	 * @函数名      : MQ4_GetCalibStatus
	 * @描述        : 获取当前校准状态
	 * @参数        : 无
	 * @返回值      : MQ4_CalibState - 当前校准状态
	 */
	MQ4_CalibState MQ4_GetCalibStatus(void);

	/**
	 * @函数名      : MQ4_GetSampleCount
	 * @描述        : 获取当前已采集的校准样本数
	 * @参数        : 无
	 * @返回值      : uint16_t - 已采集的校准样本数
	 */
	uint16_t MQ4_GetSampleCount(void);

	/**
	 * @函数名      : MQ4_GetCalibrationTotal
	 * @描述        : 获取校准所需的总样本数
	 * @参数        : 无
	 * @返回值      : uint16_t - 校准所需的总样本数(50)
	 */
	uint16_t MQ4_GetCalibrationTotal(void);

	/**
	 * @函数名      : MQ4_GetRemainingTime
	 * @描述        : 获取校准剩余时间(秒)
	 * @参数        : 无
	 * @返回值      : uint16_t - 校准剩余时间(秒)
	 */
	uint16_t MQ4_GetRemainingTime(void);

#ifdef __cplusplus
}
#endif

#endif /* MQ4_H */