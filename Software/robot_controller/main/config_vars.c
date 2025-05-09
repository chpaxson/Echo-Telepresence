#include "config_vars.h"

MotorParams r1m1_params = {
    .R = 1.3f,
    .L = 0.029f,
    .kV = 25.0f,
    .v_lim = 7.0f,
    .I_lim = 2.0f,
    .vel_lim = 20.0f,
    .sense_dir = false,
    .zea = 0.0f,
    .vel_pid = {1.0f, 0.0f, 0.0f},
    .pos_pid = {1.0f, 0.0f, 0.0f},
    .controller = 0
};
MotorParams r1m2_params = {
    .R = 1.3f,
    .L = 0.029f,
    .kV = 25.0f,
    .v_lim = 7.0f,
    .I_lim = 2.0f,
    .vel_lim = 20.0f,
    .sense_dir = false,
    .zea = 0.0f,
    .vel_pid = {1.0f, 0.0f, 0.0f},
    .pos_pid = {1.0f, 0.0f, 0.0f},
    .controller = 0
};
MotorParams r2m1_params = {
    .R = 1.85f,
    .L = 0.029f,
    .kV = 25.0f,
    .v_lim = 7.0f,
    .I_lim = 2.0f,
    .vel_lim = 20.0f,
    .sense_dir = false,
    .zea = 0.0f,
    .vel_pid = {1.0f, 0.0f, 0.0f},
    .pos_pid = {1.0f, 0.0f, 0.0f},
    .controller = 0
};
MotorParams r2m2_params = {
    .R = 1.85f,
    .L = 0.029f,
    .kV = 25.0f,
    .v_lim = 7.0f,
    .I_lim = 2.0f,
    .vel_lim = 20.0f,
    .sense_dir = false,
    .zea = 0.0f,
    .vel_pid = {1.0f, 0.0f, 0.0f},
    .pos_pid = {1.0f, 0.0f, 0.0f},
    .controller = 0
};

httpd_handle_t global_httpd_server = NULL;