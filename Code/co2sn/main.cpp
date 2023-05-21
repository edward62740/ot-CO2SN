/*
 *  main.c
 *
 *  Created on: May 20, 2023
 *      Author: edward62740
 */

#include <app_main.h>
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_burtc.h"
#include "em_prs.h"
#include "em_iadc.h"
#include "sl_i2cspm.h"
#include "sl_sleeptimer.h"

#include "sensors/scd4x/scd4x_i2c.h"
#include "sensors/scd4x/scd4x_lpc.h"
#include "sensors/opt3001/opt3001.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // !SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT


#include <openthread/cli.h>
#include <openthread/platform/logging.h>

void IADC_IRQHandler(void){

  IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}


I2C_TransferReturn_TypeDef I2C_detect1(I2C_TypeDef *i2c, uint8_t addr)
{


    I2C_TransferSeq_TypeDef i2cTransfer;
    I2C_TransferReturn_TypeDef result;

    // Initialize I2C transfer
    i2cTransfer.addr = addr << 1;
    i2cTransfer.flags = I2C_FLAG_WRITE;
    i2cTransfer.buf[0].data = NULL;
    i2cTransfer.buf[0].len = 0;


    return I2CSPM_Transfer(i2c, &i2cTransfer);


}


void initGPIO(void) {
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(PWR_ST_0_PORT, PWR_ST_0_PIN, gpioModeInput, 1);
    GPIO_PinModeSet(PWR_ST_1_PORT, PWR_ST_1_PIN, gpioModeInput, 1);
    GPIO_PinModeSet(PWR_ST_2_PORT, PWR_ST_2_PIN, gpioModeInput, 1);

    GPIO_PinModeSet(PWR_EN_ST_PORT, PWR_EN_ST_PIN, gpioModeInput, 1);
    GPIO_PinModeSet(PWR_EN_SCD_PORT, PWR_EN_SCD_PIN, gpioModeInput, 1);

    GPIO_PinModeSet(PWR_PG_MCU_PORT, PWR_PG_MCU_PIN, gpioModeInput, 1);
    GPIO_PinModeSet(PWR_PG_SCD_PORT, PWR_PG_SCD_PIN, gpioModeInput, 1);

    GPIO_PinModeSet(LOAD_SENSE_PORT, LOAD_SENSE_PIN, gpioModeInputPull, 0);

    /*GPIO_ExtIntConfig(A111_INT_PORT,
                      A111_INT_PIN,
                      A111_INT_PIN,
                        true,
                        false,
                        true);*/

    GPIO_PinModeSet(INT_OPT_PORT, INT_OPT_PIN, gpioModeInputPullFilter, 0);
    GPIO_PinModeSet(ACT_LED_PORT, ACT_LED_PIN, gpioModePushPull, 0);
    GPIO_PinModeSet(ERR_LED_PORT, ERR_LED_PIN, gpioModePushPull, 0);
}


void initBURTC(void)
{
  CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
  CMU_ClockEnable(cmuClock_BURTC, true);
  CMU_ClockEnable(cmuClock_BURAM, true);

  BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
  burtcInit.compare0Top = true; // reset counter when counter reaches compare value
  burtcInit.em4comp = true;     // BURTC compare interrupt wakes from EM4 (causes reset)
  BURTC_Init(&burtcInit);
  BURTC_IntEnable(BURTC_IEN_COMP);      // compare match
  NVIC_EnableIRQ(BURTC_IRQn);
  BURTC_Enable(true);
}

void initSensors(void)
{

}

