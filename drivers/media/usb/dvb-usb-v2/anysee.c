/*
 * DVB USB Linux driver for Anysee E30 DVB-C & DVB-T USB2.0 receiver
 *
 * Copyright (C) 2007 Antti Palosaari <crope@iki.fi>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * TODO:
 * - add smart card reader support for Conditional Access (CA)
 *
 * Card reader in Anysee is nothing more than ISO 7816 card reader.
 * There is no hardware CAM in any Anysee device sold.
 * In my understanding it should be implemented by making own module
 * for ISO 7816 card reader, like dvb_ca_en50221 is implemented. This
 * module registers serial interface that can be used to communicate
 * with any ISO 7816 smart card.
 *
 * Any help according to implement serial smart card reader support
 * is highly welcome!
 */

#include "anysee.h"
#include "dvb-pll.h"
#include "tda1002x.h"
#include "mt352.h"
#include "mt352_priv.h"
#include "zl10353.h"
#include "tda18212.h"
#include "cx24116.h"
#include "stv0900.h"
#include "stv6110.h"
#include "isl6423.h"
#include "cxd2820r.h"

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static int anysee_ctrl_msg(struct dvb_usb_device *d,
		u8 *sbuf, u8 slen, u8 *rbuf, u8 rlen)
{
	struct anysee_state *state = d_to_priv(d);
	int act_len, ret, i;

	mutex_lock(&d->usb_mutex);

	memcpy(&state->buf[0], sbuf, slen);
	state->buf[60] = state->seq++;

	dev_dbg(&d->udev->dev, "%s: >>> %*ph\n", __func__, slen, state->buf);

	/*                                                              
                                                                */
	ret = dvb_usbv2_generic_rw_locked(d, state->buf, sizeof(state->buf),
			state->buf, sizeof(state->buf));
	if (ret)
		goto error_unlock;

	/*                                                                  
                                                                   
                                                                
                                                                    
                                                                  
                                                  
                                                  
                             
                         
  */

	/*                                               */
	for (i = 0; i < 3; i++) {
		/*                    */
		ret = usb_bulk_msg(d->udev, usb_rcvbulkpipe(d->udev,
				d->props->generic_bulk_ctrl_endpoint),
				state->buf, sizeof(state->buf), &act_len, 2000);
		if (ret) {
			dev_dbg(&d->udev->dev,
					"%s: recv bulk message failed=%d\n",
					__func__, ret);
		} else {
			dev_dbg(&d->udev->dev, "%s: <<< %*ph\n", __func__,
					rlen, state->buf);

			if (state->buf[63] != 0x4f)
				dev_dbg(&d->udev->dev,
						"%s: cmd failed\n", __func__);
			break;
		}
	}

	if (ret) {
		/*                                 */
		dev_err(&d->udev->dev, "%s: recv bulk message failed=%d\n",
				KBUILD_MODNAME, ret);
		goto error_unlock;
	}

	/*                                                */
	if (rbuf && rlen)
		memcpy(rbuf, state->buf, rlen);

error_unlock:
	mutex_unlock(&d->usb_mutex);
	return ret;
}

static int anysee_read_reg(struct dvb_usb_device *d, u16 reg, u8 *val)
{
	u8 buf[] = {CMD_REG_READ, reg >> 8, reg & 0xff, 0x01};
	int ret;
	ret = anysee_ctrl_msg(d, buf, sizeof(buf), val, 1);
	dev_dbg(&d->udev->dev, "%s: reg=%04x val=%02x\n", __func__, reg, *val);
	return ret;
}

static int anysee_write_reg(struct dvb_usb_device *d, u16 reg, u8 val)
{
	u8 buf[] = {CMD_REG_WRITE, reg >> 8, reg & 0xff, 0x01, val};
	dev_dbg(&d->udev->dev, "%s: reg=%04x val=%02x\n", __func__, reg, val);
	return anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
}

/*                                 */
static int anysee_wr_reg_mask(struct dvb_usb_device *d, u16 reg, u8 val,
	u8 mask)
{
	int ret;
	u8 tmp;

	/*                                          */
	if (mask != 0xff) {
		ret = anysee_read_reg(d, reg, &tmp);
		if (ret)
			return ret;

		val &= mask;
		tmp &= ~mask;
		val |= tmp;
	}

	return anysee_write_reg(d, reg, val);
}

