//
// Simple Grayscale Animation Demo
// 简单的灰度动画演示
//
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "el.h"
#include "gray_gfx.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// Animation state
typedef struct {
    float angle;
    int center_x;
    int center_y;
} AnimState;

// Draw animated grayscale circles
void draw_animated_circles(unsigned char *gray_buf, AnimState *state) {
    clear_gray_screen(gray_buf, 0);
    
    // Draw multiple circles with different gray levels
    for (int i = 0; i < 4; i++) {
        float offset_angle = state->angle + (i * M_PI / 2.0f);
        int x = state->center_x + (int)(100.0f * cosf(offset_angle));
        int y = state->center_y + (int)(80.0f * sinf(offset_angle));
        
        fill_circle_gray(gray_buf, x, y, 40, i); // Different gray level for each
    }
    
    state->angle += 0.05f;
}

// Draw animated wave pattern
void draw_wave_pattern(unsigned char *gray_buf, AnimState *state) {
    clear_gray_screen(gray_buf, 0);
    
    for (int y = 0; y < SCR_HEIGHT; y++) {
        for (int x = 0; x < SCR_WIDTH; x++) {
            float wave = sinf((x + state->angle * 10.0f) * 0.02f) * 
                        sinf((y + state->angle * 10.0f) * 0.02f);
            uint8_t gray = (uint8_t)((wave + 1.0f) * 1.5f); // Map to 0-3
            if (gray > 3) gray = 3;
            set_gray_pixel(gray_buf, x, y, gray);
        }
    }
    
    state->angle += 0.1f;
}

// Draw moving gradient
void draw_moving_gradient(unsigned char *gray_buf, AnimState *state) {
    clear_gray_screen(gray_buf, 0);
    
    int offset = (int)(state->angle * 10.0f) % (SCR_WIDTH + 200);
    
    for (int x = 0; x < SCR_WIDTH; x++) {
        int dist = abs(x - offset + 100);
        uint8_t gray = 3 - (dist / 50);
        if (gray > 3) gray = 0;
        
        fill_rect_gray(gray_buf, x, 0, 1, SCR_HEIGHT, gray);
    }
    
    state->angle += 0.1f;
}

// Draw rotating squares
void draw_rotating_squares(unsigned char *gray_buf, AnimState *state) {
    clear_gray_screen(gray_buf, 0);
    
    for (int i = 0; i < 4; i++) {
        int size = 60 + i * 30;
        int x = state->center_x - size / 2;
        int y = state->center_y - size / 2;
        
        draw_rect_gray(gray_buf, x, y, size, size, i);
    }
    
    // Draw some filled squares that rotate
    float angle = state->angle;
    for (int i = 0; i < 3; i++) {
        float a = angle + i * (M_PI * 2.0f / 3.0f);
        int x = state->center_x + (int)(150.0f * cosf(a));
        int y = state->center_y + (int)(120.0f * sinf(a));
        fill_rect_gray(gray_buf, x - 20, y - 20, 40, 40, i + 1);
    }
    
    state->angle += 0.03f;
}

// Draw pulsing pattern
void draw_pulse_pattern(unsigned char *gray_buf, AnimState *state) {
    clear_gray_screen(gray_buf, 0);
    
    float pulse = (sinf(state->angle) + 1.0f) / 2.0f; // 0 to 1
    
    for (int ring = 0; ring < 5; ring++) {
        int radius = (int)(30.0f + ring * 40.0f + pulse * 30.0f);
        uint8_t gray = (uint8_t)(pulse * 3.0f);
        
        draw_circle_gray(gray_buf, state->center_x, state->center_y, radius, gray);
    }
    
    state->angle += 0.1f;
}

int main() {
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(400000, true);
    
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Blink LED
    for (int i = 0; i < 3; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
    }
    
    watchdog_enable(8000, 1);
    
    printf("\n=== Grayscale Animation Demo ===\n");
    printf("30Hz 4-level grayscale display\n");
    printf("Animations cycle every 10 seconds\n\n");
    
    el_start();
    
    unsigned char *gray_buf = el_get_gray_buffer();
    
    AnimState state = {
        .angle = 0.0f,
        .center_x = SCR_WIDTH / 2,
        .center_y = SCR_HEIGHT / 2
    };
    
    int animation = 0;
    uint32_t last_switch = 0;
    uint32_t frame_count = 0;
    
    printf("Starting animation loop...\n");
    
    while (1) {
        watchdog_update();
        
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // Switch animation every 10 seconds
        if (current_time - last_switch >= 10000) {
            last_switch = current_time;
            animation = (animation + 1) % 5;
            state.angle = 0.0f;
            
            printf("\n--- Animation %d ---\n", animation);
            
            // Blink LED
            gpio_put(LED_PIN, 1);
            sleep_ms(50);
            gpio_put(LED_PIN, 0);
        }
        
        // Draw current animation
        switch (animation) {
            case 0:
                draw_animated_circles(gray_buf, &state);
                break;
            case 1:
                draw_wave_pattern(gray_buf, &state);
                break;
            case 2:
                draw_moving_gradient(gray_buf, &state);
                break;
            case 3:
                draw_rotating_squares(gray_buf, &state);
                break;
            case 4:
                draw_pulse_pattern(gray_buf, &state);
                break;
        }
        
        // Convert and display
        el_update_frame();
        el_swap_buffer();
        
        frame_count++;
        if (frame_count % 300 == 0) {
            printf("Frame: %d, Animation: %d\n", frame_count, animation);
        }
        
        // Target 30 FPS (33ms per frame)
        sleep_ms(33);
    }
    
    return 0;
}
