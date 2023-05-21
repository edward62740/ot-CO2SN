
#ifndef APP_H
#define APP_H

#include "sensors/scd4x/scd4x_i2c.h"
#include "sensors/scd4x/scd4x_lpc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern LPC::SCD4X scd41;


void app_init(void);

void app_exit(void);

void app_process_action(void);

#define ACT_LED_PORT     gpioPortB
#define ACT_LED_PIN      2
#define ERR_LED_PORT     gpioPortB
#define ERR_LED_PIN      3


#define INT_OPT_PORT     gpioPortD
#define INT_OPT_PIN      5



#define PWR_ST_0_PORT    gpioPortC
#define PWR_ST_0_PIN     0
#define PWR_ST_1_PORT    gpioPortC
#define PWR_ST_1_PIN     1
#define PWR_ST_2_PORT    gpioPortC
#define PWR_ST_2_PIN     2
#define PWR_EN_ST_PORT   gpioPortC
#define PWR_EN_ST_PIN    3

#define PWR_PG_MCU_PORT  gpioPortC
#define PWR_PG_MCU_PIN   4
#define PWR_PG_SCD_PORT  gpioPortC
#define PWR_PG_SCD_PIN   5
#define PWR_EN_SCD_PORT  gpioPortC
#define PWR_EN_SCD_PIN   6

#define LOAD_SENSE_PORT  gpioPortC
#define LOAD_SENSE_PIN   7


void sleepyInit(void);
void setNetworkConfiguration(void);
void initUdp(void);
void applicationTick(void);
void sl_ot_create_instance(void);
void sl_ot_cli_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif
