// 旋转杯子动画实现

#ifndef ROT_CUP_H
#define ROT_CUP_H

#include "el.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ========== 杯子参数 ==========
#define CUP_RADIUS    1.5f    // 杯子半径
#define CUP_HEIGHT    3.0f    // 杯子高度
#define HANDLE_RADIUS 0.5f    // 把手半径
#define SEGMENTS      16      // 圆形分段数
#define SCALE        80       // 缩放因子
#define CENTER_X      (SCR_WIDTH / 2)      // 屏幕中心X坐标
#define CENTER_Y      (SCR_HEIGHT / 2)     // 屏幕中心Y坐标

// ========== 顶点和边缓存 ==========
typedef struct {
    float x, y, z;
} Vertex3D;

// 计算顶点数量
#define NUM_VERTICES_BODY (SEGMENTS * 2)  // 杯身上下圆环
#define NUM_VERTICES_HANDLE (SEGMENTS * 2) // 把手
#define NUM_VERTICES (NUM_VERTICES_BODY + NUM_VERTICES_HANDLE)

// 计算边的数量
#define NUM_EDGES_BODY (SEGMENTS * 3)     // 杯身竖线 + 上下圆环
#define NUM_EDGES_HANDLE (SEGMENTS * 2)   // 把手边
#define NUM_EDGES (NUM_EDGES_BODY + NUM_EDGES_HANDLE)

static Vertex3D vertices[NUM_VERTICES];
static int edges[NUM_EDGES][2];
static int num_edges = 0;

// ========== 角度（弧度） ==========
static float angle_x = 0.15f;
static float angle_y = 0.15f;
static float angle_z = 0.15f;
static float speed = 0.03f;

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

// ========== 工具函数：3D 旋转 ==========
static void rotate_vertex(float x, float y, float z, float* out_x, float* out_y, float* out_z) {
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
static void project_to_screen(float x, float y, float z, int16_t* screen_x, int16_t* screen_y) {
    *screen_x = CENTER_X + (int16_t)(x * SCALE);
    *screen_y = CENTER_Y - (int16_t)(z * SCALE);
}

// ========== 初始化杯子 ==========
void init_rot_cup() {
    // 生成杯身顶点
    for(int i = 0; i < SEGMENTS; i++) {
        float angle = (2.0f * M_PI * i) / SEGMENTS;
        float x = CUP_RADIUS * cosf(angle);
        float y = CUP_RADIUS * sinf(angle);
        
        // 上圆环顶点
        vertices[i] = (Vertex3D){x, y, CUP_HEIGHT/2};
        // 下圆环顶点
        vertices[i + SEGMENTS] = (Vertex3D){x, y, -CUP_HEIGHT/2};
    }

    // 生成把手顶点
    int handle_start = NUM_VERTICES_BODY;
    for(int i = 0; i < SEGMENTS; i++) {
        float angle = (2.0f * M_PI * i) / SEGMENTS;
        // 把手向Y轴正方向偏移
        float x = CUP_RADIUS + HANDLE_RADIUS * (1 - cosf(angle));
        float y = CUP_RADIUS + HANDLE_RADIUS * sinf(angle);
        float z = (CUP_HEIGHT/4) * sinf(angle);
        
        vertices[handle_start + i] = (Vertex3D){x, y, z};
    }
    for(int i = 0; i < SEGMENTS; i++) {
        float angle = (2.0f * M_PI * i) / SEGMENTS;
        float x = CUP_RADIUS + HANDLE_RADIUS * (1 - cosf(angle));
        float y = CUP_RADIUS + HANDLE_RADIUS * sinf(angle);
        float z = (-CUP_HEIGHT/4) * sinf(angle);
        
        vertices[handle_start + SEGMENTS + i] = (Vertex3D){x, y, z};
    }

    // 生成杯身边
    num_edges = 0;
    // 上下圆环
    for(int i = 0; i < SEGMENTS; i++) {
        // 上圆环
        edges[num_edges][0] = i;
        edges[num_edges][1] = (i + 1) % SEGMENTS;
        num_edges++;
        
        // 下圆环
        edges[num_edges][0] = i + SEGMENTS;
        edges[num_edges][1] = ((i + 1) % SEGMENTS) + SEGMENTS;
        num_edges++;
        
        // 连接上下圆环的竖线
        edges[num_edges][0] = i;
        edges[num_edges][1] = i + SEGMENTS;
        num_edges++;
    }

    // 生成把手边
    for(int i = 0; i < SEGMENTS; i++) {
        // 上半部分
        edges[num_edges][0] = handle_start + i;
        edges[num_edges][1] = handle_start + ((i + 1) % SEGMENTS);
        num_edges++;
        
        // 下半部分
        edges[num_edges][0] = handle_start + SEGMENTS + i;
        edges[num_edges][1] = handle_start + SEGMENTS + ((i + 1) % SEGMENTS);
        num_edges++;
    }
}

// ========== 绘制一帧 ==========
void draw_rot_cup_frame() {
    // 获取当前帧缓冲区
    unsigned char *buffer = el_swap_buffer();
    
    // 清空屏幕
    clear_screen(buffer);

    // 旋转后顶点缓存
    static Vertex3D rotated_vertices[NUM_VERTICES];

    // 旋转所有顶点
    for (int i = 0; i < NUM_VERTICES; i++) {
        float rx, ry, rz;
        rotate_vertex(
            vertices[i].x,
            vertices[i].y,
            vertices[i].z,
            &rx, &ry, &rz
        );
        rotated_vertices[i] = (Vertex3D){rx, ry, rz};
    }

    // 绘制所有线条
    for (int i = 0; i < num_edges; i++) {
        int v1_idx = edges[i][0];
        int v2_idx = edges[i][1];

        int16_t x1 = 0, y1 = 0, x2 = 0, y2 = 0;

        // 投影顶点
        project_to_screen(
            rotated_vertices[v1_idx].x,
            rotated_vertices[v1_idx].y,
            rotated_vertices[v1_idx].z,
            &x1, &y1
        );
        project_to_screen(
            rotated_vertices[v2_idx].x,
            rotated_vertices[v2_idx].y,
            rotated_vertices[v2_idx].z,
            &x2, &y2
        );

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
    
    // 交换缓冲区显示
    el_swap_buffer();
}

// 清理函数
void deinit_rot_cup() {
    // 无需特别清理
}

#endif // ROT_CUP_H