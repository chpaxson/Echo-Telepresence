#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif_net_stack.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_now.h"

/* STA/AP Configuration */
#define WIFI_SSID         "echo"
#define WIFI_PASS         "telepresence"
#define MAX_STA_CONN      1 // Max connections to AP

static const char *TAG = "MAIN";

// Forward declarations for functions you'll need to implement
static void wifi_scan_and_decide(void);
static void wifi_init_ap(void);
static void wifi_init_sta(uint8_t* ap_bssid); // Pass AP's MAC address
static esp_err_t espnow_init(void);
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);


void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");

    // Initialize TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Register Wi-Fi event handlers (implement wifi_event_handler)
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, // Reuse or create specific handler
                                                        NULL,
                                                        NULL));


    // Initialize Wi-Fi (Common base configuration)
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // Or WIFI_STORAGE_FLASH
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL)); // Start with null mode
    ESP_ERROR_CHECK(esp_wifi_start()); // Start Wi-Fi driver

    // Scan for the target network and decide whether to be AP or STA
    wifi_scan_and_decide();

    // ESP-NOW should be initialized after Wi-Fi is started in AP or STA mode
    // The initialization call might be better placed within the AP/STA setup functions
    // or triggered by a Wi-Fi connection event.
    // ESP_ERROR_CHECK(espnow_init()); // Placeholder - move as needed

}

// --- Placeholder Function Implementations ---
// You need to fill these in with the actual logic

static void wifi_scan_and_decide(void) {
    ESP_LOGI(TAG, "Starting Wi-Fi scan...");
    // 1. Configure scan parameters
    wifi_scan_config_t scan_config = {
        .ssid = NULL, // Scan for all SSIDs
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE, // Or passive
        .scan_time.active.min = 100, // ms per channel
        .scan_time.active.max = 150,
    };

    // 2. Start scan (blocking call)
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true)); // true = block until scan done

    // 3. Get scan results
    uint16_t num_results = 0;
    esp_wifi_scan_get_ap_num(&num_results);
    ESP_LOGI(TAG, "Found %d access points", num_results);

    if (num_results == 0) {
        ESP_LOGI(TAG, "No networks found. Starting AP mode.");
        wifi_init_ap();
        return;
    }

    wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(num_results * sizeof(wifi_ap_record_t));
    if (ap_records == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for scan results");
        // Handle error - maybe default to AP?
        wifi_init_ap();
        return;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num_results, ap_records));

    // 4. Check if WIFI_SSID exists
    bool found_target_ap = false;
    uint8_t target_ap_bssid[6]; // To store the MAC address if found
    for (int i = 0; i < num_results; i++) {
        ESP_LOGI(TAG, "SSID: %s, RSSI: %d", (char *)ap_records[i].ssid, ap_records[i].rssi);
        if (strcmp((char *)ap_records[i].ssid, WIFI_SSID) == 0) {
            ESP_LOGI(TAG, "Found target AP: %s", WIFI_SSID);
            found_target_ap = true;
            memcpy(target_ap_bssid, ap_records[i].bssid, 6);
            break; // Found it, no need to check further
        }
    }

    free(ap_records); // Free the allocated memory

    // 5. Decide mode
    if (found_target_ap) {
        ESP_LOGI(TAG, "Target AP found. Starting STA mode.");
        wifi_init_sta(target_ap_bssid); // Pass the MAC address
    } else {
        ESP_LOGI(TAG, "Target AP not found. Starting AP mode.");
        wifi_init_ap();
    }
     // Stop the scan state machine if it was started
    esp_wifi_scan_stop();
}


static void wifi_init_ap(void) {
    ESP_LOGI(TAG, "Initializing AP Mode...");
    esp_netif_create_default_wifi_ap(); // Create AP interface

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 1, // Or choose a specific channel
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK, // Or other auth mode
            .pmf_cfg = {
                    .required = false,
            },
        },
    };
    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    // ESP_ERROR_CHECK(esp_wifi_start()); // Already started in app_main

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             WIFI_SSID, WIFI_PASS, 1);

    // Initialize ESP-NOW after AP is configured
    ESP_ERROR_CHECK(espnow_init());
}

static void wifi_init_sta(uint8_t* ap_bssid) {
    ESP_LOGI(TAG, "Initializing STA Mode...");
    esp_netif_create_default_wifi_sta(); // Create STA interface

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
         * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK, // Adjust if needed
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH, // For WPA3
            // .bssid_set = true, // Optionally connect only to the specific BSSID found
            // .bssid = {ap_bssid[0], ap_bssid[1], ap_bssid[2], ap_bssid[3], ap_bssid[4], ap_bssid[5]},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    // ESP_ERROR_CHECK(esp_wifi_start()); // Already started in app_main
    ESP_ERROR_CHECK(esp_wifi_connect()); // Start connection attempt

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    // ESP-NOW initialization and peer adding should happen *after* connection is established.
    // Move espnow_init() and esp_now_add_peer() calls to the WIFI_EVENT_STA_CONNECTED
    // or IP_EVENT_STA_GOT_IP case within the wifi_event_handler.
    // Store ap_bssid globally or pass it to the event handler if needed there.
}

