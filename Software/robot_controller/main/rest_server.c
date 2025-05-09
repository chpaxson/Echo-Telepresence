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
#define MAX_WS_CLIENTS 4
static int ws_clients[MAX_WS_CLIENTS] = {0};
static int ws_client_count = 0;
static portMUX_TYPE ws_clients_mux = portMUX_INITIALIZER_UNLOCKED;

struct ws_async_arg {
    char *msg;
};

static const char *REST_TAG = "esp-rest";
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
// Robot parameter JSON structure
// {
//     r1: {
//         m1: { v_lim: 24 as number, R: 1.3 as number, I: 29 as number, v_kP: 0.5 as number, v_kI: 0.1 as number, v_kD: 0.0 as number, a_kP: 0.5 as number, a_kI: 0.0 as number, a_kD: 0.0 as number},
//         m2: {v_lim: 24 as number, R: 1.3 as number, I: 29 as number, v_kP: 0.5 as number, v_kI: 0.1 as number, v_kD: 0.0 as number, a_kP: 0.5 as number, a_kI: 0.0 as number, a_kD: 0.0 as number}
//     }, 
//     r2: {
//         m1: { v_lim: 24 as number, R: 1.3 as number, I: 29 as number, v_kP: 0.5 as number, v_kI: 0.1 as number, v_kD: 0.0 as number, a_kP: 0.5 as number, a_kI: 0.0 as number, a_kD: 0.0 as number},
//         m2: {v_lim: 24 as number, R: 1.3 as number, I: 29 as number, v_kP: 0.5 as number, v_kI: 0.1 as number, v_kD: 0.0 as number, a_kP: 0.5 as number, a_kI: 0.0 as number, a_kD: 0.0 as number}
//     }
// }
static esp_err_t motorParams_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();

    cJSON *r1 = cJSON_CreateObject();
    cJSON *r2 = cJSON_CreateObject();

    cJSON *r1m1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(r1m1, "v_lim", r1m1_vlim);
    cJSON_AddNumberToObject(r1m1, "R", r1m1_R);
    cJSON_AddNumberToObject(r1m1, "I", r1m1_I);
    cJSON_AddNumberToObject(r1m1, "v_kp", r1m1_vel_pid[0]);
    cJSON_AddNumberToObject(r1m1, "v_ki", r1m1_vel_pid[1]);
    cJSON_AddNumberToObject(r1m1, "v_kd", r1m1_vel_pid[2]);
    cJSON_AddNumberToObject(r1m1, "a_kp", r1m1_pos_pid[0]);
    cJSON_AddNumberToObject(r1m1, "a_ki", r1m1_pos_pid[1]);
    cJSON_AddNumberToObject(r1m1, "a_kd", r1m1_pos_pid[2]);
    cJSON_AddItemToObject(r1, "m1", r1m1);

    cJSON *r1m2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(r1m2, "v_lim", r1m2_vlim);
    cJSON_AddNumberToObject(r1m2, "R", r1m2_R);
    cJSON_AddNumberToObject(r1m2, "I", r1m2_I);
    cJSON_AddNumberToObject(r1m2, "v_kp", r1m2_vel_pid[0]);
    cJSON_AddNumberToObject(r1m2, "v_ki", r1m2_vel_pid[1]);
    cJSON_AddNumberToObject(r1m2, "v_kd", r1m2_vel_pid[2]);
    cJSON_AddNumberToObject(r1m2, "a_kp", r1m2_pos_pid[0]);
    cJSON_AddNumberToObject(r1m2, "a_ki", r1m2_pos_pid[1]);
    cJSON_AddNumberToObject(r1m2, "a_kd", r1m2_pos_pid[2]);
    cJSON_AddItemToObject(r1, "m2", r1m2);

    cJSON *r2m1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(r2m1, "v_lim", r2m1_vlim);
    cJSON_AddNumberToObject(r2m1, "R", r2m1_R);
    cJSON_AddNumberToObject(r2m1, "I", r2m1_I);
    cJSON_AddNumberToObject(r2m1, "v_kp", r2m1_vel_pid[0]);
    cJSON_AddNumberToObject(r2m1, "v_ki", r2m1_vel_pid[1]);
    cJSON_AddNumberToObject(r2m1, "v_kd", r2m1_vel_pid[2]);
    cJSON_AddNumberToObject(r2m1, "a_kp", r2m1_pos_pid[0]);
    cJSON_AddNumberToObject(r2m1, "a_ki", r2m1_pos_pid[1]);
    cJSON_AddNumberToObject(r2m1, "a_kd", r2m1_pos_pid[2]);
    cJSON_AddItemToObject(r2, "m1", r2m1);

    cJSON *r2m2 = cJSON_CreateObject();
    cJSON_AddNumberToObject(r2m2, "v_lim", r2m2_vlim);
    cJSON_AddNumberToObject(r2m2, "R", r2m2_R);
    cJSON_AddNumberToObject(r2m2, "I", r2m2_I);
    cJSON_AddNumberToObject(r2m2, "v_kp", r2m2_vel_pid[0]);
    cJSON_AddNumberToObject(r2m2, "v_ki", r2m2_vel_pid[1]);
    cJSON_AddNumberToObject(r2m2, "v_kd", r2m2_vel_pid[2]);
    cJSON_AddNumberToObject(r2m2, "a_kp", r2m2_pos_pid[0]);
    cJSON_AddNumberToObject(r2m2, "a_ki", r2m2_pos_pid[1]);
    cJSON_AddNumberToObject(r2m2, "a_kd", r2m2_pos_pid[2]);
    cJSON_AddItemToObject(r2, "m2", r2m2);

    cJSON_AddItemToObject(root, "r1", r1);
    cJSON_AddItemToObject(root, "r2", r2);

    const char *pid_params = cJSON_Print(root);
    httpd_resp_sendstr(req, pid_params);
    free((void *)pid_params);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t motParams_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    // Helper macro to extract values safely
    #define SET_PID_FROM_JSON(rob, mot, vlim, R, I, vel_arr, pos_arr) do { \
        cJSON *rob_obj = cJSON_GetObjectItem(root, #rob); \
        if (rob_obj) { \
            cJSON *mot_obj = cJSON_GetObjectItem(rob_obj, #mot); \
            if (mot_obj) { \
                cJSON *c_vlim = cJSON_GetObjectItem(mot_obj, "v_lim"); \
                cJSON *c_R = cJSON_GetObjectItem(mot_obj, "R"); \
                cJSON *c_I = cJSON_GetObjectItem(mot_obj, "I"); \
                cJSON *v_kp = cJSON_GetObjectItem(mot_obj, "v_kp"); \
                cJSON *v_ki = cJSON_GetObjectItem(mot_obj, "v_ki"); \
                cJSON *v_kd = cJSON_GetObjectItem(mot_obj, "v_kd"); \
                cJSON *a_kp = cJSON_GetObjectItem(mot_obj, "a_kp"); \
                cJSON *a_ki = cJSON_GetObjectItem(mot_obj, "a_ki"); \
                cJSON *a_kd = cJSON_GetObjectItem(mot_obj, "a_kd"); \
                if (v_kp && v_ki && v_kd && a_kp && a_ki && a_kd) { \
                    vlim = c_vlim->valuedouble; \
                    R = c_R->valuedouble; \
                    I = c_I->valuedouble; \
                    vel_arr[0] = v_kp->valuedouble; \
                    vel_arr[1] = v_ki->valuedouble; \
                    vel_arr[2] = v_kd->valuedouble; \
                    pos_arr[0] = a_kp->valuedouble; \
                    pos_arr[1] = a_ki->valuedouble; \
                    pos_arr[2] = a_kd->valuedouble; \
                } \
            } \
        } \
    } while(0)

    SET_PID_FROM_JSON(r1, m1, r1m1_vlim, r1m1_R, r1m1_I, r1m1_vel_pid, r1m1_pos_pid);
    SET_PID_FROM_JSON(r1, m2, r1m2_vlim, r1m2_R, r1m2_I, r1m2_vel_pid, r1m2_pos_pid);
    SET_PID_FROM_JSON(r2, m1, r2m1_vlim, r2m1_R, r2m1_I, r2m1_vel_pid, r2m1_pos_pid);
    SET_PID_FROM_JSON(r2, m2, r2m2_vlim, r2m2_R, r2m2_I, r2m2_vel_pid, r2m2_pos_pid);

    cJSON_Delete(root);

    httpd_resp_sendstr(req, "PID parameters updated successfully");
    return ESP_OK;
}

