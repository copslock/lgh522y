/*
 *  Maxim (Dallas) MAX3107/8 serial driver
 *
 *  Copyright (C) 2012 Alexander Shiyan <shc_work@mail.ru>
 *
 *  Based on max3100.c, by Christian Pellegrin <chripell@evolware.org>
 *  Based on max3110.c, by Feng Tang <feng.tang@intel.com>
 *  Based on max3107.c, by Aavamobile
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

/*                              */
/*                               */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/platform_data/max310x.h>

#define MAX310X_MAJOR			204
#define MAX310X_MINOR			209

/*                              */
#define MAX310X_RHR_REG			(0x00) /*         */
#define MAX310X_THR_REG			(0x00) /*         */
#define MAX310X_IRQEN_REG		(0x01) /*            */
#define MAX310X_IRQSTS_REG		(0x02) /*            */
#define MAX310X_LSR_IRQEN_REG		(0x03) /*                */
#define MAX310X_LSR_IRQSTS_REG		(0x04) /*                */
#define MAX310X_SPCHR_IRQEN_REG		(0x05) /*                         */
#define MAX310X_SPCHR_IRQSTS_REG	(0x06) /*                         */
#define MAX310X_STS_IRQEN_REG		(0x07) /*                   */
#define MAX310X_STS_IRQSTS_REG		(0x08) /*                   */
#define MAX310X_MODE1_REG		(0x09) /*       */
#define MAX310X_MODE2_REG		(0x0a) /*       */
#define MAX310X_LCR_REG			(0x0b) /*     */
#define MAX310X_RXTO_REG		(0x0c) /*            */
#define MAX310X_HDPIXDELAY_REG		(0x0d) /*                         */
#define MAX310X_IRDA_REG		(0x0e) /*               */
#define MAX310X_FLOWLVL_REG		(0x0f) /*                     */
#define MAX310X_FIFOTRIGLVL_REG		(0x10) /*                         */
#define MAX310X_TXFIFOLVL_REG		(0x11) /*               */
#define MAX310X_RXFIFOLVL_REG		(0x12) /*               */
#define MAX310X_FLOWCTRL_REG		(0x13) /*              */
#define MAX310X_XON1_REG		(0x14) /*                */
#define MAX310X_XON2_REG		(0x15) /*                */
#define MAX310X_XOFF1_REG		(0x16) /*                 */
#define MAX310X_XOFF2_REG		(0x17) /*                 */
#define MAX310X_GPIOCFG_REG		(0x18) /*             */
#define MAX310X_GPIODATA_REG		(0x19) /*           */
#define MAX310X_PLLCFG_REG		(0x1a) /*            */
#define MAX310X_BRGCFG_REG		(0x1b) /*                          */
#define MAX310X_BRGDIVLSB_REG		(0x1c) /*                       */
#define MAX310X_BRGDIVMSB_REG		(0x1d) /*                       */
#define MAX310X_CLKSRC_REG		(0x1e) /*              */
/*                         */
#define MAX3107_REVID_REG		(0x1f) /*                         */

/*                   */
#define MAX310X_IRQ_LSR_BIT		(1 << 0) /*               */
#define MAX310X_IRQ_SPCHR_BIT		(1 << 1) /*                        */
#define MAX310X_IRQ_STS_BIT		(1 << 2) /*                  */
#define MAX310X_IRQ_RXFIFO_BIT		(1 << 3) /*                   */
#define MAX310X_IRQ_TXFIFO_BIT		(1 << 4) /*                   */
#define MAX310X_IRQ_TXEMPTY_BIT		(1 << 5) /*                         */
#define MAX310X_IRQ_RXEMPTY_BIT		(1 << 6) /*                         */
#define MAX310X_IRQ_CTS_BIT		(1 << 7) /*               */

/*                   */
#define MAX310X_LSR_RXTO_BIT		(1 << 0) /*            */
#define MAX310X_LSR_RXOVR_BIT		(1 << 1) /*            */
#define MAX310X_LSR_RXPAR_BIT		(1 << 2) /*                 */
#define MAX310X_LSR_FRERR_BIT		(1 << 3) /*             */
#define MAX310X_LSR_RXBRK_BIT		(1 << 4) /*          */
#define MAX310X_LSR_RXNOISE_BIT		(1 << 5) /*          */
#define MAX310X_LSR_CTS_BIT		(1 << 7) /*               */

/*                                 */
#define MAX310X_SPCHR_XON1_BIT		(1 << 0) /*                */
#define MAX310X_SPCHR_XON2_BIT		(1 << 1) /*                */
#define MAX310X_SPCHR_XOFF1_BIT		(1 << 2) /*                 */
#define MAX310X_SPCHR_XOFF2_BIT		(1 << 3) /*                 */
#define MAX310X_SPCHR_BREAK_BIT		(1 << 4) /*          */
#define MAX310X_SPCHR_MULTIDROP_BIT	(1 << 5) /*                           */

/*                      */
#define MAX310X_STS_GPIO0_BIT		(1 << 0) /*                  */
#define MAX310X_STS_GPIO1_BIT		(1 << 1) /*                  */
#define MAX310X_STS_GPIO2_BIT		(1 << 2) /*                  */
#define MAX310X_STS_GPIO3_BIT		(1 << 3) /*                  */
#define MAX310X_STS_CLKREADY_BIT	(1 << 5) /*             */
#define MAX310X_STS_SLEEP_BIT		(1 << 6) /*                 */

