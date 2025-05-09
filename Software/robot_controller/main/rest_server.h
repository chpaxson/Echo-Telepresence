#pragma once

#include "esp_http_server.h"

void ws_broadcast_text(const char *msg);
void ws_remove_client(int sockfd);