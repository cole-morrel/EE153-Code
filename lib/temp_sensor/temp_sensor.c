#include "temp_sensor.h"

i2c_master_dev_handle_t init_i2c(void) {
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_PORT,
        .scl_io_num = I2C_SCL,
        .sda_io_num = I2C_SDA,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TEMP_SENSOR_ADDRESS,
        .scl_speed_hz = 100000,
    };

    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // Set limit registers
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, (const uint8_t*)config_low_limit_message, 3, -1));
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, (const uint8_t*)config_high_limit_message, 3, -1));

    return dev_handle;
}

double get_temp(i2c_master_dev_handle_t dev_handle) {
    uint8_t receive_buf[READ_DATA_LENGTH];

    // Initiate One shot reading
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, (const uint8_t*)config_register_message, 3, -1));
    
    // Switch to temperature register and read data out
    ESP_ERROR_CHECK(i2c_master_transmit(dev_handle, (const uint8_t*)get_temp_register_message, 1, -1));
    ESP_ERROR_CHECK(i2c_master_receive(dev_handle, (uint8_t*)receive_buf, READ_DATA_LENGTH, -1));

    // Convert binary to float and return
    double temperature = outputToDecimal(receive_buf, DATA_RESOLUTION);
    return temperature;
    
}

double outputToDecimal(uint8_t binary_data[2], uint8_t decimal_length) {
    double output = (double)((int8_t) binary_data[0]);
    uint8_t byte1 = binary_data[0];
    uint8_t byte2 = binary_data[1];
    uint8_t sign = byte1 >> 7;
    uint8_t bitmask = 0x01;

    for (int i = 0; i < 8; i++) {
        uint8_t result = bitmask & (byte1 >> (7 - i));
    }

    uint8_t byte2_array[decimal_length];
    for (int i = 0; i < decimal_length; i++) {
        byte2_array[i] = (byte2 >> (7 - i)) & bitmask;
        if (sign)
            byte2_array[i] = !byte2_array[i];
    }

    if (sign) {
        for (int i = decimal_length - 1; i >= 0; i--) {
            byte2_array[i] += 1;
            if (byte2_array[i] == 1)
                break;
            else if (byte2_array[i] == 2)
                byte2_array[i] = 0;
        }
    }

    for (int i = 0; i < decimal_length; i++) {
        double unsigned_val = pow(2.0, (double)(-1 * (i + 1))) * (double)byte2_array[i];

        if(sign)
            output -= unsigned_val;
        else
            output += unsigned_val;
        
    }

    return output;
}