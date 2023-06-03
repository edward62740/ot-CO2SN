/*
 * app_coap.h
 *
 *  Created on: Jun 2, 2023
 *      Author: edward62740
 */

#ifndef APP_COAP_H_
#define APP_COAP_H_

#include "openthread/ip6.h"
#include "app_main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	MSG_DATA,
	MSG_ALIVE,
} app_msg_t ;

extern otIp6Address selfAddr;
extern otIp6Address brAddr;
extern bool appCoapConnectionEstablished;

void appCoapInit(void);
void appCoapPermissionsHandler(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
bool appCoapCts(app_data_t *data, app_msg_t type);
void appCoapCheckConnection(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* APP_COAP_H_ */