/*                                */
static int anysee_rd_reg_mask(struct dvb_usb_device *d, u16 reg, u8 *val,
	u8 mask)
{
	int ret, i;
	u8 tmp;

	ret = anysee_read_reg(d, reg, &tmp);
	if (ret)
		return ret;

	tmp &= mask;

	/*                                */
	for (i = 0; i < 8; i++) {
		if ((mask >> i) & 0x01)
			break;
	}
	*val = tmp >> i;

	return 0;
}

static int anysee_get_hw_info(struct dvb_usb_device *d, u8 *id)
{
	u8 buf[] = {CMD_GET_HW_INFO};
	return anysee_ctrl_msg(d, buf, sizeof(buf), id, 3);
}

static int anysee_streaming_ctrl(struct dvb_frontend *fe, int onoff)
{
	u8 buf[] = {CMD_STREAMING_CTRL, (u8)onoff, 0x00};
	dev_dbg(&fe_to_d(fe)->udev->dev, "%s: onoff=%d\n", __func__, onoff);
	return anysee_ctrl_msg(fe_to_d(fe), buf, sizeof(buf), NULL, 0);
}

static int anysee_led_ctrl(struct dvb_usb_device *d, u8 mode, u8 interval)
{
	u8 buf[] = {CMD_LED_AND_IR_CTRL, 0x01, mode, interval};
	dev_dbg(&d->udev->dev, "%s: state=%d interval=%d\n", __func__,
			mode, interval);
	return anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
}

static int anysee_ir_ctrl(struct dvb_usb_device *d, u8 onoff)
{
	u8 buf[] = {CMD_LED_AND_IR_CTRL, 0x02, onoff};
	dev_dbg(&d->udev->dev, "%s: onoff=%d\n", __func__, onoff);
	return anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
}

/*     */
static int anysee_master_xfer(struct i2c_adapter *adap, struct i2c_msg *msg,
	int num)
{
	struct dvb_usb_device *d = i2c_get_adapdata(adap);
	int ret = 0, inc, i = 0;
	u8 buf[52]; /*                                                 */

	if (mutex_lock_interruptible(&d->i2c_mutex) < 0)
		return -EAGAIN;

	while (i < num) {
		if (num > i + 1 && (msg[i+1].flags & I2C_M_RD)) {
			if (msg[i].len > 2 || msg[i+1].len > 60) {
				ret = -EOPNOTSUPP;
				break;
			}
			buf[0] = CMD_I2C_READ;
			buf[1] = (msg[i].addr << 1) | 0x01;
			buf[2] = msg[i].buf[0];
			buf[3] = msg[i].buf[1];
			buf[4] = msg[i].len-1;
			buf[5] = msg[i+1].len;
			ret = anysee_ctrl_msg(d, buf, 6, msg[i+1].buf,
				msg[i+1].len);
			inc = 2;
		} else {
			if (msg[i].len > 48) {
				ret = -EOPNOTSUPP;
				break;
			}
			buf[0] = CMD_I2C_WRITE;
			buf[1] = (msg[i].addr << 1);
			buf[2] = msg[i].len;
			buf[3] = 0x01;
			memcpy(&buf[4], msg[i].buf, msg[i].len);
			ret = anysee_ctrl_msg(d, buf, 4 + msg[i].len, NULL, 0);
			inc = 1;
		}
		if (ret)
			break;

		i += inc;
	}

	mutex_unlock(&d->i2c_mutex);

	return ret ? ret : i;
}

static u32 anysee_i2c_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C;
}

static struct i2c_algorithm anysee_i2c_algo = {
	.master_xfer   = anysee_master_xfer,
	.functionality = anysee_i2c_func,
};

static int anysee_mt352_demod_init(struct dvb_frontend *fe)
{
	static u8 clock_config[]   = { CLOCK_CTL,  0x38, 0x28 };
	static u8 reset[]          = { RESET,      0x80 };
	static u8 adc_ctl_1_cfg[]  = { ADC_CTL_1,  0x40 };
	static u8 agc_cfg[]        = { AGC_TARGET, 0x28, 0x20 };
	static u8 gpp_ctl_cfg[]    = { GPP_CTL,    0x33 };
	static u8 capt_range_cfg[] = { CAPT_RANGE, 0x32 };

	mt352_write(fe, clock_config,   sizeof(clock_config));
	udelay(200);
	mt352_write(fe, reset,          sizeof(reset));
	mt352_write(fe, adc_ctl_1_cfg,  sizeof(adc_ctl_1_cfg));

	mt352_write(fe, agc_cfg,        sizeof(agc_cfg));
	mt352_write(fe, gpp_ctl_cfg,    sizeof(gpp_ctl_cfg));
	mt352_write(fe, capt_range_cfg, sizeof(capt_range_cfg));

	return 0;
}

