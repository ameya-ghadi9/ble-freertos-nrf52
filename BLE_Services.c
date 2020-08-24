//BLE_Services.c

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */

#include "Globals.h"
#include "BLE_Services.h"

uint32_t svc_err_code;

static uint32_t ble_smart_home_service_init(ble_smart_home_struct_t * p_smart_home);

static void temperature_characteristic_add(ble_smart_home_struct_t* ptr_smart_home);
static void temperature_characteristic_update(ble_smart_home_struct_t* ptr_smart_home, int32_t *temperature_value);

static void humidity_characteristic_add(ble_smart_home_struct_t* ptr_smart_home);
static void humidity_characteristic_update(ble_smart_home_struct_t* ptr_smart_home, int32_t *humidity_value);

static void air_quality_characteristic_add(ble_smart_home_struct_t* ptr_smart_home);
static void air_quality_characteristic_update(ble_smart_home_struct_t* ptr_smart_home, int32_t *air_quality_value);

void services_init(void)
{
  ble_smart_home_service_init(&smart_home);
}

void characteristic_update(int32_t *sensor_adc_value, characteristic_value_type cvt)
{
  switch(cvt)
  { 
    case temperature:
      temperature_characteristic_update(&smart_home, sensor_adc_value);
    break;
    
    case humidity:
      humidity_characteristic_update(&smart_home, sensor_adc_value);
    break;
    
    case air_quality:
      air_quality_characteristic_update(&smart_home, sensor_adc_value); 
    break;
  } 
 // temperature_characteristic_update(&smart_home, sensor_value);
  //humidity_characteristic_update(&smart_home, sensor_value);
  //air_quality_characteristic_update(&smart_home, sensor_value);  
    
}

/**@brief Function for initializing the Nordic UART Service.
 *
 * @param[out] p_nus      Nordic UART Service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 * @param[in] p_nus_init  Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_nus or p_nus_init is NULL.
 */


//uint32_t ble_nus_init(ble_smart_home_t * p_smart_home, const ble_smart_home_init_t * p_smart_home_init);
static uint32_t ble_smart_home_service_init(ble_smart_home_struct_t * p_smart_home)
{
   //--------------------Service 1----------------------------------------------
  //initializing service--type PRIMARY. Secondary if nested service
  //he third variable passed to the function is a pointer to where the service_handle number of this unique service should be stored
  //sd_ble_gatts_service_add() function will create a table containing our services and the service_handle is simply an index pointing to our particular service in the table
   
  uint32_t      err_code;
  ble_uuid_t    ble_uuid;
    
  ble_uuid_t sm_service_uuid;                                   //Declaring service uuid
  ble_uuid128_t sm_base_uuid = BLE_SMART_HOME_BASE_UUID;        //Defining 128 bit UUID for Smart Home
  sm_service_uuid.uuid = BLE_SMART_HOME_16BIT_UUID;          //Init service uuid with 16 bit value

   //p_smart_home->conn_handle = BLE_CONN_HANDLE_INVALID;
  
  // Add a custom base UUID.
  err_code = sd_ble_uuid_vs_add(&sm_base_uuid, &sm_service_uuid.type);
  VERIFY_SUCCESS(err_code);

    // Add the service.
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &sm_service_uuid,
                                        &p_smart_home->service_handle);

  temperature_characteristic_add(p_smart_home);
  humidity_characteristic_add(p_smart_home);
  air_quality_characteristic_add(p_smart_home);
  //VERIFY_SUCCESS(err_code);
  
  return NRF_SUCCESS;
   
}


static void temperature_characteristic_add(ble_smart_home_struct_t* ptr_smart_home)
{
    uint32_t err_code;   

    //Adding Custom UUID
    ble_uuid_t char_uuid;
    ble_uuid128_t sm_base_uuid = BLE_SMART_HOME_BASE_UUID;
    char_uuid.uuid = BLE_UUID_TEMPERATURE_CHARACTERISTIC;
    err_code = sd_ble_uuid_vs_add(&sm_base_uuid, &char_uuid.type);
      
    //Configure Attribute metadata
    ble_gatts_attr_md_t attr_md;//attribute metadata--structure for holding permissions and authorization levels required by characteristic value
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK; //store attribute in SofDevice part of memory
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm); //Set read permission for characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm); //Set write permission for characteristic
    
    //Configure Characteristic Value attribute
    ble_gatts_attr_t attr_char_value;//characteristic value attribute-- structure holding actual value
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    attr_char_value.max_len     = 4;  //set max length of characteristic
    attr_char_value.init_len    = 4; //set initial length of characteristic
    uint8_t value[4]            = {0x12,0x34,0x56,0x78}; //characteristic value
    attr_char_value.p_value     = value;
 
   //Add handles for the characteristic in service structure
    

    //Add read/write property to characteristic
    ble_gatts_char_md_t char_md; //characteristic metadata-- structure holding properties of the of characteristic value
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1; //add read property
    char_md.char_props.write = 1;//add write property
    
    //Configuring Client Characteristic Configuration Descriptor (CCCD) metadata
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;
    
    //Give service connection a default value
    ptr_smart_home->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    
    //Add characteristic to service
    err_code = sd_ble_gatts_characteristic_add(ptr_smart_home->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ptr_smart_home->temperature_char_handles);          
}

