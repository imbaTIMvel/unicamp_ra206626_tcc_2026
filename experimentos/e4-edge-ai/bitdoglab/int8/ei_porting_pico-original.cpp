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

void *ei_malloc(size_t size)                 { return malloc(size); }
void *ei_calloc(size_t nitems, size_t size)  { return calloc(nitems, size); }
void  ei_free(void *ptr)                     { free(ptr); }

void ei_serial_set_baudrate(int baudrate) { (void)baudrate; }

extern "C" void DebugLog(const char *s) { printf("%s", s); }