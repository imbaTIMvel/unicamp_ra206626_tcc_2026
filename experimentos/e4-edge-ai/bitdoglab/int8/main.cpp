#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "audio_fixo.h"

static const int16_t *fonte;

extern void   ei_heap_reset(void);
extern size_t ei_heap_pico(void);

static int fornecer_dados(size_t offset, size_t length, float *out) {
    for (size_t i = 0; i < length; i++) out[i] = (float)fonte[offset + i];
    return 0;
}

static void classificar(const char *rotulo) {
    signal_t sinal = { .get_data = &fornecer_dados, .total_length = AUDIO_N };
    ei_impulse_result_t r = { 0 };

    ei_heap_reset();
    uint32_t t0 = time_us_32();
    run_classifier(&sinal, &r, false);
    uint32_t t1 = time_us_32();

    printf("%s,total_us=%u,dsp_ms=%d,classif_ms=%d,heap_bytes=%u",
           rotulo, t1 - t0, r.timing.dsp, r.timing.classification,
           (unsigned)ei_heap_pico());
    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++)
        printf(",%s=%.5f", ei_classifier_inferencing_categories[i],
               r.classification[i].value);
    printf("\n");
}

int main(void) {
    stdio_init_all();

    while (!stdio_usb_connected()) sleep_ms(100);
    sleep_ms(500);

    printf("=== PARIDADE: uma execucao por classe ===\n");
    for (int i = 0; i < AUDIO_QTD; i++) {
        fonte = amostras[i].dados;
        classificar(amostras[i].nome);
    }

    printf("=== LATENCIA: 10 execucoes sobre sample_hello ===\n");
    fonte = audio_hello;
    for (int i = 0; i < 10; i++) classificar("hello");

    printf("=== FIM ===\n");
    while (true) tight_loop_contents();
}
