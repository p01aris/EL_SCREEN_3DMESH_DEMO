//
// Copyright 2021 Wenting Zhang <zephray@outlook.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "pico/multicore.h"
#include "el.h"
//#include "rot_cup.h"
#include "draw_mesh.h"

// 第二个核心运行的函数
void core1_entry() {
    // 第二个核心的主循环
    while(1) {
        // 绘制一帧网格
        draw_frame();
        
        // 通知主核心完成绘制，可以交换缓冲区
        multicore_fifo_push_blocking(1);
        
        // 控制帧率，避免画面过快
        sleep_ms(30);
    }
}

int main()
{
    // 设置电压调节器为1.2V，以支持400MHz的频率
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    
    // 超频到400MHz
    set_sys_clock_khz(420000, true);
    
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    el_start();
    
    // 初始化网格
    init_mesh();
    //init_rot_cup();
    
    // 启动第二个核心运行绘图函数
    multicore_launch_core1(core1_entry);
    
    // 第一个核心的主循环 - 等待绘制完成信号并交换缓冲区
    while(1) {
        // 等待Core1完成绘制
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking(); // 接收完成信号
            el_swap_buffer(); // 交换缓冲区
        }
        // 在这里可以添加其他任务，比如处理输入、传感器数据等
        sleep_us(100); // 短暂休眠避免过度占用CPU
    }

    // 程序结束时的清理（实际上不会执行到这里）
    deinit_mesh();
    //deinit_rot_cup();
    return 0;
}
