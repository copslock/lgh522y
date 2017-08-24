/*
 * Afatech AF9035 DVB USB driver
 *
 * Copyright (C) 2009 Antti Palosaari <crope@iki.fi>
 * Copyright (C) 2012 Antti Palosaari <crope@iki.fi>
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
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "af9035.h"

/*                                                  */
#define MAX_XFER_SIZE  64

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

static u16 af9035_checksum(const u8 *buf, size_t len)
{
	size_t i;
	u16 checksum = 0;

	for (i = 1; i < len; i++) {
		if (i % 2)
			checksum += buf[i] << 8;
		else
			checksum += buf[i];
	}
	checksum = ~checksum;

	return checksum;
}

static int af9035_ctrl_msg(struct dvb_usb_device *d, struct usb_req *req)
{
#define REQ_HDR_LEN 4 /*                  */
#define ACK_HDR_LEN 3 /*                  */
#define CHECKSUM_LEN 2
#define USB_TIMEOUT 2000
	struct state *state = d_to_priv(d);
	int ret, wlen, rlen;
	u16 checksum, tmp_checksum;

	mutex_lock(&d->usb_mutex);

	/*                       */
	if (req->wlen > (BUF_LEN - REQ_HDR_LEN - CHECKSUM_LEN) ||
			req->rlen > (BUF_LEN - ACK_HDR_LEN - CHECKSUM_LEN)) {
		dev_err(&d->udev->dev, "%s: too much data wlen=%d rlen=%d\n",
				__func__, req->wlen, req->rlen);
		ret = -EINVAL;
		goto exit;
	}

	state->buf[0] = REQ_HDR_LEN + req->wlen + CHECKSUM_LEN - 1;
	state->buf[1] = req->mbox;
	state->buf[2] = req->cmd;
	state->buf[3] = state->seq++;
	memcpy(&state->buf[REQ_HDR_LEN], req->wbuf, req->wlen);

	wlen = REQ_HDR_LEN + req->wlen + CHECKSUM_LEN;
	rlen = ACK_HDR_LEN + req->rlen + CHECKSUM_LEN;

	/*                       */
	checksum = af9035_checksum(state->buf, state->buf[0] - 1);
	state->buf[state->buf[0] - 1] = (checksum >> 8);
	state->buf[state->buf[0] - 0] = (checksum & 0xff);

	/*                          */
	if (req->cmd == CMD_FW_DL)
		rlen = 0;

	ret = dvb_usbv2_generic_rw_locked(d,
			state->buf, wlen, state->buf, rlen);
	if (ret)
		goto exit;

	/*                          */
	if (req->cmd == CMD_FW_DL)
		goto exit;

	/*                 */
	checksum = af9035_checksum(state->buf, rlen - 2);
	tmp_checksum = (state->buf[rlen - 2] << 8) | state->buf[rlen - 1];
	if (tmp_checksum != checksum) {
		dev_err(&d->udev->dev, "%s: command=%02x checksum mismatch " \
				"(%04x != %04x)\n", KBUILD_MODNAME, req->cmd,
				tmp_checksum, checksum);
		ret = -EIO;
		goto exit;
	}

	/*              */
	if (state->buf[2]) {
		/*                                                   */
		if (req->cmd == CMD_IR_GET || state->buf[2] == 1) {
			ret = 1;
			goto exit;
		}

		dev_dbg(&d->udev->dev, "%s: command=%02x failed fw error=%d\n",
				__func__, req->cmd, state->buf[2]);
		ret = -EIO;
		goto exit;
	}

	/*                                                */
	if (req->rlen)
		memcpy(req->rbuf, &state->buf[ACK_HDR_LEN], req->rlen);
exit:
	mutex_unlock(&d->usb_mutex);
	if (ret < 0)
		dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);
	return ret;
}

/*                          */
static int af9035_wr_regs(struct dvb_usb_device *d, u32 reg, u8 *val, int len)
{
	u8 wbuf[MAX_XFER_SIZE];
	u8 mbox = (reg >> 16) & 0xff;
	struct usb_req req = { CMD_MEM_WR, mbox, 6 + len, wbuf, 0, NULL };

	if (6 + len > sizeof(wbuf)) {
		dev_warn(&d->udev->dev, "%s: i2c wr: len=%d is too big!\n",
			 KBUILD_MODNAME, len);
		return -EOPNOTSUPP;
	}

	wbuf[0] = len;
	wbuf[1] = 2;
	wbuf[2] = 0;
	wbuf[3] = 0;
	wbuf[4] = (reg >> 8) & 0xff;
	wbuf[5] = (reg >> 0) & 0xff;
	memcpy(&wbuf[6], val, len);

	return af9035_ctrl_msg(d, &req);
}

/*                         */
static int af9035_rd_regs(struct dvb_usb_device *d, u32 reg, u8 *val, int len)
{
	u8 wbuf[] = { len, 2, 0, 0, (reg >> 8) & 0xff, reg & 0xff };
	u8 mbox = (reg >> 16) & 0xff;
	struct usb_req req = { CMD_MEM_RD, mbox, sizeof(wbuf), wbuf, len, val };

	return af9035_ctrl_msg(d, &req);
}

/*                       */
static int af9035_wr_reg(struct dvb_usb_device *d, u32 reg, u8 val)
{
	return af9035_wr_regs(d, reg, &val, 1);
}

/*                      */
static int af9035_rd_reg(struct dvb_usb_device *d, u32 reg, u8 *val)
{
	return af9035_rd_regs(d, reg, val, 1);
}

/*                                 */
static int af9035_wr_reg_mask(struct dvb_usb_device *d, u32 reg, u8 val,
		u8 mask)
{
	int ret;
	u8 tmp;

	/*                                          */
	if (mask != 0xff) {
		ret = af9035_rd_regs(d, reg, &tmp, 1);
		if (ret)
			return ret;

		val &= mask;
		tmp &= ~mask;
		val |= tmp;
	}

	return af9035_wr_regs(d, reg, &val, 1);
}

