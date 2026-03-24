/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : dht11.c
  * @brief          : DHT11 Driver with Software Delay (No Timer Required)
  ******************************************************************************
  */
/* USER CODE END Header */

#include "dht11.h"
#include "cmsis_os.h"

static DHT11_Data last_reading = {0};

/**
  * @brief  Accurate microsecond delay using CPU cycles
  * @note   MUST disable interrupts before calling this!
  * @param  us: Microseconds to delay
  */
__attribute__((optimize("O0")))
static inline void delay_us_precise(uint16_t us) {
    // STM32F103 @ 72MHz
    // 1 microsecond = 72 clock cycles
    // Each loop iteration ≈ 8 cycles (tested with oscilloscope)
    // Therefore: iterations = us * 9
    volatile uint32_t count = us * 9;

    while (count--) {
        __asm volatile ("nop");
    }
}

/* Set DHT11 pin as output */
static void DHT11_SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/* Set DHT11 pin as input */
static void DHT11_SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  Initialize DHT11 (no timer needed)
  * @retval None
  */
void DHT11_Init(void) {
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    osDelay(1000);  // Wait 1 second for sensor stabilization
}

/**
  * @brief  Read data from DHT11 sensor
  * @param  data: Pointer to DHT11_Data structure
  * @retval DHT11_Status
  */
DHT11_Status DHT11_Read(DHT11_Data *data) {
    uint8_t buffer[5] = {0};
    uint32_t timeout;
    DHT11_Status result = DHT11_OK;

    // ========== CRITICAL SECTION START ==========
    // Disable ALL interrupts for accurate timing
    taskENTER_CRITICAL();

    // Step 1: Send start signal
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);

    // 18ms LOW signal (software delay for long duration is OK)
    for (volatile uint32_t i = 0; i < 130000; i++);

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us_precise(30);  // 20-40μs HIGH

    // Step 2: Switch to input mode
    DHT11_SetPinInput();

    // Wait for DHT11 response: LOW pulse (80μs)
    timeout = 200;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET && timeout--) {
        delay_us_precise(1);
    }
    if (timeout == 0) {
        result = DHT11_ERROR_NO_RESPONSE;
        goto cleanup;
    }

    // Wait for DHT11 response: HIGH pulse (80μs)
    timeout = 200;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET && timeout--) {
        delay_us_precise(1);
    }
    if (timeout == 0) {
        result = DHT11_ERROR_NO_RESPONSE;
        goto cleanup;
    }

    // Wait for data transmission start (DHT11 pulls low)
    timeout = 200;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET && timeout--) {
        delay_us_precise(1);
    }
    if (timeout == 0) {
        result = DHT11_ERROR_NO_RESPONSE;
        goto cleanup;
    }

    // Step 3: Read 40 bits (5 bytes)
    for (int i = 0; i < 5; i++) {
        for (int j = 7; j >= 0; j--) {
            // Wait for bit start (LOW to HIGH transition)
            timeout = 200;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_RESET && timeout--) {
                delay_us_precise(1);
            }
            if (timeout == 0) {
                result = DHT11_ERROR_TIMEOUT;
                goto cleanup;
            }

            // Wait 40μs and check if pin is still HIGH
            delay_us_precise(40);

            // If HIGH after 40μs → bit = 1, else bit = 0
            if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET) {
                buffer[i] |= (1 << j);
            }

            // Wait for bit end (HIGH to LOW transition)
            timeout = 200;
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET && timeout--) {
                delay_us_precise(1);
            }
            if (timeout == 0) {
                result = DHT11_ERROR_TIMEOUT;
                goto cleanup;
            }
        }
    }

    // Step 4: Verify checksum
    uint8_t checksum = buffer[0] + buffer[1] + buffer[2] + buffer[3];
    if (checksum != buffer[4]) {
        result = DHT11_ERROR_CHECKSUM;
        goto cleanup;
    }

    // Step 5: Store valid data
    data->humidity_int = buffer[0];
    data->humidity_dec = buffer[1];
    data->temperature_int = buffer[2];
    data->temperature_dec = buffer[3];
    data->checksum = buffer[4];

    last_reading = *data;

cleanup:
    // ========== CRITICAL SECTION END ==========
    taskEXIT_CRITICAL();

    // Restore pin to idle state
    DHT11_SetPinOutput();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);

    return result;
}

/**
  * @brief  Get last valid temperature reading
  * @retval Temperature in Celsius
  */
float DHT11_GetTemperature(void) {
    return (float)last_reading.temperature_int +
           (float)last_reading.temperature_dec / 10.0f;
}

/**
  * @brief  Get last valid humidity reading
  * @retval Humidity in percentage
  */
float DHT11_GetHumidity(void) {
    return (float)last_reading.humidity_int +
           (float)last_reading.humidity_dec / 10.0f;
}
