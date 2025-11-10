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
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/pll.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "eldata.pio.h"
#include "el.h"

PIO el_pio = pio0;

int el_udma_chan, el_ldma_chan;

// Gray buffer stores 2-bit grayscale values (0-3) for each pixel
unsigned char gray_framebuf[SCR_STRIDE * SCR_HEIGHT * 2];
// Binary frame buffers for temporal dithering (4 frames for 4-level gray)
unsigned char binary_framebuf[GRAYSCALE_FRAMES][SCR_STRIDE * SCR_HEIGHT];

static int frame_counter = 0; // Current frame in the 4-frame cycle (0-3)
static int draw_frame_index = 0; // Which frame set is being drawn
volatile int frame_scroll_lines = 0;
volatile bool swap_buffer = false;

static void el_sm_load_reg(uint sm, enum pio_src_dest dst, uint32_t val) {
    pio_sm_put_blocking(el_pio, sm, val);
    pio_sm_exec(el_pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(el_pio, sm, pio_encode_out(dst, 32));
}

static void el_sm_load_isr(uint sm, uint32_t val) {
    el_sm_load_reg(sm, pio_isr, val);
}

static void el_dma_init_channel(uint chan, uint dreq, volatile uint32_t *dst) {
    dma_channel_config c = dma_channel_get_default_config(chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, true);
    channel_config_set_write_increment(&c, false);
    channel_config_set_dreq(&c, dreq);

    dma_channel_configure(chan, &c, dst, NULL, SCR_STRIDE_WORDS * SCR_REFRESH_LINES, false);
}

static void el_dma_config_for_udata(uint chan) {
    el_dma_init_channel(chan, DREQ_PIO0_TX0 + EL_UDATA_SM, &el_pio->txf[EL_UDATA_SM]);
}

static void el_dma_config_for_ldata(uint chan) {
    el_dma_init_channel(chan, DREQ_PIO0_TX0 + EL_LDATA_SM, &el_pio->txf[EL_LDATA_SM]);
}

static void el_dma_config_chainning(uint chan, uint chain_to) {
    dma_channel_config c = dma_get_channel_config(chan);
    channel_config_set_chain_to(&c, chain_to);
    dma_channel_set_config(chan, &c, false);
}

static void el_pio_irq_handler() {
    gpio_put(25, 1);
    
    // Use the current frame in the 4-frame cycle
    uint8_t *framebuf = binary_framebuf[frame_counter];
    
    // Advance to next frame in the cycle
    frame_counter = (frame_counter + 1) % GRAYSCALE_FRAMES;
    
    // Check if buffer swap is requested (happens after all 4 frames displayed)
    if (swap_buffer && frame_counter == 0) {
        swap_buffer = false;
    }

    uint32_t *rdptr_ud = (uint32_t *)(framebuf);
    uint32_t *rdptr_ld = (uint32_t *)(framebuf + SCR_STRIDE * SCR_HEIGHT / 2);
    dma_channel_set_read_addr(el_udma_chan, rdptr_ud, false);
    dma_channel_set_read_addr(el_ldma_chan, rdptr_ld, false);

    pio_sm_set_enabled(el_pio, EL_UDATA_SM, false);
    pio_sm_set_enabled(el_pio, EL_LDATA_SM, false);

    pio_sm_clear_fifos(el_pio, EL_UDATA_SM);
    pio_sm_clear_fifos(el_pio, EL_LDATA_SM);

    pio_sm_restart(el_pio, EL_UDATA_SM);
    pio_sm_restart(el_pio, EL_LDATA_SM);

    // Load configuration values
    el_sm_load_reg(EL_UDATA_SM, pio_y, SCR_REFRESH_LINES - 2);
    el_sm_load_reg(EL_UDATA_SM, pio_isr, SCR_LINE_TRANSFERS - 1);
    el_sm_load_reg(EL_LDATA_SM, pio_isr, SCR_LINE_TRANSFERS - 1);

    // Setup DMA
    dma_channel_start(el_udma_chan);
    dma_channel_start(el_ldma_chan);
    // Clear IRQ flag
    el_pio->irq = 0x02;
    // start SM
    pio_enable_sm_mask_in_sync(el_pio, (1u << EL_UDATA_SM) | (1u << EL_LDATA_SM));
    gpio_put(25, 0);
}

static void el_sm_init() {
    static uint udata_offset, ldata_offset;

    for (int i = 0; i < 4; i++) {
        pio_gpio_init(el_pio, UD0_PIN + i);
        pio_gpio_init(el_pio, LD0_PIN + i);
    }
    pio_gpio_init(el_pio, PIXCLK_PIN);
    pio_gpio_init(el_pio, HSYNC_PIN);
    pio_gpio_init(el_pio, VSYNC_PIN);
    pio_sm_set_consecutive_pindirs(el_pio, EL_UDATA_SM, UD0_PIN, 4, true);
    pio_sm_set_consecutive_pindirs(el_pio, EL_UDATA_SM, PIXCLK_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(el_pio, EL_UDATA_SM, VSYNC_PIN, 1, true);
    pio_sm_set_consecutive_pindirs(el_pio, EL_LDATA_SM, LD0_PIN, 4, true);
    pio_sm_set_consecutive_pindirs(el_pio, EL_LDATA_SM, HSYNC_PIN, 1, true);

    udata_offset = pio_add_program(el_pio, &el_udata_program);
    ldata_offset = pio_add_program(el_pio, &el_ldata_program);

    //printf("EL USM offset: %d, EL LSM offset: %d\n", udata_offset, ldata_offset);

    int cycles_per_pclk = 2;
    float div = clock_get_hz(clk_sys) / (EL_TARGET_PIXCLK * cycles_per_pclk);

    pio_sm_config cu = el_udata_program_get_default_config(udata_offset);
    sm_config_set_sideset_pins(&cu, PIXCLK_PIN);
    sm_config_set_out_pins(&cu, UD0_PIN, 4);
    sm_config_set_set_pins(&cu, VSYNC_PIN, 1);
    sm_config_set_fifo_join(&cu, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&cu, true, true, 32);
    sm_config_set_clkdiv(&cu, div);
    pio_sm_init(el_pio, EL_UDATA_SM, udata_offset, &cu);

    pio_sm_config cl = el_ldata_program_get_default_config(ldata_offset);
    sm_config_set_set_pins(&cl, HSYNC_PIN, 1);
    sm_config_set_out_pins(&cl, LD0_PIN, 4);
    sm_config_set_fifo_join(&cl, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&cl, true, true, 32);
    sm_config_set_clkdiv(&cl, div);
    pio_sm_init(el_pio, EL_LDATA_SM, ldata_offset, &cl);

    el_pio->inte0 = PIO_IRQ0_INTE_SM1_BITS;
    irq_set_exclusive_handler(PIO0_IRQ_0, el_pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

static void el_dma_init() {
    el_udma_chan = dma_claim_unused_channel(true);
    el_dma_config_for_udata(el_udma_chan);

    el_ldma_chan = dma_claim_unused_channel(true);
    el_dma_config_for_ldata(el_ldma_chan);
}

// Convert 2-bit grayscale buffer to 4 binary frames for temporal dithering
// Grayscale value 0 (00): all 4 frames OFF
// Grayscale value 1 (01): 1 frame ON, 3 frames OFF (25%)
// Grayscale value 2 (10): 2 frames ON, 2 frames OFF (50%)
// Grayscale value 3 (11): 4 frames ON (100%)
// 
// 优化方案：使用空间抖动减少25%亮度的闪烁
// 对于25%灰度，不是所有像素都用相同的时间模式，而是空间分布
static void convert_gray_to_binary() {
    // Temporal dithering patterns for each grayscale level
    // Pattern indicates which frames should be lit (1) or dark (0)
    // 为了减少闪烁，25%灰度使用4种不同的时间模式，空间分布
    static const uint8_t dither_pattern[4][4] = {
        {0, 0, 0, 0},  // Level 0: 0% - all OFF
        {1, 0, 0, 0},  // Level 1: 25% - 1/4 frames ON (将用4种模式轮换)
        {1, 0, 1, 0},  // Level 2: 50% - 2/4 frames ON (交替)
        {1, 1, 1, 1}   // Level 3: 100% - all ON (修改为全亮)
    };
    
    // 25%灰度的4种时间模式（空间分布以减少闪烁）
    static const uint8_t dither_pattern_25[4][4] = {
        {1, 0, 0, 0},  // 第0帧亮
        {0, 1, 0, 0},  // 第1帧亮
        {0, 0, 1, 0},  // 第2帧亮
        {0, 0, 0, 1}   // 第3帧亮
    };
    
    for (int frame = 0; frame < GRAYSCALE_FRAMES; frame++) {
        memset(binary_framebuf[frame], 0, SCR_STRIDE * SCR_HEIGHT);
    }
    
    // Process each pixel
    for (int y = 0; y < SCR_HEIGHT; y++) {
        for (int x = 0; x < SCR_WIDTH; x++) {
            // Calculate byte and bit position in gray buffer
            int pixel_index = y * SCR_WIDTH + x;
            int byte_index = pixel_index / 4; // 4 pixels per byte (2 bits each)
            int pixel_in_byte = pixel_index % 4;
            int bit_shift = (3 - pixel_in_byte) * 2; // 6, 4, 2, 0
            
            // Extract 2-bit gray value
            uint8_t gray_value = (gray_framebuf[byte_index] >> bit_shift) & 0x03;
            
            // Calculate binary frame data
            int bin_byte = y * SCR_STRIDE + x / 8;
            int bin_bit = 7 - (x % 8);
            
            if (gray_value == 0) {
                // 全黑：不做任何操作（已经memset为0）
                continue;
            } else if (gray_value == 1) {
                // 25%灰度：使用空间抖动减少闪烁
                // 根据像素位置选择不同的时间模式
                int pattern_index = (x + y) % 4;  // 2x2 Bayer模式
                for (int frame = 0; frame < GRAYSCALE_FRAMES; frame++) {
                    if (dither_pattern_25[pattern_index][frame]) {
                        binary_framebuf[frame][bin_byte] |= (1 << bin_bit);
                    }
                }
            } else if (gray_value == 2) {
                // 50%灰度：交替模式
                for (int frame = 0; frame < GRAYSCALE_FRAMES; frame++) {
                    if (dither_pattern[2][frame]) {
                        binary_framebuf[frame][bin_byte] |= (1 << bin_bit);
                    }
                }
            } else {
                // 100%灰度：全部点亮
                for (int frame = 0; frame < GRAYSCALE_FRAMES; frame++) {
                    binary_framebuf[frame][bin_byte] |= (1 << bin_bit);
                }
            }
        }
    }
}

void el_start() {
    memset(gray_framebuf, 0x00, sizeof(gray_framebuf));
    for (int i = 0; i < GRAYSCALE_FRAMES; i++) {
        memset(binary_framebuf[i], 0x00, SCR_STRIDE * SCR_HEIGHT);
    }

    el_sm_init();
    el_dma_init();
    el_pio_irq_handler();
}

void el_swap_buffer() {
    swap_buffer = true;
    // Wait until all 4 frames have been displayed
    while (swap_buffer);
}

unsigned char *el_get_gray_buffer() {
    return gray_framebuf;
}

void el_update_frame() {
    convert_gray_to_binary();
}

/*void el_debug() {
    printf("PIO USM PC: %d, LSM PC: %d, IRQ: %d\n", el_pio->sm[EL_UDATA_SM].addr, el_pio->sm[EL_LDATA_SM].addr, el_pio->irq);
}*/