static int af9035_i2c_master_xfer(struct i2c_adapter *adap,
		struct i2c_msg msg[], int num)
{
	struct dvb_usb_device *d = i2c_get_adapdata(adap);
	struct state *state = d_to_priv(d);
	int ret;

	if (mutex_lock_interruptible(&d->i2c_mutex) < 0)
		return -EAGAIN;

	/*
                                                               
               
                    
                   
                                           
                   
                                         
                   
                                              
   
                                                        
                                                                        
                                                                     
                                                           
  */
	if (num == 2 && !(msg[0].flags & I2C_M_RD) &&
			(msg[1].flags & I2C_M_RD)) {
		if (msg[0].len > 40 || msg[1].len > 40) {
			/*                           */
			ret = -EOPNOTSUPP;
		} else if ((msg[0].addr == state->af9033_config[0].i2c_addr) ||
			   (msg[0].addr == state->af9033_config[1].i2c_addr)) {
			/*                                     */
			u32 reg = msg[0].buf[0] << 16 | msg[0].buf[1] << 8 |
					msg[0].buf[2];

			if (msg[0].addr == state->af9033_config[1].i2c_addr)
				reg |= 0x100000;

			ret = af9035_rd_regs(d, reg, &msg[1].buf[0],
					msg[1].len);
		} else {
			/*     */
			u8 buf[MAX_XFER_SIZE];
			struct usb_req req = { CMD_I2C_RD, 0, 5 + msg[0].len,
					buf, msg[1].len, msg[1].buf };

			if (5 + msg[0].len > sizeof(buf)) {
				dev_warn(&d->udev->dev,
					 "%s: i2c xfer: len=%d is too big!\n",
					 KBUILD_MODNAME, msg[0].len);
				ret = -EOPNOTSUPP;
				goto unlock;
			}
			req.mbox |= ((msg[0].addr & 0x80)  >>  3);
			buf[0] = msg[1].len;
			buf[1] = msg[0].addr << 1;
			buf[2] = 0x00; /*              */
			buf[3] = 0x00; /*              */
			buf[4] = 0x00; /*              */
			memcpy(&buf[5], msg[0].buf, msg[0].len);
			ret = af9035_ctrl_msg(d, &req);
		}
	} else if (num == 1 && !(msg[0].flags & I2C_M_RD)) {
		if (msg[0].len > 40) {
			/*                           */
			ret = -EOPNOTSUPP;
		} else if ((msg[0].addr == state->af9033_config[0].i2c_addr) ||
			   (msg[0].addr == state->af9033_config[1].i2c_addr)) {
			/*                                     */
			u32 reg = msg[0].buf[0] << 16 | msg[0].buf[1] << 8 |
					msg[0].buf[2];

			if (msg[0].addr == state->af9033_config[1].i2c_addr)
				reg |= 0x100000;

			ret = af9035_wr_regs(d, reg, &msg[0].buf[3],
					msg[0].len - 3);
		} else {
			/*     */
			u8 buf[MAX_XFER_SIZE];
			struct usb_req req = { CMD_I2C_WR, 0, 5 + msg[0].len,
					buf, 0, NULL };

			if (5 + msg[0].len > sizeof(buf)) {
				dev_warn(&d->udev->dev,
					 "%s: i2c xfer: len=%d is too big!\n",
					 KBUILD_MODNAME, msg[0].len);
				ret = -EOPNOTSUPP;
				goto unlock;
			}
			req.mbox |= ((msg[0].addr & 0x80)  >>  3);
			buf[0] = msg[0].len;
			buf[1] = msg[0].addr << 1;
			buf[2] = 0x00; /*              */
			buf[3] = 0x00; /*              */
			buf[4] = 0x00; /*              */
			memcpy(&buf[5], msg[0].buf, msg[0].len);
			ret = af9035_ctrl_msg(d, &req);
		}
	} else {
		/*
                                                  
                            
                 
   */
		ret = -EOPNOTSUPP;
	}

unlock:
	mutex_unlock(&d->i2c_mutex);

	if (ret < 0)
		return ret;
	else
		return num;
}

static u32 af9035_i2c_functionality(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C;
}

static struct i2c_algorithm af9035_i2c_algo = {
	.master_xfer = af9035_i2c_master_xfer,
	.functionality = af9035_i2c_functionality,
};