/*                       */
static struct tda10023_config anysee_tda10023_config = {
	.demod_address = (0x1a >> 1),
	.invert = 0,
	.xtal   = 16000000,
	.pll_m  = 11,
	.pll_p  = 3,
	.pll_n  = 1,
	.output_mode = TDA10023_OUTPUT_MODE_PARALLEL_C,
	.deltaf = 0xfeeb,
};

static struct mt352_config anysee_mt352_config = {
	.demod_address = (0x1e >> 1),
	.demod_init    = anysee_mt352_demod_init,
};

static struct zl10353_config anysee_zl10353_config = {
	.demod_address = (0x1e >> 1),
	.parallel_ts = 1,
};

static struct zl10353_config anysee_zl10353_tda18212_config2 = {
	.demod_address = (0x1e >> 1),
	.parallel_ts = 1,
	.disable_i2c_gate_ctrl = 1,
	.no_tuner = 1,
	.if2 = 41500,
};

static struct zl10353_config anysee_zl10353_tda18212_config = {
	.demod_address = (0x18 >> 1),
	.parallel_ts = 1,
	.disable_i2c_gate_ctrl = 1,
	.no_tuner = 1,
	.if2 = 41500,
};

static struct tda10023_config anysee_tda10023_tda18212_config = {
	.demod_address = (0x1a >> 1),
	.xtal   = 16000000,
	.pll_m  = 12,
	.pll_p  = 3,
	.pll_n  = 1,
	.output_mode = TDA10023_OUTPUT_MODE_PARALLEL_B,
	.deltaf = 0xba02,
};

static struct tda18212_config anysee_tda18212_config = {
	.i2c_address = (0xc0 >> 1),
	.if_dvbt_6 = 4150,
	.if_dvbt_7 = 4150,
	.if_dvbt_8 = 4150,
	.if_dvbc = 5000,
};

static struct tda18212_config anysee_tda18212_config2 = {
	.i2c_address = 0x60 /*             */,
	.if_dvbt_6 = 3550,
	.if_dvbt_7 = 3700,
	.if_dvbt_8 = 4150,
	.if_dvbt2_6 = 3250,
	.if_dvbt2_7 = 4000,
	.if_dvbt2_8 = 4000,
	.if_dvbc = 5000,
};

static struct cx24116_config anysee_cx24116_config = {
	.demod_address = (0xaa >> 1),
	.mpg_clk_pos_pol = 0x00,
	.i2c_wr_max = 48,
};

static struct stv0900_config anysee_stv0900_config = {
	.demod_address = (0xd0 >> 1),
	.demod_mode = 0,
	.xtal = 8000000,
	.clkmode = 3,
	.diseqc_mode = 2,
	.tun1_maddress = 0,
	.tun1_adc = 1, /*       */
	.path1_mode = 3,
};

static struct stv6110_config anysee_stv6110_config = {
	.i2c_address = (0xc0 >> 1),
	.mclk = 16000000,
	.clk_div = 1,
};

static struct isl6423_config anysee_isl6423_config = {
	.current_max = SEC_CURRENT_800m,
	.curlim  = SEC_CURRENT_LIM_OFF,
	.mod_extern = 1,
	.addr = (0x10 >> 1),
};

static struct cxd2820r_config anysee_cxd2820r_config = {
	.i2c_address = 0x6d, /*             */
	.ts_mode = 0x38,
};

/*
                                                           
                          
  
                                                     
         
                                          
  
                                                   
                          
                                            
                                     
                                     
  
                                                  
                      
                                                      
                                     
                                     
                           
                      
                                                                         
  
                                                            
                      
                                              
                                     
                                     
                            
  
                                                             
                      
                                                         
                                     
                                     
                           
  
                                                            
                      
                                             
                                     
                                     
                            
                         
  
                                                                
                      
                                                      
                                     
                                     
         
                            
                         
         
                           
                         
                                   
                                    
  
                                                                   
                      
                                                    
                                     
                                     
                      
                            
         
                            
                            
                      
         
                             
                           
                      
  
                                                                   
                      
                                                 
                                     
                                     
                      
                           
  
                                                                     
                       
                                                    
                                     
                                     
                      
                            
  
                                                                    
                       
                                                    
                                     
                                     
                      
                            
         
                            
                            
                      
         
                             
                           
                      
  
                                                                    
                       
                                                 
                                     
                                     
                      
                           
 */

