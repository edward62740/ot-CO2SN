/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
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

#ifndef APP_WIRELESS_H
#define APP_WIRELESS_H



#ifdef __cplusplus
extern "C" {
#endif


#include <openthread/instance.h>

#define ACT_LED_PORT     gpioPortC
#define ACT_LED_PIN      8
#define ERR_LED_PORT     gpioPortC
#define ERR_LED_PIN      9

void app_init(void);

void app_exit(void);

void app_process_action(void);

otInstance *otGetInstance(void);
void sl_ot_create_instance(void);
void sl_ot_cli_init(void);

#ifdef __cplusplus
}
#endif

#endif