static void humidity_characteristic_add(ble_smart_home_struct_t* ptr_smart_home)
{
    uint32_t err_code;    

    //Adding Custom UUID
    ble_uuid_t char_uuid;
    ble_uuid128_t sm_base_uuid = BLE_SMART_HOME_BASE_UUID;
    char_uuid.uuid = BLE_UUID_HUMIDITY_CHARACTERISTIC;
    err_code = sd_ble_uuid_vs_add(&sm_base_uuid, &char_uuid.type);
      
    //Configure Attribute metadata
    ble_gatts_attr_md_t attr_md;//attribute metadata--structure for holding permissions and authorization levels required by characteristic value
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK; //store attribute in SofDevice part of memory
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm); //Set read permission for characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm); //Set write permission for characteristic
    
    //Configure Characteristic Value attribute
    ble_gatts_attr_t attr_char_value;//characteristic value attribute-- structure holding actual value
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    attr_char_value.max_len     = 4;  //set max length of characteristic
    attr_char_value.init_len    = 4; //set initial length of characteristic
    uint8_t value[4]            = {0x12,0x34,0x56,0x78}; //characteristic value
    attr_char_value.p_value     = value;
 
   //Add handles for the characteristic in service structure
    

    //Add read/write property to characteristic
    ble_gatts_char_md_t char_md; //characteristic metadata-- structure holding properties of the of characteristic value
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1; //add read property
    char_md.char_props.write = 1;//add write property
    
    //Configuring Client Characteristic Configuration Descriptor (CCCD) metadata
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;
    
    //Give service connection a default value
    ptr_smart_home->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    
    //Add characteristic to service
    err_code = sd_ble_gatts_characteristic_add(ptr_smart_home->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ptr_smart_home->humidity_char_handles);          
}

static void air_quality_characteristic_add(ble_smart_home_struct_t* ptr_smart_home)
{
    uint32_t err_code;    

    //Adding Custom UUID
    ble_uuid_t char_uuid;
    ble_uuid128_t sm_base_uuid = BLE_SMART_HOME_BASE_UUID;
    char_uuid.uuid = BLE_UUID_AIR_QUALITY_CHARACTERISTIC;
    err_code = sd_ble_uuid_vs_add(&sm_base_uuid, &char_uuid.type);
      
    //Configure Attribute metadata
    ble_gatts_attr_md_t attr_md;//attribute metadata--structure for holding permissions and authorization levels required by characteristic value
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc = BLE_GATTS_VLOC_STACK; //store attribute in SofDevice part of memory
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm); //Set read permission for characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm); //Set write permission for characteristic
    
    //Configure Characteristic Value attribute
    ble_gatts_attr_t attr_char_value;//characteristic value attribute-- structure holding actual value
    memset(&attr_char_value, 0, sizeof(attr_char_value));    
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    attr_char_value.max_len     = 4;  //set max length of characteristic
    attr_char_value.init_len    = 4; //set initial length of characteristic
    uint8_t value[4]            = {0x12,0x34,0x56,0x78}; //characteristic value
    attr_char_value.p_value     = value;
 
   //Add handles for the characteristic in service structure
    

    //Add read/write property to characteristic
    ble_gatts_char_md_t char_md; //characteristic metadata-- structure holding properties of the of characteristic value
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1; //add read property
    char_md.char_props.write = 1;//add write property
    
    //Configuring Client Characteristic Configuration Descriptor (CCCD) metadata
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;
    
    //Give service connection a default value
    ptr_smart_home->conn_handle = BLE_CONN_HANDLE_INVALID;
    
    
    //Add characteristic to service
    err_code = sd_ble_gatts_characteristic_add(ptr_smart_home->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ptr_smart_home->air_quality_char_handles);          
}

void temperature_characteristic_update(ble_smart_home_struct_t* ptr_smart_home, int32_t *temperature_value)
{
    uint16_t               len = 4;
    ble_gatts_hvx_params_t hvx_params;
    memset(&hvx_params, 0, sizeof(hvx_params));
      
    //Update characteristic value
    if(ptr_smart_home->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
      hvx_params.handle = ptr_smart_home->temperature_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = (uint8_t*)temperature_value;  

      sd_ble_gatts_hvx(ptr_smart_home->conn_handle, &hvx_params);
    }
}

void humidity_characteristic_update(ble_smart_home_struct_t* ptr_smart_home, int32_t *humidity_value)
{
    uint16_t               len = 4;
    ble_gatts_hvx_params_t hvx_params;
    memset(&hvx_params, 0, sizeof(hvx_params));
      
    //Update characteristic value
    if(ptr_smart_home->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
      hvx_params.handle = ptr_smart_home->humidity_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = (uint8_t*)humidity_value;  

      sd_ble_gatts_hvx(ptr_smart_home->conn_handle, &hvx_params);
    }
}

void air_quality_characteristic_update(ble_smart_home_struct_t* ptr_smart_home, int32_t *air_quality_value)
{
    uint16_t               len = 4;
    ble_gatts_hvx_params_t hvx_params;
    memset(&hvx_params, 0, sizeof(hvx_params));
      
    //Update characteristic value
    if(ptr_smart_home->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
      hvx_params.handle = ptr_smart_home->air_quality_char_handles.value_handle;
      hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
      hvx_params.offset = 0;
      hvx_params.p_len  = &len;
      hvx_params.p_data = (uint8_t*)air_quality_value;  

      sd_ble_gatts_hvx(ptr_smart_home->conn_handle, &hvx_params);
    }
}