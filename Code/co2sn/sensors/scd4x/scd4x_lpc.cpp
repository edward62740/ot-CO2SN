/*
 * scd4x_lpc.cpp
 *
 *  Created on: May 12, 2023
 *      Author: edward62740
 */
#include "scd4x_lpc.h"
#include "scd4x_i2c.h"
#include <openthread/cli.h>
#include <openthread/platform/logging.h>

namespace LPC {

  SCD4X::SCD4X(int16_t (*wu)(void), int16_t (*pd)(void), int16_t (*meas)(void),
                int16_t (*drdy)(bool *ready), int16_t (*read)(uint16_t* co2, int32_t* temp, int32_t* hum),
                int16_t (*frc)(uint16_t target, uint16_t* corr), int16_t (*persist)(void), uint32_t (*getMsTick)(void))
  {
    scd_fp.wu = wu;
    scd_fp.pd = pd;
    scd_fp.meas = meas;
    scd_fp.drdy = drdy;
    scd_fp.read = read;
    scd_fp.frc = frc;
    scd_fp.persist = persist;
    scd_fp.getMsTick = getMsTick;

    fsm.state = STATE_FIRST;
    fsm.expend = false;
    fsm.gl_min = 0xFFFF;
    fsm.ttl = _sig_get_fsm_cnt();
    fsm.limit = false;
    fsm._cons_fail = 0;

  }

lpc_ret SCD4X::sig_ext_reset_fsm(void) {
	fsm.state = STATE_FIRST;
	fsm.expend = false;
	fsm.gl_min = 0xFFFF;
	fsm.ttl = _sig_get_fsm_cnt();
	fsm.limit = false;
	fsm._cons_fail = 0;
	return ERR_NONE;
}

void SCD4X::get_last_frc(int32_t *offset, uint32_t *age, uint32_t *num)
{
	*offset = prev_frc_offset;
	*age = prev_frc_age;
	*num = frc_ctr;
}

lpc_ret SCD4X::powerOn(void) {
	if(!_mutex_request_unlock()) return ERR_WAIT;
	if (scd_fp.wu())
		return _sig_scd_fail(true);
	_sig_scd_fail(false);
	_mutex_request_lock(SCD4X_LPC_CMD_WU_LOCK_DUR_MS);
	return ERR_NONE;
}

lpc_ret SCD4X::powerOff(void) {
	if(!_mutex_request_unlock()) return ERR_WAIT;
	if (scd_fp.pd())
		return _sig_scd_fail(true);
	_sig_scd_fail(false);
	_mutex_request_lock(SCD4X_LPC_CMD_PD_LOCK_DUR_MS);
	return ERR_NONE;
}

lpc_ret SCD4X::discardMeasurement(void) {
	if(!_mutex_request_unlock()) return ERR_WAIT;
	if (scd_fp.meas())
		return _sig_scd_fail(true);
	_sig_scd_fail(false);
	_mutex_request_lock(SCD4X_LPC_CMD_MEAS_LOCK_DUR_MS);
	return ERR_NONE;
}

lpc_ret SCD4X::measure()
  {
	if(!_mutex_request_unlock()) return ERR_WAIT;
	if (scd_fp.meas())
		return _sig_scd_fail(true);
	if (fsm.ttl <= 1)
		return WARN_PRETRIG; // warn if yield fn should be passed
	_sig_scd_fail(false);
	_mutex_request_lock(SCD4X_LPC_CMD_MEAS_LOCK_DUR_MS);
	return ERR_NONE;

  }

  lpc_ret SCD4X::read(uint16_t *co2, int32_t *temp, int32_t *hum)
  {
    bool tmp = false;
    scd_fp.drdy(&tmp);
    if(!tmp) return ERR_WAIT;
    if(!_mutex_request_unlock()) return ERR_WAIT;
    if(scd_fp.read(co2, temp, hum)) return _sig_scd_fail(true);
    if(*co2 == 0) return _sig_scd_fail(true);
	_sig_scd_fail(false);
    if(*co2 < fsm.gl_min) fsm.gl_min = *co2;
    prev_frc_age++;
    return _proc_fsm(*co2);

  }

