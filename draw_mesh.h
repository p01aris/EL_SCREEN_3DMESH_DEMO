#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "el.h"
#include "simple_gfx.h"
#include "pico/time.h"

// ========== 屏幕参数 ==========
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 400
#define CENTER_X      (SCREEN_WIDTH / 2)
#define CENTER_Y      (SCREEN_HEIGHT / 2)
// ========== 网格参数 ==========
#define GRID_SIZE 64      // 每边点数（减小以避免内存问题）
#define RANGE     10.0f   // x, y 范围 [-2, 2]
#define SCALE     160/RANGE     // 投影缩放因子（像素/单位）
const uint16_t window_x = 8;

// ========== 顶点和边缓存 ==========
typedef struct {
    float x, y, z;
} Vertex3D;

// ========== 工具函数：画点 ==========
static void putpixel(unsigned char *buf, int x, int y, int c) {
    if (x >= 0 && x < SCR_WIDTH && y >= 0 && y < SCR_HEIGHT) {
        if (c) {
            buf[SCR_STRIDE * y + x / 8] |= 1 << (x % 8);
        } else {
            buf[SCR_STRIDE * y + x / 8] &= ~(1 << (x % 8));
        }
    }
}

// ========== 工具函数：画线（Bresenham算法） ==========
static void draw_line(unsigned char *buf, int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        putpixel(buf, x0, y0, 1);
        
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

// ========== 工具函数：清空屏幕 ==========
static void clear_screen(unsigned char *buf) {
    memset(buf, 0, SCR_STRIDE * SCR_HEIGHT);
}


static Vertex3D paraboloid_vertices[GRID_SIZE * GRID_SIZE];
static Vertex3D base_vertices[GRID_SIZE * GRID_SIZE];
static int edge_count;
static int edges[2 * GRID_SIZE * (GRID_SIZE - 1) * 2][2]; // 行线+列线

// 全局缓存数组，避免栈溢出
static Vertex3D rotated_paraboloid[GRID_SIZE * GRID_SIZE];
static Vertex3D rotated_base[GRID_SIZE * GRID_SIZE];

// ========== 角度（弧度） ==========
float angle_x = 0.15f;
float angle_y = 0.15f;
float angle_z = 0.15f;
float speed = 0.03f;

// 函数声明
void draw_ui_elements(unsigned char *buffer);

float calculate_zx(float u, float v) {
    float r = sqrt(u * u + v * v);  // 计算半径 r = √(u²+v²)
    
    if (r == 0.0f) {
        return 1.0f;  // sinc(0,0) 定义为 1
    } else {
        return 5*sin(r) / r;
    }
}

// ========== 工具函数：3D 旋转 ==========
void rotate_vertex(float x, float y, float z, float* out_x, float* out_y, float* out_z) {
    float cos_y = cosf(angle_y), sin_y = sinf(angle_y);
    float x1 = x * cos_y - z * sin_y;
    float z1 = x * sin_y + z * cos_y;
    float y1 = y;

    float cos_x = cosf(angle_x), sin_x = sinf(angle_x);
    float y2 = y1 * cos_x - z1 * sin_x;
    float z2 = y1 * sin_x + z1 * cos_x;
    float x2 = x1;

    float cos_z = cosf(angle_z), sin_z = sinf(angle_z);
    float x3 = x2 * cos_z - y2 * sin_z;
    float y3 = x2 * sin_z + y2 * cos_z;
    float z3 = z2;

    *out_x = x3;
    *out_y = y3;
    *out_z = z3;
}

// ========== 投影到屏幕坐标 ==========
void project_to_screen(float x, float y, float z, int16_t* screen_x, int16_t* screen_y) {
    // 正交投影：显示 X 和 Z，Y 作为深度（旋转后影响位置）
    *screen_x = CENTER_X + (int16_t)(x * SCALE);
    *screen_y = CENTER_Y - (int16_t)(z * SCALE); // Z向上，屏幕Y向下 → 用减号
}

// ========== 初始化网格 ==========
void init_mesh() {
    // 生成抛物面和底面顶点
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            float x = -RANGE + (2.0f * RANGE) * i / (GRID_SIZE - 1);
            float y = -RANGE + (2.0f * RANGE) * j / (GRID_SIZE - 1);
            float z_paraboloid = calculate_zx(x,y);
            float z_base = -2.0f;

            int idx = i * GRID_SIZE + j;
            paraboloid_vertices[idx] = (Vertex3D){x, y, z_paraboloid};
            base_vertices[idx] = (Vertex3D){x, y, z_base};
        }
    }

    // 生成边（行线 + 列线）
    edge_count = 0;

    // 行线（固定 j，i 变化）
    for (int j = 0; j < GRID_SIZE; j++) {
        for (int i = 0; i < GRID_SIZE - 1; i++) {
            int idx1 = i + j * GRID_SIZE;
            int idx2 = i + 1 + j * GRID_SIZE;
            edges[edge_count][0] = idx1;
            edges[edge_count][1] = idx2;
            edge_count++;
        }
    }

    // 列线（固定 i，j 变化）
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE - 1; j++) {
            int idx1 = i + j * GRID_SIZE;
            int idx2 = i + (j + 1) * GRID_SIZE;
            edges[edge_count][0] = idx1;
            edges[edge_count][1] = idx2;
            edge_count++;
        }
    }
}

