/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : dht11.h
  * @brief          : DHT11 Temperature & Humidity Sensor Driver Header
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "main.h"
#include "stm32f1xx_hal.h"

// DHT11 GPIO Configuration (MUST match CubeMX settings)
#define DHT11_PORT      GPIOA
#define DHT11_PIN       GPIO_PIN_4

// DHT11 Status Codes
typedef enum {
    DHT11_OK = 0,
    DHT11_ERROR_CHECKSUM,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_NO_RESPONSE
} DHT11_Status;

// DHT11 Data Structure
typedef struct {
    uint8_t humidity_int;
    uint8_t humidity_dec;
    uint8_t temperature_int;
    uint8_t temperature_dec;
    uint8_t checksum;
} DHT11_Data;

// Public Function Prototypes
void DHT11_Init(void);  // NO TIMER PARAMETER
DHT11_Status DHT11_Read(DHT11_Data *data);
float DHT11_GetTemperature(void);
float DHT11_GetHumidity(void);

#endif /* INC_DHT11_H_ */
