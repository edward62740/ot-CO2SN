/*
 * app_coap.c
 *
 *  Created on: Jun 2, 2023
 *      Author: edward62740
 */

#include <app_coap.h>
#include <assert.h>
#include <openthread-core-config.h>
#include <openthread/config.h>

#include <openthread/cli.h>
#include <openthread/diag.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include "openthread-system.h"
#include "sl_component_catalog.h"

#include <openthread/coap.h>
#include "utils/code_utils.h"

#include "stdio.h"
#include "string.h"
#include "app_main.h"

char resource_name[32];
char payload_buf[255];

const char *ack = "ack";
const char *nack = "nack";
const char *fmtPayloadData = "%d,%.8lx%.8lx,%d,%d,%d,%d,%d,%lu,%lu,%lu,%d,%lu";
const char *fmtPayloadAlive = "%d,%.8lx%.8lx,%d,%d,%d";

#define PERMISSIONS_URI "permissions"
otCoapResource mResource_PERMISSIONS;
otIp6Address brAddr;
otIp6Address selfAddr;

const char mPERMISSIONSUriPath[] = PERMISSIONS_URI;

bool appCoapConnectionEstablished = false;
uint32_t appCoapFailCtr = 0;
uint32_t appCoapSendTxCtr = 0;
const uint8_t device_type = 2U;

void appCoapInit()
{
    // GPIO_PinOutSet(IP_LED_PORT, IP_LED_PIN);

    otCoapStart(otGetInstance(), OT_DEFAULT_COAP_PORT);

    mResource_PERMISSIONS.mUriPath = mPERMISSIONSUriPath;
    mResource_PERMISSIONS.mContext = otGetInstance();
    mResource_PERMISSIONS.mHandler = &appCoapPermissionsHandler;

    strncpy((char *)mPERMISSIONSUriPath, PERMISSIONS_URI, sizeof(PERMISSIONS_URI));
    otCoapAddResource(otGetInstance(), &mResource_PERMISSIONS);

    // GPIO_PinOutClear(IP_LED_PORT, IP_LED_PIN);
}

void appCoapPermissionsHandler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo)
{
    // GPIO_PinOutSet(IP_LED_PORT, IP_LED_PIN);
    // printIPv6Addr(&aMessageInfo->mPeerAddr);
    brAddr = aMessageInfo->mPeerAddr;
    selfAddr = aMessageInfo->mSockAddr;
    otError error = OT_ERROR_NONE;
    otMessage *responseMessage;
    uint16_t offset;
    otCoapCode responseCode = OT_COAP_CODE_CHANGED;
    otCoapCode messageCode = otCoapMessageGetCode(aMessage);

    responseMessage = otCoapNewMessage((otInstance *)aContext, NULL);
    otEXPECT_ACTION(responseMessage != NULL, error = OT_ERROR_NO_BUFS);
    otCoapMessageInitResponse(responseMessage, aMessage,
                              OT_COAP_TYPE_ACKNOWLEDGMENT, responseCode);
    otCoapMessageSetToken(responseMessage, otCoapMessageGetToken(aMessage),
                          otCoapMessageGetTokenLength(aMessage));
    otCoapMessageSetPayloadMarker(responseMessage);

    offset = otMessageGetOffset(aMessage);
    otMessageRead(aMessage, offset, resource_name, sizeof(resource_name) - 1);
    // otCliOutputFormat("Unique resource ID: %s\n", resource_name);

    if (OT_COAP_CODE_GET == messageCode)
    {

        error = otMessageAppend(responseMessage, ack, strlen((const char *)ack));
        otEXPECT(OT_ERROR_NONE == error);
        error = otCoapSendResponse((otInstance *)aContext, responseMessage,
                                   aMessageInfo);
        otEXPECT(OT_ERROR_NONE == error);
    }
    else
    {
        error = otMessageAppend(responseMessage, nack, strlen((const char *)nack));
        otCoapMessageSetCode(responseMessage, OT_COAP_CODE_METHOD_NOT_ALLOWED);
        otEXPECT(OT_ERROR_NONE == error);
        error = otCoapSendResponse((otInstance *)aContext, responseMessage,
                                   aMessageInfo);
        otEXPECT(OT_ERROR_NONE == error);
    }

exit:
    if (error != OT_ERROR_NONE && responseMessage != NULL)
    {
        otMessageFree(responseMessage);
    }
    appCoapConnectionEstablished = true;
    // GPIO_PinOutClear(IP_LED_PORT, IP_LED_PIN);
}

