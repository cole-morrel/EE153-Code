#define USE_MAIN

#ifdef USE_MAIN

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "minimal_wifi.h"
#include "esp_wifi.h"
#include "esp_sleep.h"
#include "cJSON.h"
#include <stdio.h>
#include "mqtt_comms.h"
#include "store_flash.h"
#include "temp_sensor.h"

#define WIFI_SSID      "Tufts_Wireless"
#define WIFI_PASS      ""
#define SLEEP_TIME_SECS 3600LL // Sleep for 1 hour


// Function to get RSSI from connected WiFi
int get_wifi_rssi() {
    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);  // Retrieve AP info
    return ap_info.rssi;  // Return the RSSI value
}

static const char *TAG_MAIN = "MAIN";

void app_main() {
    // ---------------------- Temp Sensor Setup ------------------
    i2c_master_dev_handle_t temp_handle = init_i2c();

    // ----------------------- NVS Flash Setup -------------------
    nvs_handle_t flash_handle = init_flash();

    // ------------------ Read Temperature -------------------
    double temp_reading = get_temp(temp_handle);
    
    // ------------------- Batching Logic --------------------
    int flash_counter = read_counter(flash_handle);

    // Flash unitialized
    if (flash_counter == -1) {
        // Store data in flash
        int new_counter = 1;
        int curr_index = 0;
        write_counter(flash_handle, new_counter);
        write_temperature(flash_handle, curr_index, temp_reading);
        commit_and_close(flash_handle);
    } 
    // In the middle of batch
    else if (flash_counter >= 0 && flash_counter < BATCH_SIZE - 1) {
        // Store data in flash
        int new_counter = flash_counter + 1;
        int curr_index = flash_counter;
        write_counter(flash_handle, new_counter);
        write_temperature(flash_handle, curr_index, temp_reading);
        commit_and_close(flash_handle);
    
    }
    // Finished batch 
    else if (flash_counter >= BATCH_SIZE - 1) {
        // Connect to Wifi
        printf("Connecting to WiFi...");
        wifi_connect(WIFI_SSID, WIFI_PASS);

        // Connect to MQTT
        volatile mqtt_context_t mqtt_time = 0;
        esp_mqtt_client_handle_t client = mqtt_app_start(&mqtt_time);

        // Get RSSI
        int rssi = get_wifi_rssi();

        // Get data from flash and transmit
        double batch_temperatures[BATCH_SIZE - 1];
        int temperature_status = read_temperature(flash_handle, batch_temperatures);

        // Get time from MQTT
        while (mqtt_time == 0) {} // Spin loop until get update of time from server
        int64_t curr_time = (int64_t)mqtt_time;
        ESP_LOGI(TAG_MAIN, "curr_time from MQTT: %" PRId64, curr_time);

        // Read batched data (only continue with send if correct number of datapoints retreived)
        if (temperature_status == 1 || temperature_status == 2) {
            ESP_LOGE(TAG_MAIN, "Bad batch measurement read. No data sent.");
        } else if (temperature_status == 0) {
            while(curr_time == -1) {} 
            
            temp_time_t batch_temp_time[BATCH_SIZE];

            int64_t running_time = curr_time;
            batch_temp_time[BATCH_SIZE - 1].time = running_time;
            batch_temp_time[BATCH_SIZE - 1].temperature = temp_reading;
            
            for (int i = BATCH_SIZE - 2; i >= 0; i--) {
                running_time -= SLEEP_TIME_SECS; // Subtract 1 hour

                batch_temp_time[i].time = running_time;
                batch_temp_time[i].temperature = batch_temperatures[i];
            }
            
            publish_data(0, 0, rssi, batch_temp_time, BATCH_SIZE, client);
        }        

        // reset counter value in flash
        int new_counter = 0;
        write_counter(flash_handle, new_counter);
        commit_and_close(flash_handle);
    }

    esp_sleep_enable_timer_wakeup(SLEEP_TIME_SECS * 1000000); // one hour
    esp_deep_sleep_start();
}

#endif