/*                     */
#define MAX310X_MODE1_RXDIS_BIT		(1 << 0) /*            */
#define MAX310X_MODE1_TXDIS_BIT		(1 << 1) /*            */
#define MAX310X_MODE1_TXHIZ_BIT		(1 << 2) /*                    */
#define MAX310X_MODE1_RTSHIZ_BIT	(1 << 3) /*                     */
#define MAX310X_MODE1_TRNSCVCTRL_BIT	(1 << 4) /*                         */
#define MAX310X_MODE1_FORCESLEEP_BIT	(1 << 5) /*                  */
#define MAX310X_MODE1_AUTOSLEEP_BIT	(1 << 6) /*                   */
#define MAX310X_MODE1_IRQSEL_BIT	(1 << 7) /*                */

/*                     */
#define MAX310X_MODE2_RST_BIT		(1 << 0) /*            */
#define MAX310X_MODE2_FIFORST_BIT	(1 << 1) /*            */
#define MAX310X_MODE2_RXTRIGINV_BIT	(1 << 2) /*                    */
#define MAX310X_MODE2_RXEMPTINV_BIT	(1 << 3) /*                          */
#define MAX310X_MODE2_SPCHR_BIT		(1 << 4) /*                           */
#define MAX310X_MODE2_LOOPBACK_BIT	(1 << 5) /*                          */
#define MAX310X_MODE2_MULTIDROP_BIT	(1 << 6) /*                        */
#define MAX310X_MODE2_ECHOSUPR_BIT	(1 << 7) /*                         */

/*                   */
#define MAX310X_LCR_LENGTH0_BIT		(1 << 0) /*                   */
#define MAX310X_LCR_LENGTH1_BIT		(1 << 1) /*                  
         
                                 
                           
                           
                           
                           
        */
#define MAX310X_LCR_STOPLEN_BIT		(1 << 2) /*                
         
                                
                         
                                 
                                
                                    
        */
#define MAX310X_LCR_PARITY_BIT		(1 << 3) /*                   */
#define MAX310X_LCR_EVENPARITY_BIT	(1 << 4) /*                        */
#define MAX310X_LCR_FORCEPARITY_BIT	(1 << 5) /*                        */
#define MAX310X_LCR_TXBREAK_BIT		(1 << 6) /*                 */
#define MAX310X_LCR_RTS_BIT		(1 << 7) /*                 */
#define MAX310X_LCR_WORD_LEN_5		(0x00)
#define MAX310X_LCR_WORD_LEN_6		(0x01)
#define MAX310X_LCR_WORD_LEN_7		(0x02)
#define MAX310X_LCR_WORD_LEN_8		(0x03)

/*                    */
#define MAX310X_IRDA_IRDAEN_BIT		(1 << 0) /*                  */
#define MAX310X_IRDA_SIR_BIT		(1 << 1) /*                 */
#define MAX310X_IRDA_SHORTIR_BIT	(1 << 2) /*                       */
#define MAX310X_IRDA_MIR_BIT		(1 << 3) /*                 */
#define MAX310X_IRDA_RXINV_BIT		(1 << 4) /*                           */
#define MAX310X_IRDA_TXINV_BIT		(1 << 5) /*                           */

/*                                           */
#define MAX310X_FLOWLVL_HALT_MASK	(0x000f) /*                         */
#define MAX310X_FLOWLVL_RES_MASK	(0x00f0) /*                           */
#define MAX310X_FLOWLVL_HALT(words)	((words / 8) & 0x0f)
#define MAX310X_FLOWLVL_RES(words)	(((words / 8) & 0x0f) << 4)

/*                                             */
#define MAX310X_FIFOTRIGLVL_TX_MASK	(0x0f) /*                       */
#define MAX310X_FIFOTRIGLVL_RX_MASK	(0xf0) /*                       */
#define MAX310X_FIFOTRIGLVL_TX(words)	((words / 8) & 0x0f)
#define MAX310X_FIFOTRIGLVL_RX(words)	(((words / 8) & 0x0f) << 4)

/*                            */
#define MAX310X_FLOWCTRL_AUTORTS_BIT	(1 << 0) /*                           */
#define MAX310X_FLOWCTRL_AUTOCTS_BIT	(1 << 1) /*                           */
#define MAX310X_FLOWCTRL_GPIADDR_BIT	(1 << 2) /*                         
                                      
                                 
                            */
#define MAX310X_FLOWCTRL_SWFLOWEN_BIT	(1 << 3) /*                          */
#define MAX310X_FLOWCTRL_SWFLOW0_BIT	(1 << 4) /*              */
#define MAX310X_FLOWCTRL_SWFLOW1_BIT	(1 << 5) /*             
         
                                  
                                   
                       
                                 
                              
                            
                           
                                 
                              
                            
                           
                                 
                                     
                                  
                           
        */
#define MAX310X_FLOWCTRL_SWFLOW2_BIT	(1 << 6) /*              */
#define MAX310X_FLOWCTRL_SWFLOW3_BIT	(1 << 7) /*             
         
                                  
                                
                       
                                     
                              
                                     
                              
                                     
                                     
                     
        */

/*                                  */
#define MAX310X_GPIOCFG_GP0OUT_BIT	(1 << 0) /*                      */
#define MAX310X_GPIOCFG_GP1OUT_BIT	(1 << 1) /*                      */
#define MAX310X_GPIOCFG_GP2OUT_BIT	(1 << 2) /*                      */
#define MAX310X_GPIOCFG_GP3OUT_BIT	(1 << 3) /*                      */
#define MAX310X_GPIOCFG_GP0OD_BIT	(1 << 4) /*                          */
#define MAX310X_GPIOCFG_GP1OD_BIT	(1 << 5) /*                          */
#define MAX310X_GPIOCFG_GP2OD_BIT	(1 << 6) /*                          */
#define MAX310X_GPIOCFG_GP3OD_BIT	(1 << 7) /*                          */

