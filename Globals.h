#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define NRF_LOG_MODULE_NAME "APP"
#define CONN_CFG_TAG                     1                                          /**< A tag that refers to the BLE stack configuration we set with @ref sd_ble_cfg_set. Default tag is @ref BLE_CONN_CFG_TAG_DEFAULT. */
#define DEVICE_NAME                      "SMART HOME"
#define MANUFACTURER_NAME                "Gray Matter Tech"                  /**< Manufacturer. Will be passed to Device Information Service. */

#define BLE_UUID_TEMPERATURE_CHARACTERISTIC 0xCAFE
#define BLE_UUID_HUMIDITY_CHARACTERISTIC 0xBABE
#define BLE_UUID_AIR_QUALITY_CHARACTERISTIC 0xD00D

#define BLE_SMART_HOME_BASE_UUID      {0xEE, 0x08, 0x70, 0x24,  0xC0, 0xE9, 0x48, 0xCE, 0x8F, 0xCD, 0x04, 0x43, 0xE7, 0x9B, 0x24, 0xA1}
#define BLE_SMART_HOME_16BIT_UUID       0xABCD


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_timer.h"
#include "app_error.h"
#include "app_timer.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_bas.h"
#include "ble_hrs.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "softdevice_handler.h"
#include "peer_manager.h"
#include "bsp.h"
#include "bsp_config.h"
#include "bsp_btn_ble.h"
#include "fds.h"
#include "fstorage.h"
#include "ble_conn_state.h"
#include "nrf_drv_clock.h"
#include "nrf_ble_gatt.h"

extern uint8_t customChar[8];

void sleep_mode_enter(void);
void log_init(void);
void power_manage(void);

extern uint32_t ble_init_err_code;
extern uint32_t err_ble_advt_code;
extern uint32_t global_err_code;

#endif