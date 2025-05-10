/* HTTP Restful API Server

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <fcntl.h>
#include "esp_http_server.h"
#include "esp_chip_info.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "cJSON.h"

#include "config_vars.h"
#include <sys/param.h>
#include "driver/twai.h"
#include "freertos/queue.h"
extern QueueHandle_t ws_to_can_queue;
static const char *REST_TAG = "esp-rest";

// Helper: pack 32 bits (4 bytes) into CAN frame
static void pack_32bits(uint8_t *dest, const void *src) {
    memcpy(dest, src, 4);
}

// Helper: pack sense_dir (1 bit, LSB) and controller (8 bits, bits 1-8) into 1st byte and 2nd byte
static void pack_sense_dir_controller(uint8_t *dest, uint8_t sense_dir, int controller) {
    dest[0] = (uint8_t) controller;
}

// Parameter IDs for each field
enum {
    PARAM_R = 1,
    PARAM_L,
    PARAM_KV,
    PARAM_VLIM,
    PARAM_ILIM,
    PARAM_VELLIM,
    PARAM_ZEA,
    PARAM_VEL_KP,
    PARAM_VEL_KI,
    PARAM_VEL_KD,
    PARAM_POS_KP,
    PARAM_POS_KI,
    PARAM_POS_KD,
    PARAM_CONTROLLER_SENSEDIR // Special: controller (int, 4 bytes) + sense_dir (1 bit in byte 0)
};

// Main function to send all fields
void send_motor_params_over_can(uint8_t motor_index, const MotorParams *params) {
    twai_message_t msg = {0};
    float *float_params[] = {
        &params->R, &params->L, &params->kV, &params->vel_lim,
        &params->v_lim, &params->I_lim,
        &params->zea,
        &params->vel_pid[0], &params->vel_pid[1], &params->vel_pid[2],
        &params->pos_pid[0], &params->pos_pid[1], &params->pos_pid[2]
    };

    // Send all float parameters
    for (uint8_t i = 0; i < 13; ++i) {
        msg.identifier = (motor_index << 8) | i;
        msg.data_length_code = 4;
        pack_32bits(msg.data, float_params[i]);
        xQueueSend(ws_to_can_queue, &msg, 0);
    }
    // sense_dir (8 bits) + controller (8 bits), rest zero
    msg.identifier = (motor_index << 8) | 13;
    msg.data_length_code = 1;
    pack_sense_dir_controller(msg.data, params->sense_dir, params->controller);
    xQueueSend(ws_to_can_queue, &msg, 0);
    ESP_LOGI(REST_TAG, "Sent motor params over CAN for motor %d", motor_index);
}

#define MAX_WS_CLIENTS 4
static int ws_clients[MAX_WS_CLIENTS] = {0};
static int ws_client_count = 0;
static portMUX_TYPE ws_clients_mux = portMUX_INITIALIZER_UNLOCKED;

struct ws_async_arg {
    char *msg;
};
#define REST_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                 \
    {                                                                                  \
        if (!(a))                                                                      \
        {                                                                              \
            ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                             \
        }                                                                              \
    } while (0)

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (10240)

typedef struct rest_server_context {
    char base_path[ESP_VFS_PATH_MAX + 1];
    char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
    const char *type = "text/plain";
    if (CHECK_FILE_EXTENSION(filepath, ".html")) {
        type = "text/html";
    } else if (CHECK_FILE_EXTENSION(filepath, ".js")) {
        type = "application/javascript";
    } else if (CHECK_FILE_EXTENSION(filepath, ".css")) {
        type = "text/css";
    } else if (CHECK_FILE_EXTENSION(filepath, ".png")) {
        type = "image/png";
    } else if (CHECK_FILE_EXTENSION(filepath, ".ico")) {
        type = "image/x-icon";
    } else if (CHECK_FILE_EXTENSION(filepath, ".svg")) {
        type = "image/svg+xml";
    }
    return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
    strlcpy(filepath, rest_context->base_path, sizeof(filepath));
    if (req->uri[strlen(req->uri) - 1] == '/') {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else if (strchr(req->uri, '.') == NULL) {
        strlcat(filepath, "/index.html", sizeof(filepath));
    } else {
        strlcat(filepath, req->uri, sizeof(filepath));
    }
    int fd = open(filepath, O_RDONLY, 0);
    if (fd == -1) { /* Respond with 500 Internal Server Error */
        ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
        return ESP_FAIL;
    }
    set_content_type_from_file(req, filepath);
    char *chunk = rest_context->scratch;
    ssize_t read_bytes;
    do { /* Read file in chunks into the scratch buffer */
        read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
        if (read_bytes == -1) {
            ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
        } else if (read_bytes > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
                close(fd);
                ESP_LOGE(REST_TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (read_bytes > 0);
    /* Close file after sending complete */
    close(fd);
    ESP_LOGI(REST_TAG, "File sending complete");
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}



static esp_err_t r1m1_params_post_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // Expected order: R,L,kV,v_lim,I_lim,vel_lim,sense_dir,zea,vel_kp,vel_ki,vel_kd,pos_kp,pos_ki,pos_kd,controller
    char buf[512];
    int total_len = req->content_len;
    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }
    buf[received] = '\0';

    float vals[14];
    int controller = 0;
    int sense_dir = 0;
    ESP_LOGI(REST_TAG, "Received: %s", buf);
    int parsed = sscanf(buf, "%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%d",
        &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &sense_dir, &vals[6],
        &vals[7], &vals[8], &vals[9], &vals[10], &vals[11], &vals[12], &controller);

    ESP_LOGI(REST_TAG, "Parsed: %d", parsed);

    if (parsed < 15) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameter count");
        return ESP_FAIL;
    }

    r1m1_params.R = vals[0];
    r1m1_params.L = vals[1];
    r1m1_params.kV = vals[2];
    r1m1_params.v_lim = vals[3];
    r1m1_params.I_lim = vals[4];
    r1m1_params.vel_lim = vals[5];
    r1m1_params.sense_dir = (uint8_t)sense_dir; // Now 8-bit integer
    r1m1_params.zea = vals[6];
    r1m1_params.vel_pid[0] = vals[7];
    r1m1_params.vel_pid[1] = vals[8];
    r1m1_params.vel_pid[2] = vals[9];
    r1m1_params.pos_pid[0] = vals[10];
    r1m1_params.pos_pid[1] = vals[11];
    r1m1_params.pos_pid[2] = vals[12];
    r1m1_params.controller = controller;

    send_motor_params_over_can(0, &r1m1_params);

    httpd_resp_sendstr(req, "r1m1_params updated");
    return ESP_OK;
}

