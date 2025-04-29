#ifndef DHT11_H
#define DHT11_H

/**
 * @文件        : dht11.h
 * @描述        : DHT11温湿度传感器驱动头文件
 * @注意事项    : DHT11传感器支持温湿度测量，工作电压3.3V-5.5V
 *                温度测量范围：0-50℃，湿度测量范围：20-90%RH
 */

#include "stm32f1xx_hal.h"

/**
 * @结构体名    : DHT11_Data
 * @描述        : DHT11传感器数据结构体，存储读取到的温湿度数据
 */
typedef struct
{
    uint8_t humidity;        // 湿度整数部分 (0-100%)
    uint8_t humidity_dec;    // 湿度小数部分 (0-99)
    uint8_t temperature;     // 温度整数部分 (0-50℃)
    uint8_t temperature_dec; // 温度小数部分 (0-99)
} DHT11_Data;

/**
 * @函数名      : DHT11_Init
 * @描述        : 初始化DHT11传感器
 * @参数        : GPIOx - DHT11传感器连接的GPIO端口
 *                GPIO_Pin - DHT11传感器连接的GPIO引脚
 * @返回值      : 无
 */
void DHT11_Init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

/**
 * @函数名      : DHT11_Read
 * @描述        : 从DHT11传感器读取温湿度数据
 * @参数        : data - 指向存储读取数据的结构体的指针
 * @返回值      : HAL_OK(0x00) - 读取成功
 *                HAL_ERROR(0x01) - 读取失败
 */
uint8_t DHT11_Read(DHT11_Data *data);

/**
 * @函数名      : DHT11_SetMode
 * @描述        : 设置DHT11数据引脚的GPIO模式
 * @参数        : mode - GPIO模式 (GPIO_MODE_INPUT/GPIO_MODE_OUTPUT_PP等)
 * @返回值      : 无
 * @注意        : 此函数在.c文件中定义为static，头文件中声明为外部函数
 *                需要修改为一致性，建议在.c文件中实现为非static
 */
void DHT11_SetMode(uint32_t mode);

#endif
