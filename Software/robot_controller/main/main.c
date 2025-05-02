/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "driver/twai.h" // CAN/TWAI driver

#define WIFI_SSID      "EchoRobot"
#define WIFI_PASS      "echopassword"
#define MAX_STA_CONN   4

static const char *TAG = "echo_station";
static httpd_handle_t server = NULL;

// Motor feedback CAN frame (56 bits = 7 bytes)
typedef struct {
    uint64_t position   : 20; // Initial should be 0x7FFFF
    uint16_t current    : 12;
    uint16_t velocity   : 12;
    uint8_t  wraparound : 1; // Within 3 frames if position is < 0x1FFFF, then wraparound
    uint8_t  status     : 3; // 0: OK, 1: Error, 2: Overload, 3: Unknown
    uint8_t  crc        : 8;
} __attribute__((packed)) motor_feedback_t;

// Motor command CAN frame (56 bits = 7 bytes)
typedef struct {
    uint8_t  command    : 4;
    uint64_t data       : 44;
    uint8_t  crc        : 8;
} __attribute__((packed)) motor_command_t;

uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}



// --- Main Entry Point ---
void app_main(void) {
    ESP_LOGI(TAG, "Starting example");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize file storage */
    const char* base_path = "/data";
    ESP_ERROR_CHECK(example_mount_storage(base_path));

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Start the file server */
    ESP_ERROR_CHECK(example_start_file_server(base_path));
    ESP_LOGI(TAG, "File server started");
}
