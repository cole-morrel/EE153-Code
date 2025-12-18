#include "mqtt_comms.h"

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    mqtt_context_t *ctx = (mqtt_context_t *)handler_args;
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id) {

    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG_MQTT, "MQTT connected");

        // Subscribe to topic after connect
        esp_mqtt_client_subscribe(event->client, "time/epoch", 1);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG_MQTT, "MQTT message received");

        ESP_LOGI(TAG_MQTT, "Topic: %.*s", event->topic_len, event->topic);
        ESP_LOGI(TAG_MQTT, "Data: %.*s", event->data_len, event->data);

        // Make a null-terminated copy
        char buf[11];  // Make sure this is big enough for your payload
        int len = event->data_len;
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;

        memcpy(buf, event->data, len);
        buf[len] = '\0';

        // Convert string to int64
        char *endptr;
        int64_t num = strtoll(buf, &endptr, 10);

        if (endptr == buf) {
            ESP_LOGW(TAG_MQTT, "No digits were found.");
        } else if (*endptr != '\0') {
            ESP_LOGW(TAG_MQTT, "Invalid character at end: %c", *endptr);
        } else {
            ESP_LOGI(TAG_MQTT, "Parsed number: %" PRId64, num);
            *ctx = num;
        }

        break;

    default:
        break;
    }
}


esp_mqtt_client_handle_t mqtt_app_start(volatile mqtt_context_t *mqtt_time) {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = BROKER_URI,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(client,
                                   ESP_EVENT_ANY_ID,
                                   mqtt_event_handler,
                                   mqtt_time);

    esp_mqtt_client_start(client);

    return client;
}

void publish_data(int board_status, float battery_voltage, int signal_strength, 
                  temp_time_t *temperature_vals, int num_measurements, 
                  esp_mqtt_client_handle_t handle) {                      

    char *string = NULL;
    cJSON *heartbeat = NULL;
    cJSON *status = NULL;
    cJSON *battery = NULL;
    cJSON *sig_strength = NULL;
    cJSON *measurements = NULL;
    // cJSON *individual_measurement = NULL;
    cJSON *message = NULL;
    
    message = cJSON_CreateObject();
    heartbeat = cJSON_CreateObject();
    status = cJSON_CreateNumber((double)board_status);
    int battery_percent = (double)battery_voltage/4.5*100;
    battery = cJSON_CreateNumber((double)battery_percent);
    sig_strength = cJSON_CreateNumber((double)signal_strength);
    measurements = cJSON_CreateArray();

    for (int i = 0; i < num_measurements; i++) {
        // If temperature outside of threshold
        double temp_val = temperature_vals[i].temperature;
        if (temp_val > MAX_TEMP || temp_val < MIN_TEMP)
            continue;

        cJSON *individual_measurement = cJSON_CreateArray();
        cJSON_AddItemToArray(individual_measurement, cJSON_CreateNumber((double)temperature_vals[i].time));
        cJSON_AddItemToArray(individual_measurement, cJSON_CreateNumber((double)temperature_vals[i].temperature));
        cJSON_AddItemToArray(measurements, individual_measurement);
    }

    /* after creation was successful, immediately add it to the monitor,
     * thereby transferring ownership of the pointer to it */
    cJSON_AddItemToObject(heartbeat, "status", status);
    // cJSON_AddItemToObject(heartbeat, "battery_percent", battery);
    cJSON_AddItemToObject(heartbeat, "rssi", sig_strength);
    cJSON_AddItemToObject(message, "measurements", measurements);
    cJSON_AddItemToObject(message, "heartbeat", heartbeat);

    string = cJSON_PrintUnformatted(message);

    char topic[19];

    sprintf(topic, "teamL/node%d/update", NODE_NUM);
    esp_mqtt_client_publish(handle, topic, string, 0, 0, 0);

}