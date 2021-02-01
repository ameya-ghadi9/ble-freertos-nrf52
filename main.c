/**
 * Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
// Board/nrf6310/ble/ble_app_hrs_rtx/main.c
/**
 *
 * @brief Heart Rate Service Sample Application with RTX main file.
 *
 * This file contains the source code for a sample application using RTX and the
 * Heart Rate service (and also Battery and Device Information services).
 * This application uses the @ref srvlib_conn_params module.
 */

//Added from BLE Smart Home
#include "Globals.h"
#include "BLE_Init.h"
#include "BLE_Advertisement.h"
#include "BLE_Services.h"
#include "ADC.h"
#include "Calibration.h"
#include "LCD.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#define NRF_LOG_MODULE_NAME "APP"

#define SEC_PARAM_BOND                   1                                      /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                      /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                      /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                      /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                   /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                      /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                      /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                     /**< Maximum encryption key size. */

#define DEAD_BEEF                        0xDEADBEEF                             /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define OSTIMER_WAIT_FOR_QUEUE           2                                      /**< Number of ticks to wait for the timer queue to be ready */

#define APP_FEATURE_NOT_SUPPORTED        BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2   /**< Reply when unsupported features are requested. */


static uint16_t  m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */
          /**< RR Interval sensor simulator state. */

extern SemaphoreHandle_t m_ble_event_ready;

//task handler- data structure with which the task can be referenced
//is esentially a void*--underneath it is the respective TCB 
static TaskHandle_t m_ble_stack_thread;   //BLE task handle 
static TaskHandle_t m_adc_thread;         //ADC Sampling Thread
static TaskHandle_t m_display_thread;        //LED task handle

static QueueHandle_t m_adc_samples_queue; 

typedef struct data
{
  int32_t temperature;
  int32_t humidity;
  int32_t air_quality;
}sensor_data;


UBaseType_t uxHighWaterMark_ble;
UBaseType_t uxHighWaterMark_disp;
UBaseType_t uxHighWaterMark_adc;

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.\r\n");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.\r\n",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // Retry.
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start(false);
        } break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
        {
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist();
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        default:
            break;
    }
}

/**@brief Function for handling File Data Storage events.
 *
 * @param[in] p_evt  Peer Manager event.
 * @param[in] cmd
 */
static void fds_evt_handler(fds_evt_t const * const p_evt)
{
    if (p_evt->id == FDS_EVT_GC)
    {
        NRF_LOG_DEBUG("GC completed\n");
    }
}


/**@brief Function for the Peer Manager initialization.
 */
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = fds_register(fds_evt_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!\r\n");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}

static void application_timers_start(void)
{

}


/**@brief Thread for handling the Application's BLE Stack events.
 *
 * @details This thread is responsible for handling BLE Stack events sent from on_ble_evt().
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void ble_stack_thread(void * arg)
{
    bool erase_bonds;

    UNUSED_PARAMETER(arg);
 
    ble_stack_init();
    gap_params_init();
    gatt_init();
    
    services_init();
    conn_params_init();

    peer_manager_init();
    advertising_init();

    application_timers_start();
    
    uxHighWaterMark_ble = uxTaskGetStackHighWaterMark( NULL );
    advertising_start(erase_bonds);

    while (1)
    {
        /* Wait for event from SoftDevice */
        while (pdFALSE == xSemaphoreTake(m_ble_event_ready, portMAX_DELAY))
        {
            // Just wait again in the case when INCLUDE_vTaskSuspend is not enabled
        }

        // This function gets events from the SoftDevice and processes them by calling the function
        // registered by softdevice_ble_evt_handler_set during stack initialization.
        // In this code ble_evt_dispatch would be called for every event found.
        intern_softdevice_events_execute();

        uxHighWaterMark_ble = uxTaskGetStackHighWaterMark( NULL );
    }
}

#if NRF_LOG_ENABLED
/**@brief Thread for handling the logger.
 *
 * @details This thread is responsible for processing log entries if logs are deferred.
 *          Thread flushes all log entries and suspends. It is resumed by idle task hook.
 *
 * @param[in]   arg   Pointer used for passing some arbitrary information (context) from the
 *                    osThreadCreate() call to the thread.
 */
static void logger_thread(void * arg)
{
    UNUSED_PARAMETER(arg);

    while(1)
    {
        NRF_LOG_FLUSH();

        vTaskSuspend(NULL); // Suspend myself
    }
}
#endif //NRF_LOG_ENABLED

//debugging
uint32_t g_dbug_count = 0;

