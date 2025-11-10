#ifndef SIMPLE_GFX_H
#define SIMPLE_GFX_H

#include <stdint.h>
#include <stdbool.h>
#include "el.h"

// 简单5x7点阵字体数据
static const uint8_t font5x7[][5] = {
    // 空格 (32)
    [' ' - ' '] = {0x00, 0x00, 0x00, 0x00, 0x00},
    // 数字 0-9
    ['0' - ' '] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1' - ' '] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2' - ' '] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3' - ' '] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4' - ' '] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5' - ' '] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6' - ' '] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7' - ' '] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8' - ' '] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9' - ' '] = {0x06, 0x49, 0x49, 0x29, 0x1E},
    // 字母 A-Z
    ['A' - ' '] = {0x7E, 0x11, 0x11, 0x11, 0x7E},
    ['B' - ' '] = {0x7F, 0x49, 0x49, 0x49, 0x36},
    ['C' - ' '] = {0x3E, 0x41, 0x41, 0x41, 0x22},
    ['D' - ' '] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
    ['E' - ' '] = {0x7F, 0x49, 0x49, 0x41, 0x41},
    ['F' - ' '] = {0x7F, 0x09, 0x09, 0x01, 0x01},
    ['G' - ' '] = {0x3E, 0x41, 0x49, 0x49, 0x7A},
    ['H' - ' '] = {0x7F, 0x08, 0x08, 0x08, 0x7F},
    ['I' - ' '] = {0x00, 0x41, 0x7F, 0x41, 0x00},
    ['J' - ' '] = {0x20, 0x40, 0x41, 0x3F, 0x01},
    ['K' - ' '] = {0x7F, 0x08, 0x14, 0x22, 0x41},
    ['L' - ' '] = {0x7F, 0x40, 0x40, 0x40, 0x40},
    ['M' - ' '] = {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    ['N' - ' '] = {0x7F, 0x04, 0x08, 0x10, 0x7F},
    ['O' - ' '] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
    ['P' - ' '] = {0x7F, 0x09, 0x09, 0x09, 0x06},
    ['Q' - ' '] = {0x3E, 0x41, 0x51, 0x21, 0x5E},
    ['R' - ' '] = {0x7F, 0x09, 0x19, 0x29, 0x46},
    ['S' - ' '] = {0x46, 0x49, 0x49, 0x49, 0x31},
    ['T' - ' '] = {0x01, 0x01, 0x7F, 0x01, 0x01},
    ['U' - ' '] = {0x3F, 0x40, 0x40, 0x40, 0x3F},
    ['V' - ' '] = {0x1F, 0x20, 0x40, 0x20, 0x1F},
    ['W' - ' '] = {0x3F, 0x40, 0x38, 0x40, 0x3F},
    ['X' - ' '] = {0x63, 0x14, 0x08, 0x14, 0x63},
    ['Y' - ' '] = {0x07, 0x08, 0x70, 0x08, 0x07},
    ['Z' - ' '] = {0x61, 0x51, 0x49, 0x45, 0x43},
    // 小写字母 a-z
    ['a' - ' '] = {0x20, 0x54, 0x54, 0x54, 0x78},
    ['b' - ' '] = {0x7F, 0x48, 0x44, 0x44, 0x38},
    ['c' - ' '] = {0x38, 0x44, 0x44, 0x44, 0x20},
    ['d' - ' '] = {0x38, 0x44, 0x44, 0x48, 0x7F},
    ['e' - ' '] = {0x38, 0x54, 0x54, 0x54, 0x18},
    ['f' - ' '] = {0x08, 0x7E, 0x09, 0x01, 0x02},
    ['g' - ' '] = {0x0C, 0x52, 0x52, 0x52, 0x3E},
    ['h' - ' '] = {0x7F, 0x08, 0x04, 0x04, 0x78},
    ['i' - ' '] = {0x00, 0x44, 0x7D, 0x40, 0x00},
    ['j' - ' '] = {0x20, 0x40, 0x44, 0x3D, 0x00},
    ['k' - ' '] = {0x7F, 0x10, 0x28, 0x44, 0x00},
    ['l' - ' '] = {0x00, 0x41, 0x7F, 0x40, 0x00},
    ['m' - ' '] = {0x7C, 0x04, 0x18, 0x04, 0x78},
    ['n' - ' '] = {0x7C, 0x08, 0x04, 0x04, 0x78},
    ['o' - ' '] = {0x38, 0x44, 0x44, 0x44, 0x38},
    ['p' - ' '] = {0x7C, 0x14, 0x14, 0x14, 0x08},
    ['q' - ' '] = {0x08, 0x14, 0x14, 0x18, 0x7C},
    ['r' - ' '] = {0x7C, 0x08, 0x04, 0x04, 0x08},
    ['s' - ' '] = {0x48, 0x54, 0x54, 0x54, 0x20},
    ['t' - ' '] = {0x04, 0x3F, 0x44, 0x40, 0x20},
    ['u' - ' '] = {0x3C, 0x40, 0x40, 0x20, 0x7C},
    ['v' - ' '] = {0x1C, 0x20, 0x40, 0x20, 0x1C},
    ['w' - ' '] = {0x3C, 0x40, 0x30, 0x40, 0x3C},
    ['x' - ' '] = {0x44, 0x28, 0x10, 0x28, 0x44},
    ['y' - ' '] = {0x0C, 0x50, 0x50, 0x50, 0x3C},
    ['z' - ' '] = {0x44, 0x64, 0x54, 0x4C, 0x44},
};

