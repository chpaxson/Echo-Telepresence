#ifndef CONFIG_H
#include "esp_http_server.h"
#define CONFIG_H

// Robot 1 parameters
// Motor 1
extern float r1m1_vel_pid[3];
extern float r1m1_pos_pid[3];
extern float r1m1_R;
extern float r1m1_I;
extern float r1m1_vlim;
// Motor 2
extern float r1m2_vel_pid[3];
extern float r1m2_pos_pid[3];
extern float r1m2_R;
extern float r1m2_I;
extern float r1m2_vlim;

// Robot 2 parameters
// Motor 1
extern float r2m1_vel_pid[3];
extern float r2m2_vel_pid[3];
extern float r2m1_R;
extern float r2m1_I;
extern float r2m1_vlim;
// Motor 2
extern float r2m1_pos_pid[3];
extern float r2m2_pos_pid[3];
extern float r2m2_R;
extern float r2m2_I;
extern float r2m2_vlim;


extern float r1_max_vel;
extern float r2_max_vel;

extern httpd_handle_t global_httpd_server;

#endif