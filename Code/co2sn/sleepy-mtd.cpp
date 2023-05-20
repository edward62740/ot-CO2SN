/***************************************************************************//**
 * @file
 * @brief MTD application logic.
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

#include "sleepy-mtd.h"
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

#include "sl_button.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#include "sl_component_catalog.h"
#ifdef SL_CATALOG_POWER_MANAGER_PRESENT
#include "sl_power_manager.h"
#endif

// Constants
#define MULTICAST_ADDR "ff03::1"
#define MULTICAST_PORT 123
#define RECV_PORT 234
#define SLEEPY_POLL_PERIOD_MS 2000
#define MTD_MESSAGE "mtd button"
#define FTD_MESSAGE "ftd button"


otInstance *otGetInstance(void);
void mtdReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);




// Variables
static otUdpSocket         sMtdSocket;
static bool                sButtonPressed                 = false;
static bool                sRxOnIdleButtonPressed         = false;
static bool                sAllowSleep                    = false;

void sleepyInit(void)
{
    otError error;
    otCliOutputFormat("sleepy-demo-mtd started\r\n");

    otLinkModeConfig config;
    SuccessOrExit(error = otLinkSetPollPeriod(otGetInstance(), SLEEPY_POLL_PERIOD_MS));

    config.mRxOnWhenIdle = true;
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

void initUdp(void)
{
    otError    error;
    otSockAddr bindAddr;

    // Initialize bindAddr
    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.mPort = RECV_PORT;

    // Open the socket
    error = otUdpOpen(otGetInstance(), &sMtdSocket, mtdReceiveCallback, NULL);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("MTD failed to open udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        return;
    }

    // Bind to the socket. Close the socket if bind fails.
    error = otUdpBind(otGetInstance(), &sMtdSocket, &bindAddr, OT_NETIF_THREAD);
    if (error != OT_ERROR_NONE)
    {
        otCliOutputFormat("MTD failed to bind udp socket with: %d, %s\r\n", error, otThreadErrorToString(error));
        IgnoreReturnValue(otUdpClose(otGetInstance(), &sMtdSocket));
        return;
    }
}

void sl_button_on_change(const sl_button_t *handle)
{
    if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED)
    {
        if (&sl_button_btn0 == handle)
        {
            sRxOnIdleButtonPressed = true;
        }
        else if (&sl_button_btn1 == handle)
        {
            sButtonPressed = true;
        }
       // otSysEventSignalPending();
    }
}

#ifdef SL_CATALOG_KERNEL_PRESENT
#define applicationTick sl_ot_rtos_application_tick
#endif

void applicationTick(void)
{
    otLinkModeConfig config;
    otMessageInfo    messageInfo;
    otMessage       *message = NULL;
    const char      *payload = MTD_MESSAGE;

    // Check for BTN0 button press
    if (sRxOnIdleButtonPressed)
    {
        sRxOnIdleButtonPressed = false;
        sAllowSleep            = !sAllowSleep;
        config.mRxOnWhenIdle   = !sAllowSleep;
        config.mDeviceType     = 0;
        config.mNetworkData    = 0;

        // Reattach from network if device wants to become sleepy
        if (!config.mRxOnWhenIdle)
        {
            SuccessOrExit(otThreadBecomeDetached(otGetInstance()));
        }

        // Set the Link Mode
        // NOTE: Setting the Link Mode when a device is detached also causes the device to attempt to attach
        SuccessOrExit(otThreadSetLinkMode(otGetInstance(), config));

#if (defined(SL_CATALOG_KERNEL_PRESENT) && defined(SL_CATALOG_POWER_MANAGER_PRESENT))
        if (sAllowSleep)
        {
            sl_power_manager_remove_em_requirement(SL_POWER_MANAGER_EM1);
        }
        else
        {
            sl_power_manager_add_em_requirement(SL_POWER_MANAGER_EM1);
        }
#endif
    }

    // Check for BTN1 button press
    if (sButtonPressed)
    {
        sButtonPressed = false;

        // Get a message buffer
        VerifyOrExit((message = otUdpNewMessage(otGetInstance(), NULL)) != NULL);

        // Setup messageInfo
        memset(&messageInfo, 0, sizeof(messageInfo));
        SuccessOrExit(otIp6AddressFromString(MULTICAST_ADDR, &messageInfo.mPeerAddr));
        messageInfo.mPeerPort = MULTICAST_PORT;

        // Append the MTD_MESSAGE payload to the message buffer
        SuccessOrExit(otMessageAppend(message, payload, (uint16_t)strlen(payload)));

        // Send the button press message
        SuccessOrExit(otUdpSend(otGetInstance(), &sMtdSocket, message, &messageInfo));

        // Set message pointer to NULL so it doesn't get free'd by this function.
        // otUdpSend() executing successfully means OpenThread has taken ownership
        // of the message buffer.
        message = NULL;
    }

exit:
    if (message != NULL)
    {
        otMessageFree(message);
    }
    return;
}

void mtdReceiveCallback(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    OT_UNUSED_VARIABLE(aContext);
    OT_UNUSED_VARIABLE(aMessageInfo);
    uint8_t buf[64];
    int     length;

    // Read the received message's payload
    length      = otMessageRead(aMessage, otMessageGetOffset(aMessage), buf, sizeof(buf) - 1);
    buf[length] = '\0';

    // Check that the payload matches FTD_MESSAGE
    VerifyOrExit(strncmp((char *)buf, FTD_MESSAGE, sizeof(FTD_MESSAGE)) == 0);

    otCliOutputFormat("Message Received: %s\r\n", buf);

exit:
    return;
}
