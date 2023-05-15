/*
 * scd4x_lpc.h
 *
 *  Created on: May 12, 2023
 *      Author: edward62740
 */

#ifndef SENSORS_SCD4X_SCD4X_LPC_H_
#define SENSORS_SCD4X_SCD4X_LPC_H_

#include "stdint.h"
#include <functional>
#include <iostream>
#include <queue>
#include <string_view>
#include <vector>

const uint16_t SCD4X_LPC_BASELINE_CO2_PPM = 400;
const uint32_t SCD4X_LPC_INIT_CAL_TRIG_CNT = 576;
const uint32_t SCD4X_LPC_STD_CAL_TRIG_CNT = 2016;

const uint32_t SCD4X_LPC_INIT_MAX_CAL_OFFSET = 10000;
const uint32_t SCD4X_LPC_STD_MAX_CAL_OFFSET = 100;

namespace LPC {

  typedef enum {
    ERR_NONE, ERR_COMMS, ERR_LPC, ERR_HW, ERR_WAIT
  } lpc_ret;

  class SCD4X
  {
  public:
    SCD4X(int16_t (*wu)(void), int16_t (*pd)(void), int16_t (*meas)(void),
          int16_t (*drdy)(bool *ready), int16_t (*read)(uint16_t* co2, int32_t* temp, int32_t* hum),
          int16_t (*frc)(uint16_t target, uint16_t* corr), int16_t (*persist)(void));

    lpc_ret powerOn();
    lpc_ret powerOff();
    bool measure();
    lpc_ret discardMeasurement();

    lpc_ret read(uint16_t *co2, int32_t *temp, int32_t *hum, void (*yield)(void) = NULL);


  private:

    struct
    {
      int16_t (*wu)( void );
      int16_t (*pd)( void );
      int16_t (*meas)( void );
      int16_t (*drdy)(bool *ready );
      int16_t (*read)(uint16_t* co2, int32_t* temp, int32_t* hum);
      int16_t (*frc)(uint16_t target, uint16_t* corr);
      int16_t (*persist)( void );
    } scd_fp;

    struct { bool first; int32_t ttl; bool expend;
    uint16_t gl_min; bool limit; } state ;
    uint32_t frc_ctr = 0;
    int32_t prev_frc_offset = 0;

  };
}



#endif /* SENSORS_SCD4X_SCD4X_LPC_H_ */
