#pragma once

#include <cstdint>
#include "hardware_api.h"


typedef struct RP2040DriverParams {
  int pins[6];
  uint32_t slice[6];
  uint32_t chan[6];
  long pwm_frequency;
  float dead_zone;
} RP2040DriverParams;