
#include "Globals.h"
#include "BLE_Advertisement.h"

static ble_uuid_t m_adv_uuids[] = {{BLE_SMART_HOME_16BIT_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}};
//static ble_uuid_t m_adv_uuids[] = {{BLE_SMART_HOME_16BIT_UUID, BLE_UUID_TYPE_BLE}};

// {{BLE_UUID_TEMPERATURE_SERVICE, BLE_UUID_TYPE_BLE},
//{BLE_UUID_HUMIDITY_SERVICE, BLE_UUID_TYPE_BLE},
//{BLE_UUID_AIR_QUALITY_SERVICE, BLE_UUID_TYPE_BLE}};



static uint8_t manufData[MANUF_ADV_DATA_SIZE] = {0};
//static uint8_t manufData[4] = {0};

static uint8_t* set_manuf_data_unpair_mode(void);
static void on_adv_evt(ble_adv_evt_t ble_adv_evt);
static void delete_bonds(void);

/**@brief Function for initializing the Advertising functionality.
 */

uint32_t err_ble_advt_code;
char const* err_msg_advt;
uint8_t* manuf_arr = 0;

void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advdata_t          advdata;
    ble_advdata_t          scanrsp;
    ble_adv_modes_config_t options;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = false;
     advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
     
    memset(&scanrsp, 0, sizeof(scanrsp)); 
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_advdata_manuf_data_t manuf_specific_data;
    manuf_specific_data.data.size = MANUF_ADV_DATA_SIZE;
    manuf_specific_data.company_identifier = 0xBEBE;
    manuf_specific_data.data.p_data = set_manuf_data_unpair_mode();
    manuf_arr = manuf_specific_data.data.p_data;
    advdata.p_manuf_specific_data = &manuf_specific_data;
    
    memset(&options, 0, sizeof(options));
    options.ble_adv_fast_enabled  = true;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_ble_advt_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    err_msg_advt = nrf_strerror_get(err_ble_advt_code);
    APP_ERROR_CHECK(err_ble_advt_code);

    ble_advertising_conn_cfg_tag_set(CONN_CFG_TAG);
}

static uint32_t sd_return;

static uint8_t* set_manuf_data_unpair_mode(void)
{   
    uint8_t index;
    ble_gap_addr_t macAddress;
   

/*--------------------------------------------------------------------*/
    //PTI Seed-- Byte 0
    manufData[ADV_PTI_SEED_BYTE] = 0XAA;
/*--------------------------------------------------------------------*/
    //MAC Address-- Byte 1- Byte 6
    memset(&macAddress, 0, sizeof(macAddress));
    macAddress.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
    sd_return= sd_ble_gap_addr_get(&macAddress);
    //M_ASSERT(NRF_SUCCESS == sd_return);
    sd_return = sd_ble_gap_addr_get(&macAddress);
    //M_ASSERT(NRF_SUCCESS == sd_return);
    for(index=0; index < BLE_GAP_ADDR_LEN; index++)
    {
      manufData[ADV_MAC_OFFSET+index] = macAddress.addr[index];
    }
/*--------------------------------------------------------------------*/
    //Product Type Index-- Byte7, 8
    //Need to split into two bytes
    manufData[ADV_PTI_BYTE] = 0x01;
/*--------------------------------------------------------------------*/
    //STATUS- PAIRED/UNPAIRED Byte 9
    manufData[ADV_STATUS_BYTE] = 0x40;
/*--------------------------------------------------------------------*/
    //Rest are 0s for unpaired berry
/*--------------------------------------------------------------------*/
    return(manufData);
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.\r\n");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }
}

/**@brief Function for starting advertising.
 */
void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
    }
    else
    {
        ret_code_t err_code;

        err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
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