static esp_err_t r1m2_params_post_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    char buf[256];
    int total_len = req->content_len;
    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }
    buf[received] = '\0';

    float vals[14];
    int controller = 0;
    int sense_dir = 0;
    ESP_LOGI(REST_TAG, "Received: %s", buf);
    int parsed = sscanf(buf, "%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%d",
        &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &sense_dir, &vals[6],
        &vals[7], &vals[8], &vals[9], &vals[10], &vals[11], &vals[12], &controller);

    ESP_LOGI(REST_TAG, "Parsed: %d", parsed);

    if (parsed < 15) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameter count");
        return ESP_FAIL;
    }

    r1m2_params.R = vals[0];
    r1m2_params.L = vals[1];
    r1m2_params.kV = vals[2];
    r1m2_params.v_lim = vals[3];
    r1m2_params.I_lim = vals[4];
    r1m2_params.vel_lim = vals[5];
    r1m2_params.sense_dir = (uint8_t)sense_dir; // Now 8-bit integer
    r1m2_params.zea = vals[6];
    r1m2_params.vel_pid[0] = vals[7];
    r1m2_params.vel_pid[1] = vals[8];
    r1m2_params.vel_pid[2] = vals[9];
    r1m2_params.pos_pid[0] = vals[10];
    r1m2_params.pos_pid[1] = vals[11];
    r1m2_params.pos_pid[2] = vals[12];
    r1m2_params.controller = controller;

    send_motor_params_over_can(1, &r1m2_params);

    httpd_resp_sendstr(req, "r1m2_params updated");
    return ESP_OK;
}

