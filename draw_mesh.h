#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "el.h"

// ========== 屏幕参数 ==========
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 400
#define CENTER_X      (SCREEN_WIDTH / 2)
#define CENTER_Y      (SCREEN_HEIGHT / 2)
// ========== 网格参数 ==========
#define GRID_SIZE 32      // 每边点数（不要太大会卡）
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


Vertex3D paraboloid_vertices[GRID_SIZE * GRID_SIZE];
Vertex3D base_vertices[GRID_SIZE * GRID_SIZE];
int edge_count;
int edges[2 * GRID_SIZE * (GRID_SIZE - 1) * 2][2]; // 行线+列线

// ========== 角度（弧度） ==========
float angle_x = 0.15f;
float angle_y = 0.15f;
float angle_z = 0.15f;
float speed = 0.03f;

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
    // 清屏（假设你有 clearScreen() 或 fill 命令）
    // 如果没有，用 drawLine 画黑线效率低，建议直接操作 buffer
    // 此处假设你有 clearScreen() 函数，或自行实现
    // clearScreen(); // 伪代码
    unsigned char *buffer = el_swap_buffer();
    clear_screen(buffer);
    // 旋转后顶点缓存
    Vertex3D rotated_paraboloid[GRID_SIZE * GRID_SIZE];
    Vertex3D rotated_base[GRID_SIZE * GRID_SIZE];

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
        // 更新旋转角度
    angle_x += speed;
    if (angle_x > 2 * M_PI) angle_x -= 2 * M_PI;
    
    angle_y += speed * 1.5f;
    if (angle_y > 2 * M_PI) angle_y -= 2 * M_PI;
    
    angle_z += speed * 0.7f;
    if (angle_z > 2 * M_PI) angle_z -= 2 * M_PI;
    el_swap_buffer();
}

// 清理函数
void deinit_mesh() {
    // 无需特别清理
}
