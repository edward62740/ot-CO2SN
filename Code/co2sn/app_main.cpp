#include <app_main.h>
#include <app_thread.h>
#include <app_coap.h>
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>
#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/srp_client.h>
#include <openthread/srp_client_buffers.h>
#include "openthread-system.h"
#include "sl_ot_init.h"
#include "sl_component_catalog.h"
#include "em_burtc.h"
#include "em_iadc.h"
#include "em_gpio.h"
#include "sensors/scd4x/scd4x_i2c.h"
#include "sensors/scd4x/scd4x_lpc.h"
#include "sensors/opt3001/opt3001.h"
#include "sl_sleeptimer.h"
#include "sl_system_process_action.h"
#include <stdio.h>
#include <string.h>

static otInstance* sInstance = NULL;
static const uint32_t MEASUREMENT_INTERVAL_MS = 60000;
static const uint32_t VSENSE_DIV_MULTIPLIER = 2;
extern void otAppCliInit(otInstance *aInstance);

eui_t eui; // device EUI64
app_data_t app_data = {}; // application public variables
LPC::SCD4X scd41(scd4x_wake_up, scd4x_power_down, scd4x_measure_single_shot,
		scd4x_get_data_ready_flag, scd4x_read_measurement, scd4x_perform_forced_recalibration,
		scd4x_persist_settings, sl_sleeptimer_get_tick_count); // LPC instance

/* application scd41 state variables    */
/* isOn -> isDiscarded -> isMeas  */
/* ^----------------------------  */
static volatile struct
{
	bool isOn = false;
	bool isDiscarded = false;
	bool isMeas = false;
	bool LOCK = false;
	LPC::lpc_ret ret = LPC::ERR_NONE;
} scd41_state;

static bool appSrpDone = false;
static bool appCoapSendAlive = false;
sl_sleeptimer_timer_handle_t alive_timer;

/** HANDLERS/ISR **/
void alive_cb(sl_sleeptimer_timer_handle_t *handle, void *data)
{
	appCoapSendAlive = true;
}

void IADC_IRQHandler(void) {
	IADC_Result_t sample;
	sample = IADC_pullSingleFifoResult(IADC0);
	app_data.vdd = ((sample.data * 1200) / 1000) * VSENSE_DIV_MULTIPLIER;
	IADC_clearInt(IADC0, IADC_IF_SINGLEDONE);
}


void BURTC_IRQHandler(void)
{
    BURTC_IntClear(BURTC_IF_COMP); // compare match
	BURTC_CounterReset();
	BURTC_Stop();

	/* application state transition */
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


/** Application logic **/
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


static void appSrpInit(void)
{
    if(appSrpDone) return;
    appSrpDone = true;

    otError error = OT_ERROR_NONE;

    char *hostName;
    const char *HOST_NAME = "OT-CO2SN-0";
    uint16_t size;
    hostName = otSrpClientBuffersGetHostNameString(sInstance, &size);
    error = otSrpClientSetHostName(sInstance, HOST_NAME);
    memcpy(hostName, HOST_NAME, sizeof(HOST_NAME) + 1);


    otSrpClientEnableAutoHostAddress(sInstance);


    otSrpClientBuffersServiceEntry *entry = NULL;
    char *string;

    entry = otSrpClientBuffersAllocateService(sInstance);

    entry->mService.mPort = 33434;
    char INST_NAME[32];
    snprintf(INST_NAME, 32, "ipv6bc%d", (uint8_t)(eui._64b & 0xFF));
    const char *SERV_NAME = "_ot._udp";
    string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
    memcpy(string, INST_NAME, size);


    string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
    memcpy(string, SERV_NAME, size);

    error = otSrpClientAddService(sInstance, &entry->mService);

    assert(OT_ERROR_NONE == error);

    entry = NULL;

    otSrpClientEnableAutoStartMode(sInstance, /* aCallback */ NULL, /* aContext */ NULL);
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
    appCoapInit();
    appSrpInit();

}


static void app_power_status(void)
{
	app_data.pwr_state = (GPIO_PinInGet(PWR_ST_0_PORT, PWR_ST_0_PIN) & 0x1) |
			(GPIO_PinInGet(PWR_ST_1_PORT, PWR_ST_1_PIN) & 0x1) << 1;
}



static void app_scd_algo(void) {
	if (scd41_state.LOCK)
		return;

	scd41_state.LOCK = true;

	if (!scd41_state.isOn) {
		scd41_state.ret = scd41.powerOn();
		otCliOutputFormat("power on %d, \n", scd41_state.ret);
		BURTC_CounterReset();
		BURTC_CompareSet(0, 20);
		BURTC_IntEnable(BURTC_IEN_COMP);

	} else if (!scd41_state.isDiscarded && !scd41_state.isMeas) {
		scd41_state.ret = scd41.discardMeasurement();
		otCliOutputFormat("discard %d, \n", scd41_state.ret);
		BURTC_CounterReset();
		BURTC_CompareSet(0, 5200);
		BURTC_IntEnable(BURTC_IEN_COMP);

	} else if (scd41_state.isDiscarded && !scd41_state.isMeas) {
		scd41_state.ret = scd41.measure();
		otCliOutputFormat("meas %d, \n", scd41_state.ret);
		BURTC_CounterReset();
		BURTC_CompareSet(0, 5200);
		BURTC_IntEnable(BURTC_IEN_COMP);

	} else if (scd41_state.isDiscarded && scd41_state.isMeas) {
		scd41_state.ret = scd41.read(&app_data.co2, &app_data.temp, &app_data.hum);
		otCliOutputFormat(
				"Measurement result: CO2 %d, temp %d, hum %d  ERR code: %d \r\n",
				app_data.co2, app_data.temp, app_data.hum, scd41_state.ret);
		//GPIO_PinOutSet(PWR_EN_ST_PORT, PWR_EN_ST_PIN);
		scd41.get_last_frc(&app_data.offset, &app_data.age, &app_data.num);
		otCliOutputFormat(
				"calibration result: Cycles ago: %d, Offset: %d, Total: %d \r\n",
				app_data.age, app_data.offset, app_data.num);
		app_power_status();
		//GPIO_PinOutClear(PWR_EN_ST_PORT, PWR_EN_ST_PIN);
		scd41_state.ret = scd41.powerOff();
		app_data.isPend = !appCoapCts(&app_data, MSG_DATA);

		BURTC_CounterReset();
		BURTC_CompareSet(0, MEASUREMENT_INTERVAL_MS);
		BURTC_IntEnable(BURTC_IEN_COMP);
	}
}



void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);
    applicationTick();
    app_scd_algo();
    if(app_data.isPend) app_data.isPend = !appCoapCts(&app_data, MSG_DATA);
    else if(appCoapSendAlive) { appCoapCts(&app_data, MSG_ALIVE); appCoapSendAlive = false; }
}
/**************************************************************************//**
 * Application Exit.`
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
}