static esp_err_t r2m1_params_post_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    char buf[256];
    int total_len = req->content_len;
    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }
    buf[received] = '\0';

    float vals[14];
    int controller = 0;
    int sense_dir = 0;
    ESP_LOGI(REST_TAG, "Received: %s", buf);
    int parsed = sscanf(buf, "%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%d",
        &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &sense_dir, &vals[6],
        &vals[7], &vals[8], &vals[9], &vals[10], &vals[11], &vals[12], &controller);

    ESP_LOGI(REST_TAG, "Parsed: %d", parsed);

    if (parsed < 15) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameter count");
        return ESP_FAIL;
    }

    r2m1_params.R = vals[0];
    r2m1_params.L = vals[1];
    r2m1_params.kV = vals[2];
    r2m1_params.v_lim = vals[3];
    r2m1_params.I_lim = vals[4];
    r2m1_params.vel_lim = vals[5];
    r2m1_params.sense_dir = (uint8_t)sense_dir; // Now 8-bit integer
    r2m1_params.zea = vals[6];
    r2m1_params.vel_pid[0] = vals[7];
    r2m1_params.vel_pid[1] = vals[8];
    r2m1_params.vel_pid[2] = vals[9];
    r2m1_params.pos_pid[0] = vals[10];
    r2m1_params.pos_pid[1] = vals[11];
    r2m1_params.pos_pid[2] = vals[12];
    r2m1_params.controller = controller;

    send_motor_params_over_can(2, &r2m1_params);

    httpd_resp_sendstr(req, "r2m1_params updated");
    return ESP_OK;
}

static esp_err_t r2m2_params_post_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    char buf[256];
    int total_len = req->content_len;
    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }
    buf[received] = '\0';

    float vals[14];
    int controller = 0;
    int sense_dir = 0;
    ESP_LOGI(REST_TAG, "Received: %s", buf);
    int parsed = sscanf(buf, "%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%f,%f,%f,%d",
        &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &sense_dir, &vals[6],
        &vals[7], &vals[8], &vals[9], &vals[10], &vals[11], &vals[12], &controller);

    ESP_LOGI(REST_TAG, "Parsed: %d", parsed);

    if (parsed < 15) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameter count");
        return ESP_FAIL;
    }

    r2m2_params.R = vals[0];
    r2m2_params.L = vals[1];
    r2m2_params.kV = vals[2];
    r2m2_params.v_lim = vals[3];
    r2m2_params.I_lim = vals[4];
    r2m2_params.vel_lim = vals[5];
    r2m2_params.sense_dir = (uint8_t)sense_dir; // Now 8-bit integer
    r2m2_params.zea = vals[6];
    r2m2_params.vel_pid[0] = vals[7];
    r2m2_params.vel_pid[1] = vals[8];
    r2m2_params.vel_pid[2] = vals[9];
    r2m2_params.pos_pid[0] = vals[10];
    r2m2_params.pos_pid[1] = vals[11];
    r2m2_params.pos_pid[2] = vals[12];
    r2m2_params.controller = controller;

    send_motor_params_over_can(3, &r2m2_params);

    httpd_resp_sendstr(req, "r2m2_params updated");
    return ESP_OK;
}

static esp_err_t home_post_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    // Sends as plain text {robot} where robot is 1 or 2
    char buf[256];
    int total_len = req->content_len;
    if (total_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    int received = httpd_req_recv(req, buf, total_len);
    if (received <= 0) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive body");
        return ESP_FAIL;
    }
    buf[received] = '\0';
    
    int robot = 0;
    int parsed = sscanf(buf, "%d", &robot);
    if (parsed < 1) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid parameter count");
        return ESP_FAIL;
    }
    if (robot < 1 || robot > 2) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid robot number");
        return ESP_FAIL;
    }

    // When this message is sent, assume the robot is at home and set the joint angle offsets accordingly




    httpd_resp_sendstr(req, "Home command sent to CAN");
    return ESP_OK;
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    if (req->method == HTTP_GET) {
        int sockfd = httpd_req_to_sockfd(req);
        portENTER_CRITICAL(&ws_clients_mux);
        if (ws_client_count < MAX_WS_CLIENTS) {
            ws_clients[ws_client_count++] = sockfd;
        }
        portEXIT_CRITICAL(&ws_clients_mux);
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = NULL;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE("ws", "Failed to get frame length: %d", ret);
        return ret;
    }

    ws_pkt.payload = malloc(ws_pkt.len + 1);
    if (ws_pkt.payload == NULL) {
        ESP_LOGE("ws", "Failed to allocate memory for payload");
        return ESP_ERR_NO_MEM;
    }

    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
        ESP_LOGE("ws", "Failed to receive frame: %d", ret);
        free(ws_pkt.payload);
        return ret;
    }
    ws_pkt.payload[ws_pkt.len] = 0;

    ESP_LOGI("ws", "Received: %s", (char*)ws_pkt.payload);

    // Parse 4 comma-separated floats and send each as a CAN message
    float vals[4];
    int parsed = sscanf((char*)ws_pkt.payload, "%f,%f,%f,%f", &vals[0], &vals[1], &vals[2], &vals[3]);
    if (parsed == 4) {
        uint32_t ids[4] = {0x014, 0x114, 0x214, 0x314};
        for (int i = 0; i < 4; ++i) {
            twai_message_t msg = {0};
            msg.identifier = ids[i];
            msg.data_length_code = 4;
            memcpy(msg.data, &vals[i], sizeof(float));
            if (ws_to_can_queue) {
                xQueueSend(ws_to_can_queue, &msg, 0);
            }
        }
    } else {
        ESP_LOGE("ws", "Invalid float count in WebSocket payload");
    }

    free(ws_pkt.payload);
    return ESP_OK;
}

