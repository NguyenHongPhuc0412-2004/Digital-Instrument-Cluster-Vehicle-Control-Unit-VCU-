/*
 * buzzer.c
 *
 *  Created on: Dec 19, 2025
 *      Author: ECU Team
 */

#include "buzzer.h"
#include "cmsis_os.h"

/**
 * @brief Initialize buzzer with PWM
 * @note TIM3 prescaler = 799, period = 999
 *       Clock = 72MHz / (799+1) / (999+1) = 90Hz
 *       Để có tần số 2.5kHz cho buzzer, ta cần period = 35
 *       Nhưng vì dùng chung timer, ta giữ period = 999 và điều chỉnh duty cycle
 */
void Buzzer_Init(Buzzer_Handle *buzzer, TIM_HandleTypeDef *htim) {
    buzzer->htim = htim;
    buzzer->is_active = false;
    buzzer->is_beeping = false;
    buzzer->last_toggle_tick = 0;

    // Set PWM to 0 (buzzer off)
    buzzer->htim->Instance->BUZZER_CCR = 0;
}

/**
 * @brief Start buzzer alarm (beep pattern)
 */
void Buzzer_Start(Buzzer_Handle *buzzer) {
    buzzer->is_active = true;
    buzzer->is_beeping = true;
    buzzer->last_toggle_tick = HAL_GetTick();

    // Turn on PWM (50% duty cycle tạo âm 2.5kHz tốt)
    buzzer->htim->Instance->BUZZER_CCR = BUZZER_PWM_DUTY;
}

/**
 * @brief Stop buzzer alarm
 */
void Buzzer_Stop(Buzzer_Handle *buzzer) {
    buzzer->is_active = false;
    buzzer->is_beeping = false;

    // Turn off PWM
    buzzer->htim->Instance->BUZZER_CCR = 0;
}

/**
 * @brief Update buzzer state (call periodically in task)
 * @note Tạo pattern: BÍP 300ms → TẮT 200ms → Lặp lại
 */
void Buzzer_Update(Buzzer_Handle *buzzer) {
    if (!buzzer->is_active) {
        return;
    }

    uint32_t current_tick = HAL_GetTick();
    uint32_t elapsed = current_tick - buzzer->last_toggle_tick;

    if (buzzer->is_beeping) {
        // Đang bíp → Kiểm tra đã đủ 300ms chưa
        if (elapsed >= BUZZER_BEEP_ON_MS) {
            // Tắt buzzer
            buzzer->htim->Instance->BUZZER_CCR = 0;
            buzzer->is_beeping = false;
            buzzer->last_toggle_tick = current_tick;
        }
    } else {
        // Đang tắt → Kiểm tra đã đủ 200ms chưa
        if (elapsed >= BUZZER_BEEP_OFF_MS) {
            // Bật buzzer
            buzzer->htim->Instance->BUZZER_CCR = BUZZER_PWM_DUTY;
            buzzer->is_beeping = true;
            buzzer->last_toggle_tick = current_tick;
        }
    }
}

/**
 * @brief Check if buzzer alarm is active
 */
bool Buzzer_IsActive(Buzzer_Handle *buzzer) {
    return buzzer->is_active;
}
