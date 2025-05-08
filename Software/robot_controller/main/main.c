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

#define MDNS_INSTANCE "Web Server"
#define HOST_NAME "echo1"

static const char *TAG = "Web Server";

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
        .max_files = 5,
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
/*****************************************************************
 * Global configuration variables
 *****************************************************************/
// Default robot parameters - posted and fetched from the web server
float r1_vel_pid[3] = {0.0, 0.0, 0.0}; // Robot 1 Velocity PID Parameters (kP, kI, kD)
float r2_vel_pid[3] = {0.0, 0.0, 0.0}; // Robot 2 Velocity PID Parameters (kP, kI, kD)
float r1_pos_pid[3] = {0.0, 0.0, 0.0}; // Robot 1 Position PID Parameters (kP, kI, kD)
float r2_pos_pid[3] = {0.0, 0.0, 0.0}; // Robot 2 Position PID Parameters (kP, kI, kD)

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
    ESP_ERROR_CHECK(start_rest_server("/www"));
}