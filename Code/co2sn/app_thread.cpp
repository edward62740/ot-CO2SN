
// Define module name for Power Manager debuging feature.
#define CURRENT_MODULE_NAME    "OPENTHREAD_SAMPLE_APP"

#include <app_thread.h>
#include <string.h>
#include <assert.h>

#include <openthread/cli.h>
#include <openthread/dataset_ftd.h>
#include <openthread/instance.h>
#include <openthread/message.h>
#include <openthread/thread.h>
#include <openthread/udp.h>
#include <openthread/platform/logging.h>
#include <common/code_utils.hpp>
#include <common/logging.hpp>



#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

#define SLEEPY_POLL_PERIOD_MS 2000

otInstance *otGetInstance(void);

static bool                sAllowSleep                    = true;

void sleepyInit(void)
{
    otError error;
    otCliOutputFormat("sleepy-demo-mtd started\r\n");

    otLinkModeConfig config;
    SuccessOrExit(error = otLinkSetPollPeriod(otGetInstance(), SLEEPY_POLL_PERIOD_MS));

    config.mRxOnWhenIdle = false;
    config.mDeviceType   = 0;
    config.mNetworkData  = 0;
    SuccessOrExit(error = otThreadSetLinkMode(otGetInstance(), config));

exit:
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("Initialization failed with: %d, %s\r\n", error, otThreadErrorToString(error));
    }
    return;
}

/*
 * Callback from sl_ot_is_ok_to_sleep to check if it is ok to go to sleep.
 */
bool efr32AllowSleepCallback(void)
{
    return sAllowSleep;
}

/*
 * Override default network settings, such as panid, so the devices can join a network
 */
void setNetworkConfiguration(void)
{

}




#ifdef SL_CATALOG_KERNEL_PRESENT
#define applicationTick sl_ot_rtos_application_tick
#endif

void applicationTick(void)
{

}


