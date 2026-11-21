#ifndef STYLE_DIALOG_METRICS_H
#define STYLE_DIALOG_METRICS_H
#include <stdint.h>

typedef struct classic_file_metrics {
    uint16_t compact_width, compact_height, places_width, places_height;
    uint16_t client_dlu_width, client_dlu_height, gap, bottom_reserve;
} classic_file_metrics;

typedef struct classic_dialog_metrics {
    classic_file_metrics file;
    uint16_t print_width, print_full_height, print_large_height;
    uint16_t color_width, color_height;
    uint16_t find_width, find_native_height, find_classic_height;
    uint16_t font_width, font_height;
} classic_dialog_metrics;
#endif