void initVddMonitor(void)
{
  IADC_Init_t init = IADC_INIT_DEFAULT;
  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
  IADC_SingleInput_t singleInput = IADC_SINGLEINPUT_DEFAULT;

  CMU_ClockEnable (cmuClock_PRS, true);
  PRS_SourceAsyncSignalSet (0,
  PRS_ASYNC_CH_CTRL_SOURCESEL_MODEM,
                            PRS_MODEMH_PRESENT);
  PRS_ConnectConsumer (0, prsTypeAsync, prsConsumerIADC0_SINGLETRIGGER);
  CMU_ClockEnable (cmuClock_IADC0, true);
  initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
  initAllConfigs.configs[0].vRef = 1200;
  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;

  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale (
      IADC0, 1000000, 0, iadcCfgModeNormal, init.srcClkPrescale);
  initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
  initSingle.dataValidLevel = iadcFifoCfgDvl4;
  initSingle.start = true;
  singleInput.posInput = iadcPosInputAvdd;
  singleInput.negInput = iadcNegInputGnd;
  IADC_init (IADC0, &init, &initAllConfigs);
  IADC_initSingle (IADC0, &initSingle, &singleInput);
  IADC_clearInt (IADC0, _IADC_IF_MASK);
  IADC_enableInt (IADC0, IADC_IEN_SINGLEDONE);
  NVIC_ClearPendingIRQ (IADC_IRQn);
  NVIC_SetPriority(GPIO_ODD_IRQn, 7);
  NVIC_EnableIRQ (IADC_IRQn);
}

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();
  initGPIO();
  initBURTC();
  initVddMonitor();
  //GPIO_PinOutSet(ACT_LED_PORT, ACT_LED_PIN);

  /*
  otCliOutputFormat("I2c0 scan: \n");
    for(uint8_t i=0; i<128; i++)
    {
  	  if(i2cTransferDone == I2C_detect1(I2C0, i)) otCliOutputFormat("%d ", i);
    }

  otCliOutputFormat("I2c1 scan: \n");
  for(uint8_t i=0; i<128; i++)
  {
	  if(i2cTransferDone == I2C_detect1(I2C1, i)) otCliOutputFormat("%d ", i);
  }*/

	opt3001_init();
	float opt_buf = opt3001_conv(opt3001_read());
	otCliOutputFormat("Light result: %d \r\n", opt_buf);

	// Clean up potential SCD40 states
	scd4x_wake_up();
	sl_sleeptimer_delay_millisecond(20);

	scd4x_stop_periodic_measurement();
	sl_sleeptimer_delay_millisecond(500);

	scd4x_set_automatic_self_calibration(0);

	//scd4x_perform_factory_reset();
	//sl_sleeptimer_delay_millisecond(1200);
	//uint16_t status = 0;
	//scd4x_perform_self_test(&status);
	//otCliOutputFormat("Status of scd4x %d \r\n", status);
	// Initialize the application. For example, create periodic timer(s) or
	// task(s) if the kernel is present.
	app_init();

#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT
  while (1) {
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();

    app_process_action();


   // float opt_buf = opt3001_conv(opt3001_read());
   	/* otCliOutputFormat("Light result: %d \r\n", opt_buf);
   	LPC::lpc_ret ret = LPC::ERR_NONE;
    otCliOutputFormat("Measurement started\r\n");
    ret = scd41.powerOn();
    otCliOutputFormat("power on %d, \n", ret);
    sl_sleeptimer_delay_millisecond(20);
    ret = scd41.discardMeasurement();
    otCliOutputFormat("discard %d, \n", ret);
    sl_sleeptimer_delay_millisecond(5000);
    ret = scd41.measure();
    otCliOutputFormat("meas %d, \n", ret);
    sl_sleeptimer_delay_millisecond(5100);
    uint16_t co2;
    int32_t hum, temp;
    ret = scd41.read(&co2, &temp, &hum);
    otCliOutputFormat("Measurement result: CO2 %d, temp %d, hum %d  ERR code: %d \r\n", co2, temp, hum, ret);
    uint32_t age, num;
    int32_t offset;
    scd41.get_last_frc(&offset, &age, &num);
    otCliOutputFormat("calibration result: Cycles ago: %d, Offset: %d, Total: %d \r\n", age, offset, num);
    ret = scd41.powerOff();
    otCliOutputFormat("power off %d, \n", ret);
    sl_sleeptimer_delay_millisecond(20);*/
   // GPIO_PinOutToggle(ACT_LED_PORT, ACT_LED_PIN);
    //sl_sleeptimer_delay_millisecond(20);
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
  }
  // Clean-up when exiting the application.
  app_exit();
#endif // SL_CATALOG_KERNEL_PRESENT
}
