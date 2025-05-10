/* HTTP Restful API Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_vfs_semihost.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdmmc_cmd.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/apps/netbiosns.h"
#include "esp_mac.h"
#include "protocol_examples_common.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "config_vars.h"
#include "rest_server.h"
#include "driver/twai.h"  // Include the TWAI (CAN) driver library
#include "freertos/queue.h"

static QueueHandle_t can_msg_queue = NULL;
QueueHandle_t ws_to_can_queue = NULL; // <-- Remove 'static' so it's global

// Store the WebSocket URI for sending
#define WS_URI "/ws"

#define MDNS_INSTANCE "Web Server"
#define GPIO_INPUT_PIN 19
#define HOST_NAME "echo"

static const char *TAG = "Robot Controller";

esp_err_t start_rest_server(const char *base_path);

static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(HOST_NAME);
    mdns_instance_name_set(MDNS_INSTANCE);

    mdns_txt_item_t serviceTxtData[] = {
        {"board", "esp32"},
        {"path", "/"}
    };

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData,
                                     sizeof(serviceTxtData) / sizeof(serviceTxtData[0])));
}

esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/www",
        .partition_label = NULL,
        .max_files = 16,
        .format_if_mount_failed = false
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

void twai_receive_task(void *pvParameter)
{
    twai_message_t rx_message;
    while (1) {
        if (twai_receive(&rx_message, pdMS_TO_TICKS(40)) == ESP_OK) {
            // Send the received message to the queue
            if (can_msg_queue) {
                // ESP_LOGI(TAG, "Received CAN message: ID:0x%lX", rx_message.identifier);
                xQueueSend(can_msg_queue, &rx_message, 0);
            }
        } else {
            // ESP_LOGW(TAG, "No CAN message received in last second");
        }
    }
}

void can_ws_forward_task(void *pvParameter)
{
    twai_message_t rx_message;
    char msg[64];
    while (1) {
        if (can_msg_queue && xQueueReceive(can_msg_queue, &rx_message, portMAX_DELAY)) {
            // Cast the message data into a 32bit float
            float *data = (float *)rx_message.data;
            // Switch on the identifier to determine where the message came from
            // switch (rx_message.identifier) {
            //     case 0x018:
            //         ESP_LOGI(TAG, "R1M1 Position: %f", (double)*data);
            //         break;
            //     case 0x118:
            //         ESP_LOGI(TAG, "R1M2 Position: %f", (double)*data);
            //         break;
            //     case 0x218:
            //         ESP_LOGI(TAG, "R2M1 Position: %f", (double)*data);
            //         break;
            //     case 0x318:
            //         ESP_LOGI(TAG, "R2M2 Position: %f", (double)*data);
            //         break;
            // }
            // Broadcast the message over WebSocket
            snprintf(msg, sizeof(msg), "ID:0x%lX, Data: %f", rx_message.identifier, *data);
            ws_broadcast_text(msg);
        }
    }
}

void ws_to_can_task(void *pvParameter)
{
    twai_message_t tx_message;
    while (1) {
        if (ws_to_can_queue && xQueueReceive(ws_to_can_queue, &tx_message, portMAX_DELAY)) {
            esp_err_t err_twwai_transmit = twai_transmit(&tx_message, pdMS_TO_TICKS(150));
            if (err_twwai_transmit == ESP_OK) {
                ESP_LOGI(TAG, "Message sent successfully: ID:0x%lX", tx_message.identifier);
            } else {
                ESP_LOGE(TAG, "Failed to send message: %s", esp_err_to_name(err_twwai_transmit));
            }
            
        }
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    initialise_mdns();
    netbiosns_init();
    netbiosns_set_name(HOST_NAME);

    ESP_ERROR_CHECK(example_connect());
    ESP_ERROR_CHECK(init_fs());
    esp_err_t rest_ok = start_rest_server("/www");
    if (rest_ok == ESP_OK) {
        ESP_LOGI(TAG, "Starting WebSocket task");
    } else {
        ESP_LOGE(TAG, "REST server failed to start, not starting WebSocket task");
    }


    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_41, GPIO_NUM_42, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver");
        return;
    }



    // Start the CAN driver
    if (twai_start() == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver");
        return;
    }

    can_msg_queue = xQueueCreate(20, sizeof(twai_message_t));
    if (can_msg_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create CAN message queue");
        return;
    }

    ws_to_can_queue = xQueueCreate(40, sizeof(twai_message_t));
    if (ws_to_can_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create ws_to_can_queue");
        return;
    }

    xTaskCreate(twai_receive_task, "twai_receive_task", 4096, NULL, 5, NULL);
    xTaskCreate(can_ws_forward_task, "can_ws_forward_task", 4096, NULL, 5, NULL);
    xTaskCreate(ws_to_can_task, "ws_to_can_task", 4096, NULL, 5, NULL);

    // twai_stop();                         
    // twai_driver_uninstall();             
    // ESP_LOGI(TAG, "TWAI driver uninstalled");
}