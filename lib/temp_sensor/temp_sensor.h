#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h" // Used for timer delay
#include "driver/gpio.h"
#include "math.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_netif.h"

// Define constant values
#define I2C_PORT -1
#define I2C_SCL 7
#define I2C_SDA 6
#define TEMP_SENSOR_ADDRESS 0b1001000
#define READ_DATA_LENGTH 2
#define DATA_RESOLUTION 4
#define CONFIGURATION_MESSAGE_BYTE2 0xFF 
#define CONFIGURATION_MESSAGE_BYTE1 0x81
#define LOW_LIMIT_BYTE1 0x80 
#define LOW_LIMIT_BYTE2 0x00 
#define HIGH_LIMIT_BYTE1 0x7F
#define HIGH_LIMIT_BYTE2 0xF0
#define MAX_TEMP 127.9375
#define MIN_TEMP -128.0

// Messages used for configuration
static const uint8_t config_register_message[3] = {0x01, CONFIGURATION_MESSAGE_BYTE1, CONFIGURATION_MESSAGE_BYTE2};
static const uint8_t config_low_limit_message[3] = {0x02, LOW_LIMIT_BYTE1, LOW_LIMIT_BYTE2};
static const uint8_t config_high_limit_message[3] = {0x03, HIGH_LIMIT_BYTE1, HIGH_LIMIT_BYTE2};
static const uint8_t get_temp_register_message[1] = {0x00};

double outputToDecimal(uint8_t binary_data[2], uint8_t decimal_length);
i2c_master_dev_handle_t init_i2c(void);
double get_temp(i2c_master_dev_handle_t dev_handle);

#endif // TEMP_SENSOR_H