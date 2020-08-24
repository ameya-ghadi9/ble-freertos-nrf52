#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <stdint.h>
#include <string.h>
#include "math.h"


int32_t Calibrate_Temperature(int32_t raw_temp_adc);

int32_t Calibrate_Humidity(int32_t raw_humidity_adc);

uint16_t Calibrate_Mq2(uint16_t raw_mq2_adc);

#endif