// 内联函数实现
static inline void gfx_set_pixel(unsigned char *buf, int x, int y, bool color) {
    if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
        if (color) {
            buf[SCR_STRIDE * y + x / 8] |= 1 << (x % 8);
        } else {
            buf[SCR_STRIDE * y + x / 8] &= ~(1 << (x % 8));
        }
    }
}

static inline bool gfx_get_pixel(unsigned char *buf, int x, int y) {
    if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
        return (buf[SCR_STRIDE * y + x / 8] & (1 << (x % 8))) != 0;
    }
    return false;
}

// 绘制单个字符
static inline void gfx_draw_char(unsigned char *buf, int x, int y, char c, int size) {
    if (c < ' ' || c > 'z') return; // 超出字体范围
    
    const uint8_t *char_data = font5x7[c - ' '];
    
    for (int col = 0; col < 5; col++) {
        uint8_t line = char_data[col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                // 根据size参数绘制放大的像素
                for (int sx = 0; sx < size; sx++) {
                    for (int sy = 0; sy < size; sy++) {
                        gfx_set_pixel(buf, x + col * size + sx, y + row * size + sy, true);
                    }
                }
            }
        }
    }
}

// 绘制字符串
static inline void gfx_draw_string(unsigned char *buf, int x, int y, const char *str, int size) {
    int cursor_x = x;
    while (*str) {
        gfx_draw_char(buf, cursor_x, y, *str, size);
        cursor_x += 6 * size; // 5像素宽度 + 1像素间距
        str++;
    }
}

// 绘制矩形
static inline void gfx_draw_rect(unsigned char *buf, int x, int y, int w, int h, bool filled) {
    if (filled) {
        for (int py = y; py < y + h; py++) {
            for (int px = x; px < x + w; px++) {
                gfx_set_pixel(buf, px, py, true);
            }
        }
    } else {
        // 绘制边框
        for (int px = x; px < x + w; px++) {
            gfx_set_pixel(buf, px, y, true);         // 上边
            gfx_set_pixel(buf, px, y + h - 1, true); // 下边
        }
        for (int py = y; py < y + h; py++) {
            gfx_set_pixel(buf, x, py, true);         // 左边
            gfx_set_pixel(buf, x + w - 1, py, true); // 右边
        }
    }
}

// 绘制圆形（使用中点圆算法）
static inline void gfx_draw_circle(unsigned char *buf, int cx, int cy, int radius, bool filled) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        if (filled) {
            // 绘制填充圆
            for (int px = cx - x; px <= cx + x; px++) {
                gfx_set_pixel(buf, px, cy + y, true);
                gfx_set_pixel(buf, px, cy - y, true);
            }
            for (int px = cx - y; px <= cx + y; px++) {
                gfx_set_pixel(buf, px, cy + x, true);
                gfx_set_pixel(buf, px, cy - x, true);
            }
        } else {
            // 绘制圆周
            gfx_set_pixel(buf, cx + x, cy + y, true);
            gfx_set_pixel(buf, cx + y, cy + x, true);
            gfx_set_pixel(buf, cx - y, cy + x, true);
            gfx_set_pixel(buf, cx - x, cy + y, true);
            gfx_set_pixel(buf, cx - x, cy - y, true);
            gfx_set_pixel(buf, cx - y, cy - x, true);
            gfx_set_pixel(buf, cx + y, cy - x, true);
            gfx_set_pixel(buf, cx + x, cy - y, true);
        }

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

#endif // SIMPLE_GFX_H