static esp_err_t espnow_init(void) {
    ESP_LOGI(TAG, "Initializing ESP-NOW...");
    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());

    // Register ESP-NOW send and receive callbacks (implement these)
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));

    ESP_LOGI(TAG, "ESP-NOW Initialized.");
    return ESP_OK;
}

// --- Placeholder Event Handlers ---
// You need to implement the logic within these handlers

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
                ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d",
                         MAC2STR(event->mac), event->aid);
                // In AP mode, a station connected. Maybe add it as an ESP-NOW peer?
                esp_now_peer_info_t peer_info = {};
                memcpy(peer_info.peer_addr, event->mac, ESP_NOW_ETH_ALEN);
                peer_info.channel = 0; // Use current channel
                peer_info.ifidx = WIFI_IF_AP;
                peer_info.encrypt = false; // Or true if using encryption
                // Check if peer already exists before adding
                if (esp_now_is_peer_exist(event->mac)) {
                     ESP_LOGI(TAG, "Peer " MACSTR " already exists.", MAC2STR(event->mac));
                } else {
                    esp_err_t add_ret = esp_now_add_peer(&peer_info);
                    if (add_ret == ESP_OK) {
                        ESP_LOGI(TAG, "Added ESP-NOW peer: " MACSTR, MAC2STR(event->mac));
                    } else {
                        ESP_LOGE(TAG, "Failed to add ESP-NOW peer: %s", esp_err_to_name(add_ret));
                    }
                }
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
                ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d",
                         MAC2STR(event->mac), event->aid);
                // Optionally remove the peer from ESP-NOW
                esp_err_t del_ret = esp_now_del_peer(event->mac);
                 if (del_ret == ESP_OK) {
                    ESP_LOGI(TAG, "Removed ESP-NOW peer: " MACSTR, MAC2STR(event->mac));
                } else {
                    ESP_LOGW(TAG, "Failed to remove ESP-NOW peer: %s", esp_err_to_name(del_ret));
                }
                break;
            }
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
                // esp_wifi_connect(); // Connection attempt is now in wifi_init_sta
                break;
            case WIFI_EVENT_STA_CONNECTED:
                 ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
                 // Initialize ESP-NOW now that STA is connected
                 ESP_ERROR_CHECK(espnow_init());
                 // Add the AP as an ESP-NOW peer
                 wifi_event_sta_connected_t* event = (wifi_event_sta_connected_t*) event_data;
                 esp_now_peer_info_t peer_info = {};
                 memcpy(peer_info.peer_addr, event->bssid, ESP_NOW_ETH_ALEN);
                 peer_info.channel = event->channel; // Use the channel we connected on
                 peer_info.ifidx = WIFI_IF_STA;
                 peer_info.encrypt = false; // Match AP's ESP-NOW config
                 if (esp_now_is_peer_exist(event->bssid)) {
                     ESP_LOGI(TAG, "Peer " MACSTR " already exists.", MAC2STR(event->bssid));
                 } else {
                    esp_err_t add_ret = esp_now_add_peer(&peer_info);
                    if (add_ret == ESP_OK) {
                        ESP_LOGI(TAG, "Added AP as ESP-NOW peer: " MACSTR, MAC2STR(event->bssid));
                        // Now you can try sending data via ESP-NOW to the AP
                    } else {
                        ESP_LOGE(TAG, "Failed to add AP as ESP-NOW peer: %s", esp_err_to_name(add_ret));
                    }
                 }
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
                // Handle disconnection: maybe try to reconnect or rescan?
                // Optionally deinit ESP-NOW or remove peers
                esp_now_deinit(); // Deinitialize ESP-NOW on disconnect
                esp_wifi_connect(); // Attempt to reconnect
                ESP_LOGI(TAG, "Retrying connection...");
                break;
            default:
                ESP_LOGI(TAG, "Unhandled WIFI_EVENT: %ld", event_id);
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        // STA has connected and obtained an IP address.
        // ESP-NOW peer addition is handled in WIFI_EVENT_STA_CONNECTED
    } else {
         ESP_LOGI(TAG, "Unhandled event base: %s, event_id: %ld", event_base, event_id);
    }
}


static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "ESP-NOW Send CB error: MAC address is NULL");
        return;
    }
    ESP_LOGI(TAG, "ESP-NOW Send CB: Addr=" MACSTR ", Status=%s",
             MAC2STR(mac_addr),
             status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
    // Handle send success/failure
}

static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
     if (recv_info == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "ESP-NOW Recv CB error: Invalid arguments");
        return;
    }
    uint8_t * mac_addr = recv_info->src_addr;
    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "ESP-NOW Recv CB error: Source MAC address is NULL");
        return;
    }

    ESP_LOGI(TAG, "ESP-NOW Recv CB: Addr=" MACSTR ", Len=%d", MAC2STR(mac_addr), len);
    // Process received data (e.g., print it)
    ESP_LOG_BUFFER_HEX(TAG, data, len);
    // Example: Echo data back
    // esp_now_send(mac_addr, data, len);
}
