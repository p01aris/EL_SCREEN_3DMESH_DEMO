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
#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include "hardware/structs/sysinfo.h"
#include "el.h"
//#include "rot_cup.h"
#include "draw_mesh.h"

// 内存调试函数
void print_memory_info() {
    extern char __StackLimit, __bss_end__;
    uint32_t heap_end = (uint32_t)&__bss_end__;
    uint32_t stack_limit = (uint32_t)&__StackLimit;
    uint32_t free_memory = stack_limit - heap_end;
    
    printf("Free RAM: %d bytes\n", free_memory);
    printf("Stack starts at: 0x%08x\n", stack_limit);
    printf("Heap ends at: 0x%08x\n", heap_end);
}

// 检查栈使用情况
uint32_t get_stack_usage() {
    register uint32_t sp asm("sp");
    extern char __StackLimit;
    return (uint32_t)&__StackLimit - sp;
}

// 第二个核心运行的函数
void core1_entry() {
    uint32_t frame_count = 0;
    // 第二个核心的主循环
    while(1) {
        // 喂看门狗
        watchdog_update();
        
        // 绘制一帧网格
        draw_frame();
        
        // 每100帧打印一次栈使用情况
        if (frame_count % 100 == 0) {
            uint32_t stack_used = get_stack_usage();
            printf("Core1 frame %d, stack used: %d bytes\n", frame_count, stack_used);
        }
        frame_count++;
        
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
    
    // 打印内存信息用于调试
    print_memory_info();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    // 启用看门狗，8秒超时
    watchdog_enable(8000, 1);
    
    el_start();
    
    // 初始化网格
    init_mesh();
    //init_rot_cup();
    
    // 启动第二个核心运行绘图函数
    multicore_launch_core1(core1_entry);
    
    // 第一个核心的主循环 - 等待绘制完成信号并交换缓冲区
    uint32_t swap_count = 0;
    while(1) {
        // 等待Core1完成绘制
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking(); // 接收完成信号
            el_swap_buffer(); // 交换缓冲区
            swap_count++;
            
            // 每500次交换打印一次内存信息
            if (swap_count % 500 == 0) {
                printf("Swap count: %d\n", swap_count);
                print_memory_info();
                // 主线程也喂看门狗
                watchdog_update();
            }
        }
        // 在这里可以添加其他任务，比如处理输入、传感器数据等
        sleep_us(100); // 短暂休眠避免过度占用CPU
    }

    // 程序结束时的清理（实际上不会执行到这里）
    deinit_mesh();
    //deinit_rot_cup();
    return 0;
}
