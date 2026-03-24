/*
 * buzzer.h
 *
 *  Created on: Dec 19, 2025
 *      Author: ECU Team
 */

#ifndef INC_BUZZER_H_
#define INC_BUZZER_H_

#include "main.h"
#include <stdbool.h>

/* Buzzer PWM Configuration */
#define BUZZER_TIM              &htim3
#define BUZZER_CHANNEL          TIM_CHANNEL_3  // PB0 - TIM3_CH3
#define BUZZER_CCR              CCR3           // Compare register cho CH3

/* Buzzer timing patterns */
#define BUZZER_BEEP_ON_MS       300U   // Bíp 300ms
#define BUZZER_BEEP_OFF_MS      200U   // Tắt 200ms

/* PWM duty cycle for buzzer */
#define BUZZER_PWM_DUTY         500U   // 50% duty cycle (0-999)

/* Buzzer state */
typedef struct {
    bool is_active;              // Buzzer alarm đang bật/tắt
    bool is_beeping;             // Đang trong chu kỳ bíp hay tắt
    uint32_t last_toggle_tick;   // Tick cuối cùng toggle
    TIM_HandleTypeDef *htim;     // Timer handle
} Buzzer_Handle;

/**
 * @brief Initialize buzzer with PWM
 * @param buzzer: Pointer to Buzzer_Handle
 * @param htim: Timer handle (TIM3)
 */
void Buzzer_Init(Buzzer_Handle *buzzer, TIM_HandleTypeDef *htim);

/**
 * @brief Start buzzer alarm (beep pattern)
 * @param buzzer: Pointer to Buzzer_Handle
 */
void Buzzer_Start(Buzzer_Handle *buzzer);

/**
 * @brief Stop buzzer alarm
 * @param buzzer: Pointer to Buzzer_Handle
 */
void Buzzer_Stop(Buzzer_Handle *buzzer);

/**
 * @brief Update buzzer state (call periodically in task)
 * @param buzzer: Pointer to Buzzer_Handle
 * @note Handles the beep-pause-beep pattern
 */
void Buzzer_Update(Buzzer_Handle *buzzer);

/**
 * @brief Check if buzzer alarm is active
 * @param buzzer: Pointer to Buzzer_Handle
 * @return true if active, false otherwise
 */
bool Buzzer_IsActive(Buzzer_Handle *buzzer);

#endif /* INC_BUZZER_H_ */