/*                         */
#define MAX310X_GPIODATA_GP0OUT_BIT	(1 << 0) /*                     */
#define MAX310X_GPIODATA_GP1OUT_BIT	(1 << 1) /*                     */
#define MAX310X_GPIODATA_GP2OUT_BIT	(1 << 2) /*                     */
#define MAX310X_GPIODATA_GP3OUT_BIT	(1 << 3) /*                     */
#define MAX310X_GPIODATA_GP0IN_BIT	(1 << 4) /*                    */
#define MAX310X_GPIODATA_GP1IN_BIT	(1 << 5) /*                    */
#define MAX310X_GPIODATA_GP2IN_BIT	(1 << 6) /*                    */
#define MAX310X_GPIODATA_GP3IN_BIT	(1 << 7) /*                    */

/*                                  */
#define MAX310X_PLLCFG_PREDIV_MASK	(0x3f) /*                       */
#define MAX310X_PLLCFG_PLLFACTOR_MASK	(0xc0) /*                           */

/*                                                 */
#define MAX310X_BRGCFG_2XMODE_BIT	(1 << 4) /*                  */
#define MAX310X_BRGCFG_4XMODE_BIT	(1 << 5) /*                     */

/*                            */
#define MAX310X_CLKSRC_CRYST_BIT	(1 << 1) /*                    */
#define MAX310X_CLKSRC_PLL_BIT		(1 << 2) /*            */
#define MAX310X_CLKSRC_PLLBYP_BIT	(1 << 3) /*            */
#define MAX310X_CLKSRC_EXTCLK_BIT	(1 << 4) /*                       */
#define MAX310X_CLKSRC_CLK2RTS_BIT	(1 << 7) /*                     */

/*                  */
#define MAX310X_FIFO_SIZE		(128)

/*                  */
#define MAX3107_REV_ID			(0xa0)
#define MAX3107_REV_MASK		(0xfe)

/*                             */
#define MAX310X_IRQ_TX			(MAX310X_IRQ_TXFIFO_BIT | \
					 MAX310X_IRQ_TXEMPTY_BIT)
#define MAX310X_IRQ_RX			(MAX310X_IRQ_RXFIFO_BIT | \
					 MAX310X_IRQ_RXEMPTY_BIT)

/*                      */
enum {
	MAX310X_TYPE_MAX3107	= 3107,
	MAX310X_TYPE_MAX3108	= 3108,
};

struct max310x_port {
	struct uart_driver	uart;
	struct uart_port	port;

	const char		*name;
	int			uartclk;

	unsigned int		nr_gpio;
#ifdef CONFIG_GPIOLIB
	struct gpio_chip	gpio;
#endif

	struct regmap		*regmap;
	struct regmap_config	regcfg;

	struct workqueue_struct	*wq;
	struct work_struct	tx_work;

	struct mutex		max310x_mutex;

	struct max310x_pdata	*pdata;
};

static bool max3107_8_reg_writeable(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MAX310X_IRQSTS_REG:
	case MAX310X_LSR_IRQSTS_REG:
	case MAX310X_SPCHR_IRQSTS_REG:
	case MAX310X_STS_IRQSTS_REG:
	case MAX310X_TXFIFOLVL_REG:
	case MAX310X_RXFIFOLVL_REG:
	case MAX3107_REVID_REG: /*                           */
		return false;
	default:
		break;
	}

	return true;
}

static bool max310x_reg_volatile(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MAX310X_RHR_REG:
	case MAX310X_IRQSTS_REG:
	case MAX310X_LSR_IRQSTS_REG:
	case MAX310X_SPCHR_IRQSTS_REG:
	case MAX310X_STS_IRQSTS_REG:
	case MAX310X_TXFIFOLVL_REG:
	case MAX310X_RXFIFOLVL_REG:
	case MAX310X_GPIODATA_REG:
		return true;
	default:
		break;
	}

	return false;
}

static bool max310x_reg_precious(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case MAX310X_RHR_REG:
	case MAX310X_IRQSTS_REG:
	case MAX310X_SPCHR_IRQSTS_REG:
	case MAX310X_STS_IRQSTS_REG:
		return true;
	default:
		break;
	}

	return false;
}

static void max310x_set_baud(struct max310x_port *s, int baud)
{
	unsigned int mode = 0, div = s->uartclk / baud;

	if (!(div / 16)) {
		/*         */
		mode = MAX310X_BRGCFG_2XMODE_BIT;
		div = (s->uartclk * 2) / baud;
	}

	if (!(div / 16)) {
		/*         */
		mode = MAX310X_BRGCFG_4XMODE_BIT;
		div = (s->uartclk * 4) / baud;
	}

	regmap_write(s->regmap, MAX310X_BRGDIVMSB_REG,
		     ((div / 16) >> 8) & 0xff);
	regmap_write(s->regmap, MAX310X_BRGDIVLSB_REG, (div / 16) & 0xff);
	regmap_write(s->regmap, MAX310X_BRGCFG_REG, (div % 16) | mode);
}

static void max310x_wait_pll(struct max310x_port *s)
{
	int tryes = 1000;

	/*                                      */
	if (!(s->pdata->driver_flags & MAX310X_EXT_CLK)) {
		unsigned int sts = 0;

		while (tryes--) {
			regmap_read(s->regmap, MAX310X_STS_IRQSTS_REG, &sts);
			if (sts & MAX310X_STS_CLKREADY_BIT)
				break;
		}
	}
}

static int max310x_update_best_err(unsigned long f, long *besterr)
{
	/*                                         */
	long err = f % (115200 * 16);

	if ((*besterr < 0) || (*besterr > err)) {
		*besterr = err;
		return 0;
	}

	return 1;
}

