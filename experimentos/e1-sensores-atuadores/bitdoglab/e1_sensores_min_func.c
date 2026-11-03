#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

int main(void) {
    adc_init();
    adc_gpio_init(26);
    adc_select_input(0);
    gpio_set_function(13, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(13);
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 4.f);
    pwm_config_set_wrap(&cfg, 4095);
    pwm_init(slice, &cfg, true);
    while (true) {
        pwm_set_gpio_level(13, adc_read());
        sleep_ms(50);
    }
}