//
// Grayscale Drawing Helper Functions
// 灰度绘图辅助函数
//
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "el.h"

// Set a single pixel with 2-bit grayscale value (0-3)
static inline void set_gray_pixel(unsigned char *gray_buf, int x, int y, uint8_t gray_value) {
    if (x < 0 || x >= SCR_WIDTH || y < 0 || y >= SCR_HEIGHT) return;
    if (gray_value > 3) gray_value = 3;
    
    int pixel_index = y * SCR_WIDTH + x;
    int byte_index = pixel_index / 4; // 4 pixels per byte (2 bits each)
    int pixel_in_byte = pixel_index % 4;
    int bit_shift = (3 - pixel_in_byte) * 2; // 6, 4, 2, 0
    
    // Clear the 2 bits for this pixel
    gray_buf[byte_index] &= ~(0x03 << bit_shift);
    // Set the new grayscale value
    gray_buf[byte_index] |= (gray_value << bit_shift);
}

// Get grayscale value of a pixel
static inline uint8_t get_gray_pixel(unsigned char *gray_buf, int x, int y) {
    if (x < 0 || x >= SCR_WIDTH || y < 0 || y >= SCR_HEIGHT) return 0;
    
    int pixel_index = y * SCR_WIDTH + x;
    int byte_index = pixel_index / 4;
    int pixel_in_byte = pixel_index % 4;
    int bit_shift = (3 - pixel_in_byte) * 2;
    
    return (gray_buf[byte_index] >> bit_shift) & 0x03;
}

// Fill entire screen with a grayscale value
static inline void clear_gray_screen(unsigned char *gray_buf, uint8_t gray_value) {
    if (gray_value > 3) gray_value = 3;
    
    // Fill with repeated pattern
    uint8_t pattern = (gray_value << 6) | (gray_value << 4) | (gray_value << 2) | gray_value;
    memset(gray_buf, pattern, SCR_STRIDE * SCR_HEIGHT * 2);
}

// Fill a rectangle with a grayscale value
static inline void fill_rect_gray(unsigned char *gray_buf, int x, int y, int w, int h, uint8_t gray_value) {
    for (int j = y; j < y + h && j < SCR_HEIGHT; j++) {
        for (int i = x; i < x + w && i < SCR_WIDTH; i++) {
            set_gray_pixel(gray_buf, i, j, gray_value);
        }
    }
}

// Draw a line with grayscale value (Bresenham's algorithm)
static inline void draw_line_gray(unsigned char *gray_buf, int x0, int y0, int x1, int y1, uint8_t gray_value) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        set_gray_pixel(gray_buf, x0, y0, gray_value);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw a circle with grayscale value (Midpoint algorithm)
static inline void draw_circle_gray(unsigned char *gray_buf, int cx, int cy, int radius, uint8_t gray_value) {
    int x = radius;
    int y = 0;
    int err = 0;
    
    while (x >= y) {
        set_gray_pixel(gray_buf, cx + x, cy + y, gray_value);
        set_gray_pixel(gray_buf, cx + y, cy + x, gray_value);
        set_gray_pixel(gray_buf, cx - y, cy + x, gray_value);
        set_gray_pixel(gray_buf, cx - x, cy + y, gray_value);
        set_gray_pixel(gray_buf, cx - x, cy - y, gray_value);
        set_gray_pixel(gray_buf, cx - y, cy - x, gray_value);
        set_gray_pixel(gray_buf, cx + y, cy - x, gray_value);
        set_gray_pixel(gray_buf, cx + x, cy - y, gray_value);
        
        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// Fill a circle with grayscale value
static inline void fill_circle_gray(unsigned char *gray_buf, int cx, int cy, int radius, uint8_t gray_value) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                set_gray_pixel(gray_buf, cx + x, cy + y, gray_value);
            }
        }
    }
}

// Draw rectangle outline with grayscale value
static inline void draw_rect_gray(unsigned char *gray_buf, int x, int y, int w, int h, uint8_t gray_value) {
    // Top and bottom edges
    for (int i = x; i < x + w; i++) {
        set_gray_pixel(gray_buf, i, y, gray_value);
        set_gray_pixel(gray_buf, i, y + h - 1, gray_value);
    }
    // Left and right edges
    for (int j = y; j < y + h; j++) {
        set_gray_pixel(gray_buf, x, j, gray_value);
        set_gray_pixel(gray_buf, x + w - 1, j, gray_value);
    }
}
