/*
 * FireWire Serial driver
 *
 * Copyright (C) 2012 Peter Hurley <peter@hurleysoftware.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/mod_devicetable.h>
#include <linux/rculist.h>
#include <linux/workqueue.h>
#include <linux/ratelimit.h>
#include <linux/bug.h>
#include <linux/uaccess.h>

#include "fwserial.h"

#define be32_to_u64(hi, lo)  ((u64)be32_to_cpu(hi) << 32 | be32_to_cpu(lo))

#define LINUX_VENDOR_ID   0xd00d1eU  /*                                       */
#define FWSERIAL_VERSION  0x00e81cU  /*                                       */

/*                      */
static int num_ttys = 4;	    /*                                        */
				    /*                                        */
static bool auto_connect = true;    /*                                        */
static bool create_loop_dev = true; /*                                        */

module_param_named(ttys, num_ttys, int, S_IRUGO | S_IWUSR);
module_param_named(auto, auto_connect, bool, S_IRUGO | S_IWUSR);
module_param_named(loop, create_loop_dev, bool, S_IRUGO | S_IWUSR);

/*
                                                     
                                                                   
                                                                      
                                 
 */
#define WAKEUP_CHARS             256

/* 
                                                                  
                                    
 */
static LIST_HEAD(fwserial_list);
static DEFINE_MUTEX(fwserial_list_mutex);

/* 
                                                           
  
                                                                  
                                                                          
                                              
 */
static struct fwtty_port *port_table[MAX_TOTAL_PORTS];
static DEFINE_MUTEX(port_table_lock);
static bool port_table_corrupt;
#define FWTTY_INVALID_INDEX  MAX_TOTAL_PORTS

#define loop_idx(port)	(((port)->index) / num_ports)
#define table_idx(loop)	((loop) * num_ports + num_ttys)

/*                                          */
static int num_ports;

/*                                                 */
static struct kmem_cache *fwtty_txn_cache;

struct tty_driver *fwtty_driver;
static struct tty_driver *fwloop_driver;

static struct dentry *fwserial_debugfs;

struct fwtty_transaction;
typedef void (*fwtty_transaction_cb)(struct fw_card *card, int rcode,
				     void *data, size_t length,
				     struct fwtty_transaction *txn);

struct fwtty_transaction {
	struct fw_transaction      fw_txn;
	fwtty_transaction_cb       callback;
	struct fwtty_port	   *port;
	union {
		struct dma_pending dma_pended;
	};
};

