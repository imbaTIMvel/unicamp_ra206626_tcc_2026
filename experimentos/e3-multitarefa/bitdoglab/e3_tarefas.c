#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/multicore.h"

#define LED 13
#define PERIODO_US 10000
#define N 500

static volatile bool carga_ativa = false;

// carga concorrente: equivalente ao stress-ng, mas no mesmo nucleo
static void trabalho_inutil(void) {
    volatile uint32_t x = 1;
    for (int i = 0; i < 2000; i++) x = x * 1103515245 + 12345;
}

static void core1_carga(void) {
    while (true) {
        if (carga_ativa) trabalho_inutil();
        else tight_loop_contents();
    }
}

static int cmp(const void *a, const void *b) {
    return (*(uint32_t*)a > *(uint32_t*)b) - (*(uint32_t*)a < *(uint32_t*)b);
}

static void medir(const char *rotulo, bool carga_mesmo_nucleo) {
    static uint32_t desvios[N];
    uint32_t prox = time_us_32() + PERIODO_US;
    for (int i = 0; i < N; i++) {
        while ((int32_t)(time_us_32() - prox) < 0) {
            if (carga_mesmo_nucleo) trabalho_inutil();
        }
        uint32_t real = time_us_32();
        gpio_xor_mask(1u << LED);
        desvios[i] = real - prox;
        prox += PERIODO_US;
    }
    qsort(desvios, N, sizeof(uint32_t), cmp);
    printf("%s: mediana=%u us  p95=%u us  max=%u us\n",
           rotulo, desvios[N/2], desvios[(int)(N*0.95)], desvios[N-1]);
}

int main(void) {
    stdio_init_all();
    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(500);

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    multicore_launch_core1(core1_carga);

    printf("=== A: ocioso ===\n");
    medir("ocioso", false);

    printf("=== B: carga no SEGUNDO nucleo ===\n");
    carga_ativa = true;
    sleep_ms(100);
    medir("carga_core1", false);
    carga_ativa = false;

    printf("=== C: carga no MESMO nucleo ===\n");
    medir("carga_core0", true);

    printf("=== FIM ===\n");
    while (true) tight_loop_contents();
}
