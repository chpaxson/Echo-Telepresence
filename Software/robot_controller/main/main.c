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
#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/twai.h"  // Include the TWAI (CAN) driver library

// Store the WebSocket URI for sending
#define WS_URI "/ws"

#define MDNS_INSTANCE "Web Server"
#define GPIO_INPUT_PIN 19
char host_name[16] = ""; // Allocate enough space for "echo1" or "echo2"

static void configure_gpio(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_INPUT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

static adc_oneshot_unit_handle_t adc1_handle;

static void init_adc(void)
{
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config); // GPIO7
    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config); // GPIO5
}

static int read_adc_gpio7(void)
{
    int result = 0;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &result);
    return result;
}

static int read_adc_gpio5(void)
{
    int result = 0;
    adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &result);
    return result;
}

static const char *TAG = "Web Server";

esp_err_t start_rest_server(const char *base_path);

static void initialise_mdns(void)
{
    mdns_init();
    mdns_hostname_set(host_name);
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

void setup_espnow_as_host(void)
{
    // Initialize WiFi in AP or STA mode as needed
    // Initialize ESP-NOW
    // Register send/receive callbacks
    // Ready to accept connections from echo2
}

void setup_espnow_as_client(void)
{
    // Initialize WiFi in STA mode
    // Initialize ESP-NOW
    // Register send/receive callbacks
    // Scan for echo1 and add its MAC as a peer
    // Attempt to connect/send to echo1
}

void sendJointAngles_ws_task(void *pvParameter)
{
    while (1) {
        if (!global_httpd_server) {
            vTaskDelay(pdMS_TO_TICKS(40));
            continue;
        }
        int raw_a1 = read_adc_gpio7();
        int raw_a2 = read_adc_gpio5();
        // raaw_a1,raw_a2
        // Just send strings
        char *msg = malloc(32);
        if (msg) {
            snprintf(msg, 32, "%d,%d", raw_a1, raw_a2);
        } else {
            ESP_LOGE(TAG, "Failed to allocate memory for message");
            vTaskDelay(pdMS_TO_TICKS(40));
            continue;
        }

        if (msg) {
            ws_broadcast_text(msg);
        }
        if (msg) free(msg);

        vTaskDelay(pdMS_TO_TICKS(40)); // 25 Hz
    }
}

void twai_receive_task(void *pvParameter)
{
    twai_message_t rx_message;
    while (1) {
        if (twai_receive(&rx_message, pdMS_TO_TICKS(1000)) == ESP_OK) {
            ESP_LOGI(TAG, "Message received");
            ESP_LOGI(TAG, "ID: 0x%x, DLC: %d, Data:", (unsigned int)rx_message.identifier, rx_message.data_length_code);
            for (int i = 0; i < rx_message.data_length_code; i++) {
                ESP_LOGI(TAG, "Data[%d]: 0x%x", i, rx_message.data[i]);
            }
        } else {
            ESP_LOGW(TAG, "No CAN message received in last second");
        }
    }
}

void app_main(void)
{
    configure_gpio();
    strcpy(host_name, gpio_get_level(GPIO_INPUT_PIN) == 0 ? "echo1" : "echo2");
    ESP_LOGI(TAG, "Host name: %s", host_name);


    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_41, GPIO_NUM_42, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_125KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver");
        return; // Exit the function if driver installation fails
    }

    // Start the CAN driver
    if (twai_start() == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver");
        return; // Exit the function if driver start fails
    }

    // Start TWAI receive task
    xTaskCreate(twai_receive_task, "twai_receive_task", 4096, NULL, 5, NULL);


    // twai_message_t rx_message;  // Declare a message structure for receiving
    // if (twai_receive(&rx_message, pdMS_TO_TICKS(1000)) == ESP_OK) {
    //     ESP_LOGI(TAG, "Message received");
    //     ESP_LOGI(TAG, "ID: 0x%x, DLC: %d, Data:", (unsigned int)rx_message.identifier, rx_message.data_length_code);
    //     for (int i = 0; i < rx_message.data_length_code; i++) {
    //         ESP_LOGI(TAG, "Data[%d]: 0x%x", i, rx_message.data[i]);
    //     }
    // } else {
    //     ESP_LOGE(TAG, "Failed to receive message");
    // }


    if (gpio_get_level(GPIO_INPUT_PIN) == 0) {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        initialise_mdns();
        netbiosns_init();
        netbiosns_set_name(host_name);

        ESP_ERROR_CHECK(example_connect());
        ESP_ERROR_CHECK(init_fs());
        esp_err_t rest_ok = start_rest_server("/www");
        if (rest_ok == ESP_OK) {
            ESP_LOGI(TAG, "Starting WebSocket task");
            init_adc(); // <-- Add this line
            xTaskCreate(sendJointAngles_ws_task, "analog_ws_task", 6144, NULL, 5, NULL);
        } else {
            ESP_LOGE(TAG, "REST server failed to start, not starting WebSocket task");
        }
    } else {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        // setup_espnow_as_client();
    }

    // twai_stop();                         
    // twai_driver_uninstall();             
    // ESP_LOGI(TAG, "TWAI driver uninstalled");
}