#define to_device(a, b)			(a->b)
#define fwtty_err(p, s, v...)		dev_err(to_device(p, device), s, ##v)
#define fwtty_info(p, s, v...)		dev_info(to_device(p, device), s, ##v)
#define fwtty_notice(p, s, v...)	dev_notice(to_device(p, device), s, ##v)
#define fwtty_dbg(p, s, v...)		\
		dev_dbg(to_device(p, device), "%s: " s, __func__, ##v)
#define fwtty_err_ratelimited(p, s, v...) \
		dev_err_ratelimited(to_device(p, device), s, ##v)

#ifdef DEBUG
static inline void debug_short_write(struct fwtty_port *port, int c, int n)
{
	int avail;

	if (n < c) {
		spin_lock_bh(&port->lock);
		avail = dma_fifo_avail(&port->tx_fifo);
		spin_unlock_bh(&port->lock);
		fwtty_dbg(port, "short write: avail:%d req:%d wrote:%d",
			  avail, c, n);
	}
}
#else
#define debug_short_write(port, c, n)
#endif

static struct fwtty_peer *__fwserial_peer_by_node_id(struct fw_card *card,
						     int generation, int id);

#ifdef FWTTY_PROFILING

static void profile_fifo_avail(struct fwtty_port *port, unsigned *stat)
{
	spin_lock_bh(&port->lock);
	profile_size_distrib(stat, dma_fifo_avail(&port->tx_fifo));
	spin_unlock_bh(&port->lock);
}

static void dump_profile(struct seq_file *m, struct stats *stats)
{
	/*                                                         */
	int k = 4;
	unsigned sum;
	int j;
	char t[10];

	snprintf(t, 10, "< %d", 1 << k);
	seq_printf(m, "\n%14s  %6s", " ", t);
	for (j = k + 1; j < DISTRIBUTION_MAX_INDEX; ++j)
		seq_printf(m, "%6d", 1 << j);

	++k;
	for (j = 0, sum = 0; j <= k; ++j)
		sum += stats->reads[j];
	seq_printf(m, "\n%14s: %6d", "reads", sum);
	for (j = k + 1; j <= DISTRIBUTION_MAX_INDEX; ++j)
		seq_printf(m, "%6d", stats->reads[j]);

	for (j = 0, sum = 0; j <= k; ++j)
		sum += stats->writes[j];
	seq_printf(m, "\n%14s: %6d", "writes", sum);
	for (j = k + 1; j <= DISTRIBUTION_MAX_INDEX; ++j)
		seq_printf(m, "%6d", stats->writes[j]);

	for (j = 0, sum = 0; j <= k; ++j)
		sum += stats->txns[j];
	seq_printf(m, "\n%14s: %6d", "txns", sum);
	for (j = k + 1; j <= DISTRIBUTION_MAX_INDEX; ++j)
		seq_printf(m, "%6d", stats->txns[j]);

	for (j = 0, sum = 0; j <= k; ++j)
		sum += stats->unthrottle[j];
	seq_printf(m, "\n%14s: %6d", "avail @ unthr", sum);
	for (j = k + 1; j <= DISTRIBUTION_MAX_INDEX; ++j)
		seq_printf(m, "%6d", stats->unthrottle[j]);
}

#else
#define profile_fifo_avail(port, stat)
#define dump_profile(m, stats)
#endif

/*
                                                         
                                                                      
                                                                             
 */
static inline int device_max_receive(struct fw_device *fw_device)
{
	/*                              */
	return min(2 << fw_device->max_rec, 4096);
}

static void fwtty_log_tx_error(struct fwtty_port *port, int rcode)
{
	switch (rcode) {
	case RCODE_SEND_ERROR:
		fwtty_err_ratelimited(port, "card busy");
		break;
	case RCODE_ADDRESS_ERROR:
		fwtty_err_ratelimited(port, "bad unit addr or write length");
		break;
	case RCODE_DATA_ERROR:
		fwtty_err_ratelimited(port, "failed rx");
		break;
	case RCODE_NO_ACK:
		fwtty_err_ratelimited(port, "missing ack");
		break;
	case RCODE_BUSY:
		fwtty_err_ratelimited(port, "remote busy");
		break;
	default:
		fwtty_err_ratelimited(port, "failed tx: %d", rcode);
	}
}

static void fwtty_txn_constructor(void *this)
{
	struct fwtty_transaction *txn = this;

	init_timer(&txn->fw_txn.split_timeout_timer);
}

static void fwtty_common_callback(struct fw_card *card, int rcode,
				  void *payload, size_t len, void *cb_data)
{
	struct fwtty_transaction *txn = cb_data;
	struct fwtty_port *port = txn->port;

	if (port && rcode != RCODE_COMPLETE)
		fwtty_log_tx_error(port, rcode);
	if (txn->callback)
		txn->callback(card, rcode, payload, len, txn);
	kmem_cache_free(fwtty_txn_cache, txn);
}

static int fwtty_send_data_async(struct fwtty_peer *peer, int tcode,
				 unsigned long long addr, void *payload,
				 size_t len, fwtty_transaction_cb callback,
				 struct fwtty_port *port)
{
	struct fwtty_transaction *txn;
	int generation;

	txn = kmem_cache_alloc(fwtty_txn_cache, GFP_ATOMIC);
	if (!txn)
		return -ENOMEM;

	txn->callback = callback;
	txn->port = port;

	generation = peer->generation;
	smp_rmb();
	fw_send_request(peer->serial->card, &txn->fw_txn, tcode,
			peer->node_id, generation, peer->speed, addr, payload,
			len, fwtty_common_callback, txn);
	return 0;
}

static void fwtty_send_txn_async(struct fwtty_peer *peer,
				 struct fwtty_transaction *txn, int tcode,
				 unsigned long long addr, void *payload,
				 size_t len, fwtty_transaction_cb callback,
				 struct fwtty_port *port)
{
	int generation;

	txn->callback = callback;
	txn->port = port;

	generation = peer->generation;
	smp_rmb();
	fw_send_request(peer->serial->card, &txn->fw_txn, tcode,
			peer->node_id, generation, peer->speed, addr, payload,
			len, fwtty_common_callback, txn);
}


static void __fwtty_restart_tx(struct fwtty_port *port)
{
	int len, avail;

	len = dma_fifo_out_level(&port->tx_fifo);
	if (len)
		schedule_delayed_work(&port->drain, 0);
	avail = dma_fifo_avail(&port->tx_fifo);

	fwtty_dbg(port, "fifo len: %d avail: %d", len, avail);
}

static void fwtty_restart_tx(struct fwtty_port *port)
{
	spin_lock_bh(&port->lock);
	__fwtty_restart_tx(port);
	spin_unlock_bh(&port->lock);
}

/* 
                                                                      
  
                                                                           
                                           
 */
static void fwtty_update_port_status(struct fwtty_port *port, unsigned status)
{
	unsigned delta;
	struct tty_struct *tty;

	/*                                      */
	status &= ~MCTRL_MASK;
	delta = (port->mstatus ^ status) & ~MCTRL_MASK;
	delta &= ~(status & TIOCM_RNG);
	port->mstatus = status;

	if (delta & TIOCM_RNG)
		++port->icount.rng;
	if (delta & TIOCM_DSR)
		++port->icount.dsr;
	if (delta & TIOCM_CAR)
		++port->icount.dcd;
	if (delta & TIOCM_CTS)
		++port->icount.cts;

	fwtty_dbg(port, "status: %x delta: %x", status, delta);

	if (delta & TIOCM_CAR) {
		tty = tty_port_tty_get(&port->port);
		if (tty && !C_CLOCAL(tty)) {
			if (status & TIOCM_CAR)
				wake_up_interruptible(&port->port.open_wait);
			else
				schedule_work(&port->hangup);
		}
		tty_kref_put(tty);
	}

	if (delta & TIOCM_CTS) {
		tty = tty_port_tty_get(&port->port);
		if (tty && C_CRTSCTS(tty)) {
			if (tty->hw_stopped) {
				if (status & TIOCM_CTS) {
					tty->hw_stopped = 0;
					if (port->loopback)
						__fwtty_restart_tx(port);
					else
						fwtty_restart_tx(port);
				}
			} else {
				if (~status & TIOCM_CTS)
					tty->hw_stopped = 1;
			}
		}
		tty_kref_put(tty);

	} else if (delta & OOB_TX_THROTTLE) {
		tty = tty_port_tty_get(&port->port);
		if (tty) {
			if (tty->hw_stopped) {
				if (~status & OOB_TX_THROTTLE) {
					tty->hw_stopped = 0;
					if (port->loopback)
						__fwtty_restart_tx(port);
					else
						fwtty_restart_tx(port);
				}
			} else {
				if (status & OOB_TX_THROTTLE)
					tty->hw_stopped = 1;
			}
		}
		tty_kref_put(tty);
	}

	if (delta & (UART_LSR_BI << 24)) {
		if (status & (UART_LSR_BI << 24)) {
			port->break_last = jiffies;
			schedule_delayed_work(&port->emit_breaks, 0);
		} else {
			/*                                            */
			mod_delayed_work(system_wq, &port->emit_breaks, 0);
		}
	}

	if (delta & (TIOCM_DSR | TIOCM_CAR | TIOCM_CTS | TIOCM_RNG))
		wake_up_interruptible(&port->port.delta_msr_wait);
}

/* 
                                                                       
  
                                                                             
                                                                         
                                                                
  
                                         
 */
static unsigned __fwtty_port_line_status(struct fwtty_port *port)
{
	unsigned status = 0;

	/*                                                  */

	if (port->mctrl & TIOCM_DTR)
		status |= TIOCM_DSR | TIOCM_CAR;
	if (port->mctrl & TIOCM_RTS)
		status |= TIOCM_CTS;
	if (port->mctrl & OOB_RX_THROTTLE)
		status |= OOB_TX_THROTTLE;
	/*                                  */
	if (port->break_ctl)
		status |= UART_LSR_BI << 24;

	return status;
}

/* 
                                                                
  
                                              
 */
static int __fwtty_write_port_status(struct fwtty_port *port)
{
	struct fwtty_peer *peer;
	int err = -ENOENT;
	unsigned status = __fwtty_port_line_status(port);

	rcu_read_lock();
	peer = rcu_dereference(port->peer);
	if (peer) {
		err = fwtty_send_data_async(peer, TCODE_WRITE_QUADLET_REQUEST,
					    peer->status_addr, &status,
					    sizeof(status), NULL, port);
	}
	rcu_read_unlock();

	return err;
}

/* 
                                                                  
 */
static int fwtty_write_port_status(struct fwtty_port *port)
{
	int err;

	spin_lock_bh(&port->lock);
	err = __fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);
	return err;
}

static void __fwtty_throttle(struct fwtty_port *port, struct tty_struct *tty)
{
	unsigned old;

	old = port->mctrl;
	port->mctrl |= OOB_RX_THROTTLE;
	if (C_CRTSCTS(tty))
		port->mctrl &= ~TIOCM_RTS;
	if (~old & OOB_RX_THROTTLE)
		__fwtty_write_port_status(port);
}

/* 
                                                                               
  
                                                                              
                                                                            
                                                                             
                                           
  
                                                                       
                                                                            
                                                                      
                                                                                
                                                                         
                                                   
  
                                                                               
                                                                              
 */

/*                                                                          */
static void fwtty_do_hangup(struct work_struct *work)
{
	struct fwtty_port *port = to_port(work, hangup);
	struct tty_struct *tty;

	schedule_timeout_uninterruptible(msecs_to_jiffies(50));

	tty = tty_port_tty_get(&port->port);
	if (tty)
		tty_vhangup(tty);
	tty_kref_put(tty);
}


static void fwtty_emit_breaks(struct work_struct *work)
{
	struct fwtty_port *port = to_port(to_delayed_work(work), emit_breaks);
	static const char buf[16];
	unsigned long now = jiffies;
	unsigned long elapsed = now - port->break_last;
	int n, t, c, brk = 0;

	/*                                                   */
	n = (elapsed * port->cps) / HZ + 1;
	port->break_last = now;

	fwtty_dbg(port, "sending %d brks", n);

	while (n) {
		t = min(n, 16);
		c = tty_insert_flip_string_fixed_flag(&port->port, buf,
				TTY_BREAK, t);
		n -= c;
		brk += c;
		if (c < t)
			break;
	}
	tty_flip_buffer_push(&port->port);

	if (port->mstatus & (UART_LSR_BI << 24))
		schedule_delayed_work(&port->emit_breaks, FREQ_BREAKS);
	port->icount.brk += brk;
}

static void fwtty_pushrx(struct work_struct *work)
{
	struct fwtty_port *port = to_port(work, push);
	struct tty_struct *tty;
	struct buffered_rx *buf, *next;
	int n, c = 0;

	spin_lock_bh(&port->lock);
	list_for_each_entry_safe(buf, next, &port->buf_list, list) {
		n = tty_insert_flip_string_fixed_flag(&port->port, buf->data,
						      TTY_NORMAL, buf->n);
		c += n;
		port->buffered -= n;
		if (n < buf->n) {
			if (n > 0) {
				memmove(buf->data, buf->data + n, buf->n - n);
				buf->n -= n;
			}
			tty = tty_port_tty_get(&port->port);
			if (tty) {
				__fwtty_throttle(port, tty);
				tty_kref_put(tty);
			}
			break;
		} else {
			list_del(&buf->list);
			kfree(buf);
		}
	}
	if (c > 0)
		tty_flip_buffer_push(&port->port);

	if (list_empty(&port->buf_list))
		clear_bit(BUFFERING_RX, &port->flags);
	spin_unlock_bh(&port->lock);
}

static int fwtty_buffer_rx(struct fwtty_port *port, unsigned char *d, size_t n)
{
	struct buffered_rx *buf;
	size_t size = (n + sizeof(struct buffered_rx) + 0xFF) & ~0xFF;

	if (port->buffered + n > HIGH_WATERMARK) {
		fwtty_err_ratelimited(port, "overflowed rx buffer: buffered: %d new: %zu wtrmk: %d",
				      port->buffered, n, HIGH_WATERMARK);
		return 0;
	}
	buf = kmalloc(size, GFP_ATOMIC);
	if (!buf)
		return 0;
	INIT_LIST_HEAD(&buf->list);
	buf->n = n;
	memcpy(buf->data, d, n);

	spin_lock_bh(&port->lock);
	list_add_tail(&buf->list, &port->buf_list);
	port->buffered += n;
	if (port->buffered > port->stats.watermark)
		port->stats.watermark = port->buffered;
	set_bit(BUFFERING_RX, &port->flags);
	spin_unlock_bh(&port->lock);

	return n;
}

static int fwtty_rx(struct fwtty_port *port, unsigned char *data, size_t len)
{
	struct tty_struct *tty;
	int c, n = len;
	unsigned lsr;
	int err = 0;

	fwtty_dbg(port, "%d", n);
	profile_size_distrib(port->stats.reads, n);

	if (port->write_only) {
		n = 0;
		goto out;
	}

	/*                                                                  */
	lsr = (port->mstatus >> 24) & ~UART_LSR_BI;

	if (port->overrun)
		lsr |= UART_LSR_OE;

	if (lsr & UART_LSR_OE)
		++port->icount.overrun;

	lsr &= port->status_mask;
	if (lsr & ~port->ignore_mask & UART_LSR_OE) {
		if (!tty_insert_flip_char(&port->port, 0, TTY_OVERRUN)) {
			err = -EIO;
			goto out;
		}
	}
	port->overrun = false;

	if (lsr & port->ignore_mask & ~UART_LSR_OE) {
		/*                                           */
		n = 0;
		goto out;
	}

	if (!test_bit(BUFFERING_RX, &port->flags)) {
		c = tty_insert_flip_string_fixed_flag(&port->port, data,
				TTY_NORMAL, n);
		if (c > 0)
			tty_flip_buffer_push(&port->port);
		n -= c;

		if (n) {
			/*                                */
			n -= fwtty_buffer_rx(port, &data[c], n);

			tty = tty_port_tty_get(&port->port);
			if (tty) {
				spin_lock_bh(&port->lock);
				__fwtty_throttle(port, tty);
				spin_unlock_bh(&port->lock);
				tty_kref_put(tty);
			}
		}
	} else
		n -= fwtty_buffer_rx(port, data, n);

	if (n) {
		port->overrun = true;
		err = -EIO;
	}

out:
	port->icount.rx += len;
	port->stats.lost += n;
	return err;
}

/* 
                                                                 
                                                                             
  
                                                                                
 */
static void fwtty_port_handler(struct fw_card *card,
			       struct fw_request *request,
			       int tcode, int destination, int source,
			       int generation,
			       unsigned long long addr,
			       void *data, size_t len,
			       void *callback_data)
{
	struct fwtty_port *port = callback_data;
	struct fwtty_peer *peer;
	int err;
	int rcode;

	/*                                                          */
	rcu_read_lock();
	peer = __fwserial_peer_by_node_id(card, generation, source);
	rcu_read_unlock();
	if (!peer || peer != rcu_access_pointer(port->peer)) {
		rcode = RCODE_ADDRESS_ERROR;
		fwtty_err_ratelimited(port, "ignoring unauthenticated data");
		goto respond;
	}

	switch (tcode) {
	case TCODE_WRITE_QUADLET_REQUEST:
		if (addr != port->rx_handler.offset || len != 4)
			rcode = RCODE_ADDRESS_ERROR;
		else {
			fwtty_update_port_status(port, *(unsigned *)data);
			rcode = RCODE_COMPLETE;
		}
		break;

	case TCODE_WRITE_BLOCK_REQUEST:
		if (addr != port->rx_handler.offset + 4 ||
		    len > port->rx_handler.length - 4) {
			rcode = RCODE_ADDRESS_ERROR;
		} else {
			err = fwtty_rx(port, data, len);
			switch (err) {
			case 0:
				rcode = RCODE_COMPLETE;
				break;
			case -EIO:
				rcode = RCODE_DATA_ERROR;
				break;
			default:
				rcode = RCODE_CONFLICT_ERROR;
				break;
			}
		}
		break;

	default:
		rcode = RCODE_TYPE_ERROR;
	}

respond:
	fw_send_response(card, request, rcode);
}

/* 
                                          
                                                
                                                  
  
                                                                        
                                             
 */
static void fwtty_tx_complete(struct fw_card *card, int rcode,
			      void *data, size_t length,
			      struct fwtty_transaction *txn)
{
	struct fwtty_port *port = txn->port;
	int len;

	fwtty_dbg(port, "rcode: %d", rcode);

	switch (rcode) {
	case RCODE_COMPLETE:
		spin_lock_bh(&port->lock);
		dma_fifo_out_complete(&port->tx_fifo, &txn->dma_pended);
		len = dma_fifo_level(&port->tx_fifo);
		spin_unlock_bh(&port->lock);

		port->icount.tx += txn->dma_pended.len;
		break;

	default:
		/*                         */
		spin_lock_bh(&port->lock);
		dma_fifo_out_complete(&port->tx_fifo, &txn->dma_pended);
		len = dma_fifo_level(&port->tx_fifo);
		spin_unlock_bh(&port->lock);

		port->stats.dropped += txn->dma_pended.len;
	}

	if (len < WAKEUP_CHARS)
		tty_port_tty_wakeup(&port->port);
}

static int fwtty_tx(struct fwtty_port *port, bool drain)
{
	struct fwtty_peer *peer;
	struct fwtty_transaction *txn;
	struct tty_struct *tty;
	int n, len;

	tty = tty_port_tty_get(&port->port);
	if (!tty)
		return -ENOENT;

	rcu_read_lock();
	peer = rcu_dereference(port->peer);
	if (!peer) {
		n = -EIO;
		goto out;
	}

	if (test_and_set_bit(IN_TX, &port->flags)) {
		n = -EALREADY;
		goto out;
	}

	/*                                                       */
	n = -EAGAIN;
	while (!tty->stopped && !tty->hw_stopped &&
			!test_bit(STOP_TX, &port->flags)) {
		txn = kmem_cache_alloc(fwtty_txn_cache, GFP_ATOMIC);
		if (!txn) {
			n = -ENOMEM;
			break;
		}

		spin_lock_bh(&port->lock);
		n = dma_fifo_out_pend(&port->tx_fifo, &txn->dma_pended);
		spin_unlock_bh(&port->lock);

		fwtty_dbg(port, "out: %u rem: %d", txn->dma_pended.len, n);

		if (n < 0) {
			kmem_cache_free(fwtty_txn_cache, txn);
			if (n == -EAGAIN)
				++port->stats.tx_stall;
			else if (n == -ENODATA)
				profile_size_distrib(port->stats.txns, 0);
			else {
				++port->stats.fifo_errs;
				fwtty_err_ratelimited(port, "fifo err: %d", n);
			}
			break;
		}

		profile_size_distrib(port->stats.txns, txn->dma_pended.len);

		fwtty_send_txn_async(peer, txn, TCODE_WRITE_BLOCK_REQUEST,
				     peer->fifo_addr, txn->dma_pended.data,
				     txn->dma_pended.len, fwtty_tx_complete,
				     port);
		++port->stats.sent;

		/*
                                                          
                                                             
   */
		if (n == 0 || (!drain && n < WRITER_MINIMUM))
			break;
	}

	if (n >= 0 || n == -EAGAIN || n == -ENOMEM || n == -ENODATA) {
		spin_lock_bh(&port->lock);
		len = dma_fifo_out_level(&port->tx_fifo);
		if (len) {
			unsigned long delay = (n == -ENOMEM) ? HZ : 1;
			schedule_delayed_work(&port->drain, delay);
		}
		len = dma_fifo_level(&port->tx_fifo);
		spin_unlock_bh(&port->lock);

		/*                   */
		if (drain && len < WAKEUP_CHARS)
			tty_wakeup(tty);
	}

	clear_bit(IN_TX, &port->flags);
	wake_up_interruptible(&port->wait_tx);

out:
	rcu_read_unlock();
	tty_kref_put(tty);
	return n;
}

static void fwtty_drain_tx(struct work_struct *work)
{
	struct fwtty_port *port = to_port(to_delayed_work(work), drain);

	fwtty_tx(port, true);
}

static void fwtty_write_xchar(struct fwtty_port *port, char ch)
{
	struct fwtty_peer *peer;

	++port->stats.xchars;

	fwtty_dbg(port, "%02x", ch);

	rcu_read_lock();
	peer = rcu_dereference(port->peer);
	if (peer) {
		fwtty_send_data_async(peer, TCODE_WRITE_BLOCK_REQUEST,
				      peer->fifo_addr, &ch, sizeof(ch),
				      NULL, port);
	}
	rcu_read_unlock();
}

struct fwtty_port *fwtty_port_get(unsigned index)
{
	struct fwtty_port *port;

	if (index >= MAX_TOTAL_PORTS)
		return NULL;

	mutex_lock(&port_table_lock);
	port = port_table[index];
	if (port)
		kref_get(&port->serial->kref);
	mutex_unlock(&port_table_lock);
	return port;
}
EXPORT_SYMBOL(fwtty_port_get);

static int fwtty_ports_add(struct fw_serial *serial)
{
	int err = -EBUSY;
	int i, j;

	if (port_table_corrupt)
		return err;

	mutex_lock(&port_table_lock);
	for (i = 0; i + num_ports <= MAX_TOTAL_PORTS; i += num_ports) {
		if (!port_table[i]) {
			for (j = 0; j < num_ports; ++i, ++j) {
				serial->ports[j]->index = i;
				port_table[i] = serial->ports[j];
			}
			err = 0;
			break;
		}
	}
	mutex_unlock(&port_table_lock);
	return err;
}

static void fwserial_destroy(struct kref *kref)
{
	struct fw_serial *serial = to_serial(kref, kref);
	struct fwtty_port **ports = serial->ports;
	int j, i = ports[0]->index;

	synchronize_rcu();

	mutex_lock(&port_table_lock);
	for (j = 0; j < num_ports; ++i, ++j) {
		port_table_corrupt |= port_table[i] != ports[j];
		WARN_ONCE(port_table_corrupt, "port_table[%d]: %p != ports[%d]: %p",
		     i, port_table[i], j, ports[j]);

		port_table[i] = NULL;
	}
	mutex_unlock(&port_table_lock);

	for (j = 0; j < num_ports; ++j) {
		fw_core_remove_address_handler(&ports[j]->rx_handler);
		tty_port_destroy(&ports[j]->port);
		kfree(ports[j]);
	}
	kfree(serial);
}

void fwtty_port_put(struct fwtty_port *port)
{
	kref_put(&port->serial->kref, fwserial_destroy);
}
EXPORT_SYMBOL(fwtty_port_put);

static void fwtty_port_dtr_rts(struct tty_port *tty_port, int on)
{
	struct fwtty_port *port = to_port(tty_port, port);

	fwtty_dbg(port, "on/off: %d", on);

	spin_lock_bh(&port->lock);
	/*                                                 */
	if (!port->port.console) {
		if (on)
			port->mctrl |= TIOCM_DTR | TIOCM_RTS;
		else
			port->mctrl &= ~(TIOCM_DTR | TIOCM_RTS);
	}

	__fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);
}

