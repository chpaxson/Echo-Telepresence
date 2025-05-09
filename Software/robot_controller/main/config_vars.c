#include "config_vars.h"

// Robot 1 parameters
// Motor 1
float r1m1_vel_pid[3] = {0.5, 10.0, 0.0};
float r1m1_pos_pid[3] = {0.5, 0.0, 0.0};
float r1m1_R = 1.3;
float r1m1_I = 29;
float r1m1_vlim = 24.0;

// Motor 2
float r1m2_vel_pid[3] = {0.5, 10.0, 0.0};
float r1m2_pos_pid[3] = {0.5, 0.0, 0.0};
float r1m2_R = 1.3;
float r1m2_I = 29;
float r1m2_vlim = 24.0;

// Robot 2 parameters
// Motor 1
float r2m1_vel_pid[3] = {0.5, 10.0, 0.0};
float r2m1_pos_pid[3] = {0.5, 0.0, 0.0};
float r2m1_R = 1.3;
float r2m1_I = 29;
float r2m1_vlim = 24.0;

// Motor 2
float r2m2_vel_pid[3] = {0.5, 10.0, 0.0};
float r2m2_pos_pid[3] = {0.5, 0.0, 0.0};
float r2m2_R = 1.3;
float r2m2_I = 29;
float r2m2_vlim = 24.0;

// Maximum kinematic velocity
float r1_max_vel = 20.0;
float r2_max_vel = 20.0;

httpd_handle_t global_httpd_server = NULL;