static int max310x_set_ref_clk(struct max310x_port *s)
{
	unsigned int div, clksrc, pllcfg = 0;
	long besterr = -1;
	unsigned long fdiv, fmul, bestfreq = s->pdata->frequency;

	/*                                 */
	max310x_update_best_err(s->pdata->frequency, &besterr);

	/*                               */
	for (div = 1; (div <= 63) && besterr; div++) {
		fdiv = DIV_ROUND_CLOSEST(s->pdata->frequency, div);

		/*                  */
		fmul = fdiv * 6;
		if ((fdiv >= 500000) && (fdiv <= 800000))
			if (!max310x_update_best_err(fmul, &besterr)) {
				pllcfg = (0 << 6) | div;
				bestfreq = fmul;
			}
		/*                   */
		fmul = fdiv * 48;
		if ((fdiv >= 850000) && (fdiv <= 1200000))
			if (!max310x_update_best_err(fmul, &besterr)) {
				pllcfg = (1 << 6) | div;
				bestfreq = fmul;
			}
		/*                   */
		fmul = fdiv * 96;
		if ((fdiv >= 425000) && (fdiv <= 1000000))
			if (!max310x_update_best_err(fmul, &besterr)) {
				pllcfg = (2 << 6) | div;
				bestfreq = fmul;
			}
		/*                    */
		fmul = fdiv * 144;
		if ((fdiv >= 390000) && (fdiv <= 667000))
			if (!max310x_update_best_err(fmul, &besterr)) {
				pllcfg = (3 << 6) | div;
				bestfreq = fmul;
			}
	}

	/*                        */
	if (s->pdata->driver_flags & MAX310X_EXT_CLK)
		clksrc = MAX310X_CLKSRC_EXTCLK_BIT;
	else
		clksrc = MAX310X_CLKSRC_CRYST_BIT;

	/*               */
	if (pllcfg) {
		clksrc |= MAX310X_CLKSRC_PLL_BIT;
		regmap_write(s->regmap, MAX310X_PLLCFG_REG, pllcfg);
	} else
		clksrc |= MAX310X_CLKSRC_PLLBYP_BIT;

	regmap_write(s->regmap, MAX310X_CLKSRC_REG, clksrc);

	if (pllcfg)
		max310x_wait_pll(s);

	dev_dbg(s->port.dev, "Reference clock set to %lu Hz\n", bestfreq);

	return (int)bestfreq;
}

static void max310x_handle_rx(struct max310x_port *s, unsigned int rxlen)
{
	unsigned int sts = 0, ch = 0, flag;

	if (unlikely(rxlen >= MAX310X_FIFO_SIZE)) {
		dev_warn(s->port.dev, "Possible RX FIFO overrun %d\n", rxlen);
		/*                           */
		rxlen = MAX310X_FIFO_SIZE;
	}

	dev_dbg(s->port.dev, "RX Len = %u\n", rxlen);

	while (rxlen--) {
		regmap_read(s->regmap, MAX310X_RHR_REG, &ch);
		regmap_read(s->regmap, MAX310X_LSR_IRQSTS_REG, &sts);

		sts &= MAX310X_LSR_RXPAR_BIT | MAX310X_LSR_FRERR_BIT |
		       MAX310X_LSR_RXOVR_BIT | MAX310X_LSR_RXBRK_BIT;

		s->port.icount.rx++;
		flag = TTY_NORMAL;

		if (unlikely(sts)) {
			if (sts & MAX310X_LSR_RXBRK_BIT) {
				s->port.icount.brk++;
				if (uart_handle_break(&s->port))
					continue;
			} else if (sts & MAX310X_LSR_RXPAR_BIT)
				s->port.icount.parity++;
			else if (sts & MAX310X_LSR_FRERR_BIT)
				s->port.icount.frame++;
			else if (sts & MAX310X_LSR_RXOVR_BIT)
				s->port.icount.overrun++;

			sts &= s->port.read_status_mask;
			if (sts & MAX310X_LSR_RXBRK_BIT)
				flag = TTY_BREAK;
			else if (sts & MAX310X_LSR_RXPAR_BIT)
				flag = TTY_PARITY;
			else if (sts & MAX310X_LSR_FRERR_BIT)
				flag = TTY_FRAME;
			else if (sts & MAX310X_LSR_RXOVR_BIT)
				flag = TTY_OVERRUN;
		}

		if (uart_handle_sysrq_char(s->port, ch))
			continue;

		if (sts & s->port.ignore_status_mask)
			continue;

		uart_insert_char(&s->port, sts, MAX310X_LSR_RXOVR_BIT,
				 ch, flag);
	}

	tty_flip_buffer_push(&s->port.state->port);
}

static void max310x_handle_tx(struct max310x_port *s)
{
	struct circ_buf *xmit = &s->port.state->xmit;
	unsigned int txlen = 0, to_send;

	if (unlikely(s->port.x_char)) {
		regmap_write(s->regmap, MAX310X_THR_REG, s->port.x_char);
		s->port.icount.tx++;
		s->port.x_char = 0;
		return;
	}

	if (uart_circ_empty(xmit) || uart_tx_stopped(&s->port))
		return;

	/*                                               */
	to_send = uart_circ_chars_pending(xmit);
	if (likely(to_send)) {
		/*                          */
		regmap_read(s->regmap, MAX310X_TXFIFOLVL_REG, &txlen);
		txlen = MAX310X_FIFO_SIZE - txlen;
		to_send = (to_send > txlen) ? txlen : to_send;

		dev_dbg(s->port.dev, "TX Len = %u\n", to_send);

		/*                  */
		s->port.icount.tx += to_send;
		while (to_send--) {
			regmap_write(s->regmap, MAX310X_THR_REG,
				     xmit->buf[xmit->tail]);
			xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		};
	}

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&s->port);
}

