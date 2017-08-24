/*
    include/linux/comedidev.h
    header file for kernel-only structures, variables, and constants

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 1997-2000 David A. Schleef <ds@schleef.org>

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

#ifndef _COMEDIDEV_H
#define _COMEDIDEV_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/timer.h>

#include "comedi.h"

#define DPRINTK(format, args...)	do {		\
	if (comedi_debug)				\
		pr_debug("comedi: " format, ## args);	\
} while (0)

#define COMEDI_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#define COMEDI_VERSION_CODE COMEDI_VERSION(COMEDI_MAJORVERSION, \
	COMEDI_MINORVERSION, COMEDI_MICROVERSION)
#define COMEDI_RELEASE VERSION

#define COMEDI_NUM_BOARD_MINORS 0x30

struct comedi_subdevice {
	struct comedi_device *device;
	int index;
	int type;
	int n_chan;
	int subdev_flags;
	int len_chanlist;	/*                                     */

	void *private;

	struct comedi_async *async;

	void *lock;
	void *busy;
	unsigned runflags;
	spinlock_t spin_lock;

	unsigned int io_bits;

	unsigned int maxdata;	/*                         */
	const unsigned int *maxdata_list;	/*                          */

	unsigned int flags;
	const unsigned int *flaglist;

	unsigned int settling_time_0;

	const struct comedi_lrange *range_table;
	const struct comedi_lrange *const *range_table_list;

	unsigned int *chanlist;	/*                                  */

	int (*insn_read) (struct comedi_device *, struct comedi_subdevice *,
			  struct comedi_insn *, unsigned int *);
	int (*insn_write) (struct comedi_device *, struct comedi_subdevice *,
			   struct comedi_insn *, unsigned int *);
	int (*insn_bits) (struct comedi_device *, struct comedi_subdevice *,
			  struct comedi_insn *, unsigned int *);
	int (*insn_config) (struct comedi_device *, struct comedi_subdevice *,
			    struct comedi_insn *, unsigned int *);

	int (*do_cmd) (struct comedi_device *, struct comedi_subdevice *);
	int (*do_cmdtest) (struct comedi_device *, struct comedi_subdevice *,
			   struct comedi_cmd *);
	int (*poll) (struct comedi_device *, struct comedi_subdevice *);
	int (*cancel) (struct comedi_device *, struct comedi_subdevice *);
	/*                                                                    */
	/*                                           
                               */

	/*                                */
	int (*buf_change) (struct comedi_device *dev,
			   struct comedi_subdevice *s, unsigned long new_size);

	void (*munge) (struct comedi_device *dev, struct comedi_subdevice *s,
		       void *data, unsigned int num_bytes,
		       unsigned int start_chan_index);
	enum dma_data_direction async_dma_dir;

	unsigned int state;

	struct device *class_dev;
	int minor;
};

struct comedi_buf_page {
	void *virt_addr;
	dma_addr_t dma_addr;
};

struct comedi_async {
	struct comedi_subdevice *subdevice;

	void *prealloc_buf;	/*                      */
	unsigned int prealloc_bufsz;	/*                       */
	/*                                      */
	struct comedi_buf_page *buf_page_list;
	unsigned n_buf_pages;	/*                               */

	unsigned int max_bufsize;	/*                            */
	/*                                         */
	unsigned int mmap_count;

	/*                                         */
	unsigned int buf_write_count;
	/*                                               */
	unsigned int buf_write_alloc_count;
	/*                                        */
	unsigned int buf_read_count;
	/*                                               */
	unsigned int buf_read_alloc_count;

	unsigned int buf_write_ptr;	/*                          */
	unsigned int buf_read_ptr;	/*                          */

	unsigned int cur_chan;	/*                                      */
	/*                                                          */
	unsigned int scan_progress;
	/*                                                        */
	unsigned int munge_chan;
	/*                                       */
	unsigned int munge_count;
	/*                           */
	unsigned int munge_ptr;

	unsigned int events;	/*                           */

	struct comedi_cmd cmd;

	wait_queue_head_t wait_head;

	/*                */
	unsigned int cb_mask;
	int (*cb_func) (unsigned int flags, void *);
	void *cb_arg;

	int (*inttrig) (struct comedi_device *dev, struct comedi_subdevice *s,
			unsigned int x);
};

struct comedi_driver {
	struct comedi_driver *next;

	const char *driver_name;
	struct module *module;
	int (*attach) (struct comedi_device *, struct comedi_devconfig *);
	void (*detach) (struct comedi_device *);
	int (*auto_attach) (struct comedi_device *, unsigned long);

	/*                                                      */
	unsigned int num_names;
	const char *const *board_name;
	/*                                                         */
	int offset;
};

struct comedi_device {
	int use_count;
	struct comedi_driver *driver;
	void *private;

