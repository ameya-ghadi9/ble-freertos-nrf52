#ifndef _ADC_H_
#define _ADC_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_drv_power.h"

void saadc_init(void);
void saadc_sampling_event_init(void);
void saadc_sampling_event_enable(void);
void saadc_callback(nrf_drv_saadc_evt_t const * p_event);

uint16_t get_raw_temperature(void);
uint16_t get_raw_humidity(void);
uint16_t get_raw_air_quality(void);

uint16_t get_cal_temperature(uint16_t temp_raw);
uint16_t get_cal_humidity(uint16_t humidity_raw);
//TO BE FIXED
uint16_t get_cal_air_quality(uint16_t air_qual_raw);

#endif