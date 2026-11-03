#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/i2c.h"

#define ENDERECO 0x68
#define SDA 0
#define SCL 1
#define N_ITER   1000
#define N_BLOCOS   10

static void ler_eixos(int16_t *v) {
    uint8_t reg = 0x3B, buf[6];
    i2c_write_blocking(i2c0, ENDERECO, &reg, 1, true);
    i2c_read_blocking(i2c0, ENDERECO, buf, 6, false);
    for (int i = 0; i < 3; i++)
        v[i] = (int16_t)((buf[i*2] << 8) | buf[i*2+1]);
}

int main(void) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(500);

    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);

    uint8_t who = 0x75, id;
    i2c_write_blocking(i2c0, ENDERECO, &who, 1, true);
    i2c_read_blocking(i2c0, ENDERECO, &id, 1, false);
    printf("WHO_AM_I=0x%02x\n", id);

    uint8_t acorda[2] = {0x6B, 0x00};
    i2c_write_blocking(i2c0, ENDERECO, acorda, 2, false);
    sleep_ms(100);

    int16_t v[3];
    printf("=== FUNCIONAL: 20 leituras, mexa no modulo ===\n");
    for (int i = 0; i < 20; i++) {
        ler_eixos(v);
        printf("%d,%d,%d,%d\n", i, v[0], v[1], v[2]);
        sleep_ms(250);
    }

    printf("=== LATENCIA: %d blocos de %d iteracoes ===\n", N_BLOCOS, N_ITER);
    for (int b = 0; b < N_BLOCOS; b++) {
        uint32_t t0 = time_us_32();
        for (int k = 0; k < N_ITER; k++) ler_eixos(v);
        uint32_t dt = time_us_32() - t0;
        printf("%d,bloco_us=%u,por_iter_ns=%u\n", b, dt, (dt * 1000) / N_ITER);
    }
    printf("=== FIM ===\n");
    while (true) tight_loop_contents();
}
