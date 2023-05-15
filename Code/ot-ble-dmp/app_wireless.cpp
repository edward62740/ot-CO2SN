
/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
// Define module name for Power Manager debuging feature.
#define CURRENT_MODULE_NAME    "OPENTHREAD_SAMPLE_APP"

#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <openthread/srp_client.h>
#include <openthread/srp_client_buffers.h>
#include <openthread/coap.h>

#include "openthread-system.h"
#include <app_wireless.h>
#include "utils/code_utils.h"
#include "em_system.h"
#include "stdio.h"
#include "string.h"

#include "reset_util.h"

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif
#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
#include "sl_button.h"
#include "sl_simple_button.h"
#endif

static otInstance* sInstance = NULL;
#define SLEEPY_POLL_PERIOD_MS 5000

static bool srpDone = false;

extern "C" void otAppCliInit(otInstance *aInstance);

static bool             sButtonPressed  = false;
static bool             sStayAwake      = true;

otInstance *otGetInstance(void)
{
    return sInstance;
}

#if (defined(SL_CATALOG_BTN0_PRESENT) || defined(SL_CATALOG_BTN1_PRESENT))
void sl_button_on_change(const sl_button_t *handle)
{
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED)
    {
        sButtonPressed = true;
        otSysEventSignalPending();
    }
}
#endif

void sl_ot_rtos_application_tick(void)
{
    if (sButtonPressed)
    {
        sButtonPressed = false;
        sStayAwake     = !sStayAwake;
        if (sStayAwake)
        {
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
#endif
        }
        else
        {
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
#endif
        }
    }
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

void setNetworkConfiguration(void)
{
    otPlatRadioSetTransmitPower(otGetInstance(), 10);

    static char          aNetworkName[] = "";
    otError              error;
    otOperationalDataset aDataset;

    memset(&aDataset, 0, sizeof(otOperationalDataset));

    /*
     * Fields that can be configured in otOperationDataset to override defaults:
     *     Network Name, Mesh Local Prefix, Extended PAN ID, PAN ID, Delay Timer,
     *     Channel, Channel Mask Page 0, Network Key, PSKc, Security Policy
     */
    //aDataset.mActiveTimestamp.mSeconds             = 1;
    aDataset.mComponents.mIsActiveTimestampPresent = true;

    /* Set Channel to 15 */
    aDataset.mChannel                      = 0;
    aDataset.mComponents.mIsChannelPresent = true;

    /* Set Pan ID to 2222 */
    aDataset.mPanId                      = (otPanId)0;
    aDataset.mComponents.mIsPanIdPresent = true;

    /* Set Extended Pan ID to  */
    uint8_t extPanId[OT_EXT_PAN_ID_SIZE] = {};
    memcpy(aDataset.mExtendedPanId.m8, extPanId, sizeof(aDataset.mExtendedPanId));
    aDataset.mComponents.mIsExtendedPanIdPresent = true;

    /* Set network key to  */
    uint8_t key[OT_NETWORK_KEY_SIZE] = {};
    memcpy(aDataset.mNetworkKey.m8, key, sizeof(aDataset.mNetworkKey));
    aDataset.mComponents.mIsNetworkKeyPresent = true;

    /* Set Network Name */
    size_t length = strlen(aNetworkName);
    assert(length <= OT_NETWORK_NAME_MAX_SIZE);
    memcpy(aDataset.mNetworkName.m8, aNetworkName, length);
    aDataset.mComponents.mIsNetworkNamePresent = true;
    /* Set the Active Operational Dataset to this dataset */
    error = otDatasetSetActive(otGetInstance(), &aDataset);

    if (error != OT_ERROR_NONE)
    {
        //otCliOutputFormat("otDatasetSetActive failed with: %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }
}

void sleepyInit(void)
{
    otError error;

    otLinkModeConfig config;
    error = otLinkSetPollPeriod(otGetInstance(), SLEEPY_POLL_PERIOD_MS);

    config.mRxOnWhenIdle = false;
    config.mDeviceType   = 0;
    config.mNetworkData  = 0;
    error = otThreadSetLinkMode(otGetInstance(), config);

    if (error != OT_ERROR_NONE)
    {
        //otCliOutputFormat("Initialization failed with: %d, %s\r\n", error, otThreadErrorToString(error));
    }

}

void appSrpInit(void) {
	if (srpDone)
		return;
	srpDone = true;

	otError error = OT_ERROR_NONE;

	char *hostName;
	uint16_t size;
	uint8_t addr_prf = (uint8_t) (SYSTEM_GetUnique() & 0xFF);
	hostName = otSrpClientBuffersGetHostNameString(sInstance, &size);
	char *HOST_NAME = (char *) malloc(sizeof(char) * size);
	snprintf(HOST_NAME, size, "OT-CO2SN-%d", addr_prf);
	error = (otError) otSrpClientSetHostName(sInstance, HOST_NAME);
	memcpy(hostName, HOST_NAME, sizeof(HOST_NAME) + 1);

	otSrpClientEnableAutoHostAddress(sInstance);

	otSrpClientBuffersServiceEntry *entry = NULL;
	char *string;

	entry = otSrpClientBuffersAllocateService(sInstance);

	entry->mService.mPort = 33434;
	char INST_NAME[32];
	snprintf(INST_NAME, 32, "ipv6bc%d", (uint8_t) (SYSTEM_GetUnique() & 0xFF));
	const char *SERV_NAME = "_ot._udp";
	string = otSrpClientBuffersGetServiceEntryInstanceNameString(entry, &size);
	memcpy(string, INST_NAME, size);

	string = otSrpClientBuffersGetServiceEntryServiceNameString(entry, &size);
	memcpy(string, SERV_NAME, size);

	error = (otError) otSrpClientAddService(sInstance, &entry->mService);

	entry = NULL;

	otSrpClientEnableAutoStartMode(sInstance, /* aCallback */NULL, /* aContext */
	NULL);
	if (error != OT_ERROR_NONE)
		GPIO_PinOutSet(ERR_LED_PORT, ERR_LED_PIN);
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
    //appCoapInit();
    appSrpInit();
    OT_SETUP_RESET_JUMP(argv);
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
    otTaskletsProcess(sInstance);
    otSysProcessDrivers(sInstance);
}

/**************************************************************************//**
 * Application Exit.
 *****************************************************************************/
void app_exit(void)
{
    otInstanceFinalize(sInstance);
#if OPENTHREAD_CONFIG_MULTIPLE_INSTANCE_ENABLE
    free(otInstanceBuffer);
#endif
    // TO DO : pseudo reset?
}
