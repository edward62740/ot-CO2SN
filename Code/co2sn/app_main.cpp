#include <app_main.h>
#include <app_thread.h>
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include "openthread-system.h"
#include "sl_ot_init.h"
#include "sl_component_catalog.h"
#include "em_burtc.h"
#include "em_gpio.h"
#include "sensors/scd4x/scd4x_i2c.h"
#include "sensors/scd4x/scd4x_lpc.h"
#include "sensors/opt3001/opt3001.h"
#include "sl_sleeptimer.h"

extern void otAppCliInit(otInstance *aInstance);

static otInstance* sInstance = NULL;

LPC::SCD4X scd41(scd4x_wake_up, scd4x_power_down, scd4x_measure_single_shot,
		scd4x_get_data_ready_flag, scd4x_read_measurement, scd4x_perform_forced_recalibration,
		scd4x_persist_settings, sl_sleeptimer_get_tick_count);

volatile struct
{
	bool isOn = false;
	bool isDiscarded = false;
	bool isMeas = false;
	bool LOCK = false;
	LPC::lpc_ret ret = LPC::ERR_NONE;
} scd41_state;

void BURTC_IRQHandler(void)
{
    BURTC_IntClear(BURTC_IF_COMP); // compare match
	BURTC_CounterReset();
	if (!scd41_state.isOn)
		scd41_state.isOn = true;
	else if (!scd41_state.isDiscarded && !scd41_state.isMeas) {
		scd41_state.isDiscarded = true;
	} else if (scd41_state.isDiscarded && !scd41_state.isMeas) {
		scd41_state.isMeas = true;
	} else if (scd41_state.isDiscarded && scd41_state.isMeas) {
		scd41_state.isDiscarded = false;
		scd41_state.isMeas = false;
		scd41_state.isOn = false;
	}

	scd41_state.LOCK = false;

	BURTC_IntDisable(BURTC_IEN_COMP);
}
otInstance *otGetInstance(void)
{
    return sInstance;
}
/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);

    va_list ap;
    va_start(ap, aFormat);
    otCliPlatLogv(aLogLevel, aLogRegion, aFormat, ap);
    va_end(ap);
}
#endif

void sl_ot_create_instance(void)
{
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    size_t   otInstanceBufferLength = 0;
    uint8_t *otInstanceBuffer       = NULL;

    // Call to query the buffer size
    (void)otInstanceInit(NULL, &otInstanceBufferLength);

    // Call to allocate the buffer
    otInstanceBuffer = (uint8_t *)malloc(otInstanceBufferLength);
    assert(otInstanceBuffer);

    // Initialize OpenThread with the buffer
    sInstance = otInstanceInit(otInstanceBuffer, &otInstanceBufferLength);
#else
    sInstance = otInstanceInitSingle();
#endif
    assert(sInstance);
}

void sl_ot_cli_init(void)
{
    otAppCliInit(sInstance);
}

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/

void app_init(void)
{
    sleepyInit();
    setNetworkConfiguration();
    assert(otIp6SetEnabled(sInstance, true) == OT_ERROR_NONE);
    assert(otThreadSetEnabled(sInstance, true) == OT_ERROR_NONE);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);
    applicationTick();
    //otCliOutputFormat("ret");
    if(scd41_state.LOCK) {
    	return;
    }
    scd41_state.LOCK = true;

	if (!scd41_state.isOn) {
		scd41_state.ret = scd41.powerOn();otCliOutputFormat("power on %d, \n", scd41_state.ret);
		BURTC_CounterReset();
		BURTC_CompareSet(0, 20);
		BURTC_IntEnable(BURTC_IEN_COMP);


	} else if (!scd41_state.isDiscarded && !scd41_state.isMeas) {
		scd41_state.ret = scd41.discardMeasurement();otCliOutputFormat("discard %d, \n", scd41_state.ret);
		BURTC_CounterReset();
		BURTC_CompareSet(0, 5000);
		BURTC_IntEnable(BURTC_IEN_COMP);

	} else if (scd41_state.isDiscarded && !scd41_state.isMeas) {
		scd41_state.ret = scd41.measure();otCliOutputFormat("meas %d, \n", scd41_state.ret);
		BURTC_CounterReset();
		BURTC_CompareSet(0, 5000);
		BURTC_IntEnable(BURTC_IEN_COMP);

	} else if (scd41_state.isDiscarded && scd41_state.isMeas) {
		uint16_t co2;
		int32_t hum, temp;
		scd41_state.ret = scd41.read(&co2, &temp, &hum);
		otCliOutputFormat(
				"Measurement result: CO2 %d, temp %d, hum %d  ERR code: %d \r\n",
				co2, temp, hum, scd41_state.ret);
		uint32_t age, num;
		int32_t offset;
		scd41.get_last_frc(&offset, &age, &num);
		otCliOutputFormat(
				"calibration result: Cycles ago: %d, Offset: %d, Total: %d \r\n",
				age, offset, num);
		scd41_state.ret = scd41.powerOff();
		BURTC_CounterReset();
		BURTC_CompareSet(0, 10000);
		BURTC_IntEnable(BURTC_IEN_COMP);
	}
}
/**************************************************************************//**
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
}
