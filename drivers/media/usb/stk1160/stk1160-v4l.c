/*
 * STK1160 driver
 *
 * Copyright (C) 2012 Ezequiel Garcia
 * <elezegarcia--a.t--gmail.com>
 *
 * Based on Easycap driver by R.M. Thomas
 *	Copyright (C) 2010 R.M. Thomas
 *	<rmthomas--a.t--sciolus.org>
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
 */

#include <linux/module.h>
#include <linux/usb.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-event.h>
#include <media/v4l2-chip-ident.h>
#include <media/videobuf2-vmalloc.h>

#include <media/saa7115.h>

#include "stk1160.h"
#include "stk1160-reg.h"

static unsigned int vidioc_debug;
module_param(vidioc_debug, int, 0644);
MODULE_PARM_DESC(vidioc_debug, "enable debug messages [vidioc]");

static bool keep_buffers;
module_param(keep_buffers, bool, 0644);
MODULE_PARM_DESC(keep_buffers, "don't release buffers upon stop streaming");

/*                           */
static struct stk1160_fmt format[] = {
	{
		.name     = "16 bpp YUY2, 4:2:2, packed",
		.fourcc   = V4L2_PIX_FMT_UYVY,
		.depth    = 16,
	}
};

static void stk1160_set_std(struct stk1160 *dev)
{
	int i;

	static struct regval std525[] = {

		/*         */

		/*             */
		{STK116_CFSPO_STX_L, 0x0000},
		{STK116_CFSPO_STX_H, 0x0000},
		{STK116_CFSPO_STY_L, 0x0003},
		{STK116_CFSPO_STY_H, 0x0000},

		/*           */
		{STK116_CFEPO_ENX_L, 0x05a0},
		{STK116_CFEPO_ENX_H, 0x0005},
		{STK116_CFEPO_ENY_L, 0x00f3},
		{STK116_CFEPO_ENY_H, 0x0000},

		{0xffff, 0xffff}
	};

	static struct regval std625[] = {

		/*         */

		/*                                                   */
		/*             */
		{STK116_CFSPO,   0x0000},
		{STK116_CFSPO+1, 0x0000},
		{STK116_CFSPO+2, 0x0001},
		{STK116_CFSPO+3, 0x0000},

		/*           */
		{STK116_CFEPO,   0x05a0},
		{STK116_CFEPO+1, 0x0005},
		{STK116_CFEPO+2, 0x0121},
		{STK116_CFEPO+3, 0x0001},

		{0xffff, 0xffff}
	};

	if (dev->norm & V4L2_STD_525_60) {
		stk1160_dbg("registers to NTSC like standard\n");
		for (i = 0; std525[i].reg != 0xffff; i++)
			stk1160_write_reg(dev, std525[i].reg, std525[i].val);
	} else {
		stk1160_dbg("registers to PAL like standard\n");
		for (i = 0; std625[i].reg != 0xffff; i++)
			stk1160_write_reg(dev, std625[i].reg, std625[i].val);
	}

}

/*
                               
                                                                  
 */
static bool stk1160_set_alternate(struct stk1160 *dev)
{
	int i, prev_alt = dev->alt;
	unsigned int min_pkt_size;
	bool new_pkt_size;

	/*
                                    
                                              
  */
	min_pkt_size = STK1160_MIN_PKT_SIZE;

	for (i = 0; i < dev->num_alt; i++) {
		/*                                                            */
		if (dev->alt_max_pkt_size[i] >= min_pkt_size) {
			dev->alt = i;
			break;
		/*
                                                                  
                                                        
   */
		} else if (dev->alt_max_pkt_size[i] >
			   dev->alt_max_pkt_size[dev->alt])
			dev->alt = i;
	}

	stk1160_info("setting alternate %d\n", dev->alt);

	if (dev->alt != prev_alt) {
		stk1160_dbg("minimum isoc packet size: %u (alt=%d)\n",
				min_pkt_size, dev->alt);
		stk1160_dbg("setting alt %d with wMaxPacketSize=%u\n",
			       dev->alt, dev->alt_max_pkt_size[dev->alt]);
		usb_set_interface(dev->udev, 0, dev->alt);
	}

	new_pkt_size = dev->max_pkt_size != dev->alt_max_pkt_size[dev->alt];
	dev->max_pkt_size = dev->alt_max_pkt_size[dev->alt];

	return new_pkt_size;
}

