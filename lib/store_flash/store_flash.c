# include "store_flash.h"


nvs_handle_t init_flash() {
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open NVS handle
    ESP_LOGI(TAG_FLASH, "\nOpening Non-Volatile Storage (NVS) handle...");
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_FLASH, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    }

    return my_handle;
}

int read_counter(nvs_handle_t my_handle) {
    esp_err_t err;
    
    int32_t read_counter;
    ESP_LOGI(TAG_FLASH, "\nReading counter from NVS...");
    err = nvs_get_i32(my_handle, "counter", &read_counter);
    // Check if counter read successful
    switch (err) {
        case ESP_OK:
            ESP_LOGI(TAG_FLASH, "Read counter = %" PRIu32, read_counter);

            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGW(TAG_FLASH, "The value is not initialized yet!");
            read_counter = -1;

            break;
        default:
            ESP_LOGE(TAG_FLASH, "Error (%s) reading!", esp_err_to_name(err));
            read_counter = -1;
    }

    return read_counter;
}

void write_counter(nvs_handle_t my_handle, int counter_val) {
    esp_err_t err2 = nvs_set_i32(my_handle, "counter", (int32_t)counter_val);
    // Check if counter write successful
    if (err2 != ESP_OK) {
        ESP_LOGE(TAG_FLASH, "Failed to write counter!");
    } else {
        ESP_LOGI(TAG_FLASH, "Updated counter to %d", counter_val);
    }
}

void write_temperature(nvs_handle_t my_handle, int index, double temperature) {
    // Get key to store value at
    char key[7];
    sprintf(key, "temp%d", index);
    
    // Scale 4 decimal point float to integer
    int32_t temperature_int = (int32_t)(temperature * 10000);

    // Check if temperature write successful
    esp_err_t err2 = nvs_set_i32(my_handle, key, temperature_int);
    if (err2 != ESP_OK) {
        ESP_LOGE(TAG_FLASH, "Failed to write temperature!");
    } else {
        ESP_LOGI(TAG_FLASH, "Wrote temperature %" PRIu32, temperature_int);
    }
}

void commit_and_close(nvs_handle_t my_handle) {
    esp_err_t err;

    // Commit changes
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.
    ESP_LOGI(TAG_FLASH, "\nCommitting updates in NVS...");
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_FLASH, "Failed to commit NVS changes!");
    }

    // Close
    nvs_close(my_handle);
    ESP_LOGI(TAG_FLASH, "NVS handle closed.");

    ESP_LOGI(TAG_FLASH, "Returned to app_main");
}

int read_temperature(nvs_handle_t my_handle, double *buffer) {
    esp_err_t err;
    
    // Iterate over batched values
    for (int i = 0; i < (BATCH_SIZE - 1); i++) {
        
        // get key to read value from
        char key[7];
        sprintf(key, "temp%d", i);
        
        int32_t temperature_val;

        ESP_LOGI(TAG_FLASH, "\nReading temp%d from NVS...", i);
        // Check if temperature read successful
        err = nvs_get_i32(my_handle, key, &temperature_val);
        switch (err) {
            case ESP_OK:
                ESP_LOGI(TAG_FLASH, "Read temp = %" PRIu32, temperature_val);

                buffer[i] = ((double)temperature_val) / 10000.0; 

                break;
            case ESP_ERR_NVS_NOT_FOUND:
                ESP_LOGW(TAG_FLASH, "The value is not initialized yet!");
                temperature_val = 0;

                return 1; // Return error type 1

                break;
            default:
                ESP_LOGE(TAG_FLASH, "Error (%s) reading!", esp_err_to_name(err));
                temperature_val = 0;

                return 2; // Return error type 2
        }
    }

    return 0; // Return no error

}