static irqreturn_t max310x_ist(int irq, void *dev_id)
{
	struct max310x_port *s = (struct max310x_port *)dev_id;
	unsigned int ists = 0, lsr = 0, rxlen = 0;

	mutex_lock(&s->max310x_mutex);

	for (;;) {
		/*                                 */
		regmap_read(s->regmap, MAX310X_IRQSTS_REG, &ists);
		regmap_read(s->regmap, MAX310X_LSR_IRQSTS_REG, &lsr);
		regmap_read(s->regmap, MAX310X_RXFIFOLVL_REG, &rxlen);
		if (!ists && !(lsr & MAX310X_LSR_RXTO_BIT) && !rxlen)
			break;

		dev_dbg(s->port.dev, "IRQ status: 0x%02x\n", ists);

		if (rxlen)
			max310x_handle_rx(s, rxlen);
		if (ists & MAX310X_IRQ_TX)
			max310x_handle_tx(s);
		if (ists & MAX310X_IRQ_CTS_BIT)
			uart_handle_cts_change(&s->port,
					       !!(lsr & MAX310X_LSR_CTS_BIT));
	}

	mutex_unlock(&s->max310x_mutex);

	return IRQ_HANDLED;
}

static void max310x_wq_proc(struct work_struct *ws)
{
	struct max310x_port *s = container_of(ws, struct max310x_port, tx_work);

	mutex_lock(&s->max310x_mutex);
	max310x_handle_tx(s);
	mutex_unlock(&s->max310x_mutex);
}

static void max310x_start_tx(struct uart_port *port)
{
	struct max310x_port *s = container_of(port, struct max310x_port, port);

	queue_work(s->wq, &s->tx_work);
}

static void max310x_stop_tx(struct uart_port *port)
{
	/*            */
}

static void max310x_stop_rx(struct uart_port *port)
{
	/*            */
}

static unsigned int max310x_tx_empty(struct uart_port *port)
{
	unsigned int val = 0;
	struct max310x_port *s = container_of(port, struct max310x_port, port);

	mutex_lock(&s->max310x_mutex);
	regmap_read(s->regmap, MAX310X_TXFIFOLVL_REG, &val);
	mutex_unlock(&s->max310x_mutex);

	return val ? 0 : TIOCSER_TEMT;
}

static void max310x_enable_ms(struct uart_port *port)
{
	/*                            */
}

static unsigned int max310x_get_mctrl(struct uart_port *port)
{
	/*                                                               
                                         
  */
	return TIOCM_DSR | TIOCM_CAR;
}

static void max310x_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	/*                                                               
                 
  */
}

static void max310x_break_ctl(struct uart_port *port, int break_state)
{
	struct max310x_port *s = container_of(port, struct max310x_port, port);

	mutex_lock(&s->max310x_mutex);
	regmap_update_bits(s->regmap, MAX310X_LCR_REG,
			   MAX310X_LCR_TXBREAK_BIT,
			   break_state ? MAX310X_LCR_TXBREAK_BIT : 0);
	mutex_unlock(&s->max310x_mutex);
}

static void max310x_set_termios(struct uart_port *port,
				struct ktermios *termios,
				struct ktermios *old)
{
	struct max310x_port *s = container_of(port, struct max310x_port, port);
	unsigned int lcr, flow = 0;
	int baud;

	mutex_lock(&s->max310x_mutex);

	/*                                            */
	termios->c_cflag &= ~CMSPAR;
	termios->c_iflag &= ~IXANY;

	/*           */
	switch (termios->c_cflag & CSIZE) {
	case CS5:
		lcr = MAX310X_LCR_WORD_LEN_5;
		break;
	case CS6:
		lcr = MAX310X_LCR_WORD_LEN_6;
		break;
	case CS7:
		lcr = MAX310X_LCR_WORD_LEN_7;
		break;
	case CS8:
	default:
		lcr = MAX310X_LCR_WORD_LEN_8;
		break;
	}

	/*        */
	if (termios->c_cflag & PARENB) {
		lcr |= MAX310X_LCR_PARITY_BIT;
		if (!(termios->c_cflag & PARODD))
			lcr |= MAX310X_LCR_EVENPARITY_BIT;
	}

	/*           */
	if (termios->c_cflag & CSTOPB)
		lcr |= MAX310X_LCR_STOPLEN_BIT; /*         */

	/*                     */
	regmap_write(s->regmap, MAX310X_LCR_REG, lcr);

	/*                      */
	port->read_status_mask = MAX310X_LSR_RXOVR_BIT;
	if (termios->c_iflag & INPCK)
		port->read_status_mask |= MAX310X_LSR_RXPAR_BIT |
					  MAX310X_LSR_FRERR_BIT;
	if (termios->c_iflag & (BRKINT | PARMRK))
		port->read_status_mask |= MAX310X_LSR_RXBRK_BIT;

	/*                        */
	port->ignore_status_mask = 0;
	if (termios->c_iflag & IGNBRK)
		port->ignore_status_mask |= MAX310X_LSR_RXBRK_BIT;
	if (!(termios->c_cflag & CREAD))
		port->ignore_status_mask |= MAX310X_LSR_RXPAR_BIT |
					    MAX310X_LSR_RXOVR_BIT |
					    MAX310X_LSR_FRERR_BIT |
					    MAX310X_LSR_RXBRK_BIT;

	/*                        */
	regmap_write(s->regmap, MAX310X_XON1_REG, termios->c_cc[VSTART]);
	regmap_write(s->regmap, MAX310X_XOFF1_REG, termios->c_cc[VSTOP]);
	if (termios->c_cflag & CRTSCTS)
		flow |= MAX310X_FLOWCTRL_AUTOCTS_BIT |
			MAX310X_FLOWCTRL_AUTORTS_BIT;
	if (termios->c_iflag & IXON)
		flow |= MAX310X_FLOWCTRL_SWFLOW3_BIT |
			MAX310X_FLOWCTRL_SWFLOWEN_BIT;
	if (termios->c_iflag & IXOFF)
		flow |= MAX310X_FLOWCTRL_SWFLOW1_BIT |
			MAX310X_FLOWCTRL_SWFLOWEN_BIT;
	regmap_write(s->regmap, MAX310X_FLOWCTRL_REG, flow);