void draw_frame() {
    // 获取当前的写缓冲区
    unsigned char *buffer = el_get_draw_buffer();
    clear_screen(buffer);

    // 旋转抛物面顶点
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        float rx, ry, rz;
        rotate_vertex(
            paraboloid_vertices[i].x,
            paraboloid_vertices[i].y,
            paraboloid_vertices[i].z,
            &rx, &ry, &rz
        );
        rotated_paraboloid[i] = (Vertex3D){rx, ry, rz};
    }

    // 旋转底面顶点
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++) {
        float rx, ry, rz;
        rotate_vertex(
            base_vertices[i].x,
            base_vertices[i].y,
            base_vertices[i].z,
            &rx, &ry, &rz
        );
        rotated_base[i] = (Vertex3D){rx, ry, rz};
    }

    for (int i = 0; i < edge_count; i++) {
        int v1_idx = edges[i][0];
        int v2_idx = edges[i][1];

        if (v1_idx >= GRID_SIZE * GRID_SIZE || v2_idx >= GRID_SIZE * GRID_SIZE) continue;

        int16_t x1, y1, x2, y2;

        project_to_screen(
            rotated_paraboloid[v1_idx].x,
            rotated_paraboloid[v1_idx].y,
            rotated_paraboloid[v1_idx].z,
            &x1, &y1
        );
        project_to_screen(
            rotated_paraboloid[v2_idx].x,
            rotated_paraboloid[v2_idx].y,
            rotated_paraboloid[v2_idx].z,
            &x2, &y2
        );

        if (x1 < 0 || x1 >= SCREEN_WIDTH || y1 < 0 || y1 >= SCREEN_HEIGHT) continue;
        if (x2 < 0 || x2 >= SCREEN_WIDTH || y2 < 0 || y2 >= SCREEN_HEIGHT) continue;

        // 绘制线条
        draw_line(buffer, x1, y1, x2, y2);
    }
    
    // 绘制文字和UI元素
    draw_ui_elements(buffer);
    
    // 更新旋转角度
    angle_x += speed;
    if (angle_x > 2 * M_PI) angle_x -= 2 * M_PI;
    
    angle_y += speed * 1.5f;
    if (angle_y > 2 * M_PI) angle_y -= 2 * M_PI;
    
    angle_z += speed * 0.7f;
    if (angle_z > 2 * M_PI) angle_z -= 2 * M_PI;
}

// 绘制UI元素
void draw_ui_elements(unsigned char *buffer) {
    // 绘制标题 "3D DEMO" 在屏幕顶部中央
    const char* title = "3D DEMO";
    int title_width = strlen(title) * 6 * 3; // 每字符6像素宽度 * 3倍放大
    int title_x = (SCREEN_WIDTH - title_width) / 2;
    int title_y = 10;
    gfx_draw_string(buffer, title_x, title_y, title, 3);
    
    // 绘制边框
    gfx_draw_rect(buffer, title_x - 10, title_y - 5, title_width + 20, 25, false);
    
    // 绘制一些装饰元素
    // 左上角圆圈
    gfx_draw_circle(buffer, 20, 20, 8, false);
    gfx_draw_circle(buffer, 20, 20, 4, true);
    
    // 右上角圆圈
    gfx_draw_circle(buffer, SCREEN_WIDTH - 20, 20, 8, false);
    gfx_draw_circle(buffer, SCREEN_WIDTH - 20, 20, 4, true);
    
    // 绘制一些信息文字在屏幕底部
    char info_text[64];
    
    // FPS计算
    static uint32_t frame_counter = 0;
    static absolute_time_t last_fps_time;
    static float current_fps = 0.0f;
    static bool first_run = true;
    
    frame_counter++;
    
    if (first_run) {
        last_fps_time = get_absolute_time();
        first_run = false;
    }
    
    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff = absolute_time_diff_us(last_fps_time, current_time);
    
    // 每500毫秒更新一次FPS，让显示更流畅
    if (time_diff >= 500000) { // 500毫秒 = 500,000微秒
        current_fps = (float)frame_counter * 1000000.0f / (float)time_diff;
        frame_counter = 0;
        last_fps_time = current_time;
    }
    
    // 如果FPS为0（初始状态），显示"--"
    if (current_fps == 0.0f) {
        snprintf(info_text, sizeof(info_text), "FPS: --");
    } else {
        snprintf(info_text, sizeof(info_text), "FPS: %.1f", current_fps);
    }
    gfx_draw_string(buffer, 10, SCREEN_HEIGHT - 20, info_text, 1);
    
    // 右下角显示网格大小信息
    snprintf(info_text, sizeof(info_text), "Grid: %dx%d", GRID_SIZE, GRID_SIZE);
    int info_width = strlen(info_text) * 6; // 每字符6像素宽度
    gfx_draw_string(buffer, SCREEN_WIDTH - info_width - 10, SCREEN_HEIGHT - 20, info_text, 1);
    
    // 在中下方显示控制信息
    const char* controls = "Raspberry Pi Pico 3D Graphics Demo";
    int controls_width = strlen(controls) * 6;
    int controls_x = (SCREEN_WIDTH - controls_width) / 2;
    gfx_draw_string(buffer, controls_x, SCREEN_HEIGHT - 40, controls, 1);
}

// 清理函数
void deinit_mesh() {
    // 无需特别清理
}