  lpc_ret SCD4X::_proc_fsm(uint16_t co2)
  {

	if(fsm._cons_fail > SCD4X_LPC_SCD_FAIL_TOL_TH) return ERR_PANIC;
	if (co2 < SCD4X_LPC_BASELINE_CO2_PPM && !fsm.expend) // called if co2 < 400
			{

		fsm.expend = true;
		uint32_t offset = SCD4X_LPC_BASELINE_CO2_PPM - co2;
		fsm.limit = offset > _sig_get_fsm_offset();
		uint16_t ret = 0xFFFF;
		scd_fp.frc(fsm.limit ?co2+_sig_get_fsm_offset() : SCD4X_LPC_BASELINE_CO2_PPM, &ret);
		otCliOutputFormat("%d baseline calibration \r\n", fsm.ttl);
		_mutex_request_lock(SCD4X_LPC_CMD_FRC_LOCK_DUR_MS);
		frc_ctr++;
		prev_frc_offset = ret - 0x8000U;
		if (ret == 0xFFFF)
			return ERR_LPC; // return on next calls, ttl will be -ve
	}
	if (--fsm.ttl <= 0) {

		_sig_fsm_next();
		uint16_t _gl_tmp = fsm.gl_min;
		fsm.ttl = _sig_get_fsm_cnt();
		fsm.gl_min = 0xFFFF;

		if (fsm.expend) {
			fsm.expend = false;
			return ERR_NONE;
		} // do not frc again if already done

		// called iff co2 >= 400 for the entire period
		uint32_t offset = _gl_tmp - SCD4X_LPC_BASELINE_CO2_PPM;
		fsm.limit = offset > _sig_get_fsm_offset();
		uint16_t ret = 0xFFFF;
		scd_fp.frc(co2 - (fsm.limit ? _sig_get_fsm_offset() : offset),
				&ret);
		otCliOutputFormat("%d ttl calibration \r\n", fsm.ttl);
		_mutex_request_lock(SCD4X_LPC_CMD_FRC_LOCK_DUR_MS);
		frc_ctr++;
		prev_frc_offset = ret - 0x8000U;
		prev_frc_age = 0;
		if (ret == 0xFFFF)
			return ERR_LPC; // return on next calls, ttl will be -ve
	}

	return ERR_NONE;
  }

void SCD4X::_sig_fsm_next(void) {

	otCliOutputFormat("Next state: %d \r\n", fsm.state+1);
	switch (fsm.state) {
	case STATE_FIRST: {
		fsm.state = STATE_INIT;
		break;
	}
	case STATE_INIT: {
		fsm.state = STATE_STD;
		break;
	}
	case STATE_STD: {
		fsm.state = STATE_STD;
		break;
	}


	}
}

uint32_t SCD4X::_sig_get_fsm_offset(void) {
	switch (fsm.state) {
	case STATE_FIRST:
		return SCD4X_LPC_FIRST_MAX_CAL_OFFSET;
	case STATE_INIT:
		return SCD4X_LPC_INIT_MAX_CAL_OFFSET;
	case STATE_STD:
		return SCD4X_LPC_STD_MAX_CAL_OFFSET;
	}
	return 0;
}

uint32_t SCD4X::_sig_get_fsm_cnt(void) {
	switch (fsm.state) {
	case STATE_FIRST:
		return SCD4X_LPC_FIRST_CAL_TRIG_CNT;
	case STATE_INIT:
		return SCD4X_LPC_INIT_CAL_TRIG_CNT;
	case STATE_STD:
		return SCD4X_LPC_STD_CAL_TRIG_CNT;
	}
	return 0;
}


lpc_ret SCD4X::_sig_scd_fail(bool trig)
{
	if (!trig) {
		fsm._cons_fail = 0;
		return ERR_NONE;
	}
	else fsm._cons_fail++;
	otCliOutputFormat("_sig_scd_fail \r\n");
	return ERR_SCD;
}


bool SCD4X::_mutex_request_lock(uint32_t dur) {
	//CORE_DECLARE_IRQ_STATE;

	//CORE_ENTER_ATOMIC();

	if (_mutex.state)
		return false;
	if(dur > SCD4X_LPC_CMD_MAX_LOCK_DUR_MS) return false;
	_mutex.lastTickMs = scd_fp.getMsTick();
	_mutex.reqLockDurMs = dur;
	return true;

	//CORE_EXIT_ATOMIC();
}

bool SCD4X::_mutex_request_unlock(void) {
	if(!_mutex.state) return true;
	int64_t diff = scd_fp.getMsTick() - _mutex.lastTickMs;
	if (diff < 0 || diff > _mutex.reqLockDurMs) {
		_mutex.state = false;
		return true;
	}
	else return false;

}

}
