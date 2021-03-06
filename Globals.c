//Globals.c

#include "Globals.h"

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */

uint8_t customChar[8] = {
	0x00,
	0x0A,
	0x1F,
	0x1F,
	0x0E,
	0x04,
	0x00,
	0x00
};

void sleep_mode_enter(void)
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

/**@brief Function for initializing the nrf log module.
 */
void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
}

	/**@brief Function for the Power manager.		
 */		

 void power_manage(void)		
{		
    ret_code_t err_code = sd_app_evt_wait();		
    APP_ERROR_CHECK(err_code);		
}