/* 
                                                         
  
                                                                               
                                                                         
 */
static int fwtty_port_carrier_raised(struct tty_port *tty_port)
{
	struct fwtty_port *port = to_port(tty_port, port);
	int rc;

	rc = (port->mstatus & TIOCM_CAR);

	fwtty_dbg(port, "%d", rc);

	return rc;
}

static unsigned set_termios(struct fwtty_port *port, struct tty_struct *tty)
{
	unsigned baud, frame;

	baud = tty_termios_baud_rate(&tty->termios);
	tty_termios_encode_baud_rate(&tty->termios, baud, baud);

	/*                               */
	frame = 12 + ((C_CSTOPB(tty)) ? 4 : 2) + ((C_PARENB(tty)) ? 2 : 0);

	switch (C_CSIZE(tty)) {
	case CS5:
		frame -= (C_CSTOPB(tty)) ? 1 : 0;
		break;
	case CS6:
		frame += 2;
		break;
	case CS7:
		frame += 4;
		break;
	case CS8:
		frame += 6;
		break;
	}

	port->cps = (baud << 1) / frame;

	port->status_mask = UART_LSR_OE;
	if (_I_FLAG(tty, BRKINT | PARMRK))
		port->status_mask |= UART_LSR_BI;

	port->ignore_mask = 0;
	if (I_IGNBRK(tty)) {
		port->ignore_mask |= UART_LSR_BI;
		if (I_IGNPAR(tty))
			port->ignore_mask |= UART_LSR_OE;
	}

	port->write_only = !C_CREAD(tty);

	/*                                            */
	if (port->loopback) {
		tty->termios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE |
					  ECHONL | ECHOPRT | ECHOCTL);
		tty->termios.c_oflag &= ~ONLCR;
	}

	return baud;
}

static int fwtty_port_activate(struct tty_port *tty_port,
			       struct tty_struct *tty)
{
	struct fwtty_port *port = to_port(tty_port, port);
	unsigned baud;
	int err;

	set_bit(TTY_IO_ERROR, &tty->flags);

	err = dma_fifo_alloc(&port->tx_fifo, FWTTY_PORT_TXFIFO_LEN,
			     cache_line_size(),
			     port->max_payload,
			     FWTTY_PORT_MAX_PEND_DMA,
			     GFP_KERNEL);
	if (err)
		return err;

	spin_lock_bh(&port->lock);

	baud = set_termios(port, tty);

	/*                                        */
	if (!port->port.console) {
		port->mctrl = 0;
		if (baud != 0)
			port->mctrl = TIOCM_DTR | TIOCM_RTS;
	}

	if (C_CRTSCTS(tty) && ~port->mstatus & TIOCM_CTS)
		tty->hw_stopped = 1;

	__fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);

	clear_bit(TTY_IO_ERROR, &tty->flags);

	return 0;
}

/* 
                      
  
                                                              
                                
 */
static void fwtty_port_shutdown(struct tty_port *tty_port)
{
	struct fwtty_port *port = to_port(tty_port, port);
	struct buffered_rx *buf, *next;

	/*                                       */

	cancel_delayed_work_sync(&port->emit_breaks);
	cancel_delayed_work_sync(&port->drain);
	cancel_work_sync(&port->push);

	spin_lock_bh(&port->lock);
	list_for_each_entry_safe(buf, next, &port->buf_list, list) {
		list_del(&buf->list);
		kfree(buf);
	}
	port->buffered = 0;
	port->flags = 0;
	port->break_ctl = 0;
	port->overrun = 0;
	__fwtty_write_port_status(port);
	dma_fifo_free(&port->tx_fifo);
	spin_unlock_bh(&port->lock);
}

static int fwtty_open(struct tty_struct *tty, struct file *fp)
{
	struct fwtty_port *port = tty->driver_data;

	return tty_port_open(&port->port, tty, fp);
}

static void fwtty_close(struct tty_struct *tty, struct file *fp)
{
	struct fwtty_port *port = tty->driver_data;

	tty_port_close(&port->port, tty, fp);
}

static void fwtty_hangup(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;

	tty_port_hangup(&port->port);
}

static void fwtty_cleanup(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;

	tty->driver_data = NULL;
	fwtty_port_put(port);
}

static int fwtty_install(struct tty_driver *driver, struct tty_struct *tty)
{
	struct fwtty_port *port = fwtty_port_get(tty->index);
	int err;

	err = tty_standard_install(driver, tty);
	if (!err)
		tty->driver_data = port;
	else
		fwtty_port_put(port);
	return err;
}

static int fwloop_install(struct tty_driver *driver, struct tty_struct *tty)
{
	struct fwtty_port *port = fwtty_port_get(table_idx(tty->index));
	int err;

	err = tty_standard_install(driver, tty);
	if (!err)
		tty->driver_data = port;
	else
		fwtty_port_put(port);
	return err;
}

static int fwtty_write(struct tty_struct *tty, const unsigned char *buf, int c)
{
	struct fwtty_port *port = tty->driver_data;
	int n, len;

	fwtty_dbg(port, "%d", c);
	profile_size_distrib(port->stats.writes, c);

	spin_lock_bh(&port->lock);
	n = dma_fifo_in(&port->tx_fifo, buf, c);
	len = dma_fifo_out_level(&port->tx_fifo);
	if (len < DRAIN_THRESHOLD)
		schedule_delayed_work(&port->drain, 1);
	spin_unlock_bh(&port->lock);

	if (len >= DRAIN_THRESHOLD)
		fwtty_tx(port, false);

	debug_short_write(port, c, n);

	return (n < 0) ? 0 : n;
}