	struct device *class_dev;
	int minor;
	/*                                                                     
                                                                      
             */
	struct device *hw_dev;

	const char *board_name;
	const void *board_ptr;
	bool attached:1;
	bool in_request_module:1;
	bool ioenabled:1;
	spinlock_t spinlock;
	struct mutex mutex;

	int n_subdevices;
	struct comedi_subdevice *subdevices;

	/*      */
	unsigned long iobase;
	unsigned long iolen;
	unsigned int irq;

	struct comedi_subdevice *read_subdev;
	struct comedi_subdevice *write_subdev;

	struct fasync_struct *async_queue;

	int (*open) (struct comedi_device *dev);
	void (*close) (struct comedi_device *dev);
};

static inline const void *comedi_board(const struct comedi_device *dev)
{
	return dev->board_ptr;
}

#ifdef CONFIG_COMEDI_DEBUG
extern int comedi_debug;
#else
static const int comedi_debug;
#endif

/*
                      
 */

void comedi_event(struct comedi_device *dev, struct comedi_subdevice *s);
void comedi_error(const struct comedi_device *dev, const char *s);

/*                                                                        
                                                                        
                                        */
enum comedi_minor_bits {
	COMEDI_DEVICE_MINOR_MASK = 0xf,
	COMEDI_SUBDEVICE_MINOR_MASK = 0xf0
};
static const unsigned COMEDI_SUBDEVICE_MINOR_SHIFT = 4;
static const unsigned COMEDI_SUBDEVICE_MINOR_OFFSET = 1;

struct comedi_device *comedi_dev_from_minor(unsigned minor);

void init_polling(void);
void cleanup_polling(void);
void start_polling(struct comedi_device *);
void stop_polling(struct comedi_device *);

/*                    */
enum subdevice_runflags {
	SRF_USER = 0x00000001,
	SRF_RT = 0x00000002,
	/*                                                               
                        */
	SRF_ERROR = 0x00000004,
	SRF_RUNNING = 0x08000000
};

bool comedi_is_subdevice_running(struct comedi_subdevice *s);

int comedi_check_chanlist(struct comedi_subdevice *s,
			  int n,
			  unsigned int *chanlist);

/*             */

#define RANGE(a, b)		{(a)*1e6, (b)*1e6, 0}
#define RANGE_ext(a, b)		{(a)*1e6, (b)*1e6, RF_EXTERNAL}
#define RANGE_mA(a, b)		{(a)*1e6, (b)*1e6, UNIT_mA}
#define RANGE_unitless(a, b)	{(a)*1e6, (b)*1e6, 0}
#define BIP_RANGE(a)		{-(a)*1e6, (a)*1e6, 0}
#define UNI_RANGE(a)		{0, (a)*1e6, 0}

extern const struct comedi_lrange range_bipolar10;
extern const struct comedi_lrange range_bipolar5;
extern const struct comedi_lrange range_bipolar2_5;
extern const struct comedi_lrange range_unipolar10;
extern const struct comedi_lrange range_unipolar5;
extern const struct comedi_lrange range_unipolar2_5;
extern const struct comedi_lrange range_0_20mA;
extern const struct comedi_lrange range_4_20mA;
extern const struct comedi_lrange range_0_32mA;
extern const struct comedi_lrange range_unknown;

#define range_digital		range_unipolar5

#if __GNUC__ >= 3
#define GCC_ZERO_LENGTH_ARRAY
#else
#define GCC_ZERO_LENGTH_ARRAY 0
#endif

struct comedi_lrange {
	int length;
	struct comedi_krange range[GCC_ZERO_LENGTH_ARRAY];
};

/*                                    */

static inline unsigned int bytes_per_sample(const struct comedi_subdevice *subd)
{
	if (subd->subdev_flags & SDF_LSAMPL)
		return sizeof(unsigned int);
	else
		return sizeof(short);
}

/*
                                                                         
                                                                        
                                                                  
                                                            
 */
int comedi_set_hw_dev(struct comedi_device *dev, struct device *hw_dev);

unsigned int comedi_buf_write_alloc(struct comedi_async *, unsigned int);
unsigned int comedi_buf_write_free(struct comedi_async *, unsigned int);

unsigned int comedi_buf_read_n_available(struct comedi_async *);
unsigned int comedi_buf_read_alloc(struct comedi_async *, unsigned int);
unsigned int comedi_buf_read_free(struct comedi_async *, unsigned int);

int comedi_buf_put(struct comedi_async *, short);
int comedi_buf_get(struct comedi_async *, short *);

void comedi_buf_memcpy_to(struct comedi_async *async, unsigned int offset,
			  const void *source, unsigned int num_bytes);
void comedi_buf_memcpy_from(struct comedi_async *async, unsigned int offset,
			    void *destination, unsigned int num_bytes);

/*                                             */