static int stk1160_start_streaming(struct stk1160 *dev)
{
	bool new_pkt_size;
	int rc = 0;
	int i;

	/*                       */
	if (!dev->udev)
		return -ENODEV;

	if (mutex_lock_interruptible(&dev->v4l_lock))
		return -ERESTARTSYS;
	/*
                                                            
                                         
                                    
  */
	new_pkt_size = stk1160_set_alternate(dev);

	/*
                                 
                                       
                                       
  */
	if (!dev->isoc_ctl.num_bufs || new_pkt_size) {
		rc = stk1160_alloc_isoc(dev);
		if (rc < 0)
			goto out_stop_hw;
	}

	/*                             */
	for (i = 0; i < dev->isoc_ctl.num_bufs; i++) {
		rc = usb_submit_urb(dev->isoc_ctl.urb[i], GFP_KERNEL);
		if (rc) {
			stk1160_err("cannot submit urb[%d] (%d)\n", i, rc);
			goto out_uninit;
		}
	}

	/*               */
	v4l2_device_call_all(&dev->v4l2_dev, 0, video, s_stream, 1);

	/*               */
	stk1160_write_reg(dev, STK1160_DCTRL, 0xb3);
	stk1160_write_reg(dev, STK1160_DCTRL+3, 0x00);

	stk1160_dbg("streaming started\n");

	mutex_unlock(&dev->v4l_lock);

	return 0;

out_uninit:
	stk1160_uninit_isoc(dev);
out_stop_hw:
	usb_set_interface(dev->udev, 0, 0);
	stk1160_clear_queue(dev);

	mutex_unlock(&dev->v4l_lock);

	return rc;
}

/*                                   */
static void stk1160_stop_hw(struct stk1160 *dev)
{
	/*                                                                 */
	if (!dev->udev)
		return;

	/*                 */
	dev->alt = 0;
	stk1160_info("setting alternate %d\n", dev->alt);
	usb_set_interface(dev->udev, 0, 0);

	/*              */
	stk1160_write_reg(dev, STK1160_DCTRL, 0x00);
	stk1160_write_reg(dev, STK1160_DCTRL+3, 0x00);

	/*              */
	v4l2_device_call_all(&dev->v4l2_dev, 0, video, s_stream, 0);
}

static int stk1160_stop_streaming(struct stk1160 *dev)
{
	if (mutex_lock_interruptible(&dev->v4l_lock))
		return -ERESTARTSYS;

	stk1160_cancel_isoc(dev);

	/*
                                                                   
                                                   
  */
	if (!keep_buffers)
		stk1160_free_isoc(dev);

	stk1160_stop_hw(dev);

	stk1160_clear_queue(dev);

	stk1160_dbg("streaming stopped\n");

	mutex_unlock(&dev->v4l_lock);

	return 0;
}

static struct v4l2_file_operations stk1160_fops = {
	.owner = THIS_MODULE,
	.open = v4l2_fh_open,
	.release = vb2_fop_release,
	.read = vb2_fop_read,
	.poll = vb2_fop_poll,
	.mmap = vb2_fop_mmap,
	.unlocked_ioctl = video_ioctl2,
};

/*
                
 */
static int vidioc_querycap(struct file *file,
		void *priv, struct v4l2_capability *cap)
{
	struct stk1160 *dev = video_drvdata(file);

	strcpy(cap->driver, "stk1160");
	strcpy(cap->card, "stk1160");
	usb_make_path(dev->udev, cap->bus_info, sizeof(cap->bus_info));
	cap->device_caps =
		V4L2_CAP_VIDEO_CAPTURE |
		V4L2_CAP_STREAMING |
		V4L2_CAP_READWRITE;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void  *priv,
		struct v4l2_fmtdesc *f)
{
	if (f->index != 0)
		return -EINVAL;

	strlcpy(f->description, format[f->index].name, sizeof(f->description));
	f->pixelformat = format[f->index].fourcc;
	return 0;
}

static int vidioc_g_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct stk1160 *dev = video_drvdata(file);

	f->fmt.pix.width = dev->width;
	f->fmt.pix.height = dev->height;
	f->fmt.pix.field = V4L2_FIELD_INTERLACED;
	f->fmt.pix.pixelformat = dev->fmt->fourcc;
	f->fmt.pix.bytesperline = dev->width * 2;
	f->fmt.pix.sizeimage = dev->height * f->fmt.pix.bytesperline;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;

	return 0;
}

