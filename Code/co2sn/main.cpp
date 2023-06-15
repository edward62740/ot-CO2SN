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

/*
I2C_TransferReturn_TypeDef I2C_detect1(I2C_TypeDef *i2c, uint8_t addr) {

	I2C_TransferSeq_TypeDef i2cTransfer;
	I2C_TransferReturn_TypeDef result;

	// Initialize I2C transfer
	i2cTransfer.addr = addr << 1;
	i2cTransfer.flags = I2C_FLAG_WRITE;
	i2cTransfer.buf[0].data = NULL;
	i2cTransfer.buf[0].len = 0;

	return I2CSPM_Transfer(i2c, &i2cTransfer);

}
*/
void initGPIO(void) {
	CMU_ClockEnable(cmuClock_GPIO, true);
	GPIO_PinModeSet(PWR_ST_0_PORT, PWR_ST_0_PIN, gpioModeInput, 1); // Asserted when the LDOs can be enabled (i.e. Vbatt > Vchrdy)
	GPIO_PinModeSet(PWR_ST_1_PORT, PWR_ST_1_PIN, gpioModeInput, 1); // Asserted if the battery voltage falls below Vovdis or if the AEM is taking energy from the primary battery
	GPIO_PinModeSet(PWR_ST_2_PORT, PWR_ST_2_PIN, gpioModeInput, 1); // Asserted when the AEM performs a MPP evaluation

	GPIO_PinModeSet(PWR_EN_ST_PORT, PWR_EN_ST_PIN, gpioModeInput, 1); // Enable status pins
	GPIO_PinModeSet(PWR_EN_SCD_PORT, PWR_EN_SCD_PIN, gpioModeInput, 1); // Enable SCD power

	GPIO_PinModeSet(PWR_PG_MCU_PORT, PWR_PG_MCU_PIN, gpioModeInput, 1); // MCU power good
	GPIO_PinModeSet(PWR_PG_SCD_PORT, PWR_PG_SCD_PIN, gpioModeInput, 1); // SCD power good

	GPIO_PinModeSet(LOAD_SENSE_PORT, LOAD_SENSE_PIN, gpioModeInputPull, 0); // TBD single ended analog signal

	/*GPIO_ExtIntConfig(A111_INT_PORT,
	 A111_INT_PIN,
	 A111_INT_PIN,
	 true,
	 false,
	 true);*/

	GPIO_PinModeSet(INT_OPT_PORT, INT_OPT_PIN, gpioModeInputPullFilter, 0); // OPT interrupt
	GPIO_PinModeSet(ACT_LED_PORT, ACT_LED_PIN, gpioModePushPull, 0);
	GPIO_PinModeSet(ERR_LED_PORT, ERR_LED_PIN, gpioModePushPull, 0);

	//GPIO_PinOutSet(PWR_EN_SCD_PORT, PWR_EN_SCD_PIN);
}

void initBURTC(void) {
	CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_ULFRCO);
	CMU_ClockEnable(cmuClock_BURTC, true);
	CMU_ClockEnable(cmuClock_BURAM, true);

	BURTC_Init_TypeDef burtcInit = BURTC_INIT_DEFAULT;
	burtcInit.compare0Top = true; // reset counter when counter reaches compare value
	burtcInit.em4comp = true; // BURTC compare interrupt wakes from EM4 (causes reset)
	BURTC_Init(&burtcInit);
	BURTC_IntEnable(BURTC_IEN_COMP);      // compare match
	NVIC_EnableIRQ(BURTC_IRQn);
	BURTC_Enable(true);
}

void initSensors(void) {
	opt3001_init();
}

void initVddMonitor(void) {
	IADC_Init_t init = IADC_INIT_DEFAULT;
	IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
	IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
	IADC_SingleInput_t singleInput = IADC_SINGLEINPUT_DEFAULT;

	CMU_ClockEnable(cmuClock_PRS, true);
	PRS_SourceAsyncSignalSet(0,
	PRS_ASYNC_CH_CTRL_SOURCESEL_MODEM,
	PRS_MODEMH_PRESENT);
	PRS_ConnectConsumer(0, prsTypeAsync, prsConsumerIADC0_SINGLETRIGGER);
	CMU_ClockEnable(cmuClock_IADC0, true);
	initAllConfigs.configs[0].reference = iadcCfgReferenceInt1V2;
	initAllConfigs.configs[0].vRef = 1200;
	initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed2x;

	initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(
	IADC0, 1000000, 0, iadcCfgModeNormal, init.srcClkPrescale);
	initSingle.triggerSelect = iadcTriggerSelPrs0PosEdge;
	initSingle.dataValidLevel = iadcFifoCfgDvl4;
	initSingle.start = true;
	singleInput.posInput = iadcPosInputPortDPin2;
	singleInput.negInput = iadcNegInputGnd;
	IADC_init(IADC0, &init, &initAllConfigs);
	IADC_initSingle(IADC0, &initSingle, &singleInput);
	IADC_clearInt(IADC0, _IADC_IF_MASK);
	IADC_enableInt(IADC0, IADC_IEN_SINGLEDONE);
	NVIC_ClearPendingIRQ(IADC_IRQn);
	NVIC_SetPriority(GPIO_ODD_IRQn, 7);
	NVIC_EnableIRQ(IADC_IRQn);
}

int main(void) {
	sl_system_init();
	initGPIO();
	initBURTC();
	initVddMonitor();
	initSensors();
	GPIO_PinOutSet(ERR_LED_PORT, ERR_LED_PIN);
	GPIO_PinOutSet(ACT_LED_PORT, ACT_LED_PIN);
	GPIO_PinOutSet(PWR_EN_ST_PORT, PWR_EN_ST_PIN);
	/*
	 otCliOutputFormat("I2c0 scan: \n");
	 for (uint8_t i = 0; i < 128; i++) {
	 if (i2cTransferDone == I2C_detect1(I2C0, i))
	 otCliOutputFormat("%d ", i);
	 }

	 otCliOutputFormat("I2c1 scan: \n");
	 for (uint8_t i = 0; i < 128; i++) {
	 if (i2cTransferDone == I2C_detect1(I2C1, i))
	 otCliOutputFormat("%d ", i);
	 }*/


	float opt_buf = opt3001_conv(opt3001_read());

	GPIO_PinOutToggle(ACT_LED_PORT, ACT_LED_PIN);
	// Clean up potential SCD40 states
	scd4x_wake_up();
	sl_sleeptimer_delay_millisecond(30);
	GPIO_PinOutToggle(ACT_LED_PORT, ACT_LED_PIN);
	scd4x_set_automatic_self_calibration(0);
	sl_sleeptimer_delay_millisecond(30);
	GPIO_PinOutToggle(ACT_LED_PORT, ACT_LED_PIN);
	//scd4x_stop_periodic_measurement();
	scd4x_power_down();

	app_init();
	GPIO_PinOutClear(ERR_LED_PORT, ERR_LED_PIN);


#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT
	while (1) {
		// Do not remove this call: Silicon Labs components process action routine
		// must be called from the super loop.
		sl_system_process_action();

		app_process_action();

		// GPIO_PinOutToggle(ACT_LED_PORT, ACT_LED_PIN);
		//sl_sleeptimer_delay_millisecond(55);
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
		// Let the CPU go to sleep if the system allows it.
		sl_power_manager_sleep();
#endif
	}
	// Clean-up when exiting the application.
	app_exit();
#endif // SL_CATALOG_KERNEL_PRESENT
}