int comedi_alloc_subdevices(struct comedi_device *, int);

void comedi_spriv_free(struct comedi_device *, int subdev_num);

int __comedi_request_region(struct comedi_device *,
			    unsigned long start, unsigned long len);
int comedi_request_region(struct comedi_device *,
			  unsigned long start, unsigned long len);
void comedi_legacy_detach(struct comedi_device *);

int comedi_auto_config(struct device *, struct comedi_driver *,
		       unsigned long context);
void comedi_auto_unconfig(struct device *);

int comedi_driver_register(struct comedi_driver *);
int comedi_driver_unregister(struct comedi_driver *);

/* 
                                                                        
                                         
  
                                                                             
                                                                            
                                                                            
 */
#define module_comedi_driver(__comedi_driver) \
	module_driver(__comedi_driver, comedi_driver_register, \
			comedi_driver_unregister)

#ifdef CONFIG_COMEDI_PCI_DRIVERS

/*                                                     */

/*
                                          
 */
#define PCI_VENDOR_ID_KOLTER		0x1001
#define PCI_VENDOR_ID_ICP		0x104c
#define PCI_VENDOR_ID_AMCC		0x10e8
#define PCI_VENDOR_ID_DT		0x1116
#define PCI_VENDOR_ID_IOTECH		0x1616
#define PCI_VENDOR_ID_CONTEC		0x1221
#define PCI_VENDOR_ID_RTD		0x1435

struct pci_dev;
struct pci_driver;

struct pci_dev *comedi_to_pci_dev(struct comedi_device *);

int comedi_pci_enable(struct comedi_device *);
void comedi_pci_disable(struct comedi_device *);

int comedi_pci_auto_config(struct pci_dev *, struct comedi_driver *,
			   unsigned long context);
void comedi_pci_auto_unconfig(struct pci_dev *);

int comedi_pci_driver_register(struct comedi_driver *, struct pci_driver *);
void comedi_pci_driver_unregister(struct comedi_driver *, struct pci_driver *);

/* 
                                                                                
                                         
                                   
  
                                                                       
                                                                  
                                                               
                                  
 */
#define module_comedi_pci_driver(__comedi_driver, __pci_driver) \
	module_driver(__comedi_driver, comedi_pci_driver_register, \
			comedi_pci_driver_unregister, &(__pci_driver))

#else

/*
                                                                 
                                                                       
                  
 */

static inline struct pci_dev *comedi_to_pci_dev(struct comedi_device *dev)
{
	return NULL;
}

static inline int comedi_pci_enable(struct comedi_device *dev)
{
	return -ENOSYS;
}

static inline void comedi_pci_disable(struct comedi_device *dev)
{
}

#endif /*                           */

#ifdef CONFIG_COMEDI_PCMCIA_DRIVERS

/*                                                           */

struct pcmcia_driver;
struct pcmcia_device;

struct pcmcia_device *comedi_to_pcmcia_dev(struct comedi_device *);

int comedi_pcmcia_enable(struct comedi_device *,
			 int (*conf_check)(struct pcmcia_device *, void *));
void comedi_pcmcia_disable(struct comedi_device *);

int comedi_pcmcia_auto_config(struct pcmcia_device *, struct comedi_driver *);
void comedi_pcmcia_auto_unconfig(struct pcmcia_device *);

int comedi_pcmcia_driver_register(struct comedi_driver *,
					struct pcmcia_driver *);
void comedi_pcmcia_driver_unregister(struct comedi_driver *,
					struct pcmcia_driver *);

/* 
                                                                                      
                                         
                                         
  
                                                                          
                                                                  
                                                               
                                  
 */
#define module_comedi_pcmcia_driver(__comedi_driver, __pcmcia_driver) \
	module_driver(__comedi_driver, comedi_pcmcia_driver_register, \
			comedi_pcmcia_driver_unregister, &(__pcmcia_driver))

#endif /*                              */

#ifdef CONFIG_COMEDI_USB_DRIVERS

/*                                                     */

struct usb_driver;
struct usb_interface;

struct usb_interface *comedi_to_usb_interface(struct comedi_device *);

int comedi_usb_auto_config(struct usb_interface *, struct comedi_driver *,
			   unsigned long context);
void comedi_usb_auto_unconfig(struct usb_interface *);

int comedi_usb_driver_register(struct comedi_driver *, struct usb_driver *);
void comedi_usb_driver_unregister(struct comedi_driver *, struct usb_driver *);

/* 
                                                                                
                                         
                                   
  
                                                                       
                                                                  
                                                               
                                  
 */
#define module_comedi_usb_driver(__comedi_driver, __usb_driver) \
	module_driver(__comedi_driver, comedi_usb_driver_register, \
			comedi_usb_driver_unregister, &(__usb_driver))

#endif /*                           */

#endif /*              */
