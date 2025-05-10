#ifndef CONFIG_H
#include "esp_http_server.h"

#define CONFIG_H
typedef struct {
    float R;
    float L;
    float kV;
    float v_lim;
    float I_lim;
    float vel_lim;
    bool sense_dir;
    float zea;
    float vel_pid[3];
    float pos_pid[3];
    int controller;
} MotorParams;

extern MotorParams r1m1_params;
extern MotorParams r1m2_params;
extern MotorParams r2m1_params;
extern MotorParams r2m2_params;

extern httpd_handle_t global_httpd_server;

extern float r1a1_offset;
extern float r1a2_offset;
extern float r2a1_offset;
extern float r2a2_offset;

#endif