static int fwtty_write_room(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;
	int n;

	spin_lock_bh(&port->lock);
	n = dma_fifo_avail(&port->tx_fifo);
	spin_unlock_bh(&port->lock);

	fwtty_dbg(port, "%d", n);

	return n;
}

static int fwtty_chars_in_buffer(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;
	int n;

	spin_lock_bh(&port->lock);
	n = dma_fifo_level(&port->tx_fifo);
	spin_unlock_bh(&port->lock);

	fwtty_dbg(port, "%d", n);

	return n;
}

static void fwtty_send_xchar(struct tty_struct *tty, char ch)
{
	struct fwtty_port *port = tty->driver_data;

	fwtty_dbg(port, "%02x", ch);

	fwtty_write_xchar(port, ch);
}

static void fwtty_throttle(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;

	/*
                                             
                                                               
                                                       
                                                              
                                                                        
                                                               
                                                                   
                                        
  */

	++port->stats.throttled;
}

static void fwtty_unthrottle(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;

	fwtty_dbg(port, "CRTSCTS: %d", (C_CRTSCTS(tty) != 0));

	profile_fifo_avail(port, port->stats.unthrottle);

	schedule_work(&port->push);

	spin_lock_bh(&port->lock);
	port->mctrl &= ~OOB_RX_THROTTLE;
	if (C_CRTSCTS(tty))
		port->mctrl |= TIOCM_RTS;
	__fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);
}

static int check_msr_delta(struct fwtty_port *port, unsigned long mask,
			   struct async_icount *prev)
{
	struct async_icount now;
	int delta;

	now = port->icount;

	delta = ((mask & TIOCM_RNG && prev->rng != now.rng) ||
		 (mask & TIOCM_DSR && prev->dsr != now.dsr) ||
		 (mask & TIOCM_CAR && prev->dcd != now.dcd) ||
		 (mask & TIOCM_CTS && prev->cts != now.cts));

	*prev = now;

	return delta;
}

static int wait_msr_change(struct fwtty_port *port, unsigned long mask)
{
	struct async_icount prev;

	prev = port->icount;

	return wait_event_interruptible(port->port.delta_msr_wait,
					check_msr_delta(port, mask, &prev));
}

static int get_serial_info(struct fwtty_port *port,
			   struct serial_struct __user *info)
{
	struct serial_struct tmp;

	memset(&tmp, 0, sizeof(tmp));

	tmp.type =  PORT_UNKNOWN;
	tmp.line =  port->port.tty->index;
	tmp.flags = port->port.flags;
	tmp.xmit_fifo_size = FWTTY_PORT_TXFIFO_LEN;
	tmp.baud_base = 400000000;
	tmp.close_delay = port->port.close_delay;

	return (copy_to_user(info, &tmp, sizeof(*info))) ? -EFAULT : 0;
}

static int set_serial_info(struct fwtty_port *port,
			   struct serial_struct __user *info)
{
	struct serial_struct tmp;

	if (copy_from_user(&tmp, info, sizeof(tmp)))
		return -EFAULT;

	if (tmp.irq != 0 || tmp.port != 0 || tmp.custom_divisor != 0 ||
			tmp.baud_base != 400000000)
		return -EPERM;

	if (!capable(CAP_SYS_ADMIN)) {
		if (((tmp.flags & ~ASYNC_USR_MASK) !=
		     (port->port.flags & ~ASYNC_USR_MASK)))
			return -EPERM;
	} else
		port->port.close_delay = tmp.close_delay * HZ / 100;

	return 0;
}

static int fwtty_ioctl(struct tty_struct *tty, unsigned cmd,
		       unsigned long arg)
{
	struct fwtty_port *port = tty->driver_data;
	int err;

	switch (cmd) {
	case TIOCGSERIAL:
		mutex_lock(&port->port.mutex);
		err = get_serial_info(port, (void __user *)arg);
		mutex_unlock(&port->port.mutex);
		break;

	case TIOCSSERIAL:
		mutex_lock(&port->port.mutex);
		err = set_serial_info(port, (void __user *)arg);
		mutex_unlock(&port->port.mutex);
		break;

	case TIOCMIWAIT:
		err = wait_msr_change(port, arg);
		break;

	default:
		err = -ENOIOCTLCMD;
	}

	return err;
}

static void fwtty_set_termios(struct tty_struct *tty, struct ktermios *old)
{
	struct fwtty_port *port = tty->driver_data;
	unsigned baud;

	spin_lock_bh(&port->lock);
	baud = set_termios(port, tty);

	if ((baud == 0) && (old->c_cflag & CBAUD))
		port->mctrl &= ~(TIOCM_DTR | TIOCM_RTS);
	else if ((baud != 0) && !(old->c_cflag & CBAUD)) {
		if (C_CRTSCTS(tty) || !test_bit(TTY_THROTTLED, &tty->flags))
			port->mctrl |= TIOCM_DTR | TIOCM_RTS;
		else
			port->mctrl |= TIOCM_DTR;
	}
	__fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);

	if (old->c_cflag & CRTSCTS) {
		if (!C_CRTSCTS(tty)) {
			tty->hw_stopped = 0;
			fwtty_restart_tx(port);
		}
	} else if (C_CRTSCTS(tty) && ~port->mstatus & TIOCM_CTS) {
		tty->hw_stopped = 1;
	}
}

/* 
                                              
  
                                                                   
                                                                             
                                                                               
                                                                            
                                                                                
                    
 */
static int fwtty_break_ctl(struct tty_struct *tty, int state)
{
	struct fwtty_port *port = tty->driver_data;
	long ret;

	fwtty_dbg(port, "%d", state);

	if (state == -1) {
		set_bit(STOP_TX, &port->flags);
		ret = wait_event_interruptible_timeout(port->wait_tx,
					       !test_bit(IN_TX, &port->flags),
					       10);
		if (ret == 0 || ret == -ERESTARTSYS) {
			clear_bit(STOP_TX, &port->flags);
			fwtty_restart_tx(port);
			return -EINTR;
		}
	}

	spin_lock_bh(&port->lock);
	port->break_ctl = (state == -1);
	__fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);

	if (state == 0) {
		spin_lock_bh(&port->lock);
		dma_fifo_reset(&port->tx_fifo);
		clear_bit(STOP_TX, &port->flags);
		spin_unlock_bh(&port->lock);
	}
	return 0;
}

static int fwtty_tiocmget(struct tty_struct *tty)
{
	struct fwtty_port *port = tty->driver_data;
	unsigned tiocm;

	spin_lock_bh(&port->lock);
	tiocm = (port->mctrl & MCTRL_MASK) | (port->mstatus & ~MCTRL_MASK);
	spin_unlock_bh(&port->lock);

	fwtty_dbg(port, "%x", tiocm);

	return tiocm;
}

static int fwtty_tiocmset(struct tty_struct *tty, unsigned set, unsigned clear)
{
	struct fwtty_port *port = tty->driver_data;

	fwtty_dbg(port, "set: %x clear: %x", set, clear);

	/*                                           */

	spin_lock_bh(&port->lock);
	port->mctrl &= ~(clear & MCTRL_MASK & 0xffff);
	port->mctrl |= set & MCTRL_MASK & 0xffff;
	__fwtty_write_port_status(port);
	spin_unlock_bh(&port->lock);
	return 0;
}

static int fwtty_get_icount(struct tty_struct *tty,
			    struct serial_icounter_struct *icount)
{
	struct fwtty_port *port = tty->driver_data;
	struct stats stats;

	memcpy(&stats, &port->stats, sizeof(stats));
	if (port->port.console)
		(*port->fwcon_ops->stats)(&stats, port->con_data);

	icount->cts = port->icount.cts;
	icount->dsr = port->icount.dsr;
	icount->rng = port->icount.rng;
	icount->dcd = port->icount.dcd;
	icount->rx  = port->icount.rx;
	icount->tx  = port->icount.tx + stats.xchars;
	icount->frame   = port->icount.frame;
	icount->overrun = port->icount.overrun;
	icount->parity  = port->icount.parity;
	icount->brk     = port->icount.brk;
	icount->buf_overrun = port->icount.overrun;
	return 0;
}

static void fwtty_proc_show_port(struct seq_file *m, struct fwtty_port *port)
{
	struct stats stats;

	memcpy(&stats, &port->stats, sizeof(stats));
	if (port->port.console)
		(*port->fwcon_ops->stats)(&stats, port->con_data);

	seq_printf(m, " addr:%012llx tx:%d rx:%d", port->rx_handler.offset,
		   port->icount.tx + stats.xchars, port->icount.rx);
	seq_printf(m, " cts:%d dsr:%d rng:%d dcd:%d", port->icount.cts,
		   port->icount.dsr, port->icount.rng, port->icount.dcd);
	seq_printf(m, " fe:%d oe:%d pe:%d brk:%d", port->icount.frame,
		   port->icount.overrun, port->icount.parity, port->icount.brk);
}

static void fwtty_debugfs_show_port(struct seq_file *m, struct fwtty_port *port)
{
	struct stats stats;

	memcpy(&stats, &port->stats, sizeof(stats));
	if (port->port.console)
		(*port->fwcon_ops->stats)(&stats, port->con_data);

	seq_printf(m, " dr:%d st:%d err:%d lost:%d", stats.dropped,
		   stats.tx_stall, stats.fifo_errs, stats.lost);
	seq_printf(m, " pkts:%d thr:%d wtrmk:%d", stats.sent, stats.throttled,
		   stats.watermark);

	if (port->port.console) {
		seq_puts(m, "\n    ");
		(*port->fwcon_ops->proc_show)(m, port->con_data);
	}

	dump_profile(m, &port->stats);
}

static void fwtty_debugfs_show_peer(struct seq_file *m, struct fwtty_peer *peer)
{
	int generation = peer->generation;

	smp_rmb();
	seq_printf(m, " %s:", dev_name(&peer->unit->device));
	seq_printf(m, " node:%04x gen:%d", peer->node_id, generation);
	seq_printf(m, " sp:%d max:%d guid:%016llx", peer->speed,
		   peer->max_payload, (unsigned long long) peer->guid);
	seq_printf(m, " mgmt:%012llx", (unsigned long long) peer->mgmt_addr);
	seq_printf(m, " addr:%012llx", (unsigned long long) peer->status_addr);
	seq_putc(m, '\n');
}

static int fwtty_proc_show(struct seq_file *m, void *v)
{
	struct fwtty_port *port;
	int i;

	seq_puts(m, "fwserinfo: 1.0 driver: 1.0\n");
	for (i = 0; i < MAX_TOTAL_PORTS && (port = fwtty_port_get(i)); ++i) {
		seq_printf(m, "%2d:", i);
		if (capable(CAP_SYS_ADMIN))
			fwtty_proc_show_port(m, port);
		fwtty_port_put(port);
		seq_puts(m, "\n");
	}
	return 0;
}