static esp_err_t home_post_handler(httpd_req_t *req)
{
    int total_len = req->content_len;
    int cur_len = 0;
    char *buf = ((rest_server_context_t *)(req->user_ctx))->scratch;
    int received = 0;
    if (total_len >= SCRATCH_BUFSIZE) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }
    cJSON *r1 = cJSON_GetObjectItem(root, "r1");
    cJSON *r2 = cJSON_GetObjectItem(root, "r2");
    uint8_t r1m1 = cJSON_GetObjectItem(r1, "m1")->valueint;
    uint8_t r1m2 = cJSON_GetObjectItem(r1, "m2")->valueint;
    uint8_t r2m1 = cJSON_GetObjectItem(r2, "m1")->valueint;
    uint8_t r2m2 = cJSON_GetObjectItem(r2, "m2")->valueint;

    ESP_LOGI(REST_TAG, "r1m1: %d, r1m2: %d, r2m1: %d, r2m2: %d", r1m1, r1m2, r2m1, r2m2);
            
    cJSON_Delete(root);

    httpd_resp_sendstr(req, "Homing motors...");
    return ESP_OK;
}

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

/* Simple handler for getting temperature data */
static esp_err_t temperature_data_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "raw", esp_random() % 20);
    const char *sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void *)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t ws_handler(httpd_req_t *req)
{
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

    // Echo back (for now)
    httpd_ws_frame_t ws_res = {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = ws_pkt.payload,
        .len = ws_pkt.len
    };
    httpd_ws_send_frame(req, &ws_res);

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
    config.max_uri_handlers = 12;

    ESP_LOGI(REST_TAG, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);
    
    global_httpd_server = server;

    // Register consolidated PID GET and POST handlers
    httpd_uri_t motParams_get_uri = {
        .uri = "/api/v1/mParams ",
        .method = HTTP_GET,
        .handler = motorParams_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &motParams_get_uri);

    httpd_uri_t motParams_post_uri = {
        .uri = "/api/v1/mParams",
        .method = HTTP_POST,
        .handler = motParams_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &motParams_post_uri);

    httpd_uri_t home_post_uri = {
        .uri = "/api/v1/homing",
        .method = HTTP_POST,
        .handler = home_post_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &home_post_uri);

    /* URI handler for fetching system info */
    httpd_uri_t system_info_get_uri = {
        .uri = "/api/v1/system/info",
        .method = HTTP_GET,
        .handler = system_info_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &system_info_get_uri);

    /* URI handler for fetching temperature data */
    httpd_uri_t temperature_data_get_uri = {
        .uri = "/api/v1/temp/raw",
        .method = HTTP_GET,
        .handler = temperature_data_get_handler,
        .user_ctx = rest_context
    };
    httpd_register_uri_handler(server, &temperature_data_get_uri);

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