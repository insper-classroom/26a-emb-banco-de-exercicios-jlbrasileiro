#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"
//teste
const int LED_PIN_Y = 5;
const int BTN_PIN_G = 28;

volatile int btn_f = 0;
volatile int g_timer_g = 0;

void btn_callback(uint gpio, uint32_t events) {
    if (events & 0x4) {
        btn_f = 1;
    } else if (events & 0x8) { 
        btn_f = 2;
    }
}

bool timer_g_callback(repeating_timer_t *rt) {
    g_timer_g = 1;
    return true;
}

int main() {
    stdio_init_all();
    
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, 0x4 | 0x8, true, &btn_callback);

    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    struct repeating_timer timer;
    bool timer_ativo = false;
    uint32_t time_start = 0;
    int led_g = 0;
    uint32_t time_press = 0;

    while (1) {
        uint32_t agora = to_ms_since_boot(get_absolute_time());

        if (btn_f == 1) {
            btn_f = 0;

            if (agora - time_start > 50) {
                gpio_put(LED_PIN_Y, 0);
                led_g = 0;

                if (timer_ativo) {
                    cancel_repeating_timer(&timer);
                    timer_ativo = false;
                }
                
                time_start = agora;
            }
            
        } else if (btn_f == 2) {
            btn_f = 0;

            time_press = agora - time_start;
            
            if (time_press > 50) {
                time_start = agora;
                
                add_repeating_timer_ms(250, timer_g_callback, NULL, &timer);
                timer_ativo = true;
            }
        }

        if (g_timer_g == 1) {
            g_timer_g = 0;

            if (timer_ativo) {
                if (agora - time_start >= time_press) {
                    timer_ativo = false;
                    led_g = 0;
                    gpio_put(LED_PIN_Y, led_g);
                    cancel_repeating_timer(&timer);
                } else {
                    led_g = !led_g;
                    gpio_put(LED_PIN_Y, led_g);
                }
            }
        }
    }

    return 0;
}