static int fwtty_debugfs_stats_show(struct seq_file *m, void *v)
{
	struct fw_serial *serial = m->private;
	struct fwtty_port *port;
	int i;

	for (i = 0; i < num_ports; ++i) {
		port = fwtty_port_get(serial->ports[i]->index);
		if (port) {
			seq_printf(m, "%2d:", port->index);
			fwtty_proc_show_port(m, port);
			fwtty_debugfs_show_port(m, port);
			fwtty_port_put(port);
			seq_puts(m, "\n");
		}
	}
	return 0;
}

static int fwtty_debugfs_peers_show(struct seq_file *m, void *v)
{
	struct fw_serial *serial = m->private;
	struct fwtty_peer *peer;

	rcu_read_lock();
	seq_printf(m, "card: %s  guid: %016llx\n",
		   dev_name(serial->card->device),
		   (unsigned long long) serial->card->guid);
	list_for_each_entry_rcu(peer, &serial->peer_list, list)
		fwtty_debugfs_show_peer(m, peer);
	rcu_read_unlock();
	return 0;
}

static int fwtty_proc_open(struct inode *inode, struct file *fp)
{
	return single_open(fp, fwtty_proc_show, NULL);
}

static int fwtty_stats_open(struct inode *inode, struct file *fp)
{
	return single_open(fp, fwtty_debugfs_stats_show, inode->i_private);
}

static int fwtty_peers_open(struct inode *inode, struct file *fp)
{
	return single_open(fp, fwtty_debugfs_peers_show, inode->i_private);
}

static const struct file_operations fwtty_stats_fops = {
	.owner =	THIS_MODULE,
	.open =		fwtty_stats_open,
	.read =		seq_read,
	.llseek =	seq_lseek,
	.release =	single_release,
};

static const struct file_operations fwtty_peers_fops = {
	.owner =	THIS_MODULE,
	.open =		fwtty_peers_open,
	.read =		seq_read,
	.llseek =	seq_lseek,
	.release =	single_release,
};

static const struct file_operations fwtty_proc_fops = {
	.owner =        THIS_MODULE,
	.open =         fwtty_proc_open,
	.read =         seq_read,
	.llseek =       seq_lseek,
	.release =      single_release,
};

static const struct tty_port_operations fwtty_port_ops = {
	.dtr_rts =		fwtty_port_dtr_rts,
	.carrier_raised =	fwtty_port_carrier_raised,
	.shutdown =		fwtty_port_shutdown,
	.activate =		fwtty_port_activate,
};

static const struct tty_operations fwtty_ops = {
	.open =			fwtty_open,
	.close =		fwtty_close,
	.hangup =		fwtty_hangup,
	.cleanup =		fwtty_cleanup,
	.install =		fwtty_install,
	.write =		fwtty_write,
	.write_room =		fwtty_write_room,
	.chars_in_buffer =	fwtty_chars_in_buffer,
	.send_xchar =           fwtty_send_xchar,
	.throttle =             fwtty_throttle,
	.unthrottle =           fwtty_unthrottle,
	.ioctl =		fwtty_ioctl,
	.set_termios =		fwtty_set_termios,
	.break_ctl =		fwtty_break_ctl,
	.tiocmget =		fwtty_tiocmget,
	.tiocmset =		fwtty_tiocmset,
	.get_icount =		fwtty_get_icount,
	.proc_fops =		&fwtty_proc_fops,
};

static const struct tty_operations fwloop_ops = {
	.open =			fwtty_open,
	.close =		fwtty_close,
	.hangup =		fwtty_hangup,
	.cleanup =		fwtty_cleanup,
	.install =		fwloop_install,
	.write =		fwtty_write,
	.write_room =		fwtty_write_room,
	.chars_in_buffer =	fwtty_chars_in_buffer,
	.send_xchar =           fwtty_send_xchar,
	.throttle =             fwtty_throttle,
	.unthrottle =           fwtty_unthrottle,
	.ioctl =		fwtty_ioctl,
	.set_termios =		fwtty_set_termios,
	.break_ctl =		fwtty_break_ctl,
	.tiocmget =		fwtty_tiocmget,
	.tiocmset =		fwtty_tiocmset,
	.get_icount =		fwtty_get_icount,
};

static inline int mgmt_pkt_expected_len(__be16 code)
{
	static const struct fwserial_mgmt_pkt pkt;

	switch (be16_to_cpu(code)) {
	case FWSC_VIRT_CABLE_PLUG:
		return sizeof(pkt.hdr) + sizeof(pkt.plug_req);

	case FWSC_VIRT_CABLE_PLUG_RSP:  /*               */
		return sizeof(pkt.hdr) + sizeof(pkt.plug_rsp);


	case FWSC_VIRT_CABLE_UNPLUG:
	case FWSC_VIRT_CABLE_UNPLUG_RSP:
	case FWSC_VIRT_CABLE_PLUG_RSP | FWSC_RSP_NACK:
	case FWSC_VIRT_CABLE_UNPLUG_RSP | FWSC_RSP_NACK:
		return sizeof(pkt.hdr);

	default:
		return -1;
	}
}

static inline void fill_plug_params(struct virt_plug_params *params,
				    struct fwtty_port *port)
{
	u64 status_addr = port->rx_handler.offset;
	u64 fifo_addr = port->rx_handler.offset + 4;
	size_t fifo_len = port->rx_handler.length - 4;

	params->status_hi = cpu_to_be32(status_addr >> 32);
	params->status_lo = cpu_to_be32(status_addr);
	params->fifo_hi = cpu_to_be32(fifo_addr >> 32);
	params->fifo_lo = cpu_to_be32(fifo_addr);
	params->fifo_len = cpu_to_be32(fifo_len);
}

static inline void fill_plug_req(struct fwserial_mgmt_pkt *pkt,
				 struct fwtty_port *port)
{
	pkt->hdr.code = cpu_to_be16(FWSC_VIRT_CABLE_PLUG);
	pkt->hdr.len = cpu_to_be16(mgmt_pkt_expected_len(pkt->hdr.code));
	fill_plug_params(&pkt->plug_req, port);
}

static inline void fill_plug_rsp_ok(struct fwserial_mgmt_pkt *pkt,
				    struct fwtty_port *port)
{
	pkt->hdr.code = cpu_to_be16(FWSC_VIRT_CABLE_PLUG_RSP);
	pkt->hdr.len = cpu_to_be16(mgmt_pkt_expected_len(pkt->hdr.code));
	fill_plug_params(&pkt->plug_rsp, port);
}

static inline void fill_plug_rsp_nack(struct fwserial_mgmt_pkt *pkt)
{
	pkt->hdr.code = cpu_to_be16(FWSC_VIRT_CABLE_PLUG_RSP | FWSC_RSP_NACK);
	pkt->hdr.len = cpu_to_be16(mgmt_pkt_expected_len(pkt->hdr.code));
}

static inline void fill_unplug_req(struct fwserial_mgmt_pkt *pkt)
{
	pkt->hdr.code = cpu_to_be16(FWSC_VIRT_CABLE_UNPLUG);
	pkt->hdr.len = cpu_to_be16(mgmt_pkt_expected_len(pkt->hdr.code));
}

static inline void fill_unplug_rsp_nack(struct fwserial_mgmt_pkt *pkt)
{
	pkt->hdr.code = cpu_to_be16(FWSC_VIRT_CABLE_UNPLUG_RSP | FWSC_RSP_NACK);
	pkt->hdr.len = cpu_to_be16(mgmt_pkt_expected_len(pkt->hdr.code));
}

static inline void fill_unplug_rsp_ok(struct fwserial_mgmt_pkt *pkt)
{
	pkt->hdr.code = cpu_to_be16(FWSC_VIRT_CABLE_UNPLUG_RSP);
	pkt->hdr.len = cpu_to_be16(mgmt_pkt_expected_len(pkt->hdr.code));
}

static void fwserial_virt_plug_complete(struct fwtty_peer *peer,
					struct virt_plug_params *params)
{
	struct fwtty_port *port = peer->port;

	peer->status_addr = be32_to_u64(params->status_hi, params->status_lo);
	peer->fifo_addr = be32_to_u64(params->fifo_hi, params->fifo_lo);
	peer->fifo_len = be32_to_cpu(params->fifo_len);
	peer_set_state(peer, FWPS_ATTACHED);

	/*                                             */
	spin_lock_bh(&port->lock);
	port->max_payload = min(peer->max_payload, peer->fifo_len);
	dma_fifo_change_tx_limit(&port->tx_fifo, port->max_payload);
	spin_unlock_bh(&peer->port->lock);

	if (port->port.console && port->fwcon_ops->notify != NULL)
		(*port->fwcon_ops->notify)(FWCON_NOTIFY_ATTACH, port->con_data);

	fwtty_info(&peer->unit, "peer (guid:%016llx) connected on %s",
		   (unsigned long long)peer->guid, dev_name(port->device));
}

static inline int fwserial_send_mgmt_sync(struct fwtty_peer *peer,
					  struct fwserial_mgmt_pkt *pkt)
{
	int generation;
	int rcode, tries = 5;

	do {
		generation = peer->generation;
		smp_rmb();

		rcode = fw_run_transaction(peer->serial->card,
					   TCODE_WRITE_BLOCK_REQUEST,
					   peer->node_id,
					   generation, peer->speed,
					   peer->mgmt_addr,
					   pkt, be16_to_cpu(pkt->hdr.len));
		if (rcode == RCODE_BUSY || rcode == RCODE_SEND_ERROR ||
		    rcode == RCODE_GENERATION) {
			fwtty_dbg(&peer->unit, "mgmt write error: %d", rcode);
			continue;
		} else
			break;
	} while (--tries > 0);
	return rcode;
}

/* 
                                                               
  
                                                           
                                                  
 */
static struct fwtty_port *fwserial_claim_port(struct fwtty_peer *peer,
					      int index)
{
	struct fwtty_port *port;

	if (index < 0 || index >= num_ports)
		return ERR_PTR(-EINVAL);

	/*                                                           */
	synchronize_rcu();

	port = peer->serial->ports[index];
	spin_lock_bh(&port->lock);
	if (!rcu_access_pointer(port->peer))
		rcu_assign_pointer(port->peer, peer);
	else
		port = ERR_PTR(-EBUSY);
	spin_unlock_bh(&port->lock);

	return port;
}

/* 
                                                          
  
                                                    
                                                  
 */
static struct fwtty_port *fwserial_find_port(struct fwtty_peer *peer)
{
	struct fwtty_port **ports = peer->serial->ports;
	int i;

	/*                                                           */
	synchronize_rcu();

	/*                                                           */

	/*                                                                 */
	for (i = 0; i < num_ttys; ++i) {
		spin_lock_bh(&ports[i]->lock);
		if (!ports[i]->peer) {
			/*            */
			rcu_assign_pointer(ports[i]->peer, peer);
			spin_unlock_bh(&ports[i]->lock);
			return ports[i];
		}
		spin_unlock_bh(&ports[i]->lock);
	}
	return NULL;
}