//debugging
int32_t g_recv_data_temp;
int32_t g_recv_data_humidity;
int32_t g_recv_data_air_qual;

static void display_thread(void* arg)
{
  sensor_data data_recv;
  
  uint16_t cal_temperature;
  uint16_t cal_humidity;
  uint16_t cal_air_qual;
  uxHighWaterMark_disp = uxTaskGetStackHighWaterMark( NULL );

  for(;;)
  {

    xQueueReceive(m_adc_samples_queue, &data_recv, portMAX_DELAY);
    g_dbug_count += 1;
    //DEBUGGING  
    //g_recv_data_temp = data_recv.temperature;
    //g_recv_data_humidity = data_recv.humidity;
    //g_recv_data_air_qual = data_recv.air_quality;

    characteristic_update(&data_recv.temperature,0);
    characteristic_update(&data_recv.humidity,1);
    characteristic_update(&data_recv.air_quality,2);

    //Calibration
    cal_temperature = get_cal_temperature((uint16_t)data_recv.temperature);
    cal_humidity = get_cal_humidity((uint16_t)data_recv.humidity);
    //@TODO TO BE FIXED
    //cal_air_qual =  get_cal_air_quality((uint16_t)data_recv.air_quality);
    
    //Inspect high water mark on entering the task
    uxHighWaterMark_disp = uxTaskGetStackHighWaterMark( NULL );
  }
}

BaseType_t g_return_val;

static void adc_sampling_thread(void* arg)
{  
  sensor_data sample = {0};
  uxHighWaterMark_adc = uxTaskGetStackHighWaterMark( NULL );
  for(;;)
  {
      bsp_board_led_invert(BSP_LED_INDICATE_USER_LED2);
      
      //Sample and convert
      saadc_sampling_event_enable();
      
      //Get data
      sample.temperature = get_raw_temperature();
      sample.humidity = get_raw_humidity();
      sample.air_quality = get_raw_air_quality();

      //Push to queue
      xQueueSend(m_adc_samples_queue, (&sample), 0);

      //Inspect high water mark on entering the task
      uxHighWaterMark_adc = uxTaskGetStackHighWaterMark( NULL );

      vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/**@brief A function which is hooked to idle task.
 * @note Idle hook must be enabled in FreeRTOS configuration (configUSE_IDLE_HOOK).
 */

void vApplicationIdleHook( void )
{
}


/**@brief Function for initializing the clock.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
}

void Timer_Callback_Fn( TimerHandle_t xTimer )
{
   //bsp_board_led_invert(BSP_LED_INDICATE_USER_LED4);
}

/**@brief Function for application main entry.
 */



void* g_ptr;

int main(void)
{
    clock_init();
    bsp_board_leds_init();// Initialize LEDs for debugging

    timers_init();        // Initialize application timers
    saadc_init();

    // Do not start any interrupt that uses system functions before system initialization.
    // The best solution is to start the OS before any other initalization.

    // Init a semaphore for the BLE thread.
    m_ble_event_ready = xSemaphoreCreateBinary();
    if (NULL == m_ble_event_ready)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    log_init(); //Init log module

    // Start execution.
    g_return_val = xTaskCreate(ble_stack_thread, "BLE", 192, NULL, 3, &m_ble_stack_thread);
    if (pdPASS != g_return_val)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    g_return_val = xTaskCreate(adc_sampling_thread, "ADC", 100, NULL, 2, &m_adc_thread);
    if(pdPASS != g_return_val)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    g_return_val = xTaskCreate(display_thread, "DISP", 100, NULL, 1, &m_display_thread);
    if (pdPASS != g_return_val)
    {
        APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }
  
    m_adc_samples_queue = xQueueCreate(5, sizeof(sensor_data));
    if(NULL == m_adc_samples_queue)
    {
      APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
    }

    /* Activate deep sleep mode */
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    // Start FreeRTOS scheduler.
    vTaskStartScheduler();

    while (true)
    {
        APP_ERROR_HANDLER(NRF_ERROR_FORBIDDEN);
    }
}

char task_name[16];

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
  int i = 0;
  while(pcTaskName[i] != 0)
  {
    task_name[i] = pcTaskName[i];
    i++;
  }
  while(1)
  {
    bsp_board_led_on(BSP_LED_INDICATE_USER_LED1);
    bsp_board_led_off(BSP_LED_INDICATE_USER_LED2);
    bsp_board_led_on(BSP_LED_INDICATE_USER_LED3);
    bsp_board_led_on(BSP_LED_INDICATE_USER_LED4);
  }
}