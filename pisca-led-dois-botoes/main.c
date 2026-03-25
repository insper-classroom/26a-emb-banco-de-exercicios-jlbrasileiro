#include "hardware/gpio.h"
#include "pico/stdlib.h"

const int BTN_PIN_Y = 26;
const int BTN_PIN_G = 28;

const int LED_PIN_R = 9;
const int LED_PIN_G = 5;

volatile int flag_y = 0;
volatile int flag_g = 0;

volatile int flag_timer_y = 0;
volatile int flag_timer_g = 0;

volatile bool alarm_y = false;
volatile bool alarm_g = false;

bool timer_r_callback(repeating_timer_t *rt) {
    (void)rt;
    flag_timer_y = 1;
    return true;
}

bool timer_g_callback(repeating_timer_t *rt) {
    (void)rt;
    flag_timer_g = 1;
    return true;
}

void btn_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_Y) {
            flag_y = 1;
        } else if (gpio == BTN_PIN_G) {
            flag_g = 1;
        }
    }
}

int64_t alarm_y_callback(alarm_id_t, void *user_data){
    (void)user_data;
    alarm_y = true;
    return 0;
}

int64_t alarm_g_callback(alarm_id_t, void *user_data){
    (void)user_data;
    alarm_g = true;
    return 0;
}

int main() {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);
    gpio_put(LED_PIN_G, 0);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);

    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true);

    bool timer_y_ativo = false;
    bool timer_g_ativo = false;
    bool led_y_ativo = false;
    bool led_g_ativo = false;

    repeating_timer_t timer_y;
    repeating_timer_t timer_g;
    alarm_id_t alarm_id_y = -1;
    alarm_id_t alarm_id_g = -1;

    while (true) {
        if (flag_y) {
            flag_y = 0;
            timer_y_ativo = !timer_y_ativo;

            if (timer_y_ativo) {
                alarm_y = false;
                if (alarm_id_y >= 0) {
                    cancel_alarm(alarm_id_y);
                }
                add_repeating_timer_ms(500, timer_r_callback, NULL, &timer_y);
                alarm_id_y = add_alarm_in_ms(2000, alarm_y_callback, NULL, false);
            } else {
                if (alarm_id_y >= 0) {
                    cancel_alarm(alarm_id_y);
                    alarm_id_y = -1;
                }
                cancel_repeating_timer(&timer_y);
                led_y_ativo = false;
                gpio_put(LED_PIN_R, 0);
                flag_timer_y = 0;
            }
        }

        if (flag_g) {
            flag_g = 0;
            timer_g_ativo = !timer_g_ativo;

            if (timer_g_ativo) {
                alarm_g = false;
                if (alarm_id_g >= 0) {
                    cancel_alarm(alarm_id_g);
                }
                add_repeating_timer_ms(200, timer_g_callback, NULL, &timer_g);
                alarm_id_g = add_alarm_in_ms(1000, alarm_g_callback, NULL, false);
            } else {
                if (alarm_id_g >= 0) {
                    cancel_alarm(alarm_id_g);
                    alarm_id_g = -1;
                }
                cancel_repeating_timer(&timer_g);
                led_g_ativo = false;
                gpio_put(LED_PIN_G, 0);
                flag_timer_g = 0;
            }
        }

        if (alarm_y && timer_y_ativo) {
            alarm_y = false;
            if (timer_g_ativo) {
                timer_y_ativo = false;
                timer_g_ativo = false;

                cancel_repeating_timer(&timer_y);
                cancel_repeating_timer(&timer_g);

                if (alarm_id_y >= 0) {
                    alarm_id_y = -1;
                }
                if (alarm_id_g >= 0) {
                    cancel_alarm(alarm_id_g);
                    alarm_id_g = -1;
                }

                alarm_g = false;
                led_y_ativo = false;
                led_g_ativo = false;
                gpio_put(LED_PIN_R, 0);
                gpio_put(LED_PIN_G, 0);
                flag_timer_y = 0;
                flag_timer_g = 0;
            } else {
                timer_y_ativo = false;
                if (alarm_id_y >= 0) {
                    alarm_id_y = -1;
                }
                cancel_repeating_timer(&timer_y);
                led_y_ativo = false;
                gpio_put(LED_PIN_R, 0);
                flag_timer_y = 0;
            }
        }

        if (alarm_g && timer_g_ativo) {
            alarm_g = false;
            if (timer_y_ativo) {
                timer_g_ativo = false;
                timer_y_ativo = false;

                cancel_repeating_timer(&timer_g);
                cancel_repeating_timer(&timer_y);

                if (alarm_id_g >= 0) {
                    alarm_id_g = -1;
                }
                if (alarm_id_y >= 0) {
                    cancel_alarm(alarm_id_y);
                    alarm_id_y = -1;
                }

                alarm_y = false;
                led_g_ativo = false;
                led_y_ativo = false;
                gpio_put(LED_PIN_G, 0);
                gpio_put(LED_PIN_R, 0);
                flag_timer_g = 0;
                flag_timer_y = 0;
            } else {
                timer_g_ativo = false;
                if (alarm_id_g >= 0) {
                    alarm_id_g = -1;
                }
                cancel_repeating_timer(&timer_g);
                led_g_ativo = false;
                gpio_put(LED_PIN_G, 0);
                flag_timer_g = 0;
            }
        }

        if (flag_timer_y) {
            flag_timer_y = 0;
            led_y_ativo = !led_y_ativo;
            gpio_put(LED_PIN_R, led_y_ativo);
        }

        if (flag_timer_g) {
            flag_timer_g = 0;
            led_g_ativo = !led_g_ativo;
            gpio_put(LED_PIN_G, led_g_ativo);
        }
    }
}