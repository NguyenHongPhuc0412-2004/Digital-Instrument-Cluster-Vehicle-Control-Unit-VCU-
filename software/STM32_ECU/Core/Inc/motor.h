/*
 * motor.h
 *
 *  Created on: Dec 15, 2024
 *      Author: ECU Team
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"
#include <stdbool.h>

/* Motor direction definitions */
typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_FORWARD,
    MOTOR_DIR_BACKWARD
} Motor_Direction;

/* Motor control structure */
typedef struct {
    TIM_HandleTypeDef *htim;    // PWM timer handle
    uint32_t channel;            // PWM channel
    GPIO_TypeDef *in1_port;      // IN1 GPIO port
    uint16_t in1_pin;            // IN1 GPIO pin
    GPIO_TypeDef *in2_port;      // IN2 GPIO port
    uint16_t in2_pin;            // IN2 GPIO pin
    Motor_Direction direction;   // Current direction
    uint16_t speed;              // Current speed (0-999)
    bool enabled;                // Motor enable state
} Motor_Handle;

/**
 * @brief Initialize motor driver
 * @param motor: Pointer to Motor_Handle structure
 * @param htim: PWM timer handle (TIM1)
 * @param channel: PWM channel (TIM_CHANNEL_1 for PA8)
 * @param in1_port: IN1 GPIO port (GPIOA)
 * @param in1_pin: IN1 GPIO pin (GPIO_PIN_11)
 * @param in2_port: IN2 GPIO port (GPIOA)
 * @param in2_pin: IN2 GPIO pin (GPIO_PIN_12)
 */
void Motor_Init(Motor_Handle *motor, TIM_HandleTypeDef *htim, uint32_t channel,
                GPIO_TypeDef *in1_port, uint16_t in1_pin,
                GPIO_TypeDef *in2_port, uint16_t in2_pin);

/**
 * @brief Set motor direction
 * @param motor: Pointer to Motor_Handle
 * @param direction: MOTOR_DIR_STOP, MOTOR_DIR_FORWARD, MOTOR_DIR_BACKWARD
 */
void Motor_SetDirection(Motor_Handle *motor, Motor_Direction direction);

/**
 * @brief Set motor speed (0-999)
 * @param motor: Pointer to Motor_Handle
 * @param speed: Speed value (0 = stop, 999 = max)
 */
void Motor_SetSpeed(Motor_Handle *motor, uint16_t speed);

/**
 * @brief Enable/disable motor
 * @param motor: Pointer to Motor_Handle
 * @param enable: true to enable, false to disable
 */
void Motor_Enable(Motor_Handle *motor, bool enable);

/**
 * @brief Get current motor speed
 * @param motor: Pointer to Motor_Handle
 * @return Current speed (0-999)
 */
uint16_t Motor_GetSpeed(Motor_Handle *motor);

/**
 * @brief Get current motor direction
 * @param motor: Pointer to Motor_Handle
 * @return Current direction
 */
Motor_Direction Motor_GetDirection(Motor_Handle *motor);

/**
 * @brief Update motor from ADC value (0-4095)
 * @param motor: Pointer to Motor_Handle
 * @param adc_value: ADC reading (0-4095)
 * @note Maps ADC value to speed (0-999)
 */
void Motor_UpdateFromADC(Motor_Handle *motor, uint16_t adc_value);

/**
 * @brief Emergency stop motor
 * @param motor: Pointer to Motor_Handle
 */
void Motor_Stop(Motor_Handle *motor);

#endif /* INC_MOTOR_H_ */