	/*                                       */
	baud = uart_get_baud_rate(port, termios, old,
				  port->uartclk / 16 / 0xffff,
				  port->uartclk / 4);

	/*                          */
	max310x_set_baud(s, baud);

	/*                                           */
	uart_update_timeout(port, termios->c_cflag, baud);

	mutex_unlock(&s->max310x_mutex);
}

static int max310x_startup(struct uart_port *port)
{
	unsigned int val, line = port->line;
	struct max310x_port *s = container_of(port, struct max310x_port, port);

	if (s->pdata->suspend)
		s->pdata->suspend(0);

	mutex_lock(&s->max310x_mutex);

	/*                                      */
	max310x_set_baud(s, 9600);

	/*                                             */
	val = MAX310X_LCR_WORD_LEN_8;
	regmap_write(s->regmap, MAX310X_LCR_REG, val);

	/*                          */
	regmap_update_bits(s->regmap, MAX310X_MODE1_REG,
			   MAX310X_MODE1_TRNSCVCTRL_BIT,
			   (s->pdata->uart_flags[line] & MAX310X_AUTO_DIR_CTRL)
			   ? MAX310X_MODE1_TRNSCVCTRL_BIT : 0);

	/*                          */
	val = MAX310X_MODE2_RXEMPTINV_BIT;
	if (s->pdata->uart_flags[line] & MAX310X_LOOPBACK)
		val |= MAX310X_MODE2_LOOPBACK_BIT;
	if (s->pdata->uart_flags[line] & MAX310X_ECHO_SUPRESS)
		val |= MAX310X_MODE2_ECHOSUPR_BIT;

	/*             */
	val |= MAX310X_MODE2_FIFORST_BIT;
	regmap_write(s->regmap, MAX310X_MODE2_REG, val);

	/*                                       */
	/*                                                            */
	val = MAX310X_FIFOTRIGLVL_RX(16) | MAX310X_FIFOTRIGLVL_TX(64);
	regmap_write(s->regmap, MAX310X_FIFOTRIGLVL_REG, val);

	/*                               */
	/*                                             */
	val = MAX310X_FLOWLVL_RES(48) | MAX310X_FLOWLVL_HALT(96);
	regmap_write(s->regmap, MAX310X_FLOWLVL_REG, val);

	/*                        */
	regmap_write(s->regmap, MAX310X_RXTO_REG, 0);

	/*                                         */
	/*                             */
	val = MAX310X_LSR_RXTO_BIT;
	regmap_write(s->regmap, MAX310X_LSR_IRQEN_REG, val);

	/*                  */
	regmap_update_bits(s->regmap, MAX310X_MODE2_REG,
			   MAX310X_MODE2_FIFORST_BIT, 0);

	/*                                         */
	regmap_read(s->regmap, MAX310X_IRQSTS_REG, &val);

	/*                                     */
	/*                             */
	val = MAX310X_IRQ_CTS_BIT;
	/*                          */
	val |= MAX310X_IRQ_RX | MAX310X_IRQ_TX;
	regmap_write(s->regmap, MAX310X_IRQEN_REG, val);

	mutex_unlock(&s->max310x_mutex);

	return 0;
}

static void max310x_shutdown(struct uart_port *port)
{
	struct max310x_port *s = container_of(port, struct max310x_port, port);

	/*                        */
	mutex_lock(&s->max310x_mutex);
	regmap_write(s->regmap, MAX310X_IRQEN_REG, 0);
	mutex_unlock(&s->max310x_mutex);

	if (s->pdata->suspend)
		s->pdata->suspend(1);
}

static const char *max310x_type(struct uart_port *port)
{
	struct max310x_port *s = container_of(port, struct max310x_port, port);

	return (port->type == PORT_MAX310X) ? s->name : NULL;
}

static int max310x_request_port(struct uart_port *port)
{
	/*            */
	return 0;
}

static void max310x_release_port(struct uart_port *port)
{
	/*            */
}

static void max310x_config_port(struct uart_port *port, int flags)
{
	if (flags & UART_CONFIG_TYPE)
		port->type = PORT_MAX310X;
}

static int max310x_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if ((ser->type == PORT_UNKNOWN) || (ser->type == PORT_MAX310X))
		return 0;
	if (ser->irq == port->irq)
		return 0;

	return -EINVAL;
}

static struct uart_ops max310x_ops = {
	.tx_empty	= max310x_tx_empty,
	.set_mctrl	= max310x_set_mctrl,
	.get_mctrl	= max310x_get_mctrl,
	.stop_tx	= max310x_stop_tx,
	.start_tx	= max310x_start_tx,
	.stop_rx	= max310x_stop_rx,
	.enable_ms	= max310x_enable_ms,
	.break_ctl	= max310x_break_ctl,
	.startup	= max310x_startup,
	.shutdown	= max310x_shutdown,
	.set_termios	= max310x_set_termios,
	.type		= max310x_type,
	.request_port	= max310x_request_port,
	.release_port	= max310x_release_port,
	.config_port	= max310x_config_port,
	.verify_port	= max310x_verify_port,
};

#ifdef CONFIG_PM_SLEEP

static int max310x_suspend(struct device *dev)
{
	int ret;
	struct max310x_port *s = dev_get_drvdata(dev);

	dev_dbg(dev, "Suspend\n");

	ret = uart_suspend_port(&s->uart, &s->port);

	mutex_lock(&s->max310x_mutex);

	/*                   */
	regmap_update_bits(s->regmap, MAX310X_MODE1_REG,
			   MAX310X_MODE1_FORCESLEEP_BIT,
			   MAX310X_MODE1_FORCESLEEP_BIT);

	mutex_unlock(&s->max310x_mutex);

	if (s->pdata->suspend)
		s->pdata->suspend(1);

	return ret;
}

