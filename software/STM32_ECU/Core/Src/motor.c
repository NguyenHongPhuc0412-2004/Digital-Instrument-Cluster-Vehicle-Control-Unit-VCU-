/*
 * motor.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ECU Team
 */

#include "motor.h"

/**
 * @brief Initialize motor driver
 */
void Motor_Init(Motor_Handle *motor, TIM_HandleTypeDef *htim, uint32_t channel,
                GPIO_TypeDef *in1_port, uint16_t in1_pin,
                GPIO_TypeDef *in2_port, uint16_t in2_pin) {
    motor->htim = htim;
    motor->channel = channel;
    motor->in1_port = in1_port;
    motor->in1_pin = in1_pin;
    motor->in2_port = in2_port;
    motor->in2_pin = in2_pin;
    motor->direction = MOTOR_DIR_STOP;
    motor->speed = 0;
    motor->enabled = false;

    // Set initial state: motor stopped
    HAL_GPIO_WritePin(motor->in1_port, motor->in1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->in2_port, motor->in2_pin, GPIO_PIN_RESET);

    // Set PWM to 0
    switch (channel) {
        case TIM_CHANNEL_1:
            motor->htim->Instance->CCR1 = 0;
            break;
        case TIM_CHANNEL_2:
            motor->htim->Instance->CCR2 = 0;
            break;
        case TIM_CHANNEL_3:
            motor->htim->Instance->CCR3 = 0;
            break;
        case TIM_CHANNEL_4:
            motor->htim->Instance->CCR4 = 0;
            break;
    }
}

/**
 * @brief Set motor direction
 */
void Motor_SetDirection(Motor_Handle *motor, Motor_Direction direction) {
    motor->direction = direction;

    switch (direction) {
        case MOTOR_DIR_STOP:
            HAL_GPIO_WritePin(motor->in1_port, motor->in1_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor->in2_port, motor->in2_pin, GPIO_PIN_RESET);
            break;

        case MOTOR_DIR_FORWARD:
            HAL_GPIO_WritePin(motor->in1_port, motor->in1_pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(motor->in2_port, motor->in2_pin, GPIO_PIN_SET);
            break;

        case MOTOR_DIR_BACKWARD:
            HAL_GPIO_WritePin(motor->in1_port, motor->in1_pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(motor->in2_port, motor->in2_pin, GPIO_PIN_RESET);
            break;
    }
}

/**
 * @brief Set motor speed (0-999)
 */
void Motor_SetSpeed(Motor_Handle *motor, uint16_t speed) {
    // Clamp speed to valid range
    if (speed > 999) {
        speed = 999;
    }

    motor->speed = speed;

    // Update PWM duty cycle only if motor is enabled
    if (motor->enabled && motor->direction != MOTOR_DIR_STOP) {
        switch (motor->channel) {
            case TIM_CHANNEL_1:
                motor->htim->Instance->CCR1 = speed;
                break;
            case TIM_CHANNEL_2:
                motor->htim->Instance->CCR2 = speed;
                break;
            case TIM_CHANNEL_3:
                motor->htim->Instance->CCR3 = speed;
                break;
            case TIM_CHANNEL_4:
                motor->htim->Instance->CCR4 = speed;
                break;
        }
    } else {
        // Motor disabled or stopped, set PWM to 0
        switch (motor->channel) {
            case TIM_CHANNEL_1:
                motor->htim->Instance->CCR1 = 0;
                break;
            case TIM_CHANNEL_2:
                motor->htim->Instance->CCR2 = 0;
                break;
            case TIM_CHANNEL_3:
                motor->htim->Instance->CCR3 = 0;
                break;
            case TIM_CHANNEL_4:
                motor->htim->Instance->CCR4 = 0;
                break;
        }
    }
}

/**
 * @brief Enable/disable motor
 */
void Motor_Enable(Motor_Handle *motor, bool enable) {
    motor->enabled = enable;

    if (!enable) {
        // Disable motor: stop direction and set PWM to 0
        Motor_SetDirection(motor, MOTOR_DIR_STOP);
        Motor_SetSpeed(motor, 0);
    } else {
        // Enable motor: apply current speed
        Motor_SetSpeed(motor, motor->speed);
    }
}

/**
 * @brief Get current motor speed
 */
uint16_t Motor_GetSpeed(Motor_Handle *motor) {
    return motor->speed;
}

/**
 * @brief Get current motor direction
 */
Motor_Direction Motor_GetDirection(Motor_Handle *motor) {
    return motor->direction;
}

/**
 * @brief Update motor from ADC value (0-4095)
 * Maps ADC 0-4095 to speed 0-999
 */
void Motor_UpdateFromADC(Motor_Handle *motor, uint16_t adc_value) {
    // Map ADC value (0-4095) to motor speed (0-999)
    uint16_t speed = (adc_value * 999UL) / 4095UL;
    Motor_SetSpeed(motor, speed);
}

/**
 * @brief Emergency stop motor
 */
void Motor_Stop(Motor_Handle *motor) {
    motor->enabled = false;
    motor->direction = MOTOR_DIR_STOP;
    motor->speed = 0;

    HAL_GPIO_WritePin(motor->in1_port, motor->in1_pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(motor->in2_port, motor->in2_pin, GPIO_PIN_RESET);

    switch (motor->channel) {
        case TIM_CHANNEL_1:
            motor->htim->Instance->CCR1 = 0;
            break;
        case TIM_CHANNEL_2:
            motor->htim->Instance->CCR2 = 0;
            break;
        case TIM_CHANNEL_3:
            motor->htim->Instance->CCR3 = 0;
            break;
        case TIM_CHANNEL_4:
            motor->htim->Instance->CCR4 = 0;
            break;
    }
}