static int anysee_read_config(struct dvb_usb_device *d)
{
	struct anysee_state *state = d_to_priv(d);
	int ret;
	u8 hw_info[3];

	/*
                                 
                                                                      
  */
	ret = anysee_get_hw_info(d, hw_info);
	if (ret)
		goto error;

	ret = anysee_get_hw_info(d, hw_info);
	if (ret)
		goto error;

	/*
                                            
  */
	dev_info(&d->udev->dev, "%s: firmware version %d.%d hardware id %d\n",
			KBUILD_MODNAME, hw_info[1], hw_info[2], hw_info[0]);

	state->hw = hw_info[0];
error:
	return ret;
}

/*                                                                 */
static int anysee_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	/*                                         */
	return anysee_wr_reg_mask(fe_to_d(fe), REG_IOE, (enable << 4), 0x10);
}

static int anysee_frontend_ctrl(struct dvb_frontend *fe, int onoff)
{
	struct anysee_state *state = fe_to_priv(fe);
	struct dvb_usb_device *d = fe_to_d(fe);
	int ret;
	dev_dbg(&d->udev->dev, "%s: fe=%d onoff=%d\n", __func__, fe->id, onoff);

	/*                           */
	if (onoff == 0)
		return 0;

	switch (state->hw) {
	case ANYSEE_HW_507FA: /*    */
		/*                */
		/*            */

		if (fe->id == 0)  {
			/*                               */
			ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 0), 0x01);
			if (ret)
				goto error;

			/*                              */
			ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 5), 0x20);
			if (ret)
				goto error;

			/*                              */
			ret = anysee_wr_reg_mask(d, REG_IOE, (1 << 0), 0x01);
			if (ret)
				goto error;
		} else {
			/*                               */
			ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 5), 0x20);
			if (ret)
				goto error;

			/*                              */
			ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 0), 0x01);
			if (ret)
				goto error;

			/*                              */
			ret = anysee_wr_reg_mask(d, REG_IOE, (0 << 0), 0x01);
			if (ret)
				goto error;
		}

		break;
	case ANYSEE_HW_508TC: /*    */
	case ANYSEE_HW_508PTC: /*    */
		/*       */
		/*        */

		if (fe->id == 0)  {
			/*                               */
			ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 6), 0x40);
			if (ret)
				goto error;

			/*                              */
			ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 5), 0x20);
			if (ret)
				goto error;

			/*                           */
			ret = anysee_wr_reg_mask(d, REG_IOE, (1 << 0), 0x01);
			if (ret)
				goto error;
		} else {
			/*                               */
			ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 5), 0x20);
			if (ret)
				goto error;

			/*                              */
			ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 6), 0x40);
			if (ret)
				goto error;

			/*                           */
			ret = anysee_wr_reg_mask(d, REG_IOE, (0 << 0), 0x01);
			if (ret)
				goto error;
		}

		break;
	default:
		ret = 0;
	}

error:
	return ret;
}

