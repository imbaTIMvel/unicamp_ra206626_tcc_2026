#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"

EI_IMPULSE_ERROR ei_run_impulse_check_canceled() { return EI_IMPULSE_OK; }

EI_IMPULSE_ERROR ei_sleep(int32_t time_ms) {
    sleep_ms((uint32_t)time_ms);
    return EI_IMPULSE_OK;
}

uint64_t ei_read_timer_us() { return to_us_since_boot(get_absolute_time()); }
uint64_t ei_read_timer_ms() { return ei_read_timer_us() / 1000; }

void ei_putchar(char c) { putchar(c); }

char ei_getchar(void) {
    int c = getchar_timeout_us(0);
    return (c == PICO_ERROR_TIMEOUT) ? 0 : (char)c;
}

void ei_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void ei_printf_float(float f) { printf("%f", f); }

// --- instrumentacao de memoria -------------------------------------------
// mallinfo().uordblks informa quantos bytes estao EM USO no heap neste
// instante. Amostrando a cada alocacao, obtem-se o pico real.
#include <malloc.h>

static size_t heap_pico = 0;

static inline void amostrar_heap(void) {
    size_t em_uso = (size_t)mallinfo().uordblks;
    if (em_uso > heap_pico) heap_pico = em_uso;
}

void   ei_heap_reset(void) { heap_pico = 0; amostrar_heap(); }
size_t ei_heap_pico(void)  { return heap_pico; }

void *ei_malloc(size_t size) {
    void *p = malloc(size);
    amostrar_heap();
    return p;
}

void *ei_calloc(size_t nitems, size_t size) {
    void *p = calloc(nitems, size);
    amostrar_heap();
    return p;
}

void ei_free(void *ptr) { free(ptr); }
// -------------------------------------------------------------------------

void ei_serial_set_baudrate(int baudrate) { (void)baudrate; }

extern "C" void DebugLog(const char *s) { printf("%s", s); }