static int max310x_resume(struct device *dev)
{
	struct max310x_port *s = dev_get_drvdata(dev);

	dev_dbg(dev, "Resume\n");

	if (s->pdata->suspend)
		s->pdata->suspend(0);

	mutex_lock(&s->max310x_mutex);

	/*                    */
	regmap_update_bits(s->regmap, MAX310X_MODE1_REG,
			   MAX310X_MODE1_FORCESLEEP_BIT,
			   0);

	max310x_wait_pll(s);

	mutex_unlock(&s->max310x_mutex);

	return uart_resume_port(&s->uart, &s->port);
}

static SIMPLE_DEV_PM_OPS(max310x_pm_ops, max310x_suspend, max310x_resume);
#define MAX310X_PM_OPS (&max310x_pm_ops)

#else
#define MAX310X_PM_OPS NULL
#endif

#ifdef CONFIG_GPIOLIB
static int max310x_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	unsigned int val = 0;
	struct max310x_port *s = container_of(chip, struct max310x_port, gpio);

	mutex_lock(&s->max310x_mutex);
	regmap_read(s->regmap, MAX310X_GPIODATA_REG, &val);
	mutex_unlock(&s->max310x_mutex);

	return !!((val >> 4) & (1 << offset));
}

static void max310x_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct max310x_port *s = container_of(chip, struct max310x_port, gpio);

	mutex_lock(&s->max310x_mutex);
	regmap_update_bits(s->regmap, MAX310X_GPIODATA_REG, 1 << offset, value ?
							    1 << offset : 0);
	mutex_unlock(&s->max310x_mutex);
}

static int max310x_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct max310x_port *s = container_of(chip, struct max310x_port, gpio);

	mutex_lock(&s->max310x_mutex);

	regmap_update_bits(s->regmap, MAX310X_GPIOCFG_REG, 1 << offset, 0);

	mutex_unlock(&s->max310x_mutex);

	return 0;
}

static int max310x_gpio_direction_output(struct gpio_chip *chip,
					 unsigned offset, int value)
{
	struct max310x_port *s = container_of(chip, struct max310x_port, gpio);

	mutex_lock(&s->max310x_mutex);

	regmap_update_bits(s->regmap, MAX310X_GPIOCFG_REG, 1 << offset,
							   1 << offset);
	regmap_update_bits(s->regmap, MAX310X_GPIODATA_REG, 1 << offset, value ?
							    1 << offset : 0);

	mutex_unlock(&s->max310x_mutex);

	return 0;
}
#endif

/*                       */
static struct max310x_pdata generic_plat_data = {
	.driver_flags	= MAX310X_EXT_CLK,
	.uart_flags[0]	= MAX310X_ECHO_SUPRESS,
	.frequency	= 26000000,
};