static int anysee_frontend_attach(struct dvb_usb_adapter *adap)
{
	struct anysee_state *state = adap_to_priv(adap);
	struct dvb_usb_device *d = adap_to_d(adap);
	int ret = 0;
	u8 tmp;
	struct i2c_msg msg[2] = {
		{
			.addr = anysee_tda18212_config.i2c_address,
			.flags = 0,
			.len = 1,
			.buf = "\x00",
		}, {
			.addr = anysee_tda18212_config.i2c_address,
			.flags = I2C_M_RD,
			.len = 1,
			.buf = &tmp,
		}
	};

	switch (state->hw) {
	case ANYSEE_HW_507T: /*   */
		/*     */

		/*              */
		adap->fe[0] = dvb_attach(mt352_attach, &anysee_mt352_config,
				&d->i2c_adap);
		if (adap->fe[0])
			break;

		/*              */
		adap->fe[0] = dvb_attach(zl10353_attach, &anysee_zl10353_config,
				&d->i2c_adap);

		break;
	case ANYSEE_HW_507CD: /*   */
		/*          */

		/*                              */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 0), 0x01);
		if (ret)
			goto error;

		/*                                   */
		ret = anysee_wr_reg_mask(d, REG_IOA, (0 << 7), 0x80);
		if (ret)
			goto error;

		/*              */
		adap->fe[0] = dvb_attach(zl10353_attach, &anysee_zl10353_config,
				&d->i2c_adap);

		break;
	case ANYSEE_HW_507DC: /*    */
		/*            */

		/*                              */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 0), 0x01);
		if (ret)
			goto error;

		/*              */
		adap->fe[0] = dvb_attach(tda10023_attach,
				&anysee_tda10023_config, &d->i2c_adap, 0x48);

		break;
	case ANYSEE_HW_507SI: /*    */
		/*             */

		/*                                 */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 0), 0x01);
		if (ret)
			goto error;

		/*              */
		adap->fe[0] = dvb_attach(cx24116_attach, &anysee_cx24116_config,
				&d->i2c_adap);

		break;
	case ANYSEE_HW_507FA: /*    */
		/*                */
		/*            */

		/*                        */
		ret = anysee_wr_reg_mask(d, REG_IOE, (1 << 4), 0x10);
		if (ret)
			goto error;

		/*                */
		tmp = 0;
		ret = i2c_transfer(&d->i2c_adap, msg, 2);
		if (ret == 2 && tmp == 0xc7)
			dev_dbg(&d->udev->dev, "%s: TDA18212 found\n",
					__func__);
		else
			tmp = 0;

		/*                         */
		ret = anysee_wr_reg_mask(d, REG_IOE, (0 << 4), 0x10);
		if (ret)
			goto error;

		/*                               */
		ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 0), 0x01);
		if (ret)
			goto error;

		/*                              */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 5), 0x20);
		if (ret)
			goto error;

		/*              */
		if (tmp == 0xc7) {
			/*                 */
			adap->fe[0] = dvb_attach(tda10023_attach,
					&anysee_tda10023_tda18212_config,
					&d->i2c_adap, 0x48);

			/*                                                   */
			if (adap->fe[0])
				adap->fe[0]->ops.i2c_gate_ctrl =
						anysee_i2c_gate_ctrl;
		} else {
			/*            */
			adap->fe[0] = dvb_attach(tda10023_attach,
					&anysee_tda10023_config,
					&d->i2c_adap, 0x48);
		}

		/*                                             */
		if (!adap->fe[0])
			break;

		/*                               */
		ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 5), 0x20);
		if (ret)
			goto error;

		/*                              */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 0), 0x01);
		if (ret)
			goto error;

		/*              */
		if (tmp == 0xc7) {
			/*                 */
			adap->fe[1] = dvb_attach(zl10353_attach,
					&anysee_zl10353_tda18212_config2,
					&d->i2c_adap);

			/*                                                   */
			if (adap->fe[1])
				adap->fe[1]->ops.i2c_gate_ctrl =
						anysee_i2c_gate_ctrl;
		} else {
			/*            */
			adap->fe[1] = dvb_attach(zl10353_attach,
					&anysee_zl10353_config,
					&d->i2c_adap);
		}

		break;
	case ANYSEE_HW_508TC: /*    */
	case ANYSEE_HW_508PTC: /*    */
		/*       */
		/*        */

		/*                               */
		ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 6), 0x40);
		if (ret)
			goto error;

		/*                              */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 5), 0x20);
		if (ret)
			goto error;

		/*              */
		adap->fe[0] = dvb_attach(tda10023_attach,
				&anysee_tda10023_tda18212_config,
				&d->i2c_adap, 0x48);

		/*                                                   */
		if (adap->fe[0])
			adap->fe[0]->ops.i2c_gate_ctrl = anysee_i2c_gate_ctrl;

		/*                                             */
		if (!adap->fe[0])
			break;

		/*                               */
		ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 5), 0x20);
		if (ret)
			goto error;

		/*                              */
		ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 6), 0x40);
		if (ret)
			goto error;

		/*              */
		adap->fe[1] = dvb_attach(zl10353_attach,
				&anysee_zl10353_tda18212_config,
				&d->i2c_adap);

		/*                                                   */
		if (adap->fe[1])
			adap->fe[1]->ops.i2c_gate_ctrl = anysee_i2c_gate_ctrl;

		state->has_ci = true;

		break;
	case ANYSEE_HW_508S2: /*    */
	case ANYSEE_HW_508PS2: /*    */
		/*       */
		/*        */

		/*                                 */
		ret = anysee_wr_reg_mask(d, REG_IOE, (1 << 5), 0x20);
		if (ret)
			goto error;

		/*              */
		adap->fe[0] = dvb_attach(stv0900_attach,
				&anysee_stv0900_config, &d->i2c_adap, 0);

		state->has_ci = true;

		break;
	case ANYSEE_HW_508T2C: /*    */
		/*        */

		/*                                   */
		ret = anysee_wr_reg_mask(d, REG_IOE, (1 << 5), 0x20);
		if (ret)
			goto error;

		/*              */
		adap->fe[0] = dvb_attach(cxd2820r_attach,
				&anysee_cxd2820r_config, &d->i2c_adap, NULL);

		state->has_ci = true;

		break;
	}

	if (!adap->fe[0]) {
		/*                         */
		ret = -ENODEV;
		dev_err(&d->udev->dev,
				"%s: Unsupported Anysee version. Please report to <linux-media@vger.kernel.org>.\n",
				KBUILD_MODNAME);
	}