void ws_remove_client(int sockfd) {
    portENTER_CRITICAL(&ws_clients_mux);
    for (int i = 0; i < ws_client_count; ++i) {
        if (ws_clients[i] == sockfd) {
            for (int j = i; j < ws_client_count - 1; ++j) {
                ws_clients[j] = ws_clients[j + 1];
            }
            ws_client_count--;
            break;
        }
    }
    portEXIT_CRITICAL(&ws_clients_mux);
}

static void ws_async_send(void *arg) {
    struct ws_async_arg *ws_arg = (struct ws_async_arg *)arg;
    httpd_ws_frame_t ws_pkt = {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t *)ws_arg->msg,
        .len = strlen(ws_arg->msg)
    };

    portENTER_CRITICAL(&ws_clients_mux);
    int count = ws_client_count;
    int fds[MAX_WS_CLIENTS];
    memcpy(fds, ws_clients, sizeof(fds));
    portEXIT_CRITICAL(&ws_clients_mux);

    for (int i = 0; i < count; ++i) {
        int fd_info = httpd_ws_get_fd_info(global_httpd_server, fds[i]);
        if (fd_info == HTTPD_WS_CLIENT_WEBSOCKET) {
            httpd_ws_send_frame_async(global_httpd_server, fds[i], &ws_pkt);
        }
    }
    free(ws_arg->msg);
    free(ws_arg);
}

void ws_broadcast_text(const char *msg) {
    if (!global_httpd_server) return;
    struct ws_async_arg *arg = malloc(sizeof(struct ws_async_arg));
    if (!arg) return;
    arg->msg = strdup(msg);
    if (!arg->msg) { free(arg); return; }
    httpd_queue_work(global_httpd_server, ws_async_send, arg);
}

esp_err_t start_rest_server(const char *base_path)
{
    REST_CHECK(base_path, "wrong base path", err);
    rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
    REST_CHECK(rest_context, "No memory for rest context", err);
    strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_uri_handlers = 18;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);
    
    global_httpd_server = server;

    // Register r1m1 handlers
    httpd_uri_t r1m1_post_uri = {
        .uri = "/api/v1/r1m1_params",
        .method = HTTP_POST,
        .handler = r1m1_params_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &r1m1_post_uri);

    // Register r1m2 handlers
    httpd_uri_t r1m2_post_uri = {
        .uri = "/api/v1/r1m2_params",
        .method = HTTP_POST,
        .handler = r1m2_params_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &r1m2_post_uri);

    // Register r2m1 handlers
    httpd_uri_t r2m1_post_uri = {
        .uri = "/api/v1/r2m1_params",
        .method = HTTP_POST,
        .handler = r2m1_params_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &r2m1_post_uri);

    // Register r2m2 handlers
    httpd_uri_t r2m2_post_uri = {
        .uri = "/api/v1/r2m2_params",
        .method = HTTP_POST,
        .handler = r2m2_params_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &r2m2_post_uri);

    // Register home handler
    httpd_uri_t home_post_uri = {
        .uri = "/api/v1/home",
        .method = HTTP_POST,
        .handler = home_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &home_post_uri);

    // Register the WebSocket handler BEFORE the wildcard handler
    httpd_uri_t ws_uri = {
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = ws_handler,
        .user_ctx = rest_context,
        .is_websocket = true
    };
    httpd_register_uri_handler(server, &ws_uri);

    /* URI handler for getting web server files */
    httpd_uri_t common_get_uri = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = rest_common_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &common_get_uri);

    return ESP_OK;
err_start:
    free(rest_context);
err:
    return ESP_FAIL;
}

/* Example CORS preflight handler */
static esp_err_t cors_options_handler(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}