static void fwserial_release_port(struct fwtty_port *port, bool reset)
{
	/*                                          */
	if (reset)
		fwtty_update_port_status(port, 0);

	spin_lock_bh(&port->lock);

	/*                                                   */
	port->max_payload = link_speed_to_max_payload(SCODE_100);
	dma_fifo_change_tx_limit(&port->tx_fifo, port->max_payload);

	rcu_assign_pointer(port->peer, NULL);
	spin_unlock_bh(&port->lock);

	if (port->port.console && port->fwcon_ops->notify != NULL)
		(*port->fwcon_ops->notify)(FWCON_NOTIFY_DETACH, port->con_data);
}

static void fwserial_plug_timeout(unsigned long data)
{
	struct fwtty_peer *peer = (struct fwtty_peer *) data;
	struct fwtty_port *port;

	spin_lock_bh(&peer->lock);
	if (peer->state != FWPS_PLUG_PENDING) {
		spin_unlock_bh(&peer->lock);
		return;
	}

	port = peer_revert_state(peer);
	spin_unlock_bh(&peer->lock);

	if (port)
		fwserial_release_port(port, false);
}

/* 
                                                           
  
                                                              
                                                              
 */
static int fwserial_connect_peer(struct fwtty_peer *peer)
{
	struct fwtty_port *port;
	struct fwserial_mgmt_pkt *pkt;
	int err, rcode;

	pkt = kmalloc(sizeof(*pkt), GFP_KERNEL);
	if (!pkt)
		return -ENOMEM;

	port = fwserial_find_port(peer);
	if (!port) {
		fwtty_err(&peer->unit, "avail ports in use");
		err = -EBUSY;
		goto free_pkt;
	}

	spin_lock_bh(&peer->lock);

	/*                                                                 */
	if (peer->state != FWPS_NOT_ATTACHED) {
		err = -EBUSY;
		goto release_port;
	}

	peer->port = port;
	peer_set_state(peer, FWPS_PLUG_PENDING);

	fill_plug_req(pkt, peer->port);

	setup_timer(&peer->timer, fwserial_plug_timeout, (unsigned long)peer);
	mod_timer(&peer->timer, jiffies + VIRT_CABLE_PLUG_TIMEOUT);
	spin_unlock_bh(&peer->lock);

	rcode = fwserial_send_mgmt_sync(peer, pkt);

	spin_lock_bh(&peer->lock);
	if (peer->state == FWPS_PLUG_PENDING && rcode != RCODE_COMPLETE) {
		if (rcode == RCODE_CONFLICT_ERROR)
			err = -EAGAIN;
		else
			err = -EIO;
		goto cancel_timer;
	}
	spin_unlock_bh(&peer->lock);

	kfree(pkt);
	return 0;

cancel_timer:
	del_timer(&peer->timer);
	peer_revert_state(peer);
release_port:
	spin_unlock_bh(&peer->lock);
	fwserial_release_port(port, false);
free_pkt:
	kfree(pkt);
	return err;
}

/* 
                        
                                                                 
                                                                   
                             
  
                                                                 
               
 */
static void fwserial_close_port(struct tty_driver *driver,
				struct fwtty_port *port)
{
	struct tty_struct *tty;

	mutex_lock(&port->port.mutex);
	tty = tty_port_tty_get(&port->port);
	if (tty) {
		tty_vhangup(tty);
		tty_kref_put(tty);
	}
	mutex_unlock(&port->port.mutex);

	if (driver == fwloop_driver)
		tty_unregister_device(driver, loop_idx(port));
	else
		tty_unregister_device(driver, port->index);
}

/* 
                                                               
                          
  
                                                 
 */
static struct fw_serial *fwserial_lookup(struct fw_card *card)
{
	struct fw_serial *serial;

	list_for_each_entry(serial, &fwserial_list, list) {
		if (card == serial->card)
			return serial;
	}

	return NULL;
}

/* 
                                                                     
                          
  
                                                    
 */
static struct fw_serial *__fwserial_lookup_rcu(struct fw_card *card)
{
	struct fw_serial *serial;

	list_for_each_entry_rcu(serial, &fwserial_list, list) {
		if (card == serial->card)
			return serial;
	}

	return NULL;
}

/* 
                                                                               
  
                                                                              
                         
                                                                        
                                                                    
                                                                 
                                               
  
                                                  
 */
static struct fwtty_peer *__fwserial_peer_by_node_id(struct fw_card *card,
						     int generation, int id)
{
	struct fw_serial *serial;
	struct fwtty_peer *peer;

	serial = __fwserial_lookup_rcu(card);
	if (!serial) {
		/*
                                                         
                                                                 
                                                              
                                        
   */
		fwtty_err(card, "unknown card (guid %016llx)",
			  (unsigned long long) card->guid);
		return NULL;
	}

	list_for_each_entry_rcu(peer, &serial->peer_list, list) {
		int g = peer->generation;
		smp_rmb();
		if (generation == g && id == peer->node_id)
			return peer;
	}

	return NULL;
}

#ifdef DEBUG
static void __dump_peer_list(struct fw_card *card)
{
	struct fw_serial *serial;
	struct fwtty_peer *peer;

	serial = __fwserial_lookup_rcu(card);
	if (!serial)
		return;

	list_for_each_entry_rcu(peer, &serial->peer_list, list) {
		int g = peer->generation;
		smp_rmb();
		fwtty_dbg(card, "peer(%d:%x) guid: %016llx\n", g,
			  peer->node_id, (unsigned long long) peer->guid);
	}
}
#else
#define __dump_peer_list(s)
#endif

static void fwserial_auto_connect(struct work_struct *work)
{
	struct fwtty_peer *peer = to_peer(to_delayed_work(work), connect);
	int err;

	err = fwserial_connect_peer(peer);
	if (err == -EAGAIN && ++peer->connect_retries < MAX_CONNECT_RETRIES)
		schedule_delayed_work(&peer->connect, CONNECT_RETRY_DELAY);
}

/* 
                                                                          
                                                                          
                                                         
  
                                                                            
                                                                        
                                                                            
                                                                
  
                                                                       
                                                
  
                                                                          
                                                           
 */
static int fwserial_add_peer(struct fw_serial *serial, struct fw_unit *unit)
{
	struct device *dev = &unit->device;
	struct fw_device  *parent = fw_parent_device(unit);
	struct fwtty_peer *peer;
	struct fw_csr_iterator ci;
	int key, val;
	int generation;

	peer = kzalloc(sizeof(*peer), GFP_KERNEL);
	if (!peer)
		return -ENOMEM;

	peer_set_state(peer, FWPS_NOT_ATTACHED);

	dev_set_drvdata(dev, peer);
	peer->unit = unit;
	peer->guid = (u64)parent->config_rom[3] << 32 | parent->config_rom[4];
	peer->speed = parent->max_speed;
	peer->max_payload = min(device_max_receive(parent),
				link_speed_to_max_payload(peer->speed));

	generation = parent->generation;
	smp_rmb();
	peer->node_id = parent->node_id;
	smp_wmb();
	peer->generation = generation;

	/*                                                    */
	fw_csr_iterator_init(&ci, unit->directory);
	while (fw_csr_iterator_next(&ci, &key, &val)) {
		if (key == (CSR_OFFSET | CSR_DEPENDENT_INFO)) {
			peer->mgmt_addr = CSR_REGISTER_BASE + 4 * val;
			break;
		}
	}
	if (peer->mgmt_addr == 0ULL) {
		/*
                                                           
                                                     
   */
		peer_set_state(peer, FWPS_NO_MGMT_ADDR);
	}

	spin_lock_init(&peer->lock);
	peer->port = NULL;

	init_timer(&peer->timer);
	INIT_WORK(&peer->work, NULL);
	INIT_DELAYED_WORK(&peer->connect, fwserial_auto_connect);

	/*                                      */
	peer->serial = serial;
	list_add_rcu(&peer->list, &serial->peer_list);

	fwtty_info(&peer->unit, "peer added (guid:%016llx)",
		   (unsigned long long)peer->guid);

	/*                                                       */
	if (parent->is_local) {
		serial->self = peer;
		if (create_loop_dev) {
			struct fwtty_port *port;
			port = fwserial_claim_port(peer, num_ttys);
			if (!IS_ERR(port)) {
				struct virt_plug_params params;

				spin_lock_bh(&peer->lock);
				peer->port = port;
				fill_plug_params(&params, port);
				fwserial_virt_plug_complete(peer, &params);
				spin_unlock_bh(&peer->lock);

				fwtty_write_port_status(port);
			}
		}

	} else if (auto_connect) {
		/*                                                     */
		schedule_delayed_work(&peer->connect, 1);
	}

	return 0;
}

/* 
                                                                   
  
                                                                
                                                                 
  
                                                                    
                                                 
 */
static void fwserial_remove_peer(struct fwtty_peer *peer)
{
	struct fwtty_port *port;

	spin_lock_bh(&peer->lock);
	peer_set_state(peer, FWPS_GONE);
	spin_unlock_bh(&peer->lock);

	cancel_delayed_work_sync(&peer->connect);
	cancel_work_sync(&peer->work);

	spin_lock_bh(&peer->lock);
	/*                                            */
	if (peer == peer->serial->self)
		peer->serial->self = NULL;

	/*                                               */
	del_timer(&peer->timer);

	port = peer->port;
	peer->port = NULL;

	list_del_rcu(&peer->list);

	fwtty_info(&peer->unit, "peer removed (guid:%016llx)",
		   (unsigned long long)peer->guid);

	spin_unlock_bh(&peer->lock);

	if (port)
		fwserial_release_port(port, true);

	synchronize_rcu();
	kfree(peer);
}

/* 
                                                                          
                                                                        
  
                                                                      
                                                                           
                                            
  
                                                                          
                                                                      
                                                                 
  
                                                                            
                                 
 */