static int af9035_identify_state(struct dvb_usb_device *d, const char **name)
{
	struct state *state = d_to_priv(d);
	int ret;
	u8 wbuf[1] = { 1 };
	u8 rbuf[4];
	struct usb_req req = { CMD_FW_QUERYINFO, 0, sizeof(wbuf), wbuf,
			sizeof(rbuf), rbuf };

	ret = af9035_rd_regs(d, 0x1222, rbuf, 3);
	if (ret < 0)
		goto err;

	state->chip_version = rbuf[0];
	state->chip_type = rbuf[2] << 8 | rbuf[1] << 0;

	ret = af9035_rd_reg(d, 0x384f, &state->prechip_version);
	if (ret < 0)
		goto err;

	dev_info(&d->udev->dev,
			"%s: prechip_version=%02x chip_version=%02x chip_type=%04x\n",
			__func__, state->prechip_version, state->chip_version,
			state->chip_type);

	if (state->chip_type == 0x9135) {
		if (state->chip_version == 0x02)
			*name = AF9035_FIRMWARE_IT9135_V2;
		else
			*name = AF9035_FIRMWARE_IT9135_V1;
		state->eeprom_addr = EEPROM_BASE_IT9135;
	} else {
		*name = AF9035_FIRMWARE_AF9035;
		state->eeprom_addr = EEPROM_BASE_AF9035;
	}

	ret = af9035_ctrl_msg(d, &req);
	if (ret < 0)
		goto err;

	dev_dbg(&d->udev->dev, "%s: reply=%*ph\n", __func__, 4, rbuf);
	if (rbuf[0] || rbuf[1] || rbuf[2] || rbuf[3])
		ret = WARM;
	else
		ret = COLD;

	return ret;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_download_firmware_old(struct dvb_usb_device *d,
		const struct firmware *fw)
{
	int ret, i, j, len;
	u8 wbuf[1];
	struct usb_req req = { 0, 0, 0, NULL, 0, NULL };
	struct usb_req req_fw_dl = { CMD_FW_DL, 0, 0, wbuf, 0, NULL };
	u8 hdr_core;
	u16 hdr_addr, hdr_data_len, hdr_checksum;
	#define MAX_DATA 58
	#define HDR_SIZE 7

	/*
                                                                   
   
                       
                                                                      
                   
                                            
                                                                  
                                                                        
                                                          
  */

	for (i = fw->size; i > HDR_SIZE;) {
		hdr_core = fw->data[fw->size - i + 0];
		hdr_addr = fw->data[fw->size - i + 1] << 8;
		hdr_addr |= fw->data[fw->size - i + 2] << 0;
		hdr_data_len = fw->data[fw->size - i + 3] << 8;
		hdr_data_len |= fw->data[fw->size - i + 4] << 0;
		hdr_checksum = fw->data[fw->size - i + 5] << 8;
		hdr_checksum |= fw->data[fw->size - i + 6] << 0;

		dev_dbg(&d->udev->dev, "%s: core=%d addr=%04x data_len=%d " \
				"checksum=%04x\n", __func__, hdr_core, hdr_addr,
				hdr_data_len, hdr_checksum);

		if (((hdr_core != 1) && (hdr_core != 2)) ||
				(hdr_data_len > i)) {
			dev_dbg(&d->udev->dev, "%s: bad firmware\n", __func__);
			break;
		}

		/*                       */
		req.cmd = CMD_FW_DL_BEGIN;
		ret = af9035_ctrl_msg(d, &req);
		if (ret < 0)
			goto err;

		/*                             */
		for (j = HDR_SIZE + hdr_data_len; j > 0; j -= MAX_DATA) {
			len = j;
			if (len > MAX_DATA)
				len = MAX_DATA;
			req_fw_dl.wlen = len;
			req_fw_dl.wbuf = (u8 *) &fw->data[fw->size - i +
					HDR_SIZE + hdr_data_len - j];
			ret = af9035_ctrl_msg(d, &req_fw_dl);
			if (ret < 0)
				goto err;
		}

		/*                     */
		req.cmd = CMD_FW_DL_END;
		ret = af9035_ctrl_msg(d, &req);
		if (ret < 0)
			goto err;

		i -= hdr_data_len + HDR_SIZE;

		dev_dbg(&d->udev->dev, "%s: data uploaded=%zu\n",
				__func__, fw->size - i);
	}

	/*                                                              */
	if (i)
		dev_warn(&d->udev->dev, "%s: bad firmware\n", KBUILD_MODNAME);

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_download_firmware_new(struct dvb_usb_device *d,
		const struct firmware *fw)
{
	int ret, i, i_prev;
	struct usb_req req_fw_dl = { CMD_FW_SCATTER_WR, 0, 0, NULL, 0, NULL };
	#define HDR_SIZE 7

	/*
                                                                     
               
   
        
           
        
              
               
               
                            
  */
	for (i = HDR_SIZE, i_prev = 0; i <= fw->size; i++) {
		if (i == fw->size ||
				(fw->data[i + 0] == 0x03 &&
				(fw->data[i + 1] == 0x00 ||
				fw->data[i + 1] == 0x01) &&
				fw->data[i + 2] == 0x00)) {
			req_fw_dl.wlen = i - i_prev;
			req_fw_dl.wbuf = (u8 *) &fw->data[i_prev];
			i_prev = i;
			ret = af9035_ctrl_msg(d, &req_fw_dl);
			if (ret < 0)
				goto err;

			dev_dbg(&d->udev->dev, "%s: data uploaded=%d\n",
					__func__, i);
		}
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_download_firmware(struct dvb_usb_device *d,
		const struct firmware *fw)
{
	struct state *state = d_to_priv(d);
	int ret;
	u8 wbuf[1];
	u8 rbuf[4];
	u8 tmp;
	struct usb_req req = { 0, 0, 0, NULL, 0, NULL };
	struct usb_req req_fw_ver = { CMD_FW_QUERYINFO, 0, 1, wbuf, 4, rbuf } ;
	dev_dbg(&d->udev->dev, "%s:\n", __func__);

	/*
                                                                
                                                                    
                                  
                                                        
  */
	ret = af9035_rd_reg(d, state->eeprom_addr + EEPROM_DUAL_MODE, &tmp);
	if (ret < 0)
		goto err;

	if (tmp) {
		/*                                             */
		ret = af9035_wr_reg_mask(d, 0x00d8b0, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0x00d8b1, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0x00d8af, 0x00, 0x01);
		if (ret < 0)
			goto err;

		usleep_range(10000, 50000);

		ret = af9035_wr_reg_mask(d, 0x00d8af, 0x01, 0x01);
		if (ret < 0)
			goto err;

		/*                            */
		ret = af9035_rd_reg(d,
				state->eeprom_addr + EEPROM_2ND_DEMOD_ADDR,
				&tmp);
		if (ret < 0)
			goto err;

		if (state->chip_type == 0x9135) {
			ret = af9035_wr_reg(d, 0x004bfb, tmp);
			if (ret < 0)
				goto err;
		} else {
			ret = af9035_wr_reg(d, 0x00417f, tmp);
			if (ret < 0)
				goto err;

			/*                  */
			ret = af9035_wr_reg_mask(d, 0x00d81a, 0x01, 0x01);
			if (ret < 0)
				goto err;
		}
	}

	if (fw->data[0] == 0x01)
		ret = af9035_download_firmware_old(d, fw);
	else
		ret = af9035_download_firmware_new(d, fw);
	if (ret < 0)
		goto err;

	/*                               */
	req.cmd = CMD_FW_BOOT;
	ret = af9035_ctrl_msg(d, &req);
	if (ret < 0)
		goto err;

	/*                        */
	wbuf[0] = 1;
	ret = af9035_ctrl_msg(d, &req_fw_ver);
	if (ret < 0)
		goto err;

	if (!(rbuf[0] || rbuf[1] || rbuf[2] || rbuf[3])) {
		dev_err(&d->udev->dev, "%s: firmware did not run\n",
				KBUILD_MODNAME);
		ret = -ENODEV;
		goto err;
	}

	dev_info(&d->udev->dev, "%s: firmware version=%d.%d.%d.%d",
			KBUILD_MODNAME, rbuf[0], rbuf[1], rbuf[2], rbuf[3]);

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_read_config(struct dvb_usb_device *d)
{
	struct state *state = d_to_priv(d);
	int ret, i;
	u8 tmp;
	u16 tmp16, addr;

	/*                     */
	state->af9033_config[0].i2c_addr = 0x38;
	state->af9033_config[0].adc_multiplier = AF9033_ADC_MULTIPLIER_2X;
	state->af9033_config[1].adc_multiplier = AF9033_ADC_MULTIPLIER_2X;
	state->af9033_config[0].ts_mode = AF9033_TS_MODE_USB;
	state->af9033_config[1].ts_mode = AF9033_TS_MODE_SERIAL;

	/*                               */
	if (state->chip_type == 0x9135) {
		if (state->chip_version == 0x02) {
			state->af9033_config[0].tuner = AF9033_TUNER_IT9135_60;
			state->af9033_config[1].tuner = AF9033_TUNER_IT9135_60;
			tmp16 = 0x00461d;
		} else {
			state->af9033_config[0].tuner = AF9033_TUNER_IT9135_38;
			state->af9033_config[1].tuner = AF9033_TUNER_IT9135_38;
			tmp16 = 0x00461b;
		}

		/*                        */
		ret = af9035_rd_reg(d, tmp16, &tmp);
		if (ret < 0)
			goto err;

		if (tmp == 0x00) {
			dev_dbg(&d->udev->dev, "%s: no eeprom\n", __func__);
			goto skip_eeprom;
		}
	}

	/*                               */
	ret = af9035_rd_reg(d, state->eeprom_addr + EEPROM_DUAL_MODE, &tmp);
	if (ret < 0)
		goto err;

	state->dual_mode = tmp;
	dev_dbg(&d->udev->dev, "%s: dual mode=%d\n", __func__,
			state->dual_mode);

	if (state->dual_mode) {
		/*                                  */
		ret = af9035_rd_reg(d,
				state->eeprom_addr + EEPROM_2ND_DEMOD_ADDR,
				&tmp);
		if (ret < 0)
			goto err;

		state->af9033_config[1].i2c_addr = tmp;
		dev_dbg(&d->udev->dev, "%s: 2nd demod I2C addr=%02x\n",
				__func__, tmp);
	}

	addr = state->eeprom_addr;

	for (i = 0; i < state->dual_mode + 1; i++) {
		/*       */
		ret = af9035_rd_reg(d, addr + EEPROM_1_TUNER_ID, &tmp);
		if (ret < 0)
			goto err;

		if (tmp == 0x00)
			dev_dbg(&d->udev->dev,
					"%s: [%d]tuner not set, using default\n",
					__func__, i);
		else
			state->af9033_config[i].tuner = tmp;

		dev_dbg(&d->udev->dev, "%s: [%d]tuner=%02x\n",
				__func__, i, state->af9033_config[i].tuner);

		switch (state->af9033_config[i].tuner) {
		case AF9033_TUNER_TUA9001:
		case AF9033_TUNER_FC0011:
		case AF9033_TUNER_MXL5007T:
		case AF9033_TUNER_TDA18218:
		case AF9033_TUNER_FC2580:
		case AF9033_TUNER_FC0012:
			state->af9033_config[i].spec_inv = 1;
			break;
		case AF9033_TUNER_IT9135_38:
		case AF9033_TUNER_IT9135_51:
		case AF9033_TUNER_IT9135_52:
		case AF9033_TUNER_IT9135_60:
		case AF9033_TUNER_IT9135_61:
		case AF9033_TUNER_IT9135_62:
			break;
		default:
			dev_warn(&d->udev->dev,
					"%s: tuner id=%02x not supported, please report!",
					KBUILD_MODNAME, tmp);
		}

		/*                                                 */
		if (i == 1)
			switch (state->af9033_config[i].tuner) {
			case AF9033_TUNER_FC0012:
			case AF9033_TUNER_IT9135_38:
			case AF9033_TUNER_IT9135_51:
			case AF9033_TUNER_IT9135_52:
			case AF9033_TUNER_IT9135_60:
			case AF9033_TUNER_IT9135_61:
			case AF9033_TUNER_IT9135_62:
			case AF9033_TUNER_MXL5007T:
				break;
			default:
				state->dual_mode = false;
				dev_info(&d->udev->dev,
						"%s: driver does not support 2nd tuner and will disable it",
						KBUILD_MODNAME);
		}

		/*                    */
		ret = af9035_rd_reg(d, addr + EEPROM_1_IF_L, &tmp);
		if (ret < 0)
			goto err;

		tmp16 = tmp;

		ret = af9035_rd_reg(d, addr + EEPROM_1_IF_H, &tmp);
		if (ret < 0)
			goto err;

		tmp16 |= tmp << 8;

		dev_dbg(&d->udev->dev, "%s: [%d]IF=%d\n", __func__, i, tmp16);

		addr += 0x10; /*                                */
	}

skip_eeprom:
	/*                 */
	ret = af9035_rd_reg(d, 0x00d800, &tmp);
	if (ret < 0)
		goto err;

	tmp = (tmp >> 0) & 0x0f;

	for (i = 0; i < ARRAY_SIZE(state->af9033_config); i++) {
		if (state->chip_type == 0x9135)
			state->af9033_config[i].clock = clock_lut_it9135[tmp];
		else
			state->af9033_config[i].clock = clock_lut_af9035[tmp];
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_tua9001_tuner_callback(struct dvb_usb_device *d,
		int cmd, int arg)
{
	int ret;
	u8 val;

	dev_dbg(&d->udev->dev, "%s: cmd=%d arg=%d\n", __func__, cmd, arg);

	/*
                                             
                  
                  
  */

	switch (cmd) {
	case TUA9001_CMD_RESETN:
		if (arg)
			val = 0x00;
		else
			val = 0x01;

		ret = af9035_wr_reg_mask(d, 0x00d8e7, val, 0x01);
		if (ret < 0)
			goto err;
		break;
	case TUA9001_CMD_RXEN:
		if (arg)
			val = 0x01;
		else
			val = 0x00;

		ret = af9035_wr_reg_mask(d, 0x00d8eb, val, 0x01);
		if (ret < 0)
			goto err;
		break;
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}


static int af9035_fc0011_tuner_callback(struct dvb_usb_device *d,
		int cmd, int arg)
{
	int ret;

	switch (cmd) {
	case FC0011_FE_CALLBACK_POWER:
		/*              */
		ret = af9035_wr_reg_mask(d, 0xd8eb, 1, 1);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0xd8ec, 1, 1);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0xd8ed, 1, 1);
		if (ret < 0)
			goto err;

		/*     */
		ret = af9035_wr_reg_mask(d, 0xd8d0, 1, 1);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0xd8d1, 1, 1);
		if (ret < 0)
			goto err;

		usleep_range(10000, 50000);
		break;
	case FC0011_FE_CALLBACK_RESET:
		ret = af9035_wr_reg(d, 0xd8e9, 1);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg(d, 0xd8e8, 1);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg(d, 0xd8e7, 1);
		if (ret < 0)
			goto err;

		usleep_range(10000, 20000);

		ret = af9035_wr_reg(d, 0xd8e7, 0);
		if (ret < 0)
			goto err;

		usleep_range(10000, 20000);
		break;
	default:
		ret = -EINVAL;
		goto err;
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_tuner_callback(struct dvb_usb_device *d, int cmd, int arg)
{
	struct state *state = d_to_priv(d);

	switch (state->af9033_config[0].tuner) {
	case AF9033_TUNER_FC0011:
		return af9035_fc0011_tuner_callback(d, cmd, arg);
	case AF9033_TUNER_TUA9001:
		return af9035_tua9001_tuner_callback(d, cmd, arg);
	default:
		break;
	}

	return 0;
}

static int af9035_frontend_callback(void *adapter_priv, int component,
				    int cmd, int arg)
{
	struct i2c_adapter *adap = adapter_priv;
	struct dvb_usb_device *d = i2c_get_adapdata(adap);

	dev_dbg(&d->udev->dev, "%s: component=%d cmd=%d arg=%d\n",
			__func__, component, cmd, arg);

	switch (component) {
	case DVB_FRONTEND_COMPONENT_TUNER:
		return af9035_tuner_callback(d, cmd, arg);
	default:
		break;
	}

	return 0;
}

static int af9035_get_adapter_count(struct dvb_usb_device *d)
{
	struct state *state = d_to_priv(d);

	/*                                                              */
	if (d->udev->speed == USB_SPEED_FULL)
		return 1;
	else
		return state->dual_mode + 1;
}

static int af9035_frontend_attach(struct dvb_usb_adapter *adap)
{
	struct state *state = adap_to_priv(adap);
	struct dvb_usb_device *d = adap_to_d(adap);
	int ret;
	dev_dbg(&d->udev->dev, "%s:\n", __func__);

	if (!state->af9033_config[adap->id].tuner) {
		/*                   */
		ret = -ENODEV;
		goto err;
	}

	/*                    */
	adap->fe[0] = dvb_attach(af9033_attach, &state->af9033_config[adap->id],
			&d->i2c_adap);
	if (adap->fe[0] == NULL) {
		ret = -ENODEV;
		goto err;
	}

	/*                  */
	adap->fe[0]->ops.i2c_gate_ctrl = NULL;
	adap->fe[0]->callback = af9035_frontend_callback;

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static struct tua9001_config af9035_tua9001_config = {
	.i2c_addr = 0x60,
};

static const struct fc0011_config af9035_fc0011_config = {
	.i2c_address = 0x60,
};

static struct mxl5007t_config af9035_mxl5007t_config[] = {
	{
		.xtal_freq_hz = MxL_XTAL_24_MHZ,
		.if_freq_hz = MxL_IF_4_57_MHZ,
		.invert_if = 0,
		.loop_thru_enable = 0,
		.clk_out_enable = 0,
		.clk_out_amp = MxL_CLKOUT_AMP_0_94V,
	}, {
		.xtal_freq_hz = MxL_XTAL_24_MHZ,
		.if_freq_hz = MxL_IF_4_57_MHZ,
		.invert_if = 0,
		.loop_thru_enable = 1,
		.clk_out_enable = 1,
		.clk_out_amp = MxL_CLKOUT_AMP_0_94V,
	}
};

static struct tda18218_config af9035_tda18218_config = {
	.i2c_address = 0x60,
	.i2c_wr_max = 21,
};

static const struct fc2580_config af9035_fc2580_config = {
	.i2c_addr = 0x56,
	.clock = 16384000,
};

static const struct fc0012_config af9035_fc0012_config[] = {
	{
		.i2c_address = 0x63,
		.xtal_freq = FC_XTAL_36_MHZ,
		.dual_master = true,
		.loop_through = true,
		.clock_out = true,
	}, {
		.i2c_address = 0x63 | 0x80, /*                     */
		.xtal_freq = FC_XTAL_36_MHZ,
		.dual_master = true,
	}
};

static int af9035_tuner_attach(struct dvb_usb_adapter *adap)
{
	struct state *state = adap_to_priv(adap);
	struct dvb_usb_device *d = adap_to_d(adap);
	int ret;
	struct dvb_frontend *fe;
	struct i2c_msg msg[1];
	u8 tuner_addr;
	dev_dbg(&d->udev->dev, "%s:\n", __func__);

	/*
                                                                        
                                                                  
  */

	switch (state->af9033_config[adap->id].tuner) {
	case AF9033_TUNER_TUA9001:
		/*                               
                                  */

		/*                                       */
		ret = af9035_wr_reg_mask(d, 0x00d8ec, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0x00d8ed, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0x00d8e8, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0x00d8e9, 0x01, 0x01);
		if (ret < 0)
			goto err;

		/*              */
		fe = dvb_attach(tua9001_attach, adap->fe[0],
				&d->i2c_adap, &af9035_tua9001_config);
		break;
	case AF9033_TUNER_FC0011:
		fe = dvb_attach(fc0011_attach, adap->fe[0],
				&d->i2c_adap, &af9035_fc0011_config);
		break;
	case AF9033_TUNER_MXL5007T:
		if (adap->id == 0) {
			ret = af9035_wr_reg(d, 0x00d8e0, 1);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8e1, 1);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8df, 0);
			if (ret < 0)
				goto err;

			msleep(30);

			ret = af9035_wr_reg(d, 0x00d8df, 1);
			if (ret < 0)
				goto err;

			msleep(300);

			ret = af9035_wr_reg(d, 0x00d8c0, 1);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8c1, 1);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8bf, 0);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8b4, 1);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8b5, 1);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg(d, 0x00d8b3, 1);
			if (ret < 0)
				goto err;

			tuner_addr = 0x60;
		} else {
			tuner_addr = 0x60 | 0x80; /*              */
		}

		/*              */
		fe = dvb_attach(mxl5007t_attach, adap->fe[0], &d->i2c_adap,
				tuner_addr, &af9035_mxl5007t_config[adap->id]);
		break;
	case AF9033_TUNER_TDA18218:
		/*              */
		fe = dvb_attach(tda18218_attach, adap->fe[0],
				&d->i2c_adap, &af9035_tda18218_config);
		break;
	case AF9033_TUNER_FC2580:
		/*                                                       */
		ret = af9035_wr_reg_mask(d, 0xd8eb, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0xd8ec, 0x01, 0x01);
		if (ret < 0)
			goto err;

		ret = af9035_wr_reg_mask(d, 0xd8ed, 0x01, 0x01);
		if (ret < 0)
			goto err;

		usleep_range(10000, 50000);
		/*              */
		fe = dvb_attach(fc2580_attach, adap->fe[0],
				&d->i2c_adap, &af9035_fc2580_config);
		break;
	case AF9033_TUNER_FC0012:
		/*
                                  
                                                              
                                          
   */

		if (adap->id == 0) {
			/*                                     */
			ret = af9035_wr_reg_mask(d, 0xd8eb, 0x01, 0x01);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg_mask(d, 0xd8ec, 0x01, 0x01);
			if (ret < 0)
				goto err;

			ret = af9035_wr_reg_mask(d, 0xd8ed, 0x01, 0x01);
			if (ret < 0)
				goto err;
		} else {
			/*
                                                
                                                          
                                           
    */
			msg[0].addr = 0x63;
			msg[0].flags = 0;
			msg[0].len = 2;
			msg[0].buf = "\x0d\x02";
			ret = i2c_transfer(&d->i2c_adap, msg, 1);
			if (ret < 0)
				goto err;
		}

		usleep_range(10000, 50000);

		fe = dvb_attach(fc0012_attach, adap->fe[0], &d->i2c_adap,
				&af9035_fc0012_config[adap->id]);
		break;
	case AF9033_TUNER_IT9135_38:
	case AF9033_TUNER_IT9135_51:
	case AF9033_TUNER_IT9135_52:
	case AF9033_TUNER_IT9135_60:
	case AF9033_TUNER_IT9135_61:
	case AF9033_TUNER_IT9135_62:
		/*              */
		fe = dvb_attach(it913x_attach, adap->fe[0], &d->i2c_adap,
				state->af9033_config[adap->id].i2c_addr,
				state->af9033_config[0].tuner);
		break;
	default:
		fe = NULL;
	}

	if (fe == NULL) {
		ret = -ENODEV;
		goto err;
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_init(struct dvb_usb_device *d)
{
	struct state *state = d_to_priv(d);
	int ret, i;
	u16 frame_size = (d->udev->speed == USB_SPEED_FULL ? 5 : 87) * 188 / 4;
	u8 packet_size = (d->udev->speed == USB_SPEED_FULL ? 64 : 512) / 4;
	struct reg_val_mask tab[] = {
		{ 0x80f99d, 0x01, 0x01 },
		{ 0x80f9a4, 0x01, 0x01 },
		{ 0x00dd11, 0x00, 0x20 },
		{ 0x00dd11, 0x00, 0x40 },
		{ 0x00dd13, 0x00, 0x20 },
		{ 0x00dd13, 0x00, 0x40 },
		{ 0x00dd11, 0x20, 0x20 },
		{ 0x00dd88, (frame_size >> 0) & 0xff, 0xff},
		{ 0x00dd89, (frame_size >> 8) & 0xff, 0xff},
		{ 0x00dd0c, packet_size, 0xff},
		{ 0x00dd11, state->dual_mode << 6, 0x40 },
		{ 0x00dd8a, (frame_size >> 0) & 0xff, 0xff},
		{ 0x00dd8b, (frame_size >> 8) & 0xff, 0xff},
		{ 0x00dd0d, packet_size, 0xff },
		{ 0x80f9a3, state->dual_mode, 0x01 },
		{ 0x80f9cd, state->dual_mode, 0x01 },
		{ 0x80f99d, 0x00, 0x01 },
		{ 0x80f9a4, 0x00, 0x01 },
	};

	dev_dbg(&d->udev->dev, "%s: USB speed=%d frame_size=%04x " \
			"packet_size=%02x\n", __func__,
			d->udev->speed, frame_size, packet_size);

	/*                */
	for (i = 0; i < ARRAY_SIZE(tab); i++) {
		ret = af9035_wr_reg_mask(d, tab[i].reg, tab[i].val,
				tab[i].mask);
		if (ret < 0)
			goto err;
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

#if IS_ENABLED(CONFIG_RC_CORE)
static int af9035_rc_query(struct dvb_usb_device *d)
{
	int ret;
	u32 key;
	u8 buf[4];
	struct usb_req req = { CMD_IR_GET, 0, 0, NULL, 4, buf };

	ret = af9035_ctrl_msg(d, &req);
	if (ret == 1)
		return 0;
	else if (ret < 0)
		goto err;

	if ((buf[2] + buf[3]) == 0xff) {
		if ((buf[0] + buf[1]) == 0xff) {
			/*                    */
			key = buf[0] << 8 | buf[2];
		} else {
			/*                    */
			key = buf[0] << 16 | buf[1] << 8 | buf[2];
		}
	} else {
		/*                     */
		key = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
	}

	dev_dbg(&d->udev->dev, "%s: %*ph\n", __func__, 4, buf);

	rc_keydown(d->rc_dev, key, 0);

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_get_rc_config(struct dvb_usb_device *d, struct dvb_usb_rc *rc)
{
	struct state *state = d_to_priv(d);
	int ret;
	u8 tmp;

	ret = af9035_rd_reg(d, state->eeprom_addr + EEPROM_IR_MODE, &tmp);
	if (ret < 0)
		goto err;

	dev_dbg(&d->udev->dev, "%s: ir_mode=%02x\n", __func__, tmp);

	/*                                                      */
	if (tmp == 5) {
		ret = af9035_rd_reg(d, state->eeprom_addr + EEPROM_IR_TYPE,
				&tmp);
		if (ret < 0)
			goto err;

		dev_dbg(&d->udev->dev, "%s: ir_type=%02x\n", __func__, tmp);

		switch (tmp) {
		case 0: /*     */
		default:
			rc->allowed_protos = RC_BIT_NEC;
			break;
		case 1: /*     */
			rc->allowed_protos = RC_BIT_RC6_MCE;
			break;
		}

		rc->query = af9035_rc_query;
		rc->interval = 500;

		/*                         */
		if (!rc->map_name)
			rc->map_name = RC_MAP_EMPTY;
	}

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}
#else
	#define af9035_get_rc_config NULL
#endif

static int af9035_get_stream_config(struct dvb_frontend *fe, u8 *ts_type,
		struct usb_data_stream_properties *stream)
{
	struct dvb_usb_device *d = fe_to_d(fe);
	dev_dbg(&d->udev->dev, "%s: adap=%d\n", __func__, fe_to_adap(fe)->id);

	if (d->udev->speed == USB_SPEED_FULL)
		stream->u.bulk.buffersize = 5 * 188;

	return 0;
}

/*
                                                                          
                                                                      
                                        
 */
static int af9035_pid_filter_ctrl(struct dvb_usb_adapter *adap, int onoff)
{
	struct dvb_usb_device *d = adap_to_d(adap);
	int ret;

	dev_dbg(&d->udev->dev, "%s: onoff=%d\n", __func__, onoff);

	ret = af9035_wr_reg_mask(d, 0x80f993, onoff, 0x01);
	if (ret < 0)
		goto err;

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_pid_filter(struct dvb_usb_adapter *adap, int index, u16 pid,
		int onoff)
{
	struct dvb_usb_device *d = adap_to_d(adap);
	int ret;
	u8 wbuf[2] = {(pid >> 0) & 0xff, (pid >> 8) & 0xff};

	dev_dbg(&d->udev->dev, "%s: index=%d pid=%04x onoff=%d\n",
			__func__, index, pid, onoff);

	ret = af9035_wr_regs(d, 0x80f996, wbuf, 2);
	if (ret < 0)
		goto err;

	ret = af9035_wr_reg(d, 0x80f994, onoff);
	if (ret < 0)
		goto err;

	ret = af9035_wr_reg(d, 0x80f995, index);
	if (ret < 0)
		goto err;

	return 0;

err:
	dev_dbg(&d->udev->dev, "%s: failed=%d\n", __func__, ret);

	return ret;
}

static int af9035_probe(struct usb_interface *intf,
		const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	char manufacturer[sizeof("Afatech")];

	memset(manufacturer, 0, sizeof(manufacturer));
	usb_string(udev, udev->descriptor.iManufacturer,
			manufacturer, sizeof(manufacturer));
	/*
                                                                       
                                                                      
                            
   
                                                      
                             
                             
                                     
                                     
   
                                                      
                             
                             
                                                    
                                            
  */
	if ((le16_to_cpu(udev->descriptor.idVendor) == USB_VID_TERRATEC) &&
			(le16_to_cpu(udev->descriptor.idProduct) == 0x0099)) {
		if (!strcmp("Afatech", manufacturer)) {
			dev_dbg(&udev->dev, "%s: rejecting device\n", __func__);
			return -ENODEV;
		}
	}

	return dvb_usbv2_probe(intf, id);
}

/*                                          
                                              */
static const struct dvb_usb_device_properties af9035_props = {
	.driver_name = KBUILD_MODNAME,
	.owner = THIS_MODULE,
	.adapter_nr = adapter_nr,
	.size_of_priv = sizeof(struct state),

	.generic_bulk_ctrl_endpoint = 0x02,
	.generic_bulk_ctrl_endpoint_response = 0x81,

	.identify_state = af9035_identify_state,
	.download_firmware = af9035_download_firmware,

	.i2c_algo = &af9035_i2c_algo,
	.read_config = af9035_read_config,
	.frontend_attach = af9035_frontend_attach,
	.tuner_attach = af9035_tuner_attach,
	.init = af9035_init,
	.get_rc_config = af9035_get_rc_config,
	.get_stream_config = af9035_get_stream_config,

	.get_adapter_count = af9035_get_adapter_count,
	.adapter = {
		{
			.caps = DVB_USB_ADAP_HAS_PID_FILTER |
				DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF,

			.pid_filter_count = 32,
			.pid_filter_ctrl = af9035_pid_filter_ctrl,
			.pid_filter = af9035_pid_filter,

			.stream = DVB_USB_STREAM_BULK(0x84, 6, 87 * 188),
		}, {
			.stream = DVB_USB_STREAM_BULK(0x85, 6, 87 * 188),
		},
	},
};

static const struct usb_device_id af9035_id_table[] = {
	/*                */
	{ DVB_USB_DEVICE(USB_VID_AFATECH, USB_PID_AFATECH_AF9035_9035,
		&af9035_props, "Afatech AF9035 reference design", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AFATECH, USB_PID_AFATECH_AF9035_1000,
		&af9035_props, "Afatech AF9035 reference design", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AFATECH, USB_PID_AFATECH_AF9035_1001,
		&af9035_props, "Afatech AF9035 reference design", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AFATECH, USB_PID_AFATECH_AF9035_1002,
		&af9035_props, "Afatech AF9035 reference design", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AFATECH, USB_PID_AFATECH_AF9035_1003,
		&af9035_props, "Afatech AF9035 reference design", NULL) },
	{ DVB_USB_DEVICE(USB_VID_TERRATEC, USB_PID_TERRATEC_CINERGY_T_STICK,
		&af9035_props, "TerraTec Cinergy T Stick", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AVERMEDIA, USB_PID_AVERMEDIA_A835,
		&af9035_props, "AVerMedia AVerTV Volar HD/PRO (A835)", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AVERMEDIA, USB_PID_AVERMEDIA_B835,
		&af9035_props, "AVerMedia AVerTV Volar HD/PRO (A835)", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AVERMEDIA, USB_PID_AVERMEDIA_1867,
		&af9035_props, "AVerMedia HD Volar (A867)", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AVERMEDIA, USB_PID_AVERMEDIA_A867,
		&af9035_props, "AVerMedia HD Volar (A867)", NULL) },
	{ DVB_USB_DEVICE(USB_VID_AVERMEDIA, USB_PID_AVERMEDIA_TWINSTAR,
		&af9035_props, "AVerMedia Twinstar (A825)", NULL) },
	{ DVB_USB_DEVICE(USB_VID_ASUS, USB_PID_ASUS_U3100MINI_PLUS,
		&af9035_props, "Asus U3100Mini Plus", NULL) },
        { DVB_USB_DEVICE(USB_VID_TERRATEC, 0x00aa,
		&af9035_props, "TerraTec Cinergy T Stick (rev. 2)", NULL) },
	/*                */
#if 0
	{ DVB_USB_DEVICE(0x048d, 0x9135,
		&af9035_props, "IT9135 reference design", NULL) },
	{ DVB_USB_DEVICE(0x048d, 0x9006,
		&af9035_props, "IT9135 reference design", NULL) },
#endif
	/*                                                            */
	{ DVB_USB_DEVICE(USB_VID_TERRATEC, 0x0099,
		&af9035_props, "TerraTec Cinergy T Stick Dual RC (rev. 2)", NULL) },
	{ DVB_USB_DEVICE(USB_VID_LEADTEK, 0x6a05,
		&af9035_props, "Leadtek WinFast DTV Dongle Dual", NULL) },
	{ DVB_USB_DEVICE(USB_VID_HAUPPAUGE, 0xf900,
		&af9035_props, "Hauppauge WinTV-MiniStick 2", NULL) },
	{ }
};
MODULE_DEVICE_TABLE(usb, af9035_id_table);

static struct usb_driver af9035_usb_driver = {
	.name = KBUILD_MODNAME,
	.id_table = af9035_id_table,
	.probe = af9035_probe,
	.disconnect = dvb_usbv2_disconnect,
	.suspend = dvb_usbv2_suspend,
	.resume = dvb_usbv2_resume,
	.reset_resume = dvb_usbv2_reset_resume,
	.no_dynamic_id = 1,
	.soft_unbind = 1,
};

module_usb_driver(af9035_usb_driver);

MODULE_AUTHOR("Antti Palosaari <crope@iki.fi>");
MODULE_DESCRIPTION("Afatech AF9035 driver");
MODULE_LICENSE("GPL");
MODULE_FIRMWARE(AF9035_FIRMWARE_AF9035);
MODULE_FIRMWARE(AF9035_FIRMWARE_IT9135_V1);
MODULE_FIRMWARE(AF9035_FIRMWARE_IT9135_V2);
