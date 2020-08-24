#ifndef _BLE_ADVERTISEMENT_H
#define _BLE_ADVERTISEMENT_H

#include <stdint.h>
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_gap.h"

#define HLTH_BRRY_BLE_UUID                      0xFACE
#define APP_ADV_INTERVAL                300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                     /**< The advertising timeout in units of seconds. */

#define HEALTH_BERRY_ID (0x5BD0)

/*
#define MANUF_ADV_DATA_SIZE   (BLE_GAP_ADV_MAX_SIZE                - \
                               (AD_TYPE_FLAGS_SIZE                 + \
                                ADV_AD_DATA_OFFSET                 + \
                                AD_TYPE_SERV_DATA_16BIT_UUID_SIZE  + \
                                ADV_AD_DATA_OFFSET                 + \
                                AD_TYPE_MANUF_SPEC_DATA_ID_SIZE))
*/
#define MANUF_ADV_DATA_SIZE 10

#define ADV_PTI_SEED_BYTE                       0
#define ADV_MAC_OFFSET                          1
#define ADV_PTI_BYTE                            7
#define ADV_STATUS_BYTE                         8

void advertising_init(void);
void advertising_start(bool erase_bonds);

#endif