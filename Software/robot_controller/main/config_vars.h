#ifndef CONFIG_H
#define CONFIG_H

// Robot 1 Velocity PID Parameters (kP, kI, kD)
extern float r1m1_vel_pid[3];
extern float r1m2_vel_pid[3];
// Robot 1 Position PID Parameters (kP, kI, kD)
extern float r1m1_pos_pid[3];
extern float r1m2_pos_pid[3];

// Robot 2 Velocity PID Parameters (kP, kI, kD)
extern float r2m1_vel_pid[3];
extern float r2m2_vel_pid[3];
// Robot 2 Position PID Parameters (kP, kI, kD)
extern float r2m1_pos_pid[3];
extern float r2m2_pos_pid[3];

extern float r1_max_vel;
extern float r1_v_chop;
extern float r2_max_vel;
extern float r2_v_chop;

// Robot 1 motor parameters
extern float r1m1_R;
extern float r1m1_I;
extern float r1m2_R;
extern float r1m2_I;
// Robot 2 motor parameters
extern float r2m1_R;
extern float r2m1_I;
extern float r2m2_R;
extern float r2m2_I;

#endif