static int fwserial_create(struct fw_unit *unit)
{
	struct fw_device *parent = fw_parent_device(unit);
	struct fw_card *card = parent->card;
	struct fw_serial *serial;
	struct fwtty_port *port;
	struct device *tty_dev;
	int i, j;
	int err;

	serial = kzalloc(sizeof(*serial), GFP_KERNEL);
	if (!serial)
		return -ENOMEM;

	kref_init(&serial->kref);
	serial->card = card;
	INIT_LIST_HEAD(&serial->peer_list);

	for (i = 0; i < num_ports; ++i) {
		port = kzalloc(sizeof(*port), GFP_KERNEL);
		if (!port) {
			err = -ENOMEM;
			goto free_ports;
		}
		tty_port_init(&port->port);
		port->index = FWTTY_INVALID_INDEX;
		port->port.ops = &fwtty_port_ops;
		port->serial = serial;

		spin_lock_init(&port->lock);
		INIT_DELAYED_WORK(&port->drain, fwtty_drain_tx);
		INIT_DELAYED_WORK(&port->emit_breaks, fwtty_emit_breaks);
		INIT_WORK(&port->hangup, fwtty_do_hangup);
		INIT_WORK(&port->push, fwtty_pushrx);
		INIT_LIST_HEAD(&port->buf_list);
		init_waitqueue_head(&port->wait_tx);
		port->max_payload = link_speed_to_max_payload(SCODE_100);
		dma_fifo_init(&port->tx_fifo);

		rcu_assign_pointer(port->peer, NULL);
		serial->ports[i] = port;

		/*                                                          */
		port->rx_handler.length = FWTTY_PORT_RXFIFO_LEN + 4;
		port->rx_handler.address_callback = fwtty_port_handler;
		port->rx_handler.callback_data = port;
		/*
                                                                  
                                                       
   */
		err = fw_core_add_address_handler(&port->rx_handler,
						  &fw_high_memory_region);
		if (err) {
			kfree(port);
			goto free_ports;
		}
	}
	/*                              */

	err = fwtty_ports_add(serial);
	if (err) {
		fwtty_err(&unit, "no space in port table");
		goto free_ports;
	}

	for (j = 0; j < num_ttys; ++j) {
		tty_dev = tty_port_register_device(&serial->ports[j]->port,
						   fwtty_driver,
						   serial->ports[j]->index,
						   card->device);
		if (IS_ERR(tty_dev)) {
			err = PTR_ERR(tty_dev);
			fwtty_err(&unit, "register tty device error (%d)", err);
			goto unregister_ttys;
		}

		serial->ports[j]->device = tty_dev;
	}
	/*                              */

	if (create_loop_dev) {
		struct device *loop_dev;

		loop_dev = tty_port_register_device(&serial->ports[j]->port,
						    fwloop_driver,
						    loop_idx(serial->ports[j]),
						    card->device);
		if (IS_ERR(loop_dev)) {
			err = PTR_ERR(loop_dev);
			fwtty_err(&unit, "create loop device failed (%d)", err);
			goto unregister_ttys;
		}
		serial->ports[j]->device = loop_dev;
		serial->ports[j]->loopback = true;
	}

	if (!IS_ERR_OR_NULL(fwserial_debugfs)) {
		serial->debugfs = debugfs_create_dir(dev_name(&unit->device),
						     fwserial_debugfs);
		if (!IS_ERR_OR_NULL(serial->debugfs)) {
			debugfs_create_file("peers", 0444, serial->debugfs,
					    serial, &fwtty_peers_fops);
			debugfs_create_file("stats", 0444, serial->debugfs,
					    serial, &fwtty_stats_fops);
		}
	}

	list_add_rcu(&serial->list, &fwserial_list);

	fwtty_notice(&unit, "TTY over FireWire on device %s (guid %016llx)",
		     dev_name(card->device), (unsigned long long) card->guid);

	err = fwserial_add_peer(serial, unit);
	if (!err)
		return 0;

	fwtty_err(&unit, "unable to add peer unit device (%d)", err);

	/*                                  */
	debugfs_remove_recursive(serial->debugfs);

	list_del_rcu(&serial->list);
	if (create_loop_dev)
		tty_unregister_device(fwloop_driver, loop_idx(serial->ports[j]));
unregister_ttys:
	for (--j; j >= 0; --j)
		tty_unregister_device(fwtty_driver, serial->ports[j]->index);
	kref_put(&serial->kref, fwserial_destroy);
	return err;

free_ports:
	for (--i; i >= 0; --i) {
		tty_port_destroy(&serial->ports[i]->port);
		kfree(serial->ports[i]);
	}
	kfree(serial);
	return err;
}

/* 
                                                                        
  
                                                               
                                                                          
                               
                                                                     
  
                                                                           
                                                                         
                                               
  
                                          
                                                                              
                                                                       
                        
                                                                    
  
                                                                             
                                                                            
                                                                            
                                                                               
                                                                    
  
                                                                                
                                                                      
                                                                           
                                                                   
  
                                                                   
                                                                                
                                                                          
                                                                         
                                                                       
                                  
 */
static int fwserial_probe(struct device *dev)
{
	struct fw_unit *unit = fw_unit(dev);
	struct fw_serial *serial;
	int err;

	mutex_lock(&fwserial_list_mutex);
	serial = fwserial_lookup(fw_parent_device(unit)->card);
	if (!serial)
		err = fwserial_create(unit);
	else
		err = fwserial_add_peer(serial, unit);
	mutex_unlock(&fwserial_list_mutex);
	return err;
}

/* 
                                                                           
  
                                                                            
                                                                            
                                                                          
                                          
 */
static int fwserial_remove(struct device *dev)
{
	struct fwtty_peer *peer = dev_get_drvdata(dev);
	struct fw_serial *serial = peer->serial;
	int i;

	mutex_lock(&fwserial_list_mutex);
	fwserial_remove_peer(peer);

	if (list_empty(&serial->peer_list)) {
		/*                                    */
		list_del_rcu(&serial->list);

		debugfs_remove_recursive(serial->debugfs);

		for (i = 0; i < num_ttys; ++i)
			fwserial_close_port(fwtty_driver, serial->ports[i]);
		if (create_loop_dev)
			fwserial_close_port(fwloop_driver, serial->ports[i]);
		kref_put(&serial->kref, fwserial_destroy);
	}
	mutex_unlock(&fwserial_list_mutex);

	return 0;
}

/* 
                                                                          
  
                                                                              
                                                                               
                                                
  
                                                                          
                                                                         
                                                                           
  
                                                                             
                                                                            
 */
static void fwserial_update(struct fw_unit *unit)
{
	struct fw_device *parent = fw_parent_device(unit);
	struct fwtty_peer *peer = dev_get_drvdata(&unit->device);
	int generation;

	generation = parent->generation;
	smp_rmb();
	peer->node_id = parent->node_id;
	smp_wmb();
	peer->generation = generation;
}

static const struct ieee1394_device_id fwserial_id_table[] = {
	{
		.match_flags  = IEEE1394_MATCH_SPECIFIER_ID |
				IEEE1394_MATCH_VERSION,
		.specifier_id = LINUX_VENDOR_ID,
		.version      = FWSERIAL_VERSION,
	},
	{ }
};

static struct fw_driver fwserial_driver = {
	.driver = {
		.owner  = THIS_MODULE,
		.name   = KBUILD_MODNAME,
		.bus    = &fw_bus_type,
		.probe  = fwserial_probe,
		.remove = fwserial_remove,
	},
	.update   = fwserial_update,
	.id_table = fwserial_id_table,
};

#define FW_UNIT_SPECIFIER(id)	((CSR_SPECIFIER_ID << 24) | (id))
#define FW_UNIT_VERSION(ver)	((CSR_VERSION << 24) | (ver))
#define FW_UNIT_ADDRESS(ofs)	(((CSR_OFFSET | CSR_DEPENDENT_INFO) << 24)  \
				 | (((ofs) - CSR_REGISTER_BASE) >> 2))
/*                                                                        
                         
 */
#define FW_ROM_LEN(quads)	((quads) << 16)
#define FW_ROM_DESCRIPTOR(ofs)	(((CSR_LEAF | CSR_DESCRIPTOR) << 24) | (ofs))

struct fwserial_unit_directory_data {
	u32	len_crc;
	u32	unit_specifier;
	u32	unit_sw_version;
	u32	unit_addr_offset;
	u32	desc1_ofs;
	u32	desc1_len_crc;
	u32	desc1_data[5];
} __packed;

static struct fwserial_unit_directory_data fwserial_unit_directory_data = {
	.len_crc = FW_ROM_LEN(4),
	.unit_specifier = FW_UNIT_SPECIFIER(LINUX_VENDOR_ID),
	.unit_sw_version = FW_UNIT_VERSION(FWSERIAL_VERSION),
	.desc1_ofs = FW_ROM_DESCRIPTOR(1),
	.desc1_len_crc = FW_ROM_LEN(5),
	.desc1_data = {
		0x00000000,			/*                          */
		0x00000000,			/*                          */
		0x4c696e75,			/*                          */
		0x78205454,
		0x59000000,
	},
};

static struct fw_descriptor fwserial_unit_directory = {
	.length = sizeof(fwserial_unit_directory_data) / sizeof(u32),
	.key    = (CSR_DIRECTORY | CSR_UNIT) << 24,
	.data   = (u32 *)&fwserial_unit_directory_data,
};

/*
                                                                           
                                                         
 */
static const struct fw_address_region fwserial_mgmt_addr_region = {
	.start = CSR_REGISTER_BASE + 0x1e0000ULL,
	.end = 0x1000000000000ULL,
};

static struct fw_address_handler fwserial_mgmt_addr_handler;

/* 
                                                                 
                           
  
                                                                             
  
                                                                              
                                                                           
                                                  
  
                                                                  
                                   
 */
static void fwserial_handle_plug_req(struct work_struct *work)
{
	struct fwtty_peer *peer = to_peer(work, work);
	struct virt_plug_params *plug_req = &peer->work_params.plug_req;
	struct fwtty_port *port;
	struct fwserial_mgmt_pkt *pkt;
	int rcode;

	pkt = kmalloc(sizeof(*pkt), GFP_KERNEL);
	if (!pkt)
		return;

	port = fwserial_find_port(peer);

	spin_lock_bh(&peer->lock);

	switch (peer->state) {
	case FWPS_NOT_ATTACHED:
		if (!port) {
			fwtty_err(&peer->unit, "no more ports avail");
			fill_plug_rsp_nack(pkt);
		} else {
			peer->port = port;
			fill_plug_rsp_ok(pkt, peer->port);
			peer_set_state(peer, FWPS_PLUG_RESPONDING);
			/*                            */
			port = NULL;
		}
		break;

	case FWPS_PLUG_PENDING:
		if (peer->serial->card->guid > peer->guid)
			goto cleanup;

		/*                                                       */
		del_timer(&peer->timer);
		fill_plug_rsp_ok(pkt, peer->port);
		peer_set_state(peer, FWPS_PLUG_RESPONDING);
		break;

	default:
		fill_plug_rsp_nack(pkt);
	}

	spin_unlock_bh(&peer->lock);
	if (port)
		fwserial_release_port(port, false);

	rcode = fwserial_send_mgmt_sync(peer, pkt);

	spin_lock_bh(&peer->lock);
	if (peer->state == FWPS_PLUG_RESPONDING) {
		if (rcode == RCODE_COMPLETE) {
			struct fwtty_port *tmp = peer->port;

			fwserial_virt_plug_complete(peer, plug_req);
			spin_unlock_bh(&peer->lock);

			fwtty_write_port_status(tmp);
			spin_lock_bh(&peer->lock);
		} else {
			fwtty_err(&peer->unit, "PLUG_RSP error (%d)", rcode);
			port = peer_revert_state(peer);
		}
	}
cleanup:
	spin_unlock_bh(&peer->lock);
	if (port)
		fwserial_release_port(port, false);
	kfree(pkt);
	return;
}

