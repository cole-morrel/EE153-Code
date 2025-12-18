#ifndef MQTT_COMMS_H
#define MQTT_COMMS_H

#include <stdio.h>
#include "esp_event.h"
#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "store_flash.h"
#include "temp_sensor.h"

#define BROKER_URI "mqtt://bell-mqtt.eecs.tufts.edu"
#define NODE_NUM 2

static const char *TAG_MQTT = "MQTT";

typedef long long mqtt_context_t;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data);
esp_mqtt_client_handle_t mqtt_app_start(volatile mqtt_context_t *mqtt_time);
void publish_data(int board_status, float battery_voltage, int signal_strength, 
                  temp_time_t *temperature_vals, int num_measurements, 
                  esp_mqtt_client_handle_t handle);


#endif // MQTT_COMMS_H