static int vidioc_try_fmt_vid_cap(struct file *file, void *priv,
			struct v4l2_format *f)
{
	struct stk1160 *dev = video_drvdata(file);

	/*
                                           
                                                 
                          
                                  
  */

	f->fmt.pix.pixelformat = dev->fmt->fourcc;
	f->fmt.pix.width = dev->width;
	f->fmt.pix.height = dev->height;
	f->fmt.pix.field = V4L2_FIELD_INTERLACED;
	f->fmt.pix.bytesperline = dev->width * 2;
	f->fmt.pix.sizeimage = dev->height * f->fmt.pix.bytesperline;
	f->fmt.pix.colorspace = V4L2_COLORSPACE_SMPTE170M;

	return 0;
}

static int vidioc_s_fmt_vid_cap(struct file *file, void *priv,
					struct v4l2_format *f)
{
	struct stk1160 *dev = video_drvdata(file);
	struct vb2_queue *q = &dev->vb_vidq;

	if (vb2_is_busy(q))
		return -EBUSY;

	vidioc_try_fmt_vid_cap(file, priv, f);

	/*                                     */

	return 0;
}

static int vidioc_querystd(struct file *file, void *priv, v4l2_std_id *norm)
{
	struct stk1160 *dev = video_drvdata(file);
	v4l2_device_call_all(&dev->v4l2_dev, 0, video, querystd, norm);
	return 0;
}

static int vidioc_g_std(struct file *file, void *priv, v4l2_std_id *norm)
{
	struct stk1160 *dev = video_drvdata(file);

	*norm = dev->norm;
	return 0;
}

static int vidioc_s_std(struct file *file, void *priv, v4l2_std_id norm)
{
	struct stk1160 *dev = video_drvdata(file);
	struct vb2_queue *q = &dev->vb_vidq;

	if (vb2_is_busy(q))
		return -EBUSY;

	/*                       */
	if (!dev->udev)
		return -ENODEV;

	/*                                                         */
	dev->norm = norm;

	/*                                          */
	if (dev->norm & V4L2_STD_525_60) {
		dev->width = 720;
		dev->height = 480;
	} else if (dev->norm & V4L2_STD_625_50) {
		dev->width = 720;
		dev->height = 576;
	} else {
		stk1160_err("invalid standard\n");
		return -EINVAL;
	}

	stk1160_set_std(dev);

	v4l2_device_call_all(&dev->v4l2_dev, 0, core, s_std,
			dev->norm);

	return 0;
}


static int vidioc_enum_input(struct file *file, void *priv,
				struct v4l2_input *i)
{
	struct stk1160 *dev = video_drvdata(file);

	if (i->index > STK1160_MAX_INPUT)
		return -EINVAL;

	/*                          */
	if (i->index == STK1160_SVIDEO_INPUT)
		sprintf(i->name, "S-Video");
	else
		sprintf(i->name, "Composite%d", i->index);

	i->type = V4L2_INPUT_TYPE_CAMERA;
	i->std = dev->vdev.tvnorms;
	return 0;
}

static int vidioc_g_input(struct file *file, void *priv, unsigned int *i)
{
	struct stk1160 *dev = video_drvdata(file);
	*i = dev->ctl_input;
	return 0;
}

static int vidioc_s_input(struct file *file, void *priv, unsigned int i)
{
	struct stk1160 *dev = video_drvdata(file);

	if (vb2_is_busy(&dev->vb_vidq))
		return -EBUSY;

	if (i > STK1160_MAX_INPUT)
		return -EINVAL;

	dev->ctl_input = i;

	stk1160_select_input(dev);

	return 0;
}

static int vidioc_g_chip_ident(struct file *file, void *priv,
	       struct v4l2_dbg_chip_ident *chip)
{
	switch (chip->match.type) {
	case V4L2_CHIP_MATCH_BRIDGE:
		chip->ident = V4L2_IDENT_NONE;
		chip->revision = 0;
		return 0;
	default:
		return -EINVAL;
	}
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int vidioc_g_register(struct file *file, void *priv,
			     struct v4l2_dbg_register *reg)
{
	struct stk1160 *dev = video_drvdata(file);
	int rc;
	u8 val;

