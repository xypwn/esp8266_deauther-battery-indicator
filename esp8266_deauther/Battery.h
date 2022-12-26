/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#pragma once

#include "Arduino.h"

namespace battery {
double calibrate(double voltage, size_t samples);
double getPercentage(double calibrationFactor, size_t samples);
}
