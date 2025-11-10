//
// Grayscale Test Pattern Generator
// 测试4级灰度显示效果
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "el.h"

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// Set a 2-bit grayscale pixel value (0-3)
void set_gray_pixel(unsigned char *gray_buf, int x, int y, uint8_t gray_value) {
    if (x < 0 || x >= SCR_WIDTH || y < 0 || y >= SCR_HEIGHT) return;
    if (gray_value > 3) gray_value = 3;
    
    int pixel_index = y * SCR_WIDTH + x;
    int byte_index = pixel_index / 4; // 4 pixels per byte
    int pixel_in_byte = pixel_index % 4;
    int bit_shift = (3 - pixel_in_byte) * 2; // 6, 4, 2, 0
    
    // Clear the 2 bits for this pixel
    gray_buf[byte_index] &= ~(0x03 << bit_shift);
    // Set the new value
    gray_buf[byte_index] |= (gray_value << bit_shift);
}

// Fill a rectangle with a grayscale value
void fill_rect_gray(unsigned char *gray_buf, int x, int y, int w, int h, uint8_t gray_value) {
    for (int j = y; j < y + h && j < SCR_HEIGHT; j++) {
        for (int i = x; i < x + w && i < SCR_WIDTH; i++) {
            set_gray_pixel(gray_buf, i, j, gray_value);
        }
    }
}

// Draw test pattern 1: Vertical gradient bars
void draw_gradient_bars(unsigned char *gray_buf) {
    // Clear buffer
    memset(gray_buf, 0, SCR_STRIDE * SCR_HEIGHT * 2);
    
    // 4 vertical bars, each showing a different gray level
    int bar_width = SCR_WIDTH / 4;
    
    fill_rect_gray(gray_buf, 0 * bar_width, 0, bar_width, SCR_HEIGHT, 0); // 0% - Black
    fill_rect_gray(gray_buf, 1 * bar_width, 0, bar_width, SCR_HEIGHT, 1); // 25%
    fill_rect_gray(gray_buf, 2 * bar_width, 0, bar_width, SCR_HEIGHT, 2); // 50%
    fill_rect_gray(gray_buf, 3 * bar_width, 0, bar_width, SCR_HEIGHT, 3); // 75-100% - White
    
    printf("Test Pattern: 4 vertical gradient bars (0%%, 25%%, 50%%, 75-100%%)\n");
}

// Draw test pattern 2: Horizontal gradient
void draw_horizontal_gradient(unsigned char *gray_buf) {
    memset(gray_buf, 0, SCR_STRIDE * SCR_HEIGHT * 2);
    
    int section_height = SCR_HEIGHT / 4;
    
    fill_rect_gray(gray_buf, 0, 0 * section_height, SCR_WIDTH, section_height, 0);
    fill_rect_gray(gray_buf, 0, 1 * section_height, SCR_WIDTH, section_height, 1);
    fill_rect_gray(gray_buf, 0, 2 * section_height, SCR_WIDTH, section_height, 2);
    fill_rect_gray(gray_buf, 0, 3 * section_height, SCR_WIDTH, section_height, 3);
    
    printf("Test Pattern: 4 horizontal gradient bars\n");
}

// Draw test pattern 3: Checkerboard pattern with different gray levels
void draw_checkerboard(unsigned char *gray_buf) {
    memset(gray_buf, 0, SCR_STRIDE * SCR_HEIGHT * 2);
    
    int cell_size = 40;
    
    for (int y = 0; y < SCR_HEIGHT; y++) {
        for (int x = 0; x < SCR_WIDTH; x++) {
            int cell_x = x / cell_size;
            int cell_y = y / cell_size;
            uint8_t gray = ((cell_x + cell_y) % 4);
            set_gray_pixel(gray_buf, x, y, gray);
        }
    }
    
    printf("Test Pattern: Checkerboard with 4 gray levels\n");
}

// Draw test pattern 4: Concentric rectangles
void draw_concentric_rects(unsigned char *gray_buf) {
    memset(gray_buf, 0, SCR_STRIDE * SCR_HEIGHT * 2);
    
    int border = 50;
    for (int level = 0; level < 4; level++) {
        int x = level * border;
        int y = level * border;
        int w = SCR_WIDTH - 2 * level * border;
        int h = SCR_HEIGHT - 2 * level * border;
        
        if (w > 0 && h > 0) {
            fill_rect_gray(gray_buf, x, y, w, h, 3 - level);
        }
    }
    
    printf("Test Pattern: Concentric rectangles\n");
}

// Draw test pattern 5: Full screen of each level (cycling)
void draw_solid_level(unsigned char *gray_buf, uint8_t level) {
    memset(gray_buf, 0, SCR_STRIDE * SCR_HEIGHT * 2);
    fill_rect_gray(gray_buf, 0, 0, SCR_WIDTH, SCR_HEIGHT, level);
    printf("Test Pattern: Solid level %d\n", level);
}

// Draw test pattern 6: Grid pattern
void draw_grid(unsigned char *gray_buf) {
    memset(gray_buf, 0, SCR_STRIDE * SCR_HEIGHT * 2);
    
    // Background
    fill_rect_gray(gray_buf, 0, 0, SCR_WIDTH, SCR_HEIGHT, 0);
    
    // Draw grid lines with level 3
    int spacing = 80;
    for (int x = 0; x < SCR_WIDTH; x += spacing) {
        for (int y = 0; y < SCR_HEIGHT; y++) {
            set_gray_pixel(gray_buf, x, y, 3);
        }
    }
    for (int y = 0; y < SCR_HEIGHT; y += spacing) {
        for (int x = 0; x < SCR_WIDTH; x++) {
            set_gray_pixel(gray_buf, x, y, 3);
        }
    }
    
    printf("Test Pattern: Grid\n");
}

int main() {
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(400000, true);
    
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Blink LED to indicate start
    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    
    watchdog_enable(8000, 1);
    
    printf("\n=== 4-Level Grayscale Test ===\n");
    printf("Screen: %dx%d @ 120Hz\n", SCR_WIDTH, SCR_HEIGHT);
    printf("Effective: 30Hz with 4-level grayscale\n");
    printf("Pattern cycles every 5 seconds\n\n");
    
    el_start();
    
    unsigned char *gray_buf = el_get_gray_buffer();
    
    int pattern = 0;
    int solid_level = 0;
    uint32_t last_switch_time = 0;
    
    while (1) {
        watchdog_update();
        
        uint32_t current_time = to_ms_since_boot(get_absolute_time());
        
        // Switch pattern every 5 seconds
        if (current_time - last_switch_time >= 5000) {
            last_switch_time = current_time;
            
            printf("\n--- Switching to pattern %d ---\n", pattern);
            
            switch (pattern) {
                case 0:
                    draw_gradient_bars(gray_buf);
                    break;
                case 1:
                    draw_horizontal_gradient(gray_buf);
                    break;
                case 2:
                    draw_checkerboard(gray_buf);
                    break;
                case 3:
                    draw_concentric_rects(gray_buf);
                    break;
                case 4:
                    draw_grid(gray_buf);
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                    draw_solid_level(gray_buf, solid_level);
                    solid_level = (solid_level + 1) % 4;
                    break;
            }
            
            // Convert gray buffer to binary frames
            el_update_frame();
            
            // Swap buffer (wait for 4-frame cycle to complete)
            el_swap_buffer();
            
            pattern = (pattern + 1) % 9;
            
            // Blink LED
            gpio_put(LED_PIN, 1);
            sleep_ms(10);
            gpio_put(LED_PIN, 0);
        }
        
        sleep_ms(100);
    }
    
    return 0;
}
