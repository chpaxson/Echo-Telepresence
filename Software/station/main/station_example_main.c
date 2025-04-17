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

// --- WiFi AP Mode Setup ---
void wifi_init_softap(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID:%s password:%s", WIFI_SSID, WIFI_PASS);
}

// --- CAN/TWAI Setup (stub) ---
void can_init(void) {
    // Configure and start CAN/TWAI driver here
    // twai_general_config_t g_config = ...;
    // twai_timing_config_t t_config = ...;
    // twai_filter_config_t f_config = ...;
    // ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
    // ESP_ERROR_CHECK(twai_start());
    ESP_LOGI(TAG, "CAN/TWAI initialized (stub)");
}

// --- WebSocket Handler ---
esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        // WebSocket handshake
        return ESP_OK;
    }
    // Here, you would handle incoming WebSocket frames if needed
    return ESP_OK;
}

// --- HTTP Server Setup ---
void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t ws_uri = {
            .uri = "/ws",
            .method = HTTP_GET,
            .handler = ws_handler,
            .user_ctx = NULL,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws_uri);

        // TODO: Register static file handlers for Vue GUI
    }
}

// --- CAN-to-WebSocket Streaming Task (stub) ---
void can_stream_task(void *arg) {
    while (1) {
        // 1. Read CAN/TWAI message
        // 2. Format as JSON or binary
        // 3. Send to all connected WebSocket clients
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// --- Main Entry Point ---
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_softap();
    can_init();
    start_webserver();

    // Start CAN-to-WebSocket streaming task
    xTaskCreate(can_stream_task, "can_stream_task", 4096, NULL, 5, NULL);
}
