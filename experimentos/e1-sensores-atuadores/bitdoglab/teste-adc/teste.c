#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/adc.h"

int main(void) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    adc_init();
    adc_gpio_init(28);
    adc_select_input(2);
    while (true) {
        printf("GP28: %u\n", adc_read());
        sleep_ms(200);
    }
}
