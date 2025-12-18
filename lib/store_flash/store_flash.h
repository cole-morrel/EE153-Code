#ifndef STORE_FLASH_H
#define STORE_FLASH_H

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#define BATCH_SIZE 12

static const char *TAG_FLASH = "NVS_FLASH";

typedef struct {
    double temperature;
    int64_t time;
} temp_time_t; 

typedef struct {
    nvs_type_t type;
    const char *str;
} type_str_pair_t;

static const type_str_pair_t type_str_pair[] = {
    { NVS_TYPE_I8, "i8" },
    { NVS_TYPE_U8, "u8" },
    { NVS_TYPE_U16, "u16" },
    { NVS_TYPE_I16, "i16" },
    { NVS_TYPE_U32, "u32" },
    { NVS_TYPE_I32, "i32" },
    { NVS_TYPE_U64, "u64" },
    { NVS_TYPE_I64, "i64" },
    { NVS_TYPE_STR, "str" },
    { NVS_TYPE_BLOB, "blob" },
    { NVS_TYPE_ANY, "any" },
};

static const size_t TYPE_STR_PAIR_SIZE = sizeof(type_str_pair) / sizeof(type_str_pair[0]);

nvs_handle_t init_flash();
int read_counter(nvs_handle_t my_handle);
void write_counter(nvs_handle_t my_handle, int counter_val);
void write_temperature(nvs_handle_t my_handle, int index, double temperature);
void commit_and_close(nvs_handle_t my_handle);
int read_temperature(nvs_handle_t my_handle, double *buffer);

#endif // STORE_FLASH_H