static void fwserial_handle_unplug_req(struct work_struct *work)
{
	struct fwtty_peer *peer = to_peer(work, work);
	struct fwtty_port *port = NULL;
	struct fwserial_mgmt_pkt *pkt;
	int rcode;

	pkt = kmalloc(sizeof(*pkt), GFP_KERNEL);
	if (!pkt)
		return;

	spin_lock_bh(&peer->lock);

	switch (peer->state) {
	case FWPS_ATTACHED:
		fill_unplug_rsp_ok(pkt);
		peer_set_state(peer, FWPS_UNPLUG_RESPONDING);
		break;

	case FWPS_UNPLUG_PENDING:
		if (peer->serial->card->guid > peer->guid)
			goto cleanup;

		/*                           */
		del_timer(&peer->timer);
		fill_unplug_rsp_ok(pkt);
		peer_set_state(peer, FWPS_UNPLUG_RESPONDING);
		break;

	default:
		fill_unplug_rsp_nack(pkt);
	}

	spin_unlock_bh(&peer->lock);

	rcode = fwserial_send_mgmt_sync(peer, pkt);

	spin_lock_bh(&peer->lock);
	if (peer->state == FWPS_UNPLUG_RESPONDING) {
		if (rcode != RCODE_COMPLETE)
			fwtty_err(&peer->unit, "UNPLUG_RSP error (%d)", rcode);
		port = peer_revert_state(peer);
	}
cleanup:
	spin_unlock_bh(&peer->lock);
	if (port)
		fwserial_release_port(port, true);
	kfree(pkt);
	return;
}

static int fwserial_parse_mgmt_write(struct fwtty_peer *peer,
				     struct fwserial_mgmt_pkt *pkt,
				     unsigned long long addr,
				     size_t len)
{
	struct fwtty_port *port = NULL;
	bool reset = false;
	int rcode;

	if (addr != fwserial_mgmt_addr_handler.offset || len < sizeof(pkt->hdr))
		return RCODE_ADDRESS_ERROR;

	if (len != be16_to_cpu(pkt->hdr.len) ||
	    len != mgmt_pkt_expected_len(pkt->hdr.code))
		return RCODE_DATA_ERROR;

	spin_lock_bh(&peer->lock);
	if (peer->state == FWPS_GONE) {
		/*
                                                      
                                                     
                                                        
                                               
   */
		fwtty_err(&peer->unit, "peer already removed");
		spin_unlock_bh(&peer->lock);
		return RCODE_ADDRESS_ERROR;
	}

	rcode = RCODE_COMPLETE;

	fwtty_dbg(&peer->unit, "mgmt: hdr.code: %04hx", pkt->hdr.code);

	switch (be16_to_cpu(pkt->hdr.code) & FWSC_CODE_MASK) {
	case FWSC_VIRT_CABLE_PLUG:
		if (work_pending(&peer->work)) {
			fwtty_err(&peer->unit, "plug req: busy");
			rcode = RCODE_CONFLICT_ERROR;

		} else {
			peer->work_params.plug_req = pkt->plug_req;
			PREPARE_WORK(&peer->work, fwserial_handle_plug_req);
			queue_work(system_unbound_wq, &peer->work);
		}
		break;

	case FWSC_VIRT_CABLE_PLUG_RSP:
		if (peer->state != FWPS_PLUG_PENDING) {
			rcode = RCODE_CONFLICT_ERROR;

		} else if (be16_to_cpu(pkt->hdr.code) & FWSC_RSP_NACK) {
			fwtty_notice(&peer->unit, "NACK plug rsp");
			port = peer_revert_state(peer);

		} else {
			struct fwtty_port *tmp = peer->port;

			fwserial_virt_plug_complete(peer, &pkt->plug_rsp);
			spin_unlock_bh(&peer->lock);

			fwtty_write_port_status(tmp);
			spin_lock_bh(&peer->lock);
		}
		break;

	case FWSC_VIRT_CABLE_UNPLUG:
		if (work_pending(&peer->work)) {
			fwtty_err(&peer->unit, "unplug req: busy");
			rcode = RCODE_CONFLICT_ERROR;
		} else {
			PREPARE_WORK(&peer->work, fwserial_handle_unplug_req);
			queue_work(system_unbound_wq, &peer->work);
		}
		break;

	case FWSC_VIRT_CABLE_UNPLUG_RSP:
		if (peer->state != FWPS_UNPLUG_PENDING)
			rcode = RCODE_CONFLICT_ERROR;
		else {
			if (be16_to_cpu(pkt->hdr.code) & FWSC_RSP_NACK)
				fwtty_notice(&peer->unit, "NACK unplug?");
			port = peer_revert_state(peer);
			reset = true;
		}
		break;

	default:
		fwtty_err(&peer->unit, "unknown mgmt code %d",
			  be16_to_cpu(pkt->hdr.code));
		rcode = RCODE_DATA_ERROR;
	}
	spin_unlock_bh(&peer->lock);

	if (port)
		fwserial_release_port(port, reset);

	return rcode;
}

/* 
                                                               
                                                                             
  
                                                                               
                 
 */
static void fwserial_mgmt_handler(struct fw_card *card,
				  struct fw_request *request,
				  int tcode, int destination, int source,
				  int generation,
				  unsigned long long addr,
				  void *data, size_t len,
				  void *callback_data)
{
	struct fwserial_mgmt_pkt *pkt = data;
	struct fwtty_peer *peer;
	int rcode;

	rcu_read_lock();
	peer = __fwserial_peer_by_node_id(card, generation, source);
	if (!peer) {
		fwtty_dbg(card, "peer(%d:%x) not found", generation, source);
		__dump_peer_list(card);
		rcode = RCODE_CONFLICT_ERROR;

	} else {
		switch (tcode) {
		case TCODE_WRITE_BLOCK_REQUEST:
			rcode = fwserial_parse_mgmt_write(peer, pkt, addr, len);
			break;

		default:
			rcode = RCODE_TYPE_ERROR;
		}
	}

	rcu_read_unlock();
	fw_send_response(card, request, rcode);
}

static int __init fwserial_init(void)
{
	int err, num_loops = !!(create_loop_dev);

	/*                                                */
	fwserial_debugfs = debugfs_create_dir(KBUILD_MODNAME, NULL);

	/*                                                                 */
	if (num_ttys + num_loops > MAX_CARD_PORTS)
		num_ttys = MAX_CARD_PORTS - num_loops;
	num_ports = num_ttys + num_loops;

	fwtty_driver = tty_alloc_driver(MAX_TOTAL_PORTS, TTY_DRIVER_REAL_RAW
					| TTY_DRIVER_DYNAMIC_DEV);
	if (IS_ERR(fwtty_driver)) {
		err = PTR_ERR(fwtty_driver);
		return err;
	}

	fwtty_driver->driver_name	= KBUILD_MODNAME;
	fwtty_driver->name		= tty_dev_name;
	fwtty_driver->major		= 0;
	fwtty_driver->minor_start	= 0;
	fwtty_driver->type		= TTY_DRIVER_TYPE_SERIAL;
	fwtty_driver->subtype		= SERIAL_TYPE_NORMAL;
	fwtty_driver->init_termios	    = tty_std_termios;
	fwtty_driver->init_termios.c_cflag  |= CLOCAL;
	tty_set_operations(fwtty_driver, &fwtty_ops);

	err = tty_register_driver(fwtty_driver);
	if (err) {
		driver_err("register tty driver failed (%d)", err);
		goto put_tty;
	}

	if (create_loop_dev) {
		fwloop_driver = tty_alloc_driver(MAX_TOTAL_PORTS / num_ports,
						 TTY_DRIVER_REAL_RAW
						 | TTY_DRIVER_DYNAMIC_DEV);
		if (IS_ERR(fwloop_driver)) {
			err = PTR_ERR(fwloop_driver);
			goto unregister_driver;
		}

		fwloop_driver->driver_name	= KBUILD_MODNAME "_loop";
		fwloop_driver->name		= loop_dev_name;
		fwloop_driver->major		= 0;
		fwloop_driver->minor_start	= 0;
		fwloop_driver->type		= TTY_DRIVER_TYPE_SERIAL;
		fwloop_driver->subtype		= SERIAL_TYPE_NORMAL;
		fwloop_driver->init_termios	    = tty_std_termios;
		fwloop_driver->init_termios.c_cflag  |= CLOCAL;
		tty_set_operations(fwloop_driver, &fwloop_ops);

		err = tty_register_driver(fwloop_driver);
		if (err) {
			driver_err("register loop driver failed (%d)", err);
			goto put_loop;
		}
	}

	fwtty_txn_cache = kmem_cache_create("fwtty_txn_cache",
					    sizeof(struct fwtty_transaction),
					    0, 0, fwtty_txn_constructor);
	if (!fwtty_txn_cache) {
		err = -ENOMEM;
		goto unregister_loop;
	}

	/*
                                                                    
                                                                
                                                                       
                                                                     
                                            
  */
	fwserial_mgmt_addr_handler.length = sizeof(struct fwserial_mgmt_pkt);
	fwserial_mgmt_addr_handler.address_callback = fwserial_mgmt_handler;

	err = fw_core_add_address_handler(&fwserial_mgmt_addr_handler,
					  &fwserial_mgmt_addr_region);
	if (err) {
		driver_err("add management handler failed (%d)", err);
		goto destroy_cache;
	}

	fwserial_unit_directory_data.unit_addr_offset =
		FW_UNIT_ADDRESS(fwserial_mgmt_addr_handler.offset);
	err = fw_core_add_descriptor(&fwserial_unit_directory);
	if (err) {
		driver_err("add unit descriptor failed (%d)", err);
		goto remove_handler;
	}

	err = driver_register(&fwserial_driver.driver);
	if (err) {
		driver_err("register fwserial driver failed (%d)", err);
		goto remove_descriptor;
	}

	return 0;

remove_descriptor:
	fw_core_remove_descriptor(&fwserial_unit_directory);
remove_handler:
	fw_core_remove_address_handler(&fwserial_mgmt_addr_handler);
destroy_cache:
	kmem_cache_destroy(fwtty_txn_cache);
unregister_loop:
	if (create_loop_dev)
		tty_unregister_driver(fwloop_driver);
put_loop:
	if (create_loop_dev)
		put_tty_driver(fwloop_driver);
unregister_driver:
	tty_unregister_driver(fwtty_driver);
put_tty:
	put_tty_driver(fwtty_driver);
	debugfs_remove_recursive(fwserial_debugfs);
	return err;
}

static void __exit fwserial_exit(void)
{
	driver_unregister(&fwserial_driver.driver);
	fw_core_remove_descriptor(&fwserial_unit_directory);
	fw_core_remove_address_handler(&fwserial_mgmt_addr_handler);
	kmem_cache_destroy(fwtty_txn_cache);
	if (create_loop_dev) {
		tty_unregister_driver(fwloop_driver);
		put_tty_driver(fwloop_driver);
	}
	tty_unregister_driver(fwtty_driver);
	put_tty_driver(fwtty_driver);
	debugfs_remove_recursive(fwserial_debugfs);
}

module_init(fwserial_init);
module_exit(fwserial_exit);

MODULE_AUTHOR("Peter Hurley (peter@hurleysoftware.com)");
MODULE_DESCRIPTION("FireWire Serial TTY Driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(ieee1394, fwserial_id_table);
MODULE_PARM_DESC(ttys, "Number of ttys to create for each local firewire node");
MODULE_PARM_DESC(auto, "Auto-connect a tty to each firewire node discovered");
MODULE_PARM_DESC(loop, "Create a loopback device, fwloop<n>, with ttys");
