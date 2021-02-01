#include "Calibration.h"
#include "boards.h"
#include "bsp_config.h"

#define DIVIDER_RATIO           1.515//(5/3.3)
#define ADC_REFERENCE           0.6
#define GAIN                    (2)
#define ADC_COUNTS              1024
#define VC_VOLTS                5
#define RL_KOHM                 5.07
#define VC_VOLTS_TIMES_RL_KOHM  VC_VOLTS*RL_KOHM
#define PPM_FRESH_AIR           9.5

static double Convert_Mq2_Volts(uint16_t mq2_adc);
static double Calculate_Rs(double V_RL);
static double Calculate_PPM(double ratio);


int32_t Calibrate_Temperature(int32_t raw_temp_adc)
{
  //@todo- try to make the calculations suitable for int types
  int32_t cal_temp = ((raw_temp_adc - 592)/11);        //x = (y-b)/m
  return cal_temp;
}

int32_t Calibrate_Humidity(int32_t raw_humidity_adc)
{
  //@todo- try to make the calculations suitable for int types
  int32_t cal_humidity = ((raw_humidity_adc - 569)/26);
  return cal_humidity;
}

//TO BE MODIFIED
uint16_t Calibrate_Mq2(uint16_t raw_mq2_adc)
{
  double raw_mq2_volts, Rs,Ro,ratio_gas, cal_ppm;
  raw_mq2_volts = Convert_Mq2_Volts(raw_mq2_adc);  //converting to volts;
  Rs = Calculate_Rs(raw_mq2_volts);     
  Ro = Rs/PPM_FRESH_AIR;
  ratio_gas = Rs/Ro;
  cal_ppm = Calculate_PPM(ratio_gas);
  return (uint16_t)cal_ppm;
}

static double Convert_Mq2_Volts(uint16_t mq2_adc)
{
  double mq2_volts;
  mq2_adc = (double)mq2_adc*DIVIDER_RATIO;
  mq2_volts = mq2_adc*(double)((GAIN*ADC_REFERENCE)/ADC_COUNTS);
  return mq2_volts;
}

static double Calculate_Rs(double V_RL)
{
  double rs = (VC_VOLTS_TIMES_RL_KOHM/V_RL)-RL_KOHM;
  return rs;
}

static double Calculate_PPM(double ratio)
{
  double ppm_log, ppm;
  ppm_log = ((log10(ratio) - 1.284)/(-4.464));
  ppm = pow(10.0,ppm_log); 
  return ppm;
}