#include <stdio.h>
#include "pico/stdlib.h"

#define LED_R 13   // LED RGB da BitDogLab, canal vermelho

int main(void) {
    stdio_init_all();
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);

    int n = 0;
    while (true) {
        gpio_put(LED_R, 1); sleep_ms(200);
        gpio_put(LED_R, 0); sleep_ms(800);
        printf("vivo %d\n", n++);
    }
}