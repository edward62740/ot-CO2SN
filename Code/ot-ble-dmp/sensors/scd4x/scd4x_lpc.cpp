/*
 * scd4x_lpc.cpp
 *
 *  Created on: May 12, 2023
 *      Author: edward62740
 */
#include "scd4x_lpc.h"
#include "scd4x_i2c.h"

namespace LPC {

  SCD4X::SCD4X(int16_t (*wu)(void), int16_t (*pd)(void), int16_t (*meas)(void),
                int16_t (*drdy)(bool *ready), int16_t (*read)(uint16_t* co2, int32_t* temp, int32_t* hum),
                int16_t (*frc)(uint16_t target, uint16_t* corr), int16_t (*persist)(void))
  {
    scd_fp.wu = wu;
    scd_fp.pd = pd;
    scd_fp.meas = meas;
    scd_fp.drdy = drdy;
    scd_fp.read = read;
    scd_fp.frc = frc;
    scd_fp.persist = persist;


    state.expend = false;
    state.gl_min = 0xFFFF;
    state.ttl = SCD4X_LPC_INIT_CAL_TRIG_CNT;
    state.limit = false;
    state.first = true;
  }


  lpc_ret SCD4X::powerOn()
  {
    scd_fp.wu();
  }

  lpc_ret SCD4X::powerOff()
  {
    scd_fp.pd();
  }

  lpc_ret SCD4X::discardMeasurement()
  {
    scd_fp.meas();
  }

  bool SCD4X::measure()
  {
    scd_fp.meas();
    if(state.ttl <= 1) return true; // warn if yield fn should be passed
    else return false;
  }

  lpc_ret SCD4X::read(uint16_t *co2, int32_t *temp, int32_t *hum, void (*yield)(void))
  {
    bool tmp = false;
    scd_fp.drdy(&tmp);
    if(!tmp) return ERR_WAIT;

    scd_fp.read(co2, temp, hum);
    if(*co2 == 0) return ERR_HW;
    if(*co2 < state.gl_min) state.gl_min = *co2;
    if(*co2 < SCD4X_LPC_BASELINE_CO2_PPM) // called if co2 < 400
      {
        state.expend = true;
        uint32_t offset = SCD4X_LPC_BASELINE_CO2_PPM - *co2;
        state.limit = offset>(state.first?SCD4X_LPC_INIT_MAX_CAL_OFFSET:SCD4X_LPC_STD_MAX_CAL_OFFSET);
        uint16_t ret;
        scd_fp.frc(state.limit?(state.first?SCD4X_LPC_INIT_MAX_CAL_OFFSET:SCD4X_LPC_STD_MAX_CAL_OFFSET)
            :SCD4X_LPC_BASELINE_CO2_PPM, &ret); frc_ctr++;
        prev_frc_offset = ret - 0x8000U;
        if(yield != NULL) yield();
        if(ret == 0xFFFF) return ERR_LPC; // return on next calls, ttl will be -ve
      }
    if(--state.ttl <= 0) // called iff co2 >= 400 for the entire period
      {

        state.ttl = SCD4X_LPC_STD_CAL_TRIG_CNT;
        state.gl_min = 0xFFFF;

        if(state.expend) { state.expend = false; return ERR_NONE; } // do not frc again if already done

        uint32_t offset = state.gl_min - SCD4X_LPC_BASELINE_CO2_PPM;
        state.limit = offset>SCD4X_LPC_STD_MAX_CAL_OFFSET;
        uint16_t ret;
        scd_fp.frc(*co2 - state.limit?SCD4X_LPC_STD_MAX_CAL_OFFSET:offset, &ret); frc_ctr++;
        prev_frc_offset = ret - 0x8000U;
        if(yield != NULL) yield();
        if(ret == 0xFFFF) return ERR_LPC; // return on next calls, ttl will be -ve
      }
    state.first = false;
    return ERR_NONE;

  }


}
