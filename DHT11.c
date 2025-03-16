#include "DHT11_New.h"
#include "Delay.h"

#define DHT11_START_SIGNAL_MS  18
#define DHT11_TIMEOUT_US       200
#define DHT11_TIMEHIGH_0       28
#define DHT11_TIMEHIGH_1       70

void DHT11_Init(DHT11_Dev* dev, GPIO_TypeDef* port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStructure;

    dev->port = port;
    dev->pin = pin;

    if (port == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (port == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if (port == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &GPIO_InitStructure);
    GPIO_SetBits(port, pin);
}

static void DHT11_SetIO(DHT11_Dev* dev, GPIOMode_TypeDef mode) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = dev->pin;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(dev->port, &GPIO_InitStructure);
}

static uint8_t DHT11_Check(DHT11_Dev* dev) {
    uint32_t retry = 0;
    
    // 输出模式
    DHT11_SetIO(dev, GPIO_Mode_Out_PP);
    // 发送>18ms低电平起始信号
    GPIO_ResetBits(dev->port, dev->pin);
    Delay_ms(20);
    GPIO_SetBits(dev->port, dev->pin);
    
    // 等待20~40us
    Delay_us(30);
    
    // 输入模式
    DHT11_SetIO(dev, GPIO_Mode_IPU);
    
    // 等待拉低
    while(GPIO_ReadInputDataBit(dev->port, dev->pin) && retry < 100) {
        retry++;
        Delay_us(1);
    }
    if(retry >= 100) return 1; // 无响应
    
    retry = 0;
    // 80us后拉高
    while(!GPIO_ReadInputDataBit(dev->port, dev->pin) && retry < 100) {
        retry++;
        Delay_us(1);
    }
    if(retry >= 100) return 1; // 无响应
    
    return 0;
}

static uint8_t DHT11_ReadBit(DHT11_Dev* dev) {
    uint32_t retry = 0;
    
    // 等待变为低电平，80us
    while(GPIO_ReadInputDataBit(dev->port, dev->pin) && retry < 100) {
        retry++;
        Delay_us(1);
    }
    if(retry >= 100) return 100; // 无响应
    
    retry = 0;
    // 等待拉高，50us
    while(!GPIO_ReadInputDataBit(dev->port, dev->pin) && retry < 100) {
        retry++;
        Delay_us(1);
    }
    if(retry >= 100) return 100; // 无响应
    
    // 延时40us，读取电平 低电平 ----- 0  高电平 ----- 1
    Delay_us(40);
    return GPIO_ReadInputDataBit(dev->port, dev->pin);
}

static uint8_t DHT11_ReadByte(DHT11_Dev* dev) {
    uint8_t i, dat = 0;
    
    // 连续读取8位
    for(i = 0; i < 8; i++) {
        dat |= DHT11_ReadBit(dev) << (7-i);
    }
    
    return dat;
}

DHT11_Status DHT11_Read(DHT11_Dev* dev) {
    uint8_t buf[5];
    
    // 起始信号
    if(DHT11_Check(dev) == 0) {
        // 读取40bit数据
        for(uint8_t i = 0; i < 5; i++) {
            buf[i] = DHT11_ReadByte(dev);
        }
        // 验证校验和
        if(((buf[0] + buf[1] + buf[2] + buf[3]) & 0xff) == buf[4]) {
            dev->humidity = buf[0];
            dev->temperature = buf[2];
            return DHT11_SUCCESS;
        }
    }
    
    return DHT11_ERROR_TIMEOUT;
}
