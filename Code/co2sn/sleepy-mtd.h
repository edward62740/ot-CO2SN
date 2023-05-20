/*
 * sleepy-mtd.h
 *
 *  Created on: May 20, 2023
 *      Author: Workstation
 */

#ifndef SLEEPY_MTD_H_
#define SLEEPY_MTD_H_

#ifdef __cplusplus
extern "C" {
#endif


void applicationTick(void);
void sleepyInit(void);
void setNetworkConfiguration(void);
void initUdp(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SLEEPY_MTD_H_ */
