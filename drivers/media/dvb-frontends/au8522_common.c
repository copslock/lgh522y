/*
    Auvitek AU8522 QAM/8VSB demodulator driver

    Copyright (C) 2008 Steven Toth <stoth@linuxtv.org>
    Copyright (C) 2008 Devin Heitmueller <dheitmueller@linuxtv.org>
    Copyright (C) 2005-2008 Auvitek International, Ltd.
    Copyright (C) 2012 Michael Krufky <mkrufky@linuxtv.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <linux/i2c.h>
#include "dvb_frontend.h"
#include "au8522_priv.h"

static int debug;

#define dprintk(arg...)\
  do { if (debug)\
	 printk(arg);\
  } while (0)

/*                                                                      
                                  */
static LIST_HEAD(hybrid_tuner_instance_list);
static DEFINE_MUTEX(au8522_list_mutex);

/*                                */
int au8522_writereg(struct au8522_state *state, u16 reg, u8 data)
{
	int ret;
	u8 buf[] = { (reg >> 8) | 0x80, reg & 0xff, data };

	struct i2c_msg msg = { .addr = state->config->demod_address,
			       .flags = 0, .buf = buf, .len = 3 };

	ret = i2c_transfer(state->i2c, &msg, 1);

	if (ret != 1)
		printk("%s: writereg error (reg == 0x%02x, val == 0x%04x, "
		       "ret == %i)\n", __func__, reg, data, ret);

	return (ret != 1) ? -1 : 0;
}
EXPORT_SYMBOL(au8522_writereg);

u8 au8522_readreg(struct au8522_state *state, u16 reg)
{
	int ret;
	u8 b0[] = { (reg >> 8) | 0x40, reg & 0xff };
	u8 b1[] = { 0 };

	struct i2c_msg msg[] = {
		{ .addr = state->config->demod_address, .flags = 0,
		  .buf = b0, .len = 2 },
		{ .addr = state->config->demod_address, .flags = I2C_M_RD,
		  .buf = b1, .len = 1 } };

	ret = i2c_transfer(state->i2c, msg, 2);

	if (ret != 2)
		printk(KERN_ERR "%s: readreg error (ret == %i)\n",
		       __func__, ret);
	return b1[0];
}
EXPORT_SYMBOL(au8522_readreg);

int au8522_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	struct au8522_state *state = fe->demodulator_priv;

	dprintk("%s(%d)\n", __func__, enable);

	if (state->operational_mode == AU8522_ANALOG_MODE) {
		/*                                                       
                                                            
                                                              
                             */
		return 0;
	}

	if (enable)
		return au8522_writereg(state, 0x106, 1);
	else
		return au8522_writereg(state, 0x106, 0);
}
EXPORT_SYMBOL(au8522_i2c_gate_ctrl);

int au8522_analog_i2c_gate_ctrl(struct dvb_frontend *fe, int enable)
{
	struct au8522_state *state = fe->demodulator_priv;

	dprintk("%s(%d)\n", __func__, enable);

	if (enable)
		return au8522_writereg(state, 0x106, 1);
	else
		return au8522_writereg(state, 0x106, 0);
}
EXPORT_SYMBOL(au8522_analog_i2c_gate_ctrl);

/*                                                                      
                       */
int au8522_get_state(struct au8522_state **state, struct i2c_adapter *i2c,
		     u8 client_address)
{
	int ret;

	mutex_lock(&au8522_list_mutex);
	ret = hybrid_tuner_request_state(struct au8522_state, (*state),
					 hybrid_tuner_instance_list,
					 i2c, client_address, "au8522");
	mutex_unlock(&au8522_list_mutex);

	return ret;
}
EXPORT_SYMBOL(au8522_get_state);

void au8522_release_state(struct au8522_state *state)
{
	mutex_lock(&au8522_list_mutex);
	if (state != NULL)
		hybrid_tuner_release_state(state);
	mutex_unlock(&au8522_list_mutex);
}
EXPORT_SYMBOL(au8522_release_state);

static int au8522_led_gpio_enable(struct au8522_state *state, int onoff)
{
	struct au8522_led_config *led_config = state->config->led_cfg;
	u8 val;

	/*                                     */
	if (!led_config || !led_config->gpio_output ||
	    !led_config->gpio_output_enable || !led_config->gpio_output_disable)
		return 0;

	val = au8522_readreg(state, 0x4000 |
			     (led_config->gpio_output & ~0xc000));
	if (onoff) {
		/*                    */
		val &= ~((led_config->gpio_output_enable >> 8) & 0xff);
		val |=  (led_config->gpio_output_enable & 0xff);
	} else {
		/*                     */
		val &= ~((led_config->gpio_output_disable >> 8) & 0xff);
		val |=  (led_config->gpio_output_disable & 0xff);
	}
	return au8522_writereg(state, 0x8000 |
			       (led_config->gpio_output & ~0xc000), val);
}

/*              
                      
                          
                                                     
 */
int au8522_led_ctrl(struct au8522_state *state, int led)
{
	struct au8522_led_config *led_config = state->config->led_cfg;
	int i, ret = 0;

	/*                                     */
	if (!led_config || !led_config->gpio_leds ||
	    !led_config->num_led_states || !led_config->led_states)
		return 0;

	if (led < 0) {
		/*                                            */
		if (state->led_state)
			return 0;
		else
			led *= -1;
	}

	/*                              */
	if (state->led_state != led) {
		u8 val;

		dprintk("%s: %d\n", __func__, led);

		au8522_led_gpio_enable(state, 1);

		val = au8522_readreg(state, 0x4000 |
				     (led_config->gpio_leds & ~0xc000));

		/*                         */
		for (i = 0; i < led_config->num_led_states; i++)
			val &= ~led_config->led_states[i];

		/*                        */
		if (led < led_config->num_led_states)
			val |= led_config->led_states[led];
		else if (led_config->num_led_states)
			val |=
			led_config->led_states[led_config->num_led_states - 1];

		ret = au8522_writereg(state, 0x8000 |
				      (led_config->gpio_leds & ~0xc000), val);
		if (ret < 0)
			return ret;

		state->led_state = led;

		if (led == 0)
			au8522_led_gpio_enable(state, 0);
	}

	return 0;
}
EXPORT_SYMBOL(au8522_led_ctrl);

int au8522_init(struct dvb_frontend *fe)
{
	struct au8522_state *state = fe->demodulator_priv;
	dprintk("%s()\n", __func__);

	state->operational_mode = AU8522_DIGITAL_MODE;

	/*                                                            
                                                             
                             */
	state->current_frequency = 0;

	au8522_writereg(state, 0xa4, 1 << 5);

	au8522_i2c_gate_ctrl(fe, 1);

	return 0;
}
EXPORT_SYMBOL(au8522_init);

int au8522_sleep(struct dvb_frontend *fe)
{
	struct au8522_state *state = fe->demodulator_priv;
	dprintk("%s()\n", __func__);

	/*                                                                 */
	if (state->operational_mode == AU8522_ANALOG_MODE) {
		/*                                                          
                                                              
                                                              
                                      */
		return 0;
	}

	/*              */
	au8522_led_ctrl(state, 0);

	/*                     */
	au8522_writereg(state, 0xa4, 1 << 5);

	state->current_frequency = 0;

	return 0;
}
EXPORT_SYMBOL(au8522_sleep);

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable verbose debug messages");

MODULE_DESCRIPTION("Auvitek AU8522 QAM-B/ATSC Demodulator driver");
MODULE_AUTHOR("Steven Toth");
MODULE_LICENSE("GPL");
