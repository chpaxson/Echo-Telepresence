#include "config_vars.h"

// Robot 1 Velocity PID Parameters (kP, kI, kD)
float r1m1_vel_pid[3] = {0.5, 10.0, 0.0};
float r1m2_vel_pid[3] = {0.5, 10.0, 0.0};
// Robot 1 Position PID Parameters (kP, kI, kD)
float r1m1_pos_pid[3] = {0.5, 0.0, 0.0};
float r1m2_pos_pid[3] = {0.5, 0.0, 0.0};

// Robot 2 Velocity PID Parameters (kP, kI, kD)
float r2m1_vel_pid[3] = {0.5, 10.0, 0.0};
float r2m2_vel_pid[3] = {0.5, 10.0, 0.0};
// Robot 2 Position PID Parameters (kP, kI, kD)
float r2m1_pos_pid[3] = {0.5, 0.0, 0.0};
float r2m2_pos_pid[3] = {0.5, 0.0, 0.0};

// Input voltage chopping parameter
float r1_v_chop = 24.0;
float r2_v_chop = 24.0;

// Robot 1 motor parameters
float r1m1_R = 1.3;
float r1m2_R = 1.3;
float r1m1_I = 29;
float r1m2_I = 29;
// Robot 2 motor parameters
float r2m1_R = 1.3;
float r2m2_R = 1.3;
float r2m1_I = 29;
float r2m2_I = 29;

// Maximum kinematic velocity
float r1_max_vel = 20.0;
float r2_max_vel = 20.0;