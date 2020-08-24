#ifndef _BLE_SERVICES_H_
#define _BLE_SERVICES_H_


void services_uuid_init(void);

/**@file
 *
 * @defgroup ble_nus Nordic UART Service
 * @{
 * @ingroup  ble_sdk_srv
 * @brief    Nordic UART Service implementation.
 *
 * @details The Nordic UART Service is a simple GATT-based service with TX and RX characteristics.
 *          Data received from the peer is passed to the application, and the data received
 *          from the application of this service is sent to the peer as Handle Value
 *          Notifications. This module demonstrates how to implement a custom GATT-based
 *          service and characteristics using the SoftDevice. The service
 *          is used by the application to send and receive ASCII text strings to and from the
 *          peer.
 *
 * @note The application must propagate SoftDevice events to the Nordic UART Service module
 *       by calling the ble_nus_on_ble_evt() function from the ble_stack_handler callback.
 */

#include "sdk_config.h"
#include "ble_stack_handler_types.h"

#include "ble.h"
#include "ble_srv_common.h"

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2

#if defined(NRF_BLE_GATT_MAX_MTU_SIZE) && (NRF_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_NUS_MAX_DATA_LEN (NRF_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
#else
    #define BLE_NUS_MAX_DATA_LEN (BLE_GATT_MTU_SIZE_DEFAULT - OPCODE_LENGTH - HANDLE_LENGTH) /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
    #warning NRF_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif

   /**@brief Nordic UART Service structure.
 *
 * @details This structure contains status information related to the service.
 */
typedef struct ble_smart_home_struct
{
    uint16_t                 service_handle;          /**< Handle of Nordic UART Service (as provided by the SoftDevice). */
    //ble_gatts_char_handles_t tx_handles;              /**< Handles related to the TX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t temperature_char_handles;              /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t humidity_char_handles;              /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t air_quality_char_handles;              /**< Handles related to the RX characteristic (as provided by the SoftDevice). */
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    //bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
    //ble_smart_home_handler_t   data_handler;            /**< Event handler to be called for handling received data. */
} ble_smart_home_struct_t;

extern ble_smart_home_struct_t smart_home;

void services_init(void);

typedef enum characteristic_value_t{
  temperature = 0,
  humidity,
  air_quality
}characteristic_value_type;

void characteristic_update(int32_t *sensor_adc_value, characteristic_value_type cvt);

#endif // BLE_NUS_H__