#ifndef DHT11_NEW_H
#define DHT11_NEW_H

#include "stm32f10x.h"

typedef enum {
    DHT11_SUCCESS,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_CHECKSUM
} DHT11_Status;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t temperature;
    uint8_t humidity;
} DHT11_Dev;

void DHT11_Init(DHT11_Dev* dev, GPIO_TypeDef* port, uint16_t pin);
DHT11_Status DHT11_Read(DHT11_Dev* dev);

#endif
