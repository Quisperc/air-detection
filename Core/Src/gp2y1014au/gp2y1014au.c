#include "gp2y1014au.h"
#include "math.h"

static ADC_HandleTypeDef *hadc_dust = NULL;
static TIM_HandleTypeDef *htim_pwm = NULL;

// 微秒级延时（需提前实现）
static void delay_us(uint32_t us)
{
	uint32_t ticks = us * (SystemCoreClock / 1000000); // 计算延时需要的CPU周期数
	uint32_t start = DWT->CYCCNT;					   // 获取当前DWT计数器值
	while ((DWT->CYCCNT - start) < ticks)			   // 等待直到经过指定的CPU周期数
		;
}

void GP2Y1014AU_Init(ADC_HandleTypeDef *hadc, TIM_HandleTypeDef *htim)
{
	hadc_dust = hadc;
	htim_pwm = htim;

	// 启动PWM
	HAL_TIM_PWM_Start(htim_pwm, TIM_CHANNEL_1);
}

static uint32_t read_adc(void)
{
	HAL_ADC_Start(hadc_dust);
	HAL_ADC_PollForConversion(hadc_dust, 100);
	return HAL_ADC_GetValue(hadc_dust);
}

float GP2Y1014AU_ReadDustDensity(void)
{
	// 触发采样脉冲
	__HAL_TIM_SET_COMPARE(htim_pwm, TIM_CHANNEL_1, 32); // 0.32ms脉宽

	// 等待有效采样时间
	delay_us(DUST_PULSE_US); // 使用精确延时

	// 读取ADC值
	uint32_t adc_val = read_adc();
	float voltage = adc_val * 3.3f / 4095.0f;

	// 计算公式（根据手册曲线拟合）
	if (voltage < 0.5)
		return 0.0f;
	return (0.17f * voltage - 0.085f) * 1000.0f;
}
