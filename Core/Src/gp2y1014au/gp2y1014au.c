#include "gp2y1014au.h"
#include "math.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

static ADC_HandleTypeDef *hadc_dust = NULL;
static TIM_HandleTypeDef *htim_pwm = NULL;
static uint32_t pwm_channel = TIM_CHANNEL_4; // 默认使用通道4

// GP2Y1014AU规格参数
#define LED_PULSE_WIDTH 320 // LED脉冲宽度(0.32ms)
#define SAMPLING_TIME 280	// 采样时间(0.28ms)
#define DELTA_TIME 40		// 脉冲后延时(0.04ms)
#define SLEEP_TIME 9680		// 睡眠时间(9.68ms)
#define CYCLE_TIME 10000	// 总周期(10ms)

// ADC采样参数
#define NUM_SAMPLES 5		 // 采样次数
#define FIRST_SAMPLE_DELAY 2 // 第一次采样前延时(ms)

// 简单延时函数，使用HAL_Delay替代微秒级延时
static void delay_ms(uint16_t ms)
{
	HAL_Delay(ms);
}

void GP2Y1014AU_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim)
{
	hadc_dust = hadc;
	htim_pwm = htim;

	// 检查TIM3通道4是否配置正确，这是PB1引脚
	if (htim->Instance == TIM3)
	{
		pwm_channel = TIM_CHANNEL_4;
	}

	// 启动PWM (初始状态LED关闭)
	__HAL_TIM_SET_COMPARE(htim_pwm, pwm_channel, 0);
	HAL_TIM_PWM_Start(htim_pwm, pwm_channel);
}

static uint16_t read_adc_value(void)
{
	HAL_ADC_Start(hadc_dust);
	HAL_ADC_PollForConversion(hadc_dust, 50); // 50ms超时
	uint16_t value = HAL_ADC_GetValue(hadc_dust);
	return value;
}

float GP2Y1014AU_ReadDustDensity(void)
{
	uint32_t adc_sum = 0;
	uint16_t adc_values[NUM_SAMPLES];
	uint16_t adc_value;

	// 多次采样取平均值
	for (uint8_t i = 0; i < NUM_SAMPLES; i++)
	{
		// 1. 打开LED (拉低IR LED引脚)
		__HAL_TIM_SET_COMPARE(htim_pwm, pwm_channel, LED_PULSE_WIDTH);

		// 2. 等待粉尘稳定
		HAL_Delay(FIRST_SAMPLE_DELAY);

		// 3. 读取ADC值
		adc_values[i] = read_adc_value();

		// 4. 关闭LED
		__HAL_TIM_SET_COMPARE(htim_pwm, pwm_channel, 0);

		// 5. 等待下一个周期
		HAL_Delay(10);
	}

	// 对采样值进行排序(简单冒泡排序)
	for (uint8_t i = 0; i < NUM_SAMPLES - 1; i++)
	{
		for (uint8_t j = 0; j < NUM_SAMPLES - i - 1; j++)
		{
			if (adc_values[j] > adc_values[j + 1])
			{
				uint16_t temp = adc_values[j];
				adc_values[j] = adc_values[j + 1];
				adc_values[j + 1] = temp;
			}
		}
	}

	// 去除最高值和最低值，取中间值的平均
	for (uint8_t i = 1; i < NUM_SAMPLES - 1; i++)
	{
		adc_sum += adc_values[i];
	}

	adc_value = adc_sum / (NUM_SAMPLES - 2);

	// 电压换算 (12位ADC, 3.3V参考电压)
	float voltage = (float)adc_value * 3.3f / 4096.0f;

	// 按照数据手册计算粉尘浓度
	// 如果电压低于0.5V，认为传感器未检测到灰尘
	if (voltage < 0.5f)
	{
		return 0.0f;
	}

	// 根据Sharp GP2Y1014AU数据手册，使用校准系数计算粉尘浓度
	// Dust density (μg/m³) = (Voltage - 0.5) * Coefficient
	// 标准系数为0.17，但根据实际环境可能需要校准
	float dust_density = (voltage - 0.5f) * 0.17f * 1000.0f;

	// 限制范围，避免不合理数值
	if (dust_density < 0.0f)
	{
		dust_density = 0.0f;
	}
	else if (dust_density > 1000.0f)
	{
		dust_density = 1000.0f;
	}

	return dust_density;
}

// 读取原始ADC值，用于校准
uint16_t GP2Y1014AU_ReadRawValue(void)
{
	// 打开LED
	__HAL_TIM_SET_COMPARE(htim_pwm, pwm_channel, LED_PULSE_WIDTH);

	// 等待粉尘稳定
	HAL_Delay(FIRST_SAMPLE_DELAY);

	// 读取ADC值
	uint16_t adc_value = read_adc_value();

	// 关闭LED
	__HAL_TIM_SET_COMPARE(htim_pwm, pwm_channel, 0);

	return adc_value;
}

// 获取原始电压值
float GP2Y1014AU_ReadVoltage(void)
{
	uint16_t adc_value = GP2Y1014AU_ReadRawValue();
	return (float)adc_value * 3.3f / 4096.0f;
}