static int max310x_probe(struct spi_device *spi)
{
	struct max310x_port *s;
	struct device *dev = &spi->dev;
	int chiptype = spi_get_device_id(spi)->driver_data;
	struct max310x_pdata *pdata = dev->platform_data;
	unsigned int val = 0;
	int ret;

	/*               */
	if (spi->irq <= 0) {
		dev_err(dev, "No IRQ specified\n");
		return -ENOTSUPP;
	}

	/*                      */
	s = devm_kzalloc(dev, sizeof(struct max310x_port), GFP_KERNEL);
	if (!s) {
		dev_err(dev, "Error allocating port structure\n");
		return -ENOMEM;
	}
	dev_set_drvdata(dev, s);

	if (!pdata) {
		dev_warn(dev, "No platform data supplied, using defaults\n");
		pdata = &generic_plat_data;
	}
	s->pdata = pdata;

	/*                          */
	switch (chiptype) {
	case MAX310X_TYPE_MAX3107:
		s->name = "MAX3107";
		s->nr_gpio = 4;
		s->uart.nr = 1;
		s->regcfg.max_register = 0x1f;
		break;
	case MAX310X_TYPE_MAX3108:
		s->name = "MAX3108";
		s->nr_gpio = 4;
		s->uart.nr = 1;
		s->regcfg.max_register = 0x1e;
		break;
	default:
		dev_err(dev, "Unsupported chip type %i\n", chiptype);
		return -ENOTSUPP;
	}

	/*                       */
	if ((pdata->driver_flags & MAX310X_EXT_CLK) &&
	   ((pdata->frequency < 500000) || (pdata->frequency > 35000000)))
		goto err_freq;
	/*                            */
	if (!(pdata->driver_flags & MAX310X_EXT_CLK) &&
	   ((pdata->frequency < 1000000) || (pdata->frequency > 4000000)))
		goto err_freq;

	mutex_init(&s->max310x_mutex);

	/*               */
	spi->mode		= SPI_MODE_0;
	spi->bits_per_word	= 8;
	spi->max_speed_hz	= 26000000;
	spi_setup(spi);

	/*              */
	s->regcfg.reg_bits		= 8;
	s->regcfg.val_bits		= 8;
	s->regcfg.read_flag_mask	= 0x00;
	s->regcfg.write_flag_mask	= 0x80;
	s->regcfg.cache_type		= REGCACHE_RBTREE;
	s->regcfg.writeable_reg		= max3107_8_reg_writeable;
	s->regcfg.volatile_reg		= max310x_reg_volatile;
	s->regcfg.precious_reg		= max310x_reg_precious;
	s->regmap = devm_regmap_init_spi(spi, &s->regcfg);
	if (IS_ERR(s->regmap)) {
		ret = PTR_ERR(s->regmap);
		dev_err(dev, "Failed to initialize register map\n");
		goto err_out;
	}

	/*                                 */
	ret = regmap_write(s->regmap, MAX310X_MODE2_REG, MAX310X_MODE2_RST_BIT);
	if (ret) {
		dev_err(dev, "SPI transfer failed\n");
		goto err_out;
	}
	/*                  */
	regmap_write(s->regmap, MAX310X_MODE2_REG, 0);

	switch (chiptype) {
	case MAX310X_TYPE_MAX3107:
		/*                                                         */
		regmap_read(s->regmap, MAX3107_REVID_REG, &val);
		if (((val & MAX3107_REV_MASK) != MAX3107_REV_ID)) {
			dev_err(dev, "%s ID 0x%02x does not match\n",
				s->name, val);
			ret = -ENODEV;
			goto err_out;
		}
		break;
	case MAX310X_TYPE_MAX3108:
		/*                                                              
                                                             
   */
		regmap_read(s->regmap, MAX310X_CLKSRC_REG, &val);
		if (val != (MAX310X_CLKSRC_EXTCLK_BIT |
			    MAX310X_CLKSRC_PLLBYP_BIT)) {
			dev_err(dev, "%s not present\n", s->name);
			ret = -ENODEV;
			goto err_out;
		}
		break;
	}

	/*                          */
	if (pdata->init)
		pdata->init();
	if (pdata->suspend)
		pdata->suspend(0);

	/*                           */
	s->uartclk = max310x_set_ref_clk(s);

	/*                        */
	regmap_write(s->regmap, MAX310X_IRQEN_REG, 0);

	/*                      */
	val = MAX310X_MODE1_IRQSEL_BIT; /*                */
	if (pdata->driver_flags & MAX310X_AUTOSLEEP)
		val = MAX310X_MODE1_AUTOSLEEP_BIT;
	regmap_write(s->regmap, MAX310X_MODE1_REG, val);

	/*                 */
	ret = devm_request_threaded_irq(dev, spi->irq, NULL, max310x_ist,
					IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					dev_name(dev), s);
	if (ret) {
		dev_err(dev, "Unable to reguest IRQ %i\n", spi->irq);
		goto err_out;
	}

	/*                      */
	s->uart.owner		= THIS_MODULE;
	s->uart.driver_name	= dev_name(dev);
	s->uart.dev_name	= "ttyMAX";
	s->uart.major		= MAX310X_MAJOR;
	s->uart.minor		= MAX310X_MINOR;
	ret = uart_register_driver(&s->uart);
	if (ret) {
		dev_err(dev, "Registering UART driver failed\n");
		goto err_out;
	}

	/*                                   */
	s->wq = create_freezable_workqueue(dev_name(dev));
	INIT_WORK(&s->tx_work, max310x_wq_proc);

	/*                           */
	s->port.line		= 0;
	s->port.dev		= dev;
	s->port.irq		= spi->irq;
	s->port.type		= PORT_MAX310X;
	s->port.fifosize	= MAX310X_FIFO_SIZE;
	s->port.flags		= UPF_SKIP_TEST | UPF_FIXED_TYPE;
	s->port.iotype		= UPIO_PORT;
	s->port.membase		= (void __iomem *)0xffffffff; /*             */
	s->port.uartclk		= s->uartclk;
	s->port.ops		= &max310x_ops;
	uart_add_one_port(&s->uart, &s->port);

#ifdef CONFIG_GPIOLIB
	/*                      */
	if (pdata->gpio_base) {
		s->gpio.owner		= THIS_MODULE;
		s->gpio.dev		= dev;
		s->gpio.label		= dev_name(dev);
		s->gpio.direction_input	= max310x_gpio_direction_input;
		s->gpio.get		= max310x_gpio_get;
		s->gpio.direction_output= max310x_gpio_direction_output;
		s->gpio.set		= max310x_gpio_set;
		s->gpio.base		= pdata->gpio_base;
		s->gpio.ngpio		= s->nr_gpio;
		s->gpio.can_sleep	= 1;
		if (gpiochip_add(&s->gpio)) {
			/*                                                  */
			s->gpio.base = 0;
		}
	} else
		dev_info(dev, "GPIO support not enabled\n");
#endif

	/*                    */
	if (pdata->suspend)
		pdata->suspend(1);

	return 0;

err_freq:
	dev_err(dev, "Frequency parameter incorrect\n");
	ret = -EINVAL;

err_out:
	dev_set_drvdata(dev, NULL);

	return ret;
}

static int max310x_remove(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct max310x_port *s = dev_get_drvdata(dev);
	int ret = 0;

	dev_dbg(dev, "Removing port\n");

	devm_free_irq(dev, s->port.irq, s);

	destroy_workqueue(s->wq);

	uart_remove_one_port(&s->uart, &s->port);

	uart_unregister_driver(&s->uart);

#ifdef CONFIG_GPIOLIB
	if (s->pdata->gpio_base) {
		ret = gpiochip_remove(&s->gpio);
		if (ret)
			dev_err(dev, "Failed to remove gpio chip: %d\n", ret);
	}
#endif

	dev_set_drvdata(dev, NULL);

	if (s->pdata->suspend)
		s->pdata->suspend(1);
	if (s->pdata->exit)
		s->pdata->exit();

	return ret;
}

static const struct spi_device_id max310x_id_table[] = {
	{ "max3107",	MAX310X_TYPE_MAX3107 },
	{ "max3108",	MAX310X_TYPE_MAX3108 },
	{ }
};
MODULE_DEVICE_TABLE(spi, max310x_id_table);

static struct spi_driver max310x_driver = {
	.driver = {
		.name	= "max310x",
		.owner	= THIS_MODULE,
		.pm	= MAX310X_PM_OPS,
	},
	.probe		= max310x_probe,
	.remove		= max310x_remove,
	.id_table	= max310x_id_table,
};
module_spi_driver(max310x_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Alexander Shiyan <shc_work@mail.ru>");
MODULE_DESCRIPTION("MAX310X serial driver");
