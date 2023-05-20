/***************************************************************************//**
 * @file
 * @brief main() function.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_component_catalog.h"
#include "sl_system_init.h"
#include "app.h"
#include "em_i2c.h"
#include "sl_i2cspm.h"

#include "sensors/scd4x/scd4x_i2c.h"
#include "sensors/opt3001/opt3001.h"
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
#include "sl_power_manager.h"
#endif // SL_CATALOG_POWER_MANAGER_PRESENT
#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_system_kernel.h"
#else // !SL_CATALOG_KERNEL_PRESENT
#include "sl_system_process_action.h"
#endif // SL_CATALOG_KERNEL_PRESENT
#define I2CSPM_TRANSFER_TIMEOUT 100

#include <openthread/cli.h>
#include <openthread/platform/logging.h>
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

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  // Note that if the kernel is present, processing task(s) will be created by
  // this call.
  sl_system_init();
  otCliOutputFormat("I2c0 scan: \n");
    for(uint8_t i=0; i<128; i++)
    {
  	  if(i2cTransferDone == I2C_detect1(I2C0, i)) otCliOutputFormat("%d ", i);
    }

  otCliOutputFormat("I2c1 scan: \n");
  for(uint8_t i=0; i<128; i++)
  {
	  if(i2cTransferDone == I2C_detect1(I2C1, i)) otCliOutputFormat("%d ", i);
  }

	opt3001_init();
	  float opt_buf = opt3001_conv(opt3001_read());
	   	 otCliOutputFormat("Light result: %d \r\n", opt_buf);


	 	// Clean up potential SCD40 states
	 	scd4x_wake_up();
	 	scd4x_stop_periodic_measurement();

	 	//scd4x_reinit();

	 	sl_sleeptimer_delay_millisecond(100);
	 	uint16_t status = 0;
	 	scd4x_perform_self_test(&status);
	 	otCliOutputFormat("Status of scd4x %d \r\n", status);
  // Initialize the application. For example, create periodic timer(s) or
  // task(s) if the kernel is present.
  app_init();


//  sl_sleeptimer_delay_millisecond(200);
  //	scd4x_stop_periodic_measurement();
  	//sl_sleeptimer_delay_millisecond(500);
  //	scd4x_reinit();
  //	sl_sleeptimer_delay_millisecond(200);
#if defined(SL_CATALOG_KERNEL_PRESENT)
  // Start the kernel. Task(s) created in app_init() will start running.
  sl_system_kernel_start();
#else // SL_CATALOG_KERNEL_PRESENT
  while (1) {
    // Do not remove this call: Silicon Labs components process action routine
    // must be called from the super loop.
    sl_system_process_action();
    otCliOutputFormat("I2c0 scan: \n");
      for(uint8_t i=0; i<128; i++)
      {
    	  if(i2cTransferDone == I2C_detect1(I2C0, i)) otCliOutputFormat("%d ", i);
      }

    otCliOutputFormat("I2c1 scan: \n");
    for(uint8_t i=0; i<128; i++)
    {
  	  if(i2cTransferDone == I2C_detect1(I2C1, i)) otCliOutputFormat("%d ", i);
    }

    // Application process.
    app_process_action();
    float opt_buf = opt3001_conv(opt3001_read());
   	 otCliOutputFormat("Light result: %d \r\n", opt_buf);

    otCliOutputFormat("Measurement started\r\n");
    scd4x_measure_single_shot();
    //sl_sleeptimer_delay_millisecond(5500);
    otCliOutputFormat("Measurement done\r\n");
    uint16_t co2;
    int32_t hum, temp;
    int16_t ret = scd4x_read_measurement(&co2, &temp, &hum);
    otCliOutputFormat("Measurement result: CO2 %d, temp %d, hum %d  %d \r\n", co2, temp, hum, ret);
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#endif
  }
  // Clean-up when exiting the application.
  app_exit();
#endif // SL_CATALOG_KERNEL_PRESENT
}