error:
	return ret;
}

static int anysee_tuner_attach(struct dvb_usb_adapter *adap)
{
	struct anysee_state *state = adap_to_priv(adap);
	struct dvb_usb_device *d = adap_to_d(adap);
	struct dvb_frontend *fe;
	int ret;
	dev_dbg(&d->udev->dev, "%s:\n", __func__);

	switch (state->hw) {
	case ANYSEE_HW_507T: /*   */
		/*     */

		/*              */
		fe = dvb_attach(dvb_pll_attach, adap->fe[0], (0xc2 >> 1), NULL,
				DVB_PLL_THOMSON_DTT7579);

		break;
	case ANYSEE_HW_507CD: /*   */
		/*          */

		/*              */
		fe = dvb_attach(dvb_pll_attach, adap->fe[0], (0xc2 >> 1),
				&d->i2c_adap, DVB_PLL_THOMSON_DTT7579);

		break;
	case ANYSEE_HW_507DC: /*    */
		/*            */

		/*              */
		fe = dvb_attach(dvb_pll_attach, adap->fe[0], (0xc0 >> 1),
				&d->i2c_adap, DVB_PLL_SAMSUNG_DTOS403IH102A);

		break;
	case ANYSEE_HW_507SI: /*    */
		/*             */

		/*                       */
		fe = dvb_attach(isl6423_attach, adap->fe[0], &d->i2c_adap,
				&anysee_isl6423_config);

		break;
	case ANYSEE_HW_507FA: /*    */
		/*                */
		/*            */

		/*                                                           
                                  */

		/*              */
		fe = dvb_attach(tda18212_attach, adap->fe[0], &d->i2c_adap,
				&anysee_tda18212_config);

		if (fe && adap->fe[1]) {
			/*                         */
			fe = dvb_attach(tda18212_attach, adap->fe[1],
					&d->i2c_adap, &anysee_tda18212_config);
			break;
		} else if (fe) {
			break;
		}

		/*              */
		fe = dvb_attach(dvb_pll_attach, adap->fe[0], (0xc0 >> 1),
				&d->i2c_adap, DVB_PLL_SAMSUNG_DTOS403IH102A);

		if (fe && adap->fe[1]) {
			/*                         */
			fe = dvb_attach(dvb_pll_attach, adap->fe[1],
					(0xc0 >> 1), &d->i2c_adap,
					DVB_PLL_SAMSUNG_DTOS403IH102A);
		}

		break;
	case ANYSEE_HW_508TC: /*    */
	case ANYSEE_HW_508PTC: /*    */
		/*       */
		/*        */

		/*              */
		fe = dvb_attach(tda18212_attach, adap->fe[0], &d->i2c_adap,
				&anysee_tda18212_config);

		if (fe) {
			/*                         */
			fe = dvb_attach(tda18212_attach, adap->fe[1],
					&d->i2c_adap, &anysee_tda18212_config);
		}

		break;
	case ANYSEE_HW_508S2: /*    */
	case ANYSEE_HW_508PS2: /*    */
		/*       */
		/*        */

		/*              */
		fe = dvb_attach(stv6110_attach, adap->fe[0],
				&anysee_stv6110_config, &d->i2c_adap);

		if (fe) {
			/*                       */
			fe = dvb_attach(isl6423_attach, adap->fe[0],
					&d->i2c_adap, &anysee_isl6423_config);
		}

		break;

	case ANYSEE_HW_508T2C: /*    */
		/*        */

		/*              */
		fe = dvb_attach(tda18212_attach, adap->fe[0], &d->i2c_adap,
				&anysee_tda18212_config2);

		break;
	default:
		fe = NULL;
	}

	if (fe)
		ret = 0;
	else
		ret = -ENODEV;

	return ret;
}

#if IS_ENABLED(CONFIG_RC_CORE)
static int anysee_rc_query(struct dvb_usb_device *d)
{
	u8 buf[] = {CMD_GET_IR_CODE};
	u8 ircode[2];
	int ret;

	/*                                                        
                                                                   
                                                                  
                                                                
                                                    
                                                                  
                          */

	ret = anysee_ctrl_msg(d, buf, sizeof(buf), ircode, sizeof(ircode));
	if (ret)
		return ret;

	if (ircode[0]) {
		dev_dbg(&d->udev->dev, "%s: key pressed %02x\n", __func__,
				ircode[1]);
		rc_keydown(d->rc_dev, 0x08 << 8 | ircode[1], 0);
	}

	return 0;
}

