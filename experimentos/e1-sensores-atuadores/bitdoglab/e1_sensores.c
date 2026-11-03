#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define PIN_ANALOG 26
#define ADC_CANAL   0
#define PIN_LED    13
#define N_ITER   1000
#define N_BLOCOS   10

int main(void) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(500);

    adc_init();
    adc_gpio_init(PIN_ANALOG);
    adc_select_input(ADC_CANAL);

    gpio_set_function(PIN_LED, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(PIN_LED);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 4.f);
    pwm_config_set_wrap(&cfg, 4095);
    pwm_init(slice, &cfg, true);

    printf("=== FUNCIONAL: 30 leituras, mexa no joystick ===\n");
    for (int i = 0; i < 30; i++) {
        uint16_t leitura = adc_read();
        pwm_set_gpio_level(PIN_LED, leitura);
        printf("%d,%u\n", i, leitura);
        sleep_ms(50);
    }

    printf("=== LATENCIA: %d blocos de %d iteracoes ===\n", N_BLOCOS, N_ITER);
    for (int b = 0; b < N_BLOCOS; b++) {
        uint32_t t0 = time_us_32();
        for (int k = 0; k < N_ITER; k++) {
            pwm_set_gpio_level(PIN_LED, adc_read());
        }
        uint32_t t1 = time_us_32();
        uint32_t dt = t1 - t0;
        printf("%d,bloco_us=%u,por_iter_ns=%u\n", b, dt, (dt * 1000) / N_ITER);
    }

    printf("=== FIM ===\n");
    while (true) tight_loop_contents();
}