//
// 3D Model Demo - 展示如何使用预定义的3D模型
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include "hardware/structs/sysinfo.h"
#include "el.h"

#include "draw_mesh.h"
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

typedef enum {
    MODEL_CUBE = 0,
    MODEL_PYRAMID = 1,
    MODEL_HEART = 2,
    MODEL_MESH = 3,
    MODEL_COUNT = 4
} ModelType;

static ModelType current_model = MODEL_MESH;
static uint32_t model_switch_counter = 0;

void print_memory_info() {
    extern char __StackLimit, __bss_end__;
    uint32_t heap_end = (uint32_t)&__bss_end__;
    uint32_t stack_limit = (uint32_t)&__StackLimit;
    uint32_t free_memory = stack_limit - heap_end;
    
    printf("Free RAM: %d bytes\n", free_memory);
    printf("Stack starts at: 0x%08x\n", stack_limit);
    printf("Heap ends at: 0x%08x\n", heap_end);
}

uint32_t get_stack_usage() {
    register uint32_t sp asm("sp");
    extern char __StackLimit;
    return (uint32_t)&__StackLimit - sp;
}

void core1_entry() {
    uint32_t frame_count = 0;

    init_mesh();

    while(1) {
        watchdog_update();

        if (frame_count > 0 && frame_count % 2000 == 0) {
            current_model = (current_model + 1) % MODEL_COUNT;
            printf("Switching to model %d\n", current_model);
        }

        switch (current_model) {
            case MODEL_MESH:
                draw_frame();
                break;
        }

        if (frame_count % 100 == 0) {
            uint32_t stack_used = get_stack_usage();
            printf("Core1 frame %d, model %d, stack used: %d bytes\n", 
                   frame_count, current_model, stack_used);
        }
        frame_count++;

        multicore_fifo_push_blocking(1);

        sleep_ms(30);
    }
}

int main()
{
    vreg_set_voltage(VREG_VOLTAGE_1_30);

    set_sys_clock_khz(400000, true);

    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);
    print_memory_info();

    watchdog_enable(8000, 1);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_put(LED_PIN, 1);
    sleep_ms(250);
    gpio_put(LED_PIN, 0);
    sleep_ms(250);

    el_start();

    printf("3D Model Demo Starting...\n");
    printf("Auto-switching every 2000 frames\n");

    multicore_launch_core1(core1_entry);

    uint32_t swap_count = 0;
    while(1) {
        if (multicore_fifo_rvalid()) {
            multicore_fifo_pop_blocking();
            el_swap_buffer();
            swap_count++;

            if (swap_count % 500 == 0) {
                printf("Swap count: %d, current model: %d\n", swap_count, current_model);
                print_memory_info();
                watchdog_update();
                gpio_put(LED_PIN, 1);
                sleep_ms(2);
                gpio_put(LED_PIN, 0);
            }
        }
        sleep_us(100);
    }

    deinit_mesh();
    return 0;
}