static int anysee_get_rc_config(struct dvb_usb_device *d, struct dvb_usb_rc *rc)
{
	rc->allowed_protos = RC_BIT_NEC;
	rc->query          = anysee_rc_query;
	rc->interval       = 250;  /*                           */

	return 0;
}
#else
	#define anysee_get_rc_config NULL
#endif

static int anysee_ci_read_attribute_mem(struct dvb_ca_en50221 *ci, int slot,
	int addr)
{
	struct dvb_usb_device *d = ci->data;
	int ret;
	u8 buf[] = {CMD_CI, 0x02, 0x40 | addr >> 8, addr & 0xff, 0x00, 1};
	u8 val;

	ret = anysee_ctrl_msg(d, buf, sizeof(buf), &val, 1);
	if (ret)
		return ret;

	return val;
}

static int anysee_ci_write_attribute_mem(struct dvb_ca_en50221 *ci, int slot,
	int addr, u8 val)
{
	struct dvb_usb_device *d = ci->data;
	int ret;
	u8 buf[] = {CMD_CI, 0x03, 0x40 | addr >> 8, addr & 0xff, 0x00, 1, val};

	ret = anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
	if (ret)
		return ret;

	return 0;
}

static int anysee_ci_read_cam_control(struct dvb_ca_en50221 *ci, int slot,
	u8 addr)
{
	struct dvb_usb_device *d = ci->data;
	int ret;
	u8 buf[] = {CMD_CI, 0x04, 0x40, addr, 0x00, 1};
	u8 val;

	ret = anysee_ctrl_msg(d, buf, sizeof(buf), &val, 1);
	if (ret)
		return ret;

	return val;
}

static int anysee_ci_write_cam_control(struct dvb_ca_en50221 *ci, int slot,
	u8 addr, u8 val)
{
	struct dvb_usb_device *d = ci->data;
	int ret;
	u8 buf[] = {CMD_CI, 0x05, 0x40, addr, 0x00, 1, val};

	ret = anysee_ctrl_msg(d, buf, sizeof(buf), NULL, 0);
	if (ret)
		return ret;

	return 0;
}

static int anysee_ci_slot_reset(struct dvb_ca_en50221 *ci, int slot)
{
	struct dvb_usb_device *d = ci->data;
	int ret;
	struct anysee_state *state = d_to_priv(d);

	state->ci_cam_ready = jiffies + msecs_to_jiffies(1000);

	ret = anysee_wr_reg_mask(d, REG_IOA, (0 << 7), 0x80);
	if (ret)
		return ret;

	msleep(300);

	ret = anysee_wr_reg_mask(d, REG_IOA, (1 << 7), 0x80);
	if (ret)
		return ret;

	return 0;
}

static int anysee_ci_slot_shutdown(struct dvb_ca_en50221 *ci, int slot)
{
	struct dvb_usb_device *d = ci->data;
	int ret;

	ret = anysee_wr_reg_mask(d, REG_IOA, (0 << 7), 0x80);
	if (ret)
		return ret;

	msleep(30);

	ret = anysee_wr_reg_mask(d, REG_IOA, (1 << 7), 0x80);
	if (ret)
		return ret;

	return 0;
}

static int anysee_ci_slot_ts_enable(struct dvb_ca_en50221 *ci, int slot)
{
	struct dvb_usb_device *d = ci->data;
	int ret;

	ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 1), 0x02);
	if (ret)
		return ret;

	return 0;
}

static int anysee_ci_poll_slot_status(struct dvb_ca_en50221 *ci, int slot,
	int open)
{
	struct dvb_usb_device *d = ci->data;
	struct anysee_state *state = d_to_priv(d);
	int ret;
	u8 tmp = 0;

	ret = anysee_rd_reg_mask(d, REG_IOC, &tmp, 0x40);
	if (ret)
		return ret;

	if (tmp == 0) {
		ret = DVB_CA_EN50221_POLL_CAM_PRESENT;
		if (time_after(jiffies, state->ci_cam_ready))
			ret |= DVB_CA_EN50221_POLL_CAM_READY;
	}

	return ret;
}

