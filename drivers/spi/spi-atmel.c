/*
 * Driver for Atmel AT32 and AT91 SPI Controllers
 *
 * Copyright (C) 2006 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>
#include <linux/platform_data/atmel.h>
#include <linux/platform_data/dma-atmel.h>
#include <linux/of.h>

#include <linux/io.h>
#include <linux/gpio.h>

/*                      */
#define SPI_CR					0x0000
#define SPI_MR					0x0004
#define SPI_RDR					0x0008
#define SPI_TDR					0x000c
#define SPI_SR					0x0010
#define SPI_IER					0x0014
#define SPI_IDR					0x0018
#define SPI_IMR					0x001c
#define SPI_CSR0				0x0030
#define SPI_CSR1				0x0034
#define SPI_CSR2				0x0038
#define SPI_CSR3				0x003c
#define SPI_VERSION				0x00fc
#define SPI_RPR					0x0100
#define SPI_RCR					0x0104
#define SPI_TPR					0x0108
#define SPI_TCR					0x010c
#define SPI_RNPR				0x0110
#define SPI_RNCR				0x0114
#define SPI_TNPR				0x0118
#define SPI_TNCR				0x011c
#define SPI_PTCR				0x0120
#define SPI_PTSR				0x0124

/*                 */
#define SPI_SPIEN_OFFSET			0
#define SPI_SPIEN_SIZE				1
#define SPI_SPIDIS_OFFSET			1
#define SPI_SPIDIS_SIZE				1
#define SPI_SWRST_OFFSET			7
#define SPI_SWRST_SIZE				1
#define SPI_LASTXFER_OFFSET			24
#define SPI_LASTXFER_SIZE			1

/*                 */
#define SPI_MSTR_OFFSET				0
#define SPI_MSTR_SIZE				1
#define SPI_PS_OFFSET				1
#define SPI_PS_SIZE				1
#define SPI_PCSDEC_OFFSET			2
#define SPI_PCSDEC_SIZE				1
#define SPI_FDIV_OFFSET				3
#define SPI_FDIV_SIZE				1
#define SPI_MODFDIS_OFFSET			4
#define SPI_MODFDIS_SIZE			1
#define SPI_WDRBT_OFFSET			5
#define SPI_WDRBT_SIZE				1
#define SPI_LLB_OFFSET				7
#define SPI_LLB_SIZE				1
#define SPI_PCS_OFFSET				16
#define SPI_PCS_SIZE				4
#define SPI_DLYBCS_OFFSET			24
#define SPI_DLYBCS_SIZE				8

/*                  */
#define SPI_RD_OFFSET				0
#define SPI_RD_SIZE				16

/*                  */
#define SPI_TD_OFFSET				0
#define SPI_TD_SIZE				16

/*                 */
#define SPI_RDRF_OFFSET				0
#define SPI_RDRF_SIZE				1
#define SPI_TDRE_OFFSET				1
#define SPI_TDRE_SIZE				1
#define SPI_MODF_OFFSET				2
#define SPI_MODF_SIZE				1
#define SPI_OVRES_OFFSET			3
#define SPI_OVRES_SIZE				1
#define SPI_ENDRX_OFFSET			4
#define SPI_ENDRX_SIZE				1
#define SPI_ENDTX_OFFSET			5
#define SPI_ENDTX_SIZE				1
#define SPI_RXBUFF_OFFSET			6
#define SPI_RXBUFF_SIZE				1
#define SPI_TXBUFE_OFFSET			7
#define SPI_TXBUFE_SIZE				1
#define SPI_NSSR_OFFSET				8
#define SPI_NSSR_SIZE				1
#define SPI_TXEMPTY_OFFSET			9
#define SPI_TXEMPTY_SIZE			1
#define SPI_SPIENS_OFFSET			16
#define SPI_SPIENS_SIZE				1

/*                   */
#define SPI_CPOL_OFFSET				0
#define SPI_CPOL_SIZE				1
#define SPI_NCPHA_OFFSET			1
#define SPI_NCPHA_SIZE				1
#define SPI_CSAAT_OFFSET			3
#define SPI_CSAAT_SIZE				1
#define SPI_BITS_OFFSET				4
#define SPI_BITS_SIZE				4
#define SPI_SCBR_OFFSET				8
#define SPI_SCBR_SIZE				8
#define SPI_DLYBS_OFFSET			16
#define SPI_DLYBS_SIZE				8
#define SPI_DLYBCT_OFFSET			24
#define SPI_DLYBCT_SIZE				8

/*                  */
#define SPI_RXCTR_OFFSET			0
#define SPI_RXCTR_SIZE				16

/*                  */
#define SPI_TXCTR_OFFSET			0
#define SPI_TXCTR_SIZE				16

/*                   */
#define SPI_RXNCR_OFFSET			0
#define SPI_RXNCR_SIZE				16

/*                   */
#define SPI_TXNCR_OFFSET			0
#define SPI_TXNCR_SIZE				16

/*                   */
#define SPI_RXTEN_OFFSET			0
#define SPI_RXTEN_SIZE				1
#define SPI_RXTDIS_OFFSET			1
#define SPI_RXTDIS_SIZE				1
#define SPI_TXTEN_OFFSET			8
#define SPI_TXTEN_SIZE				1
#define SPI_TXTDIS_OFFSET			9
#define SPI_TXTDIS_SIZE				1

/*                    */
#define SPI_BITS_8_BPT				0
#define SPI_BITS_9_BPT				1
#define SPI_BITS_10_BPT				2
#define SPI_BITS_11_BPT				3
#define SPI_BITS_12_BPT				4
#define SPI_BITS_13_BPT				5
#define SPI_BITS_14_BPT				6
#define SPI_BITS_15_BPT				7
#define SPI_BITS_16_BPT				8