	switch (reg->match.type) {
	case V4L2_CHIP_MATCH_I2C_DRIVER:
		v4l2_device_call_all(&dev->v4l2_dev, 0, core, g_register, reg);
		return 0;
	case V4L2_CHIP_MATCH_I2C_ADDR:
		/*                        */
		v4l2_device_call_all(&dev->v4l2_dev, 0, core, g_register, reg);
		return 0;
	default:
		if (!v4l2_chip_match_host(&reg->match))
			return -EINVAL;
	}

	/*            */
	rc = stk1160_read_reg(dev, reg->reg, &val);
	reg->val = val;
	reg->size = 1;

	return rc;
}

static int vidioc_s_register(struct file *file, void *priv,
			     const struct v4l2_dbg_register *reg)
{
	struct stk1160 *dev = video_drvdata(file);

	switch (reg->match.type) {
	case V4L2_CHIP_MATCH_I2C_DRIVER:
		v4l2_device_call_all(&dev->v4l2_dev, 0, core, s_register, reg);
		return 0;
	case V4L2_CHIP_MATCH_I2C_ADDR:
		/*                        */
		v4l2_device_call_all(&dev->v4l2_dev, 0, core, s_register, reg);
		return 0;
	default:
		if (!v4l2_chip_match_host(&reg->match))
			return -EINVAL;
	}

	/*            */
	return stk1160_write_reg(dev, reg->reg, cpu_to_le16(reg->val));
}
#endif

static const struct v4l2_ioctl_ops stk1160_ioctl_ops = {
	.vidioc_querycap      = vidioc_querycap,
	.vidioc_enum_fmt_vid_cap  = vidioc_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap     = vidioc_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap   = vidioc_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap     = vidioc_s_fmt_vid_cap,
	.vidioc_querystd      = vidioc_querystd,
	.vidioc_g_std         = vidioc_g_std,
	.vidioc_s_std         = vidioc_s_std,
	.vidioc_enum_input    = vidioc_enum_input,
	.vidioc_g_input       = vidioc_g_input,
	.vidioc_s_input       = vidioc_s_input,

	/*                         */
	.vidioc_reqbufs       = vb2_ioctl_reqbufs,
	.vidioc_querybuf      = vb2_ioctl_querybuf,
	.vidioc_qbuf          = vb2_ioctl_qbuf,
	.vidioc_dqbuf         = vb2_ioctl_dqbuf,
	.vidioc_streamon      = vb2_ioctl_streamon,
	.vidioc_streamoff     = vb2_ioctl_streamoff,

	.vidioc_log_status  = v4l2_ctrl_log_status,
	.vidioc_subscribe_event = v4l2_ctrl_subscribe_event,
	.vidioc_unsubscribe_event = v4l2_event_unsubscribe,
	.vidioc_g_chip_ident = vidioc_g_chip_ident,

#ifdef CONFIG_VIDEO_ADV_DEBUG
	.vidioc_g_register = vidioc_g_register,
	.vidioc_s_register = vidioc_s_register,
#endif
};

/*                                                                  */

/*
                       
 */
static int queue_setup(struct vb2_queue *vq, const struct v4l2_format *v4l_fmt,
				unsigned int *nbuffers, unsigned int *nplanes,
				unsigned int sizes[], void *alloc_ctxs[])
{
	struct stk1160 *dev = vb2_get_drv_priv(vq);
	unsigned long size;

	size = dev->width * dev->height * 2;

	/*
                                                             
                                                 
  */
	*nbuffers = clamp_t(unsigned int, *nbuffers,
			STK1160_MIN_VIDEO_BUFFERS, STK1160_MAX_VIDEO_BUFFERS);

	/*                                 */
	*nplanes = 1;

	sizes[0] = size;

	stk1160_info("%s: buffer count %d, each %ld bytes\n",
			__func__, *nbuffers, size);

	return 0;
}

static void buffer_queue(struct vb2_buffer *vb)
{
	unsigned long flags;
	struct stk1160 *dev = vb2_get_drv_priv(vb->vb2_queue);
	struct stk1160_buffer *buf =
		container_of(vb, struct stk1160_buffer, vb);

	spin_lock_irqsave(&dev->buf_lock, flags);
	if (!dev->udev) {
		/*
                                                                 
                                                         
   */
		vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
	} else {

		buf->mem = vb2_plane_vaddr(vb, 0);
		buf->length = vb2_plane_size(vb, 0);
		buf->bytesused = 0;
		buf->pos = 0;

		/*
                                                          
                                      
   */
		if (buf->length < dev->width * dev->height * 2)
			vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
		else
			list_add_tail(&buf->list, &dev->avail_bufs);

	}
	spin_unlock_irqrestore(&dev->buf_lock, flags);
}

static int start_streaming(struct vb2_queue *vq, unsigned int count)
{
	struct stk1160 *dev = vb2_get_drv_priv(vq);
	return stk1160_start_streaming(dev);
}

/*                                          */
static int stop_streaming(struct vb2_queue *vq)
{
	struct stk1160 *dev = vb2_get_drv_priv(vq);
	return stk1160_stop_streaming(dev);
}

static struct vb2_ops stk1160_video_qops = {
	.queue_setup		= queue_setup,
	.buf_queue		= buffer_queue,
	.start_streaming	= start_streaming,
	.stop_streaming		= stop_streaming,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
};

static struct video_device v4l_template = {
	.name = "stk1160",
	.tvnorms = V4L2_STD_525_60 | V4L2_STD_625_50,
	.fops = &stk1160_fops,
	.ioctl_ops = &stk1160_ioctl_ops,
	.release = video_device_release_empty,
};

/*                                                                  */

/*                                                          */
void stk1160_clear_queue(struct stk1160 *dev)
{
	struct stk1160_buffer *buf;
	unsigned long flags;

	/*                            */
	spin_lock_irqsave(&dev->buf_lock, flags);
	while (!list_empty(&dev->avail_bufs)) {
		buf = list_first_entry(&dev->avail_bufs,
			struct stk1160_buffer, list);
		list_del(&buf->list);
		vb2_buffer_done(&buf->vb, VB2_BUF_STATE_ERROR);
		stk1160_info("buffer [%p/%d] aborted\n",
				buf, buf->vb.v4l2_buf.index);
	}
	/*                                        */
	dev->isoc_ctl.buf = NULL;
	spin_unlock_irqrestore(&dev->buf_lock, flags);
}

int stk1160_vb2_setup(struct stk1160 *dev)
{
	int rc;
	struct vb2_queue *q;

	q = &dev->vb_vidq;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_READ | VB2_MMAP | VB2_USERPTR;
	q->drv_priv = dev;
	q->buf_struct_size = sizeof(struct stk1160_buffer);
	q->ops = &stk1160_video_qops;
	q->mem_ops = &vb2_vmalloc_memops;
	q->timestamp_type = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;

	rc = vb2_queue_init(q);
	if (rc < 0)
		return rc;

	/*                            */
	INIT_LIST_HEAD(&dev->avail_bufs);

	return 0;
}

int stk1160_video_register(struct stk1160 *dev)
{
	int rc;

	/*                                                   */
	dev->vdev = v4l_template;
	dev->vdev.debug = vidioc_debug;
	dev->vdev.queue = &dev->vb_vidq;

	/*
                                                          
                                                  
  */
	dev->vdev.lock = &dev->v4l_lock;
	dev->vdev.queue->lock = &dev->vb_queue_lock;

	/*                                              */
	dev->vdev.v4l2_dev = &dev->v4l2_dev;
	set_bit(V4L2_FL_USE_FH_PRIO, &dev->vdev.flags);

	/*                 */
	dev->norm = V4L2_STD_NTSC_M;
	dev->width = 720;
	dev->height = 480;

	/*                    */
	dev->fmt = &format[0];
	stk1160_set_std(dev);

	v4l2_device_call_all(&dev->v4l2_dev, 0, core, s_std,
			dev->norm);

	video_set_drvdata(&dev->vdev, dev);
	rc = video_register_device(&dev->vdev, VFL_TYPE_GRABBER, -1);
	if (rc < 0) {
		stk1160_err("video_register_device failed (%d)\n", rc);
		return rc;
	}

	v4l2_info(&dev->v4l2_dev, "V4L2 device registered as %s\n",
		  video_device_node_name(&dev->vdev));

	return 0;
}