static int anysee_ci_init(struct dvb_usb_device *d)
{
	struct anysee_state *state = d_to_priv(d);
	int ret;

	state->ci.owner               = THIS_MODULE;
	state->ci.read_attribute_mem  = anysee_ci_read_attribute_mem;
	state->ci.write_attribute_mem = anysee_ci_write_attribute_mem;
	state->ci.read_cam_control    = anysee_ci_read_cam_control;
	state->ci.write_cam_control   = anysee_ci_write_cam_control;
	state->ci.slot_reset          = anysee_ci_slot_reset;
	state->ci.slot_shutdown       = anysee_ci_slot_shutdown;
	state->ci.slot_ts_enable      = anysee_ci_slot_ts_enable;
	state->ci.poll_slot_status    = anysee_ci_poll_slot_status;
	state->ci.data                = d;

	ret = anysee_wr_reg_mask(d, REG_IOA, (1 << 7), 0x80);
	if (ret)
		return ret;

	ret = anysee_wr_reg_mask(d, REG_IOD, (0 << 2)|(0 << 1)|(0 << 0), 0x07);
	if (ret)
		return ret;

	ret = anysee_wr_reg_mask(d, REG_IOD, (1 << 2)|(1 << 1)|(1 << 0), 0x07);
	if (ret)
		return ret;

	ret = dvb_ca_en50221_init(&d->adapter[0].dvb_adap, &state->ci, 0, 1);
	if (ret)
		return ret;

	state->ci_attached = true;

	return 0;
}

static void anysee_ci_release(struct dvb_usb_device *d)
{
	struct anysee_state *state = d_to_priv(d);

	/*           */
	if (state->ci_attached)
		dvb_ca_en50221_release(&state->ci);

	return;
}

static int anysee_init(struct dvb_usb_device *d)
{
	struct anysee_state *state = d_to_priv(d);
	int ret;

	/*                                                    
                                             
                                                    
                                                */
	ret = usb_set_interface(d->udev, 0, 0);
	if (ret)
		return ret;

	/*           */
	ret = anysee_led_ctrl(d, 0x01, 0x03);
	if (ret)
		return ret;

	/*           */
	ret = anysee_ir_ctrl(d, 1);
	if (ret)
		return ret;

	/*           */
	if (state->has_ci) {
		ret = anysee_ci_init(d);
		if (ret)
			return ret;
	}

	return 0;
}

static void anysee_exit(struct dvb_usb_device *d)
{
	return anysee_ci_release(d);
}

/*                      */
static struct dvb_usb_device_properties anysee_props = {
	.driver_name = KBUILD_MODNAME,
	.owner = THIS_MODULE,
	.adapter_nr = adapter_nr,
	.size_of_priv = sizeof(struct anysee_state),

	.generic_bulk_ctrl_endpoint = 0x01,
	.generic_bulk_ctrl_endpoint_response = 0x81,

	.i2c_algo         = &anysee_i2c_algo,
	.read_config      = anysee_read_config,
	.frontend_attach  = anysee_frontend_attach,
	.tuner_attach     = anysee_tuner_attach,
	.init             = anysee_init,
	.get_rc_config    = anysee_get_rc_config,
	.frontend_ctrl    = anysee_frontend_ctrl,
	.streaming_ctrl   = anysee_streaming_ctrl,
	.exit             = anysee_exit,

	.num_adapters = 1,
	.adapter = {
		{
			.stream = DVB_USB_STREAM_BULK(0x82, 8, 16 * 512),
		}
	}
};

static const struct usb_device_id anysee_id_table[] = {
	{ DVB_USB_DEVICE(USB_VID_CYPRESS, USB_PID_ANYSEE,
		&anysee_props, "Anysee", RC_MAP_ANYSEE) },
	{ DVB_USB_DEVICE(USB_VID_AMT, USB_PID_ANYSEE,
		&anysee_props, "Anysee", RC_MAP_ANYSEE) },
	{ }
};
MODULE_DEVICE_TABLE(usb, anysee_id_table);

static struct usb_driver anysee_usb_driver = {
	.name = KBUILD_MODNAME,
	.id_table = anysee_id_table,
	.probe = dvb_usbv2_probe,
	.disconnect = dvb_usbv2_disconnect,
	.suspend = dvb_usbv2_suspend,
	.resume = dvb_usbv2_resume,
	.reset_resume = dvb_usbv2_reset_resume,
	.no_dynamic_id = 1,
	.soft_unbind = 1,
};

module_usb_driver(anysee_usb_driver);

MODULE_AUTHOR("Antti Palosaari <crope@iki.fi>");
MODULE_DESCRIPTION("Driver Anysee E30 DVB-C & DVB-T USB2.0");
MODULE_LICENSE("GPL");
