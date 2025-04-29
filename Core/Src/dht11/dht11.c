/**
 * @文件        : dht11.c
 * @描述        : DHT11温湿度传感器驱动实现
 * @注意事项    : DHT11采用单总线通信方式，需要严格遵循时序要求
 */

#include "dht11.h"
#include "main.h"

/* 私有变量 */
static GPIO_TypeDef *DHT_GPIO;	   // DHT11连接的GPIO端口
static uint16_t DHT_PIN;		   // DHT11连接的GPIO引脚
static void delay_us(uint32_t us); // 微秒延时函数声明

/**
 * @函数名      : DHT11_Init
 * @描述        : 初始化DHT11传感器
 * @参数        : GPIOx - DHT11传感器连接的GPIO端口
 *                GPIO_Pin - DHT11传感器连接的GPIO引脚
 * @返回值      : 无
 * @实现细节    : 保存DHT11连接的GPIO信息，为后续通信做准备
 */
void DHT11_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	DHT_GPIO = GPIOx;
	DHT_PIN = GPIO_Pin;
}

/**
 * @函数名      : DHT11_Read
 * @描述        : 从DHT11传感器读取温湿度数据
 * @参数        : data - 指向存储读取数据的结构体的指针
 * @返回值      : HAL_OK(0x00) - 读取成功
 *                HAL_ERROR(0x01) - 读取失败
 * @实现细节    :
 *   1. DHT11通信过程：
 *      - 主机发送开始信号（拉低>18ms，再拉高）
 *      - DHT11响应（先拉低80us，再拉高80us）
 *      - DHT11传输40位数据（每位以50us低电平开始）：
 *        * 数据0：26-28us高电平
 *        * 数据1：70us高电平
 *      - 数据格式：8位湿度整数 + 8位湿度小数 + 8位温度整数 + 8位温度小数 + 8位校验和
 *   2. 通过测量高电平时间来判断数据位是0还是1
 *   3. 通过校验和验证数据正确性（校验和 = 前四个字节之和）
 */
uint8_t DHT11_Read(DHT11_Data *data)
{
	uint8_t buffer[5] = {0}; // 用于存储接收到的5个字节数据
	uint8_t retry = 0;		 // 重试计数变量（未使用）
	uint32_t timeout = 0;	 // 超时计数器，防止无限等待

	/* 阶段1: 发送开始信号 */
	DHT11_SetMode(GPIO_MODE_OUTPUT_PP);					  // 设置为推挽输出模式，提供更强的驱动能力
	HAL_GPIO_WritePin(DHT_GPIO, DHT_PIN, GPIO_PIN_RESET); // 拉低总线
	HAL_Delay(20);										  // 保持低电平至少18ms，此处设为20ms确保足够
	HAL_GPIO_WritePin(DHT_GPIO, DHT_PIN, GPIO_PIN_SET);	  // 拉高总线
	delay_us(30);										  // 主机拉高后等待20-40us

	/* 阶段2: 等待DHT11响应 */
	DHT11_SetMode(GPIO_MODE_INPUT); // 切换为输入模式，准备接收DHT11响应和数据

	// 等待DHT11的响应信号（开始拉低总线）
	timeout = 0;
	while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN) == GPIO_PIN_SET)
	{
		delay_us(1); // 每次延时1us
		timeout++;
		if (timeout > 100) // 超时保护，防止DHT11无响应导致死循环
			return HAL_ERROR;
	}

	// 等待DHT11的低电平响应结束
	timeout = 0;
	while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN) == GPIO_PIN_RESET)
	{
		delay_us(1);
		timeout++;
		if (timeout > 100) // 正常约80us，超过100us认为异常
			return HAL_ERROR;
	}

	// 等待DHT11的高电平响应结束
	timeout = 0;
	while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN) == GPIO_PIN_SET)
	{
		delay_us(1);
		timeout++;
		if (timeout > 100) // 正常约80us，超过100us认为异常
			return HAL_ERROR;
	}

	/* 阶段3: 接收40位数据 */
	for (uint8_t i = 0; i < 5; i++) // 5个字节数据
	{
		for (uint8_t j = 0; j < 8; j++) // 每个字节8位
		{
			// 等待数据位的前导低电平结束（固定50us）
			timeout = 0;
			while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN) == GPIO_PIN_RESET)
			{
				delay_us(1);
				timeout++;
				if (timeout > 100) // 正常约50us，超过100us认为异常
					return HAL_ERROR;
			}

			// 延时40微秒后检测电平，判断数据位是0还是1
			delay_us(40); // 数据0的高电平约26-28us，数据1的高电平约70us

			// 如果40us后仍为高电平，则数据位为1；否则为0
			buffer[i] <<= 1; // 数据左移1位，为新数据位腾出位置
			if (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN) == GPIO_PIN_SET)
				buffer[i] |= 1; // 数据位为1

			// 等待当前数据位的高电平结束，准备接收下一位
			timeout = 0;
			while (HAL_GPIO_ReadPin(DHT_GPIO, DHT_PIN) == GPIO_PIN_SET)
			{
				delay_us(1);
				timeout++;
				if (timeout > 100) // 防止无限等待
					return HAL_ERROR;
			}
		}
	}

	/* 阶段4: 数据校验与保存 */
	// 校验数据（校验和 = 前四个字节之和）
	if (buffer[4] == (buffer[0] + buffer[1] + buffer[2] + buffer[3]))
	{
		// 保存读取到的温湿度数据
		data->humidity = buffer[0];
		data->humidity_dec = buffer[1];
		data->temperature = buffer[2];
		data->temperature_dec = buffer[3];

		// 检查数据范围合理性（湿度0-100%，温度0-85℃）
		if (data->humidity > 100 || data->temperature > 85)
			return HAL_ERROR; // 数据超出传感器范围，视为无效

		return HAL_OK; // 读取成功
	}

	return HAL_ERROR; // 校验和错误，读取失败
}

/**
 * @函数名      : delay_us
 * @描述        : 微秒级延时函数
 * @参数        : us - 延时时间，单位微秒
 * @返回值      : 无
 * @依赖        : DWT (Data Watchpoint and Trace)单元必须已启用
 * @实现细节    : 利用Cortex-M内核的DWT计数器实现精确微秒延时
 */
static void delay_us(uint32_t us)
{
	uint32_t ticks = us * (SystemCoreClock / 1000000); // 计算延时需要的CPU周期数
	uint32_t start = DWT->CYCCNT;					   // 获取当前DWT计数器值
	while ((DWT->CYCCNT - start) < ticks)			   // 等待直到经过指定的CPU周期数
		;
}

/**
 * @函数名      : DHT11_SetMode
 * @描述        : 设置DHT11数据引脚的GPIO模式
 * @参数        : mode - GPIO模式 (GPIO_MODE_INPUT/GPIO_MODE_OUTPUT_PP等)
 * @返回值      : 无
 * @注意        : 此函数在读取过程中需要多次切换GPIO模式
 */
void DHT11_SetMode(uint32_t mode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};		  // 定义GPIO初始化结构体
	GPIO_InitStruct.Pin = DHT_PIN;				  // 设置引脚
	GPIO_InitStruct.Mode = mode;				  // 设置模式
	GPIO_InitStruct.Pull = GPIO_PULLUP;			  // 使用上拉电阻
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 高速模式
	HAL_GPIO_Init(DHT_GPIO, &GPIO_InitStruct);	  // 初始化GPIO
}