/*                         */
#define SPI_BIT(name) \
	(1 << SPI_##name##_OFFSET)
#define SPI_BF(name,value) \
	(((value) & ((1 << SPI_##name##_SIZE) - 1)) << SPI_##name##_OFFSET)
#define SPI_BFEXT(name,value) \
	(((value) >> SPI_##name##_OFFSET) & ((1 << SPI_##name##_SIZE) - 1))
#define SPI_BFINS(name,value,old) \
	( ((old) & ~(((1 << SPI_##name##_SIZE) - 1) << SPI_##name##_OFFSET)) \
	  | SPI_BF(name,value))

/*                        */
#define spi_readl(port,reg) \
	__raw_readl((port)->regs + SPI_##reg)
#define spi_writel(port,reg,value) \
	__raw_writel((value), (port)->regs + SPI_##reg)

/*                                                                      
                                                                     
 */
#define DMA_MIN_BYTES	16

struct atmel_spi_dma {
	struct dma_chan			*chan_rx;
	struct dma_chan			*chan_tx;
	struct scatterlist		sgrx;
	struct scatterlist		sgtx;
	struct dma_async_tx_descriptor	*data_desc_rx;
	struct dma_async_tx_descriptor	*data_desc_tx;

	struct at_dma_slave	dma_slave;
};

struct atmel_spi_caps {
	bool	is_spi2;
	bool	has_wdrbt;
	bool	has_dma_support;
};

/*
                                                                       
                                                                       
                                                                     
 */
struct atmel_spi {
	spinlock_t		lock;
	unsigned long		flags;

	phys_addr_t		phybase;
	void __iomem		*regs;
	int			irq;
	struct clk		*clk;
	struct platform_device	*pdev;
	struct spi_device	*stay;

	u8			stopping;
	struct list_head	queue;
	struct tasklet_struct	tasklet;
	struct spi_transfer	*current_transfer;
	unsigned long		current_remaining_bytes;
	struct spi_transfer	*next_transfer;
	unsigned long		next_remaining_bytes;
	int			done_status;

	/*                */
	void			*buffer;
	dma_addr_t		buffer_dma;

	struct atmel_spi_caps	caps;

	bool			use_dma;
	bool			use_pdc;
	/*                */
	struct atmel_spi_dma	dma;
};

/*                                     */
struct atmel_spi_device {
	unsigned int		npcs_pin;
	u32			csr;
};

#define BUFFER_SIZE		PAGE_SIZE
#define INVALID_DMA_ADDRESS	0xffffffff

/*
                                      
                 
                                                                      
                                                          
                    
                                          
 */
static bool atmel_spi_is_v2(struct atmel_spi *as)
{
	return as->caps.is_spi2;
}

/*
                                                                         
                                                                          
                                                                       
                                                                       
                                      
  
                                                                       
                                                                      
                                                                      
                                                                       
                                          
  
                                                                     
                                                                          
                                                                       
                                                                           
                                                         
 */

static void cs_activate(struct atmel_spi *as, struct spi_device *spi)
{
	struct atmel_spi_device *asd = spi->controller_state;
	unsigned active = spi->mode & SPI_CS_HIGH;
	u32 mr;

	if (atmel_spi_is_v2(as)) {
		spi_writel(as, CSR0 + 4 * spi->chip_select, asd->csr);
		/*                                                            
                                                               
   */
		spi_writel(as, CSR0, asd->csr);
		if (as->caps.has_wdrbt) {
			spi_writel(as, MR,
					SPI_BF(PCS, ~(0x01 << spi->chip_select))
					| SPI_BIT(WDRBT)
					| SPI_BIT(MODFDIS)
					| SPI_BIT(MSTR));
		} else {
			spi_writel(as, MR,
					SPI_BF(PCS, ~(0x01 << spi->chip_select))
					| SPI_BIT(MODFDIS)
					| SPI_BIT(MSTR));
		}

		mr = spi_readl(as, MR);
		gpio_set_value(asd->npcs_pin, active);
	} else {
		u32 cpol = (spi->mode & SPI_CPOL) ? SPI_BIT(CPOL) : 0;
		int i;
		u32 csr;

		/*                                     */
		for (i = 0; i < spi->master->num_chipselect; i++) {
			csr = spi_readl(as, CSR0 + 4 * i);
			if ((csr ^ cpol) & SPI_BIT(CPOL))
				spi_writel(as, CSR0 + 4 * i,
						csr ^ SPI_BIT(CPOL));
		}

		mr = spi_readl(as, MR);
		mr = SPI_BFINS(PCS, ~(1 << spi->chip_select), mr);
		if (spi->chip_select != 0)
			gpio_set_value(asd->npcs_pin, active);
		spi_writel(as, MR, mr);
	}

	dev_dbg(&spi->dev, "activate %u%s, mr %08x\n",
			asd->npcs_pin, active ? " (high)" : "",
			mr);
}

static void cs_deactivate(struct atmel_spi *as, struct spi_device *spi)
{
	struct atmel_spi_device *asd = spi->controller_state;
	unsigned active = spi->mode & SPI_CS_HIGH;
	u32 mr;

	/*                                                      
                                                             
  */
	mr = spi_readl(as, MR);
	if (~SPI_BFEXT(PCS, mr) & (1 << spi->chip_select)) {
		mr = SPI_BFINS(PCS, 0xf, mr);
		spi_writel(as, MR, mr);
	}

	dev_dbg(&spi->dev, "DEactivate %u%s, mr %08x\n",
			asd->npcs_pin, active ? " (low)" : "",
			mr);

	if (atmel_spi_is_v2(as) || spi->chip_select != 0)
		gpio_set_value(asd->npcs_pin, !active);
}

static void atmel_spi_lock(struct atmel_spi *as)
{
	spin_lock_irqsave(&as->lock, as->flags);
}

static void atmel_spi_unlock(struct atmel_spi *as)
{
	spin_unlock_irqrestore(&as->lock, as->flags);
}

static inline bool atmel_spi_use_dma(struct atmel_spi *as,
				struct spi_transfer *xfer)
{
	return as->use_dma && xfer->len >= DMA_MIN_BYTES;
}

static inline int atmel_spi_xfer_is_last(struct spi_message *msg,
					struct spi_transfer *xfer)
{
	return msg->transfers.prev == &xfer->transfer_list;
}

static inline int atmel_spi_xfer_can_be_chained(struct spi_transfer *xfer)
{
	return xfer->delay_usecs == 0 && !xfer->cs_change;
}

static int atmel_spi_dma_slave_config(struct atmel_spi *as,
				struct dma_slave_config *slave_config,
				u8 bits_per_word)
{
	int err = 0;

	if (bits_per_word > 8) {
		slave_config->dst_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		slave_config->src_addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	} else {
		slave_config->dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
		slave_config->src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	}

	slave_config->dst_addr = (dma_addr_t)as->phybase + SPI_TDR;
	slave_config->src_addr = (dma_addr_t)as->phybase + SPI_RDR;
	slave_config->src_maxburst = 1;
	slave_config->dst_maxburst = 1;
	slave_config->device_fc = false;

	slave_config->direction = DMA_MEM_TO_DEV;
	if (dmaengine_slave_config(as->dma.chan_tx, slave_config)) {
		dev_err(&as->pdev->dev,
			"failed to configure tx dma channel\n");
		err = -EINVAL;
	}

	slave_config->direction = DMA_DEV_TO_MEM;
	if (dmaengine_slave_config(as->dma.chan_rx, slave_config)) {
		dev_err(&as->pdev->dev,
			"failed to configure rx dma channel\n");
		err = -EINVAL;
	}

	return err;
}

static bool filter(struct dma_chan *chan, void *slave)
{
	struct	at_dma_slave *sl = slave;

	if (sl->dma_dev == chan->device->dev) {
		chan->private = sl;
		return true;
	} else {
		return false;
	}
}

static int atmel_spi_configure_dma(struct atmel_spi *as)
{
	struct at_dma_slave *sdata = &as->dma.dma_slave;
	struct dma_slave_config	slave_config;
	int err;

	if (sdata && sdata->dma_dev) {
		dma_cap_mask_t mask;

		/*                              */
		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);
		as->dma.chan_tx = dma_request_channel(mask, filter, sdata);
		if (as->dma.chan_tx)
			as->dma.chan_rx =
				dma_request_channel(mask, filter, sdata);
	}
	if (!as->dma.chan_rx || !as->dma.chan_tx) {
		dev_err(&as->pdev->dev,
			"DMA channel not available, SPI unable to use DMA\n");
		err = -EBUSY;
		goto error;
	}

	err = atmel_spi_dma_slave_config(as, &slave_config, 8);
	if (err)
		goto error;

	dev_info(&as->pdev->dev,
			"Using %s (tx) and %s (rx) for DMA transfers\n",
			dma_chan_name(as->dma.chan_tx),
			dma_chan_name(as->dma.chan_rx));
	return 0;
error:
	if (as->dma.chan_rx)
		dma_release_channel(as->dma.chan_rx);
	if (as->dma.chan_tx)
		dma_release_channel(as->dma.chan_tx);
	return err;
}

static void atmel_spi_stop_dma(struct atmel_spi *as)
{
	if (as->dma.chan_rx)
		as->dma.chan_rx->device->device_control(as->dma.chan_rx,
							DMA_TERMINATE_ALL, 0);
	if (as->dma.chan_tx)
		as->dma.chan_tx->device->device_control(as->dma.chan_tx,
							DMA_TERMINATE_ALL, 0);
}

static void atmel_spi_release_dma(struct atmel_spi *as)
{
	if (as->dma.chan_rx)
		dma_release_channel(as->dma.chan_rx);
	if (as->dma.chan_tx)
		dma_release_channel(as->dma.chan_tx);
}

/*                                                                */
static void dma_callback(void *data)
{
	struct spi_master	*master = data;
	struct atmel_spi	*as = spi_master_get_devdata(master);

	/*                     */
	tasklet_schedule(&as->tasklet);
}

/*
                           
                                       
 */
static void atmel_spi_next_xfer_pio(struct spi_master *master,
				struct spi_transfer *xfer)
{
	struct atmel_spi	*as = spi_master_get_devdata(master);

	dev_vdbg(master->dev.parent, "atmel_spi_next_xfer_pio\n");

	as->current_remaining_bytes = xfer->len;

	/*                                        */
	spi_readl(as, RDR);
	while (spi_readl(as, SR) & SPI_BIT(RDRF)) {
		spi_readl(as, RDR);
		cpu_relax();
	}

	if (xfer->tx_buf)
		if (xfer->bits_per_word > 8)
			spi_writel(as, TDR, *(u16 *)(xfer->tx_buf));
		else
			spi_writel(as, TDR, *(u8 *)(xfer->tx_buf));
	else
		spi_writel(as, TDR, 0);

	dev_dbg(master->dev.parent,
		"  start pio xfer %p: len %u tx %p rx %p bitpw %d\n",
		xfer, xfer->len, xfer->tx_buf, xfer->rx_buf,
		xfer->bits_per_word);

	/*                            */
	spi_writel(as, IER, SPI_BIT(RDRF) | SPI_BIT(OVRES));
}

/*
                                
                                       
 */
static int atmel_spi_next_xfer_dma_submit(struct spi_master *master,
				struct spi_transfer *xfer,
				u32 *plen)
{
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct dma_chan		*rxchan = as->dma.chan_rx;
	struct dma_chan		*txchan = as->dma.chan_tx;
	struct dma_async_tx_descriptor *rxdesc;
	struct dma_async_tx_descriptor *txdesc;
	struct dma_slave_config	slave_config;
	dma_cookie_t		cookie;
	u32	len = *plen;

	dev_vdbg(master->dev.parent, "atmel_spi_next_xfer_dma_submit\n");

	/*                                       */
	if (!rxchan || !txchan)
		return -ENODEV;

	/*                                 */
	atmel_spi_unlock(as);

	/*                             */
	sg_init_table(&as->dma.sgrx, 1);
	if (xfer->rx_buf) {
		as->dma.sgrx.dma_address = xfer->rx_dma + xfer->len - *plen;
	} else {
		as->dma.sgrx.dma_address = as->buffer_dma;
		if (len > BUFFER_SIZE)
			len = BUFFER_SIZE;
	}

	/*                             */
	sg_init_table(&as->dma.sgtx, 1);
	if (xfer->tx_buf) {
		as->dma.sgtx.dma_address = xfer->tx_dma + xfer->len - *plen;
	} else {
		as->dma.sgtx.dma_address = as->buffer_dma;
		if (len > BUFFER_SIZE)
			len = BUFFER_SIZE;
		memset(as->buffer, 0, len);
	}

	sg_dma_len(&as->dma.sgtx) = len;
	sg_dma_len(&as->dma.sgrx) = len;

	*plen = len;

	if (atmel_spi_dma_slave_config(as, &slave_config, 8))
		goto err_exit;

	/*                        */
	rxdesc = rxchan->device->device_prep_slave_sg(rxchan,
					&as->dma.sgrx,
					1,
					DMA_FROM_DEVICE,
					DMA_PREP_INTERRUPT | DMA_CTRL_ACK,
					NULL);
	if (!rxdesc)
		goto err_dma;

	txdesc = txchan->device->device_prep_slave_sg(txchan,
					&as->dma.sgtx,
					1,
					DMA_TO_DEVICE,
					DMA_PREP_INTERRUPT | DMA_CTRL_ACK,
					NULL);
	if (!txdesc)
		goto err_dma;

	dev_dbg(master->dev.parent,
		"  start dma xfer %p: len %u tx %p/%08x rx %p/%08x\n",
		xfer, xfer->len, xfer->tx_buf, xfer->tx_dma,
		xfer->rx_buf, xfer->rx_dma);

	/*                            */
	spi_writel(as, IER, SPI_BIT(OVRES));

	/*                                                                   */
	rxdesc->callback = dma_callback;
	rxdesc->callback_param = master;

	/*                                                                */
	cookie = rxdesc->tx_submit(rxdesc);
	if (dma_submit_error(cookie))
		goto err_dma;
	cookie = txdesc->tx_submit(txdesc);
	if (dma_submit_error(cookie))
		goto err_dma;
	rxchan->device->device_issue_pending(rxchan);
	txchan->device->device_issue_pending(txchan);

	/*                */
	atmel_spi_lock(as);
	return 0;

err_dma:
	spi_writel(as, IDR, SPI_BIT(OVRES));
	atmel_spi_stop_dma(as);
err_exit:
	atmel_spi_lock(as);
	return -ENOMEM;
}

static void atmel_spi_next_xfer_data(struct spi_master *master,
				struct spi_transfer *xfer,
				dma_addr_t *tx_dma,
				dma_addr_t *rx_dma,
				u32 *plen)
{
	struct atmel_spi	*as = spi_master_get_devdata(master);
	u32			len = *plen;

	/*                                                           */
	if (xfer->rx_buf)
		*rx_dma = xfer->rx_dma + xfer->len - *plen;
	else {
		*rx_dma = as->buffer_dma;
		if (len > BUFFER_SIZE)
			len = BUFFER_SIZE;
	}

	if (xfer->tx_buf)
		*tx_dma = xfer->tx_dma + xfer->len - *plen;
	else {
		*tx_dma = as->buffer_dma;
		if (len > BUFFER_SIZE)
			len = BUFFER_SIZE;
		memset(as->buffer, 0, len);
		dma_sync_single_for_device(&as->pdev->dev,
				as->buffer_dma, len, DMA_TO_DEVICE);
	}

	*plen = len;
}

/*
                                
                                   
 */
static void atmel_spi_pdc_next_xfer(struct spi_master *master,
				struct spi_message *msg)
{
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct spi_transfer	*xfer;
	u32			len, remaining;
	u32			ieval;
	dma_addr_t		tx_dma, rx_dma;

	if (!as->current_transfer)
		xfer = list_entry(msg->transfers.next,
				struct spi_transfer, transfer_list);
	else if (!as->next_transfer)
		xfer = list_entry(as->current_transfer->transfer_list.next,
				struct spi_transfer, transfer_list);
	else
		xfer = NULL;

	if (xfer) {
		spi_writel(as, PTCR, SPI_BIT(RXTDIS) | SPI_BIT(TXTDIS));

		len = xfer->len;
		atmel_spi_next_xfer_data(master, xfer, &tx_dma, &rx_dma, &len);
		remaining = xfer->len - len;

		spi_writel(as, RPR, rx_dma);
		spi_writel(as, TPR, tx_dma);

		if (msg->spi->bits_per_word > 8)
			len >>= 1;
		spi_writel(as, RCR, len);
		spi_writel(as, TCR, len);

		dev_dbg(&msg->spi->dev,
			"  start xfer %p: len %u tx %p/%08x rx %p/%08x\n",
			xfer, xfer->len, xfer->tx_buf, xfer->tx_dma,
			xfer->rx_buf, xfer->rx_dma);
	} else {
		xfer = as->next_transfer;
		remaining = as->next_remaining_bytes;
	}

	as->current_transfer = xfer;
	as->current_remaining_bytes = remaining;

	if (remaining > 0)
		len = remaining;
	else if (!atmel_spi_xfer_is_last(msg, xfer)
			&& atmel_spi_xfer_can_be_chained(xfer)) {
		xfer = list_entry(xfer->transfer_list.next,
				struct spi_transfer, transfer_list);
		len = xfer->len;
	} else
		xfer = NULL;

	as->next_transfer = xfer;

	if (xfer) {
		u32	total;

		total = len;
		atmel_spi_next_xfer_data(master, xfer, &tx_dma, &rx_dma, &len);
		as->next_remaining_bytes = total - len;

		spi_writel(as, RNPR, rx_dma);
		spi_writel(as, TNPR, tx_dma);

		if (msg->spi->bits_per_word > 8)
			len >>= 1;
		spi_writel(as, RNCR, len);
		spi_writel(as, TNCR, len);

		dev_dbg(&msg->spi->dev,
			"  next xfer %p: len %u tx %p/%08x rx %p/%08x\n",
			xfer, xfer->len, xfer->tx_buf, xfer->tx_dma,
			xfer->rx_buf, xfer->rx_dma);
		ieval = SPI_BIT(ENDRX) | SPI_BIT(OVRES);
	} else {
		spi_writel(as, RNCR, 0);
		spi_writel(as, TNCR, 0);
		ieval = SPI_BIT(RXBUFF) | SPI_BIT(ENDRX) | SPI_BIT(OVRES);
	}

	/*                                                          
                                                            
                                                              
                                                             
                                                            
                                                       
                      
   
                                                
  */
	spi_writel(as, IER, ieval);
	spi_writel(as, PTCR, SPI_BIT(TXTEN) | SPI_BIT(RXTEN));
}

/*
                                                   
                                       
 */
static void atmel_spi_dma_next_xfer(struct spi_master *master,
				struct spi_message *msg)
{
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct spi_transfer	*xfer;
	u32	remaining, len;

	remaining = as->current_remaining_bytes;
	if (remaining) {
		xfer = as->current_transfer;
		len = remaining;
	} else {
		if (!as->current_transfer)
			xfer = list_entry(msg->transfers.next,
				struct spi_transfer, transfer_list);
		else
			xfer = list_entry(
				as->current_transfer->transfer_list.next,
					struct spi_transfer, transfer_list);

		as->current_transfer = xfer;
		len = xfer->len;
	}

	if (atmel_spi_use_dma(as, xfer)) {
		u32 total = len;
		if (!atmel_spi_next_xfer_dma_submit(master, xfer, &len)) {
			as->current_remaining_bytes = total - len;
			return;
		} else {
			dev_err(&msg->spi->dev, "unable to use DMA, fallback to PIO\n");
		}
	}

	/*                                    */
	atmel_spi_next_xfer_pio(master, xfer);
}

static void atmel_spi_next_message(struct spi_master *master)
{
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct spi_message	*msg;
	struct spi_device	*spi;

	BUG_ON(as->current_transfer);

	msg = list_entry(as->queue.next, struct spi_message, queue);
	spi = msg->spi;

	dev_dbg(master->dev.parent, "start message %p for %s\n",
			msg, dev_name(&spi->dev));

	/*                                      */
	if (as->stay) {
		if (as->stay != spi) {
			cs_deactivate(as, as->stay);
			cs_activate(as, spi);
		}
		as->stay = NULL;
	} else
		cs_activate(as, spi);

	if (as->use_pdc)
		atmel_spi_pdc_next_xfer(master, msg);
	else
		atmel_spi_dma_next_xfer(master, msg);
}

/*
                                                                      
                                                          
                                                   
  
                                                                     
 */
static int
atmel_spi_dma_map_xfer(struct atmel_spi *as, struct spi_transfer *xfer)
{
	struct device	*dev = &as->pdev->dev;

	xfer->tx_dma = xfer->rx_dma = INVALID_DMA_ADDRESS;
	if (xfer->tx_buf) {
		/*                                                           
             */
		void *nonconst_tx = (void *)xfer->tx_buf;

		xfer->tx_dma = dma_map_single(dev,
				nonconst_tx, xfer->len,
				DMA_TO_DEVICE);
		if (dma_mapping_error(dev, xfer->tx_dma))
			return -ENOMEM;
	}
	if (xfer->rx_buf) {
		xfer->rx_dma = dma_map_single(dev,
				xfer->rx_buf, xfer->len,
				DMA_FROM_DEVICE);
		if (dma_mapping_error(dev, xfer->rx_dma)) {
			if (xfer->tx_buf)
				dma_unmap_single(dev,
						xfer->tx_dma, xfer->len,
						DMA_TO_DEVICE);
			return -ENOMEM;
		}
	}
	return 0;
}

static void atmel_spi_dma_unmap_xfer(struct spi_master *master,
				     struct spi_transfer *xfer)
{
	if (xfer->tx_dma != INVALID_DMA_ADDRESS)
		dma_unmap_single(master->dev.parent, xfer->tx_dma,
				 xfer->len, DMA_TO_DEVICE);
	if (xfer->rx_dma != INVALID_DMA_ADDRESS)
		dma_unmap_single(master->dev.parent, xfer->rx_dma,
				 xfer->len, DMA_FROM_DEVICE);
}

static void atmel_spi_disable_pdc_transfer(struct atmel_spi *as)
{
	spi_writel(as, PTCR, SPI_BIT(RXTDIS) | SPI_BIT(TXTDIS));
}

static void
atmel_spi_msg_done(struct spi_master *master, struct atmel_spi *as,
		struct spi_message *msg, int stay)
{
	if (!stay || as->done_status < 0)
		cs_deactivate(as, msg->spi);
	else
		as->stay = msg->spi;

	list_del(&msg->queue);
	msg->status = as->done_status;

	dev_dbg(master->dev.parent,
		"xfer complete: %u bytes transferred\n",
		msg->actual_length);

	atmel_spi_unlock(as);
	msg->complete(msg->context);
	atmel_spi_lock(as);

	as->current_transfer = NULL;
	as->next_transfer = NULL;
	as->done_status = 0;

	/*                    */
	if (list_empty(&as->queue) || as->stopping) {
		if (as->use_pdc)
			atmel_spi_disable_pdc_transfer(as);
	} else {
		atmel_spi_next_message(master);
	}
}

/*                
               
  
                                                              
               
 */
static void
atmel_spi_pump_pio_data(struct atmel_spi *as, struct spi_transfer *xfer)
{
	u8		*txp;
	u8		*rxp;
	u16		*txp16;
	u16		*rxp16;
	unsigned long	xfer_pos = xfer->len - as->current_remaining_bytes;

	if (xfer->rx_buf) {
		if (xfer->bits_per_word > 8) {
			rxp16 = (u16 *)(((u8 *)xfer->rx_buf) + xfer_pos);
			*rxp16 = spi_readl(as, RDR);
		} else {
			rxp = ((u8 *)xfer->rx_buf) + xfer_pos;
			*rxp = spi_readl(as, RDR);
		}
	} else {
		spi_readl(as, RDR);
	}
	if (xfer->bits_per_word > 8) {
		as->current_remaining_bytes -= 2;
		if (as->current_remaining_bytes < 0)
			as->current_remaining_bytes = 0;
	} else {
		as->current_remaining_bytes--;
	}

	if (as->current_remaining_bytes) {
		if (xfer->tx_buf) {
			if (xfer->bits_per_word > 8) {
				txp16 = (u16 *)(((u8 *)xfer->tx_buf)
							+ xfer_pos + 2);
				spi_writel(as, TDR, *txp16);
			} else {
				txp = ((u8 *)xfer->tx_buf) + xfer_pos + 1;
				spi_writel(as, TDR, *txp);
			}
		} else {
			spi_writel(as, TDR, 0);
		}
	}
}

/*        
                                                           
 */
static void atmel_spi_tasklet_func(unsigned long data)
{
	struct spi_master	*master = (struct spi_master *)data;
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct spi_message	*msg;
	struct spi_transfer	*xfer;

	dev_vdbg(master->dev.parent, "atmel_spi_tasklet_func\n");

	atmel_spi_lock(as);

	xfer = as->current_transfer;

	if (xfer == NULL)
		/*                    */
		goto tasklet_out;

	msg = list_entry(as->queue.next, struct spi_message, queue);

	if (as->current_remaining_bytes == 0) {
		if (as->done_status < 0) {
			/*                          */
			if (atmel_spi_use_dma(as, xfer))
				atmel_spi_stop_dma(as);
		} else {
			/*                                */
			msg->actual_length += xfer->len;
		}

		if (atmel_spi_use_dma(as, xfer))
			if (!msg->is_dma_mapped)
				atmel_spi_dma_unmap_xfer(master, xfer);

		if (xfer->delay_usecs)
			udelay(xfer->delay_usecs);

		if (atmel_spi_xfer_is_last(msg, xfer) || as->done_status < 0) {
			/*                                         */
			atmel_spi_msg_done(master, as, msg, xfer->cs_change);
		} else {
			if (xfer->cs_change) {
				cs_deactivate(as, msg->spi);
				udelay(1);
				cs_activate(as, msg->spi);
			}

			/*
                                             
     
                                            
    */
			atmel_spi_dma_next_xfer(master, msg);
		}
	} else {
		/*
                                              
                          
   */
		atmel_spi_dma_next_xfer(master, msg);
	}

tasklet_out:
	atmel_spi_unlock(as);
}

/*          
  
                                                                    
                                                                      
                                                         
 */
static irqreturn_t
atmel_spi_pio_interrupt(int irq, void *dev_id)
{
	struct spi_master	*master = dev_id;
	struct atmel_spi	*as = spi_master_get_devdata(master);
	u32			status, pending, imr;
	struct spi_transfer	*xfer;
	int			ret = IRQ_NONE;

	imr = spi_readl(as, IMR);
	status = spi_readl(as, SR);
	pending = status & imr;

	if (pending & SPI_BIT(OVRES)) {
		ret = IRQ_HANDLED;
		spi_writel(as, IDR, SPI_BIT(OVRES));
		dev_warn(master->dev.parent, "overrun\n");

		/*
                                                     
                                                    
                                                  
                                
    
                                                       
                 
    
                                                                
   */
		as->done_status = -EIO;
		smp_wmb();

		/*                                               */
		spi_readl(as, SR);

		tasklet_schedule(&as->tasklet);

	} else if (pending & SPI_BIT(RDRF)) {
		atmel_spi_lock(as);

		if (as->current_remaining_bytes) {
			ret = IRQ_HANDLED;
			xfer = as->current_transfer;
			atmel_spi_pump_pio_data(as, xfer);
			if (!as->current_remaining_bytes) {
				/*                                    */
				spi_writel(as, IDR, pending);
				tasklet_schedule(&as->tasklet);
			}
		}

		atmel_spi_unlock(as);
	} else {
		WARN_ONCE(pending, "IRQ not handled, pending = %x\n", pending);
		ret = IRQ_HANDLED;
		spi_writel(as, IDR, pending);
	}

	return ret;
}

static irqreturn_t
atmel_spi_pdc_interrupt(int irq, void *dev_id)
{
	struct spi_master	*master = dev_id;
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct spi_message	*msg;
	struct spi_transfer	*xfer;
	u32			status, pending, imr;
	int			ret = IRQ_NONE;

	atmel_spi_lock(as);

	xfer = as->current_transfer;
	msg = list_entry(as->queue.next, struct spi_message, queue);

	imr = spi_readl(as, IMR);
	status = spi_readl(as, SR);
	pending = status & imr;

	if (pending & SPI_BIT(OVRES)) {
		int timeout;

		ret = IRQ_HANDLED;

		spi_writel(as, IDR, (SPI_BIT(RXBUFF) | SPI_BIT(ENDRX)
				     | SPI_BIT(OVRES)));

		/*
                                                     
                                                    
                                                  
                                
    
                                                       
                 
    
                                                        
   */
		spi_writel(as, PTCR, SPI_BIT(RXTDIS) | SPI_BIT(TXTDIS));
		if (!msg->is_dma_mapped)
			atmel_spi_dma_unmap_xfer(master, xfer);

		/*                                      */
		if (xfer->delay_usecs)
			udelay(xfer->delay_usecs);

		dev_warn(master->dev.parent, "overrun (%u/%u remaining)\n",
			 spi_readl(as, TCR), spi_readl(as, RCR));

		/*
                                                  
                         
   */
		spi_writel(as, RNCR, 0);
		spi_writel(as, TNCR, 0);
		spi_writel(as, RCR, 0);
		spi_writel(as, TCR, 0);
		for (timeout = 1000; timeout; timeout--)
			if (spi_readl(as, SR) & SPI_BIT(TXEMPTY))
				break;
		if (!timeout)
			dev_warn(master->dev.parent,
				 "timeout waiting for TXEMPTY");
		while (spi_readl(as, SR) & SPI_BIT(RDRF))
			spi_readl(as, RDR);

		/*                                               */
		spi_readl(as, SR);

		as->done_status = -EIO;
		atmel_spi_msg_done(master, as, msg, 0);
	} else if (pending & (SPI_BIT(RXBUFF) | SPI_BIT(ENDRX))) {
		ret = IRQ_HANDLED;

		spi_writel(as, IDR, pending);

		if (as->current_remaining_bytes == 0) {
			msg->actual_length += xfer->len;

			if (!msg->is_dma_mapped)
				atmel_spi_dma_unmap_xfer(master, xfer);

			/*                                      */
			if (xfer->delay_usecs)
				udelay(xfer->delay_usecs);

			if (atmel_spi_xfer_is_last(msg, xfer)) {
				/*                          */
				atmel_spi_msg_done(master, as, msg,
						xfer->cs_change);
			} else {
				if (xfer->cs_change) {
					cs_deactivate(as, msg->spi);
					udelay(1);
					cs_activate(as, msg->spi);
				}

				/*
                                              
      
                                             
     */
				atmel_spi_pdc_next_xfer(master, msg);
			}
		} else {
			/*
                                               
                           
    */
			atmel_spi_pdc_next_xfer(master, msg);
		}
	}

	atmel_spi_unlock(as);

	return ret;
}

static int atmel_spi_setup(struct spi_device *spi)
{
	struct atmel_spi	*as;
	struct atmel_spi_device	*asd;
	u32			scbr, csr;
	unsigned int		bits = spi->bits_per_word;
	unsigned long		bus_hz;
	unsigned int		npcs_pin;
	int			ret;

	as = spi_master_get_devdata(spi->master);

	if (as->stopping)
		return -ESHUTDOWN;

	if (spi->chip_select > spi->master->num_chipselect) {
		dev_dbg(&spi->dev,
				"setup: invalid chipselect %u (%u defined)\n",
				spi->chip_select, spi->master->num_chipselect);
		return -EINVAL;
	}

	if (bits < 8 || bits > 16) {
		dev_dbg(&spi->dev,
				"setup: invalid bits_per_word %u (8 to 16)\n",
				bits);
		return -EINVAL;
	}

	/*                               */
	if (!atmel_spi_is_v2(as)
			&& spi->chip_select == 0
			&& (spi->mode & SPI_CS_HIGH)) {
		dev_dbg(&spi->dev, "setup: can't be active-high\n");
		return -EINVAL;
	}

	/*                                                      */
	bus_hz = clk_get_rate(as->clk);
	if (!atmel_spi_is_v2(as))
		bus_hz /= 2;

	if (spi->max_speed_hz) {
		/*
                                                    
                                              
   */
		scbr = DIV_ROUND_UP(bus_hz, spi->max_speed_hz);

		/*
                                                  
                                                        
   */
		if (scbr >= (1 << SPI_SCBR_SIZE)) {
			dev_dbg(&spi->dev,
				"setup: %d Hz too slow, scbr %u; min %ld Hz\n",
				spi->max_speed_hz, scbr, bus_hz/255);
			return -EINVAL;
		}
	} else
		/*                                        */
		scbr = 0xff;

	csr = SPI_BF(SCBR, scbr) | SPI_BF(BITS, bits - 8);
	if (spi->mode & SPI_CPOL)
		csr |= SPI_BIT(CPOL);
	if (!(spi->mode & SPI_CPHA))
		csr |= SPI_BIT(NCPHA);

	/*                                                                   
   
                                                                  
                                                                    
                                                                  
  */
	csr |= SPI_BF(DLYBS, 0);
	csr |= SPI_BF(DLYBCT, 0);

	/*                                                               */
	npcs_pin = (unsigned int)spi->controller_data;

	if (gpio_is_valid(spi->cs_gpio))
		npcs_pin = spi->cs_gpio;

	asd = spi->controller_state;
	if (!asd) {
		asd = kzalloc(sizeof(struct atmel_spi_device), GFP_KERNEL);
		if (!asd)
			return -ENOMEM;

		ret = gpio_request(npcs_pin, dev_name(&spi->dev));
		if (ret) {
			kfree(asd);
			return ret;
		}

		asd->npcs_pin = npcs_pin;
		spi->controller_state = asd;
		gpio_direction_output(npcs_pin, !(spi->mode & SPI_CS_HIGH));
	} else {
		atmel_spi_lock(as);
		if (as->stay == spi)
			as->stay = NULL;
		cs_deactivate(as, spi);
		atmel_spi_unlock(as);
	}

	asd->csr = csr;

	dev_dbg(&spi->dev,
		"setup: %lu Hz bpw %u mode 0x%x -> csr%d %08x\n",
		bus_hz / scbr, bits, spi->mode, spi->chip_select, csr);

	if (!atmel_spi_is_v2(as))
		spi_writel(as, CSR0 + 4 * spi->chip_select, csr);

	return 0;
}

static int atmel_spi_transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct atmel_spi	*as;
	struct spi_transfer	*xfer;
	struct device		*controller = spi->master->dev.parent;
	u8			bits;
	struct atmel_spi_device	*asd;

	as = spi_master_get_devdata(spi->master);

	dev_dbg(controller, "new message %p submitted for %s\n",
			msg, dev_name(&spi->dev));

	if (unlikely(list_empty(&msg->transfers)))
		return -EINVAL;

	if (as->stopping)
		return -ESHUTDOWN;

	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		if (!(xfer->tx_buf || xfer->rx_buf) && xfer->len) {
			dev_dbg(&spi->dev, "missing rx or tx buf\n");
			return -EINVAL;
		}

		if (xfer->bits_per_word) {
			asd = spi->controller_state;
			bits = (asd->csr >> 4) & 0xf;
			if (bits != xfer->bits_per_word - 8) {
				dev_dbg(&spi->dev, "you can't yet change "
					 "bits_per_word in transfers\n");
				return -ENOPROTOOPT;
			}
		}

		if (xfer->bits_per_word > 8) {
			if (xfer->len % 2) {
				dev_dbg(&spi->dev, "buffer len should be 16 bits aligned\n");
				return -EINVAL;
			}
		}

		/*                                          */
		if (xfer->speed_hz < spi->max_speed_hz) {
			dev_dbg(&spi->dev, "can't change speed in transfer\n");
			return -ENOPROTOOPT;
		}

		/*
                                                             
                            
   */
		if ((!msg->is_dma_mapped) && (atmel_spi_use_dma(as, xfer)
			|| as->use_pdc)) {
			if (atmel_spi_dma_map_xfer(as, xfer) < 0)
				return -ENOMEM;
		}
	}

#ifdef VERBOSE
	list_for_each_entry(xfer, &msg->transfers, transfer_list) {
		dev_dbg(controller,
			"  xfer %p: len %u tx %p/%08x rx %p/%08x\n",
			xfer, xfer->len,
			xfer->tx_buf, xfer->tx_dma,
			xfer->rx_buf, xfer->rx_dma);
	}
#endif

	msg->status = -EINPROGRESS;
	msg->actual_length = 0;

	atmel_spi_lock(as);
	list_add_tail(&msg->queue, &as->queue);
	if (!as->current_transfer)
		atmel_spi_next_message(spi->master);
	atmel_spi_unlock(as);

	return 0;
}

static void atmel_spi_cleanup(struct spi_device *spi)
{
	struct atmel_spi	*as = spi_master_get_devdata(spi->master);
	struct atmel_spi_device	*asd = spi->controller_state;
	unsigned		gpio = (unsigned) spi->controller_data;

	if (!asd)
		return;

	atmel_spi_lock(as);
	if (as->stay == spi) {
		as->stay = NULL;
		cs_deactivate(as, spi);
	}
	atmel_spi_unlock(as);

	spi->controller_state = NULL;
	gpio_free(gpio);
	kfree(asd);
}

static inline unsigned int atmel_get_version(struct atmel_spi *as)
{
	return spi_readl(as, VERSION) & 0x00000fff;
}

static void atmel_get_caps(struct atmel_spi *as)
{
	unsigned int version;

	version = atmel_get_version(as);
	dev_info(&as->pdev->dev, "version: 0x%x\n", version);

	as->caps.is_spi2 = version > 0x121;
	as->caps.has_wdrbt = version >= 0x210;
	as->caps.has_dma_support = version >= 0x212;
}

/*                                                                         */

static int atmel_spi_probe(struct platform_device *pdev)
{
	struct resource		*regs;
	int			irq;
	struct clk		*clk;
	int			ret;
	struct spi_master	*master;
	struct atmel_spi	*as;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
		return -ENXIO;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	clk = clk_get(&pdev->dev, "spi_clk");
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	/*                                                 */
	ret = -ENOMEM;
	master = spi_alloc_master(&pdev->dev, sizeof *as);
	if (!master)
		goto out_free;

	/*                                               */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;

	master->dev.of_node = pdev->dev.of_node;
	master->bus_num = pdev->id;
	master->num_chipselect = master->dev.of_node ? 0 : 4;
	master->setup = atmel_spi_setup;
	master->transfer = atmel_spi_transfer;
	master->cleanup = atmel_spi_cleanup;
	platform_set_drvdata(pdev, master);

	as = spi_master_get_devdata(master);

	/*
                                                        
                                               
  */
	as->buffer = dma_alloc_coherent(&pdev->dev, BUFFER_SIZE,
					&as->buffer_dma, GFP_KERNEL);
	if (!as->buffer)
		goto out_free;

	spin_lock_init(&as->lock);
	INIT_LIST_HEAD(&as->queue);

	as->pdev = pdev;
	as->regs = ioremap(regs->start, resource_size(regs));
	if (!as->regs)
		goto out_free_buffer;
	as->phybase = regs->start;
	as->irq = irq;
	as->clk = clk;

	atmel_get_caps(as);

	as->use_dma = false;
	as->use_pdc = false;
	if (as->caps.has_dma_support) {
		if (atmel_spi_configure_dma(as) == 0)
			as->use_dma = true;
	} else {
		as->use_pdc = true;
	}

	if (as->caps.has_dma_support && !as->use_dma)
		dev_info(&pdev->dev, "Atmel SPI Controller using PIO only\n");

	if (as->use_pdc) {
		ret = request_irq(irq, atmel_spi_pdc_interrupt, 0,
					dev_name(&pdev->dev), master);
	} else {
		tasklet_init(&as->tasklet, atmel_spi_tasklet_func,
					(unsigned long)master);

		ret = request_irq(irq, atmel_spi_pio_interrupt, 0,
					dev_name(&pdev->dev), master);
	}
	if (ret)
		goto out_unmap_regs;

	/*                         */
	clk_enable(clk);
	spi_writel(as, CR, SPI_BIT(SWRST));
	spi_writel(as, CR, SPI_BIT(SWRST)); /*                              */
	if (as->caps.has_wdrbt) {
		spi_writel(as, MR, SPI_BIT(WDRBT) | SPI_BIT(MODFDIS)
				| SPI_BIT(MSTR));
	} else {
		spi_writel(as, MR, SPI_BIT(MSTR) | SPI_BIT(MODFDIS));
	}

	if (as->use_pdc)
		spi_writel(as, PTCR, SPI_BIT(RXTDIS) | SPI_BIT(TXTDIS));
	spi_writel(as, CR, SPI_BIT(SPIEN));

	/*     */
	dev_info(&pdev->dev, "Atmel SPI Controller at 0x%08lx (irq %d)\n",
			(unsigned long)regs->start, irq);

	ret = spi_register_master(master);
	if (ret)
		goto out_free_dma;

	return 0;

out_free_dma:
	if (as->use_dma)
		atmel_spi_release_dma(as);

	spi_writel(as, CR, SPI_BIT(SWRST));
	spi_writel(as, CR, SPI_BIT(SWRST)); /*                              */
	clk_disable(clk);
	free_irq(irq, master);
out_unmap_regs:
	iounmap(as->regs);
out_free_buffer:
	if (!as->use_pdc)
		tasklet_kill(&as->tasklet);
	dma_free_coherent(&pdev->dev, BUFFER_SIZE, as->buffer,
			as->buffer_dma);
out_free:
	clk_put(clk);
	spi_master_put(master);
	return ret;
}

static int atmel_spi_remove(struct platform_device *pdev)
{
	struct spi_master	*master = platform_get_drvdata(pdev);
	struct atmel_spi	*as = spi_master_get_devdata(master);
	struct spi_message	*msg;
	struct spi_transfer	*xfer;

	/*                                             */
	spin_lock_irq(&as->lock);
	as->stopping = 1;
	if (as->use_dma) {
		atmel_spi_stop_dma(as);
		atmel_spi_release_dma(as);
	}

	spi_writel(as, CR, SPI_BIT(SWRST));
	spi_writel(as, CR, SPI_BIT(SWRST)); /*                              */
	spi_readl(as, SR);
	spin_unlock_irq(&as->lock);

	/*                                      */
	list_for_each_entry(msg, &as->queue, queue) {
		list_for_each_entry(xfer, &msg->transfers, transfer_list) {
			if (!msg->is_dma_mapped
				&& (atmel_spi_use_dma(as, xfer)
					|| as->use_pdc))
				atmel_spi_dma_unmap_xfer(master, xfer);
		}
		msg->status = -ESHUTDOWN;
		msg->complete(msg->context);
	}

	if (!as->use_pdc)
		tasklet_kill(&as->tasklet);
	dma_free_coherent(&pdev->dev, BUFFER_SIZE, as->buffer,
			as->buffer_dma);

	clk_disable(as->clk);
	clk_put(as->clk);
	free_irq(as->irq, master);
	iounmap(as->regs);

	spi_unregister_master(master);

	return 0;
}

#ifdef	CONFIG_PM

static int atmel_spi_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	struct spi_master	*master = platform_get_drvdata(pdev);
	struct atmel_spi	*as = spi_master_get_devdata(master);

	clk_disable(as->clk);
	return 0;
}

static int atmel_spi_resume(struct platform_device *pdev)
{
	struct spi_master	*master = platform_get_drvdata(pdev);
	struct atmel_spi	*as = spi_master_get_devdata(master);

	clk_enable(as->clk);
	return 0;
}

#else
#define	atmel_spi_suspend	NULL
#define	atmel_spi_resume	NULL
#endif

#if defined(CONFIG_OF)
static const struct of_device_id atmel_spi_dt_ids[] = {
	{ .compatible = "atmel,at91rm9200-spi" },
	{ /*          */ }
};

MODULE_DEVICE_TABLE(of, atmel_spi_dt_ids);
#endif

static struct platform_driver atmel_spi_driver = {
	.driver		= {
		.name	= "atmel_spi",
		.owner	= THIS_MODULE,
		.of_match_table	= of_match_ptr(atmel_spi_dt_ids),
	},
	.suspend	= atmel_spi_suspend,
	.resume		= atmel_spi_resume,
	.probe		= atmel_spi_probe,
	.remove		= atmel_spi_remove,
};
module_platform_driver(atmel_spi_driver);

MODULE_DESCRIPTION("Atmel AT32/AT91 SPI Controller driver");
MODULE_AUTHOR("Haavard Skinnemoen (Atmel)");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:atmel_spi");