static bool appCoapSend(char *payload, bool require_ack)
{
    //appCoapCheckConnection();
    if (!appCoapConnectionEstablished)
        return false;
    // GPIO_PinOutSet(IP_LED_PORT, IP_LED_PIN);
    otError error = OT_ERROR_NONE;
    otMessage *message = NULL;
    otMessageInfo messageInfo;
    uint16_t payloadLength = 0;

    // Default parameters
    otCoapType coapType = require_ack ? OT_COAP_TYPE_CONFIRMABLE : OT_COAP_TYPE_NON_CONFIRMABLE;
    otIp6Address coapDestinationIp = brAddr;
    message = otCoapNewMessage(otGetInstance(), NULL);

    otCoapMessageInit(message, coapType, OT_COAP_CODE_PUT);
    otCoapMessageGenerateToken(message, OT_COAP_DEFAULT_TOKEN_LENGTH);
    error = otCoapMessageAppendUriPathOptions(message, resource_name);
    otEXPECT(OT_ERROR_NONE == error);

    payloadLength = strlen(payload);

    if (payloadLength > 0)
    {
        error = otCoapMessageSetPayloadMarker(message);
        otEXPECT(OT_ERROR_NONE == error);
    }

    // Embed content into message if given
    if (payloadLength > 0)
    {
        error = otMessageAppend(message, payload, payloadLength);
        otEXPECT(OT_ERROR_NONE == error);
    }

    memset(&messageInfo, 0, sizeof(messageInfo));
    messageInfo.mPeerAddr = coapDestinationIp;
    messageInfo.mPeerPort = OT_DEFAULT_COAP_PORT;
    error = otCoapSendRequestWithParameters(otGetInstance(), message,
                                            &messageInfo, NULL, NULL,
                                            NULL);
    otEXPECT(OT_ERROR_NONE == error);

exit:
    if ((error != OT_ERROR_NONE) && (message != NULL))
    {
        appCoapFailCtr = 0;
        otMessageFree(message);
    }

    if (error != OT_ERROR_NONE)
    {
        appCoapFailCtr++;
        return false;
        // GPIO_PinOutSet(ERR_LED_PORT, ERR_LED_PIN);
    }
    // else GPIO_PinOutClear(ERR_LED_PORT, ERR_LED_PIN);
    return true;
    // otCliOutputFormat("Sent message: %d\n", error);
    // GPIO_PinOutClear(IP_LED_PORT, IP_LED_PIN);
}

bool appCoapCts(app_data_t *data, app_msg_t type)
{
    if (!appCoapConnectionEstablished)
    {
        data->isPend = false;
        return false;
    }
    memset(payload_buf, 0, sizeof(char));
    int8_t rssi;
    otThreadGetParentLastRssi(otGetInstance(), &rssi);
    uint32_t sz = 255;
    switch (type)
    {
    case MSG_DATA:
    {
        /** CoAP Payload String (MSG_DATA) **
         * device_type (uint8_t): internal use number for indicating sensor type
         * eui64 (uint32_t): unique id MSB
         * eui64 (uint32_t): unique id LSB
         * error (uint8_t): indicate validity of data
         * co2 (uint16_t): co2 in ppm
         * temp (int32_t): temperature in degC
         * hum (int32_t): RH %
         * offset (int32_t): offset of last known calibration
         * age (uint32_t): age of last known calibration; 0 if this cycle
         * num (uint32_t): total calibrations performed since POR
         *
         * vdd (uint32_t): primary battery voltage
         *
         * rssi (int8_t): last rssi from parent
         * appCoapSendTxCtr (uint32_t): total CoAP transmissions
         */
        sz = snprintf(payload_buf, 254, fmtPayloadData, device_type, eui._32b.h,
                      eui._32b.l, (uint8_t)data->error, data->co2, data->temp,
                      data->hum, data->offset, data->age, data->num, 0, rssi,
                      ++appCoapSendTxCtr);
        break;
    }
    case MSG_ALIVE:
    {
        /** CoAP Payload String (MSG_ALIVE) **
         * device_type (uint8_t): internal use number for indicating sensor type
         * eui64 (uint32_t): unique id MSB
         * eui64 (uint32_t): unique id LSB
         * vdd (uint32_t): primary battery voltage
         * rssi (int8_t): last rssi from parent
         * appCoapSendTxCtr (uint32_t): total CoAP transmissions
         */
        sz = snprintf(payload_buf, 254, fmtPayloadAlive, device_type, eui._32b.h,
                      eui._32b.l, 0, rssi,
                      ++appCoapSendTxCtr);
        break;
    }
    }
    if (sz >= 254)
        return false;
    bool ret = appCoapSend(payload_buf, data->isPend);
    if (!ret)
        return false;
    data->isPend = false;
    return true;
}

void appCoapCheckConnection(void)
{

    if (otThreadGetDeviceRole(otGetInstance()) != OT_DEVICE_ROLE_CHILD || appCoapFailCtr > 10)
    {
        otThreadBecomeDetached(otGetInstance());
        // otInstanceErasePersistentInfo();
        // sleepyInit();
        setNetworkConfiguration();
        otIp6SetEnabled(otGetInstance(), true);
        otThreadSetEnabled(otGetInstance(), true);
        appCoapInit();
        appCoapFailCtr = 0;
    }
}
