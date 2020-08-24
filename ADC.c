#include "ADC.h"
#include "Calibration.h"
#include "boards.h"
#include "bsp_config.h"
//#include "application_timer.h"

#define SAMPLES_IN_BUFFER 3
#define MQ2_DELAY_CNTS      480     //4 mins

//static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);
static nrf_saadc_value_t     m_buffer_pool[1][SAMPLES_IN_BUFFER];
//static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;

static uint16_t raw_adc_value_temp;  //3.3V/2048(signed 12 bit)
static uint16_t raw_adc_value_humidity;
static uint16_t raw_adc_air_qual;

static void saadc_callback(nrf_drv_saadc_evt_t const * p_event);
//static void sampling_timer_handler(nrf_timer_event_t event_type, void * p_context);

static void ADC_Calibrate_Sensor(void);

void saadc_init()
{
    ret_code_t err_code;
    
    nrf_saadc_channel_config_t channel_config_0 = NRF_DRV_SAADC_TEMPERATURE_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);        //Temperature--P0.2--AIN0

    nrf_saadc_channel_config_t channel_config_1 = NRF_DRV_SAADC_HUMIDITY_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN1);        //Humidity--P0.3--AIN1
    
    nrf_saadc_channel_config_t channel_config_2 = NRF_DRV_SAADC_MQ2_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN2);            //MQ2--P0.4--AIN2
    
    
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config_0);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_saadc_channel_init(1, &channel_config_1);
    APP_ERROR_CHECK(err_code);
    
    err_code = nrf_drv_saadc_channel_init(2, &channel_config_2);
    APP_ERROR_CHECK(err_code);
    
    
    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);       //Buffer 0 containing 3 samples-- 1 sample each of Ch[0], Ch[1], Ch[2]
    APP_ERROR_CHECK(err_code);

    //err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);      //Buffer 1 containing 3 samples-- 1 sample each of Ch[0], Ch[1], Ch[2]
    //APP_ERROR_CHECK(err_code);
    
}

//void saadc_sampling_event_init()
//{
//    ret_code_t err_code;
//
//    err_code = nrf_drv_ppi_init();
//    APP_ERROR_CHECK(err_code);
//
//    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
//    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
//    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, sampling_timer_handler);
//    APP_ERROR_CHECK(err_code);
//
//    /* setup m_timer for compare event every 100ms */
//    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 100);
//    nrf_drv_timer_extended_compare(&m_timer,
//                                   NRF_TIMER_CC_CHANNEL1,
//                                   ticks,
//                                   //NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
//                                   NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK,
//                                   false);
//    nrf_drv_timer_enable(&m_timer);
//
//    
//    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
//                                                                                NRF_TIMER_CC_CHANNEL1);
//    
//    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();
//
//    
//    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
//    APP_ERROR_CHECK(err_code);
//
//    // setup ppi channel so that timer compare event is triggering sample task in SAADC
//    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
//                                          timer_compare_event_addr,
//                                          saadc_sample_task_addr);
//    APP_ERROR_CHECK(err_code);
//    
//}

void saadc_sampling_event_enable(void)
{
//    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);
//
//    APP_ERROR_CHECK(err_code);
      nrf_drv_saadc_sample();

}

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    uint8_t i = 0;
    //int32_t sum_adc_values = 0;
    uint16_t adc_values[3] = {0};
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)      //sampling done and buffer filled with samples evt.
    {
      
      ret_code_t err_code; 
      err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);        //convert values stored in buffer
      APP_ERROR_CHECK(err_code);
      
      for (i = 0; i < SAMPLES_IN_BUFFER; i++)
      {
          //NRF_LOG_INFO("%d\r\n", p_event->data.done.p_buffer[i]);
          //sum_adc_values+= p_event->data.done.p_buffer[i];
          adc_values[i]= p_event->data.done.p_buffer[i];
          
      }
      m_adc_evt_counter++;
      
      //adc_value_temp = sum_adc_values>>2;
     raw_adc_value_temp = adc_values[0];
     raw_adc_value_humidity = adc_values[1];
     raw_adc_air_qual = adc_values[2];

    }

}

uint16_t get_raw_temperature()
{
  return raw_adc_value_temp;
}

uint16_t get_raw_humidity()
{
  return raw_adc_value_humidity;
}

uint16_t get_raw_air_quality()
{
  return raw_adc_air_qual;
}

uint16_t get_cal_temperature(uint16_t temp_raw)
{
  return Calibrate_Temperature(temp_raw);
}

uint16_t get_cal_humidity(uint16_t humidity_raw)
{
  return Calibrate_Humidity(humidity_raw);
}

//TO BE FIXED
uint16_t get_cal_air_quality(uint16_t air_qual_raw)
{
  return raw_adc_air_qual;
  //return Calibrate_Mq2(air_qual_raw);
}

//void sampling_timer_handler(nrf_timer_event_t event_type, void * p_context)
//{
//  //unused-- timer interrupt is disabled and routed to PPI(?)
//}
