/*
 * SPCA505 chip based cameras initialization data
 *
 * V4L2 by Jean-Francis Moine <http://moinejf.free.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#define MODULE_NAME "spca505"

#include "gspca.h"

MODULE_AUTHOR("Michel Xhaard <mxhaard@users.sourceforge.net>");
MODULE_DESCRIPTION("GSPCA/SPCA505 USB Camera Driver");
MODULE_LICENSE("GPL");

/*                            */
struct sd {
	struct gspca_dev gspca_dev;		/*                           */

	u8 subtype;
#define IntelPCCameraPro 0
#define Nxultra 1
};

static const struct v4l2_pix_format vga_mode[] = {
	{160, 120, V4L2_PIX_FMT_SPCA505, V4L2_FIELD_NONE,
		.bytesperline = 160,
		.sizeimage = 160 * 120 * 3 / 2,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 4},
	{176, 144, V4L2_PIX_FMT_SPCA505, V4L2_FIELD_NONE,
		.bytesperline = 176,
		.sizeimage = 176 * 144 * 3 / 2,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 3},
	{320, 240, V4L2_PIX_FMT_SPCA505, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240 * 3 / 2,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 2},
	{352, 288, V4L2_PIX_FMT_SPCA505, V4L2_FIELD_NONE,
		.bytesperline = 352,
		.sizeimage = 352 * 288 * 3 / 2,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 1},
	{640, 480, V4L2_PIX_FMT_SPCA505, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480 * 3 / 2,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0},
};

#define SPCA50X_OFFSET_DATA 10

#define SPCA50X_REG_USB 0x02	/*             */

#define SPCA50X_USB_CTRL 0x00	/*         */
#define SPCA50X_CUSB_ENABLE 0x01 /*         */

#define SPCA50X_REG_GLOBAL 0x03	/*         */
#define SPCA50X_GMISC0_IDSEL 0x01 /*                                         */
#define SPCA50X_GLOBAL_MISC0 0x00 /*                                        */

#define SPCA50X_GLOBAL_MISC1 0x01 /*     */
#define SPCA50X_GLOBAL_MISC3 0x03 /*     */
#define SPCA50X_GMISC3_SAA7113RST 0x20	/*                                 */

/*                                      */
#define SPCA50X_REG_COMPRESS 0x04

/*
                                                                     
 */
static const u8 spca505_init_data[][3] = {
	/*                       */
	{SPCA50X_REG_GLOBAL, SPCA50X_GMISC3_SAA7113RST, SPCA50X_GLOBAL_MISC3},
	/*              */
	{SPCA50X_REG_GLOBAL, 0x00, SPCA50X_GLOBAL_MISC3},
	{SPCA50X_REG_GLOBAL, 0x00, SPCA50X_GLOBAL_MISC1},
	/*                 */
	{SPCA50X_REG_GLOBAL, SPCA50X_GMISC0_IDSEL, SPCA50X_GLOBAL_MISC0},

	{0x05, 0x01, 0x10},
					/*                             */
	{0x05, 0x0f, 0x11},

	/*                       */
	{0x06, 0x10, 0x08},
	{0x06, 0x00, 0x09},
	{0x06, 0x00, 0x0a},
	{0x06, 0x00, 0x0b},
	{0x06, 0x10, 0x0c},
	{0x06, 0x00, 0x0d},
	{0x06, 0x00, 0x0e},
	{0x06, 0x00, 0x0f},
	{0x06, 0x10, 0x10},
	{0x06, 0x02, 0x11},
	{0x06, 0x00, 0x12},
	{0x06, 0x04, 0x13},
	{0x06, 0x02, 0x14},
	{0x06, 0x8a, 0x51},
	{0x06, 0x40, 0x52},
	{0x06, 0xb6, 0x53},
	{0x06, 0x3d, 0x54},
	{}
};

/*
                                                       
 */
static const u8 spca505_open_data_ccd[][3] = {
	/*                       */
	/*                       */
	{0x03, 0x04, 0x01},
	/*                       */
	{0x03, 0x00, 0x01},

	/*                                                              
                                                          */
		{0x04, 0x10, 0x01},
		/*                        */
	{0x04, 0x00, 0x04},
	{0x04, 0x00, 0x05},
	{0x04, 0x20, 0x06},
	{0x04, 0x20, 0x07},

	{0x08, 0x0a, 0x00},
	/*                        */

	{0x05, 0x00, 0x10},
	{0x05, 0x00, 0x11},
	{0x05, 0x00, 0x00},
	/*                  */
	{0x05, 0x00, 0x01},
	/*                  */
	{0x05, 0x00, 0x02},
	/*                  */
	{0x05, 0x00, 0x03},
	/*                  */
	{0x05, 0x00, 0x04},
	/*                  */
		{0x05, 0x80, 0x05},
		/*                  */
		{0x05, 0xe0, 0x06},
		/*                  */
		{0x05, 0x20, 0x07},
		/*                  */
		{0x05, 0xa0, 0x08},
		/*                  */
		{0x05, 0x0, 0x12},
		/*                  */
	{0x05, 0x02, 0x0f},
	/*                  */
		{0x05, 0x10, 0x46},
		/*                  */
		{0x05, 0x8, 0x4a},
		/*                  */

	{0x03, 0x08, 0x03},
	/*                     */
	{0x03, 0x08, 0x01},
	{0x03, 0x0c, 0x03},
	/*                  */
		{0x03, 0x21, 0x00},
		/*             */

/*                                                                         */
	{0x06, 0x10, 0x08},
	{0x06, 0x00, 0x09},
	{0x06, 0x00, 0x0a},
	{0x06, 0x00, 0x0b},
	{0x06, 0x10, 0x0c},
	{0x06, 0x00, 0x0d},
	{0x06, 0x00, 0x0e},
	{0x06, 0x00, 0x0f},
	{0x06, 0x10, 0x10},
	{0x06, 0x02, 0x11},
	{0x06, 0x00, 0x12},
	{0x06, 0x04, 0x13},
	{0x06, 0x02, 0x14},
	{0x06, 0x8a, 0x51},
	{0x06, 0x40, 0x52},
	{0x06, 0xb6, 0x53},
	{0x06, 0x3d, 0x54},
	/*                    */

		{0x06, 0x3f, 0x1},
		/*               */
	{0x06, 0x10, 0x02},
	{0x06, 0x64, 0x07},
	{0x06, 0x10, 0x08},
	{0x06, 0x00, 0x09},
	{0x06, 0x00, 0x0a},
	{0x06, 0x00, 0x0b},
	{0x06, 0x10, 0x0c},
	{0x06, 0x00, 0x0d},
	{0x06, 0x00, 0x0e},
	{0x06, 0x00, 0x0f},
	{0x06, 0x10, 0x10},
	{0x06, 0x02, 0x11},
	{0x06, 0x00, 0x12},
	{0x06, 0x04, 0x13},
	{0x06, 0x02, 0x14},
	{0x06, 0x8a, 0x51},
	{0x06, 0x40, 0x52},
	{0x06, 0xb6, 0x53},
	{0x06, 0x3d, 0x54},
	{0x06, 0x60, 0x57},
	{0x06, 0x20, 0x58},
	{0x06, 0x15, 0x59},
	{0x06, 0x05, 0x5a},

	{0x05, 0x01, 0xc0},
	{0x05, 0x10, 0xcb},
		{0x05, 0x80, 0xc1},
		/* */
		{0x05, 0x0, 0xc2},
		/*         */
	{0x05, 0x00, 0xca},
		{0x05, 0x80, 0xc1},
		/*  */
	{0x05, 0x04, 0xc2},
	{0x05, 0x00, 0xca},
		{0x05, 0x0, 0xc1},
		/*  */
	{0x05, 0x00, 0xc2},
	{0x05, 0x00, 0xca},
		{0x05, 0x40, 0xc1},
		/* */
	{0x05, 0x17, 0xc2},
	{0x05, 0x00, 0xca},
		{0x05, 0x80, 0xc1},
		/* */
	{0x05, 0x06, 0xc2},
	{0x05, 0x00, 0xca},
		{0x05, 0x80, 0xc1},
		/* */
	{0x05, 0x04, 0xc2},
	{0x05, 0x00, 0xca},

	{0x03, 0x4c, 0x3},
	{0x03, 0x18, 0x1},

	{0x06, 0x70, 0x51},
	{0x06, 0xbe, 0x53},
	{0x06, 0x71, 0x57},
	{0x06, 0x20, 0x58},
	{0x06, 0x05, 0x59},
	{0x06, 0x15, 0x5a},

	{0x04, 0x00, 0x08},
	/*                                 */
	{0x04, 0x12, 0x09},
	{0x04, 0x21, 0x0a},
	{0x04, 0x10, 0x0b},
	{0x04, 0x21, 0x0c},
	{0x04, 0x05, 0x00},
	/*                       */
	{0x04, 0x00, 0x01},

	{0x06, 0x3f, 0x01},

	{0x04, 0x00, 0x04},
	{0x04, 0x00, 0x05},
	{0x04, 0x40, 0x06},
	{0x04, 0x40, 0x07},

	{0x06, 0x1c, 0x17},
	{0x06, 0xe2, 0x19},
	{0x06, 0x1c, 0x1b},
	{0x06, 0xe2, 0x1d},
	{0x06, 0xaa, 0x1f},
	{0x06, 0x70, 0x20},

	{0x05, 0x01, 0x10},
	{0x05, 0x00, 0x11},
	{0x05, 0x01, 0x00},
	{0x05, 0x05, 0x01},
		{0x05, 0x00, 0xc1},
		/* */
	{0x05, 0x00, 0xc2},
	{0x05, 0x00, 0xca},

	{0x06, 0x70, 0x51},
	{0x06, 0xbe, 0x53},
	{}
};

/*
                                                       
                                                  
 */
/*     */
#define initial_brightness 0x7f	/*                        */
/*                                                         */
/*
                                                                     
 */
static const u8 spca505b_init_data[][3] = {
/*       */
	{0x02, 0x00, 0x00},		/*      */
	{0x02, 0x00, 0x01},
	{0x02, 0x00, 0x02},
	{0x02, 0x00, 0x03},
	{0x02, 0x00, 0x04},
	{0x02, 0x00, 0x05},
	{0x02, 0x00, 0x06},
	{0x02, 0x00, 0x07},
	{0x02, 0x00, 0x08},
	{0x02, 0x00, 0x09},
	{0x03, 0x00, 0x00},
	{0x03, 0x00, 0x01},
	{0x03, 0x00, 0x02},
	{0x03, 0x00, 0x03},
	{0x03, 0x00, 0x04},
	{0x03, 0x00, 0x05},
	{0x03, 0x00, 0x06},
	{0x04, 0x00, 0x00},
	{0x04, 0x00, 0x02},
	{0x04, 0x00, 0x04},
	{0x04, 0x00, 0x05},
	{0x04, 0x00, 0x06},
	{0x04, 0x00, 0x07},
	{0x04, 0x00, 0x08},
	{0x04, 0x00, 0x09},
	{0x04, 0x00, 0x0a},
	{0x04, 0x00, 0x0b},
	{0x04, 0x00, 0x0c},
	{0x07, 0x00, 0x00},
	{0x07, 0x00, 0x03},
	{0x08, 0x00, 0x00},
	{0x08, 0x00, 0x01},
	{0x08, 0x00, 0x02},
	{0x06, 0x18, 0x08},
	{0x06, 0xfc, 0x09},
	{0x06, 0xfc, 0x0a},
	{0x06, 0xfc, 0x0b},
	{0x06, 0x18, 0x0c},
	{0x06, 0xfc, 0x0d},
	{0x06, 0xfc, 0x0e},
	{0x06, 0xfc, 0x0f},
	{0x06, 0x18, 0x10},
	{0x06, 0xfe, 0x12},
	{0x06, 0x00, 0x11},
	{0x06, 0x00, 0x14},
	{0x06, 0x00, 0x13},
	{0x06, 0x28, 0x51},
	{0x06, 0xff, 0x53},
	{0x02, 0x00, 0x08},

	{0x03, 0x00, 0x03},
	{0x03, 0x10, 0x03},
	{}
};

/*
                                                       
 */
static const u8 spca505b_open_data_ccd[][3] = {

/*                   */
	{0x03, 0x04, 0x01},		/*     */
	{0x03, 0x00, 0x01},
	{0x03, 0x00, 0x00},
	{0x03, 0x21, 0x00},
	{0x03, 0x00, 0x04},
	{0x03, 0x00, 0x03},
	{0x03, 0x18, 0x03},
	{0x03, 0x08, 0x01},
	{0x03, 0x1c, 0x03},
	{0x03, 0x5c, 0x03},
	{0x03, 0x5c, 0x03},
	{0x03, 0x18, 0x01},

/*             */
	{0x04, 0x10, 0x01},
	{0x04, 0x00, 0x04},
	{0x04, 0x00, 0x05},
	{0x04, 0x20, 0x06},
	{0x04, 0x20, 0x07},

	{0x08, 0x0a, 0x00},

	{0x05, 0x00, 0x10},
	{0x05, 0x00, 0x11},
	{0x05, 0x00, 0x12},
	{0x05, 0x6f, 0x00},
	{0x05, initial_brightness >> 6, 0x00},
	{0x05, (initial_brightness << 2) & 0xff, 0x01},
	{0x05, 0x00, 0x02},
	{0x05, 0x01, 0x03},
	{0x05, 0x00, 0x04},
	{0x05, 0x03, 0x05},
	{0x05, 0xe0, 0x06},
	{0x05, 0x20, 0x07},
	{0x05, 0xa0, 0x08},
	{0x05, 0x00, 0x12},
	{0x05, 0x02, 0x0f},
	{0x05, 0x80, 0x14},		/*                         */
	{0x05, 0x01, 0xb0},
	{0x05, 0x01, 0xbf},
	{0x03, 0x02, 0x06},
	{0x05, 0x10, 0x46},
	{0x05, 0x08, 0x4a},

	{0x06, 0x00, 0x01},
	{0x06, 0x10, 0x02},
	{0x06, 0x64, 0x07},
	{0x06, 0x18, 0x08},
	{0x06, 0xfc, 0x09},
	{0x06, 0xfc, 0x0a},
	{0x06, 0xfc, 0x0b},
	{0x04, 0x00, 0x01},
	{0x06, 0x18, 0x0c},
	{0x06, 0xfc, 0x0d},
	{0x06, 0xfc, 0x0e},
	{0x06, 0xfc, 0x0f},
	{0x06, 0x11, 0x10},		/*          */
	{0x06, 0x00, 0x11},
	{0x06, 0xfe, 0x12},
	{0x06, 0x00, 0x13},
	{0x06, 0x00, 0x14},
	{0x06, 0x9d, 0x51},
	{0x06, 0x40, 0x52},
	{0x06, 0x7c, 0x53},
	{0x06, 0x40, 0x54},
	{0x06, 0x02, 0x57},
	{0x06, 0x03, 0x58},
	{0x06, 0x15, 0x59},
	{0x06, 0x05, 0x5a},
	{0x06, 0x03, 0x56},
	{0x06, 0x02, 0x3f},
	{0x06, 0x00, 0x40},
	{0x06, 0x39, 0x41},
	{0x06, 0x69, 0x42},
	{0x06, 0x87, 0x43},
	{0x06, 0x9e, 0x44},
	{0x06, 0xb1, 0x45},
	{0x06, 0xbf, 0x46},
	{0x06, 0xcc, 0x47},
	{0x06, 0xd5, 0x48},
	{0x06, 0xdd, 0x49},
	{0x06, 0xe3, 0x4a},
	{0x06, 0xe8, 0x4b},
	{0x06, 0xed, 0x4c},
	{0x06, 0xf2, 0x4d},
	{0x06, 0xf7, 0x4e},
	{0x06, 0xfc, 0x4f},
	{0x06, 0xff, 0x50},

	{0x05, 0x01, 0xc0},
	{0x05, 0x10, 0xcb},
	{0x05, 0x40, 0xc1},
	{0x05, 0x04, 0xc2},
	{0x05, 0x00, 0xca},
	{0x05, 0x40, 0xc1},
	{0x05, 0x09, 0xc2},
	{0x05, 0x00, 0xca},
	{0x05, 0xc0, 0xc1},
	{0x05, 0x09, 0xc2},
	{0x05, 0x00, 0xca},
	{0x05, 0x40, 0xc1},
	{0x05, 0x59, 0xc2},
	{0x05, 0x00, 0xca},
	{0x04, 0x00, 0x01},
	{0x05, 0x80, 0xc1},
	{0x05, 0xec, 0xc2},
	{0x05, 0x0, 0xca},

	{0x06, 0x02, 0x57},
	{0x06, 0x01, 0x58},
	{0x06, 0x15, 0x59},
	{0x06, 0x0a, 0x5a},
	{0x06, 0x01, 0x57},
	{0x06, 0x8a, 0x03},
	{0x06, 0x0a, 0x6c},
	{0x06, 0x30, 0x01},
	{0x06, 0x20, 0x02},
	{0x06, 0x00, 0x03},

	{0x05, 0x8c, 0x25},

	{0x06, 0x4d, 0x51},		/*                       */
	{0x06, 0x84, 0x53},		/*                   */
	{0x06, 0x00, 0x57},		/*               */
	{0x06, 0x18, 0x08},
	{0x06, 0xfc, 0x09},
	{0x06, 0xfc, 0x0a},
	{0x06, 0xfc, 0x0b},
	{0x06, 0x18, 0x0c},		/*                */
	{0x06, 0xfc, 0x0d},
	{0x06, 0xfc, 0x0e},
	{0x06, 0xfc, 0x0f},
	{0x06, 0x18, 0x10},		/*                     */

	{0x05, 0x01, 0x02},

	{0x04, 0x00, 0x08},		/*             */
	{0x04, 0x12, 0x09},
	{0x04, 0x21, 0x0a},
	{0x04, 0x10, 0x0b},
	{0x04, 0x21, 0x0c},
	{0x04, 0x1d, 0x00},		/*                */
	{0x04, 0x41, 0x01},		/*                      */

	{0x04, 0x00, 0x04},
	{0x04, 0x00, 0x05},
	{0x04, 0x10, 0x06},
	{0x04, 0x10, 0x07},
	{0x04, 0x40, 0x06},
	{0x04, 0x40, 0x07},
	{0x04, 0x00, 0x04},
	{0x04, 0x00, 0x05},

	{0x06, 0x1c, 0x17},
	{0x06, 0xe2, 0x19},
	{0x06, 0x1c, 0x1b},
	{0x06, 0xe2, 0x1d},
	{0x06, 0x5f, 0x1f},
	{0x06, 0x32, 0x20},

	{0x05, initial_brightness >> 6, 0x00},
	{0x05, (initial_brightness << 2) & 0xff, 0x01},
	{0x05, 0x06, 0xc1},
	{0x05, 0x58, 0xc2},
	{0x05, 0x00, 0xca},
	{0x05, 0x00, 0x11},
	{}
};

static int reg_write(struct gspca_dev *gspca_dev,
		     u16 req, u16 index, u16 value)
{
	int ret;
	struct usb_device *dev = gspca_dev->dev;

	ret = usb_control_msg(dev,
			usb_sndctrlpipe(dev, 0),
			req,
			USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			value, index, NULL, 0, 500);
	PDEBUG(D_USBO, "reg write: 0x%02x,0x%02x:0x%02x, %d",
		req, index, value, ret);
	if (ret < 0)
		pr_err("reg write: error %d\n", ret);
	return ret;
}

/*                                                 */
static int reg_read(struct gspca_dev *gspca_dev,
			u16 req,	/*          */
			u16 index)	/*        */
{
	int ret;

	ret = usb_control_msg(gspca_dev->dev,
			usb_rcvctrlpipe(gspca_dev->dev, 0),
			req,
			USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			0,			/*       */
			index,
			gspca_dev->usb_buf, 2,
			500);			/*         */
	if (ret < 0)
		return ret;
	return (gspca_dev->usb_buf[1] << 8) + gspca_dev->usb_buf[0];
}

static int write_vector(struct gspca_dev *gspca_dev,
			const u8 data[][3])
{
	int ret, i = 0;

	while (data[i][0] != 0) {
		ret = reg_write(gspca_dev, data[i][0], data[i][2],
								data[i][1]);
		if (ret < 0)
			return ret;
		i++;
	}
	return 0;
}

/*                                       */
static int sd_config(struct gspca_dev *gspca_dev,
			const struct usb_device_id *id)
{
	struct sd *sd = (struct sd *) gspca_dev;
	struct cam *cam;

	cam = &gspca_dev->cam;
	cam->cam_mode = vga_mode;
	sd->subtype = id->driver_info;
	if (sd->subtype != IntelPCCameraPro)
		cam->nmodes = ARRAY_SIZE(vga_mode);
	else			/*                                 */
		cam->nmodes = ARRAY_SIZE(vga_mode) - 1;

	return 0;
}

/*                                                  */
static int sd_init(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;

	if (write_vector(gspca_dev,
			 sd->subtype == Nxultra
				? spca505b_init_data
				: spca505_init_data))
		return -EIO;
	return 0;
}

static void setbrightness(struct gspca_dev *gspca_dev, s32 brightness)
{
	reg_write(gspca_dev, 0x05, 0x00, (255 - brightness) >> 6);
	reg_write(gspca_dev, 0x05, 0x01, (255 - brightness) << 2);
}

static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *sd = (struct sd *) gspca_dev;
	int ret, mode;
	static u8 mode_tb[][3] = {
	/*                   */
		{0x00, 0x10, 0x10},	/*         */
		{0x01, 0x1a, 0x1a},	/*         */
		{0x02, 0x1c, 0x1d},	/*         */
		{0x04, 0x34, 0x34},	/*         */
		{0x05, 0x40, 0x40}	/*         */
	};

	if (sd->subtype == Nxultra)
		write_vector(gspca_dev, spca505b_open_data_ccd);
	else
		write_vector(gspca_dev, spca505_open_data_ccd);
	ret = reg_read(gspca_dev, 0x06, 0x16);

	if (ret < 0) {
		PERR("register read failed err: %d", ret);
		return ret;
	}
	if (ret != 0x0101) {
		pr_err("After vector read returns 0x%04x should be 0x0101\n",
		       ret);
	}

	ret = reg_write(gspca_dev, 0x06, 0x16, 0x0a);
	if (ret < 0)
		return ret;
	reg_write(gspca_dev, 0x05, 0xc2, 0x12);

	/*                                               
                                   */
	/*                                      */
	reg_write(gspca_dev, 0x02, 0x00, 0x00);

	mode = gspca_dev->cam.cam_mode[(int) gspca_dev->curr_mode].priv;
	reg_write(gspca_dev, SPCA50X_REG_COMPRESS, 0x00, mode_tb[mode][0]);
	reg_write(gspca_dev, SPCA50X_REG_COMPRESS, 0x06, mode_tb[mode][1]);
	reg_write(gspca_dev, SPCA50X_REG_COMPRESS, 0x07, mode_tb[mode][2]);

	return reg_write(gspca_dev, SPCA50X_REG_USB,
			 SPCA50X_USB_CTRL,
			 SPCA50X_CUSB_ENABLE);
}

static void sd_stopN(struct gspca_dev *gspca_dev)
{
	/*                            */
	reg_write(gspca_dev, 0x02, 0x00, 0x00);
}

/*                                                  */
static void sd_stop0(struct gspca_dev *gspca_dev)
{
	if (!gspca_dev->present)
		return;

	/*                                   */
	reg_write(gspca_dev, 0x03, 0x03, 0x20);
	reg_write(gspca_dev, 0x03, 0x01, 0x00);
	reg_write(gspca_dev, 0x03, 0x00, 0x01);
	reg_write(gspca_dev, 0x05, 0x10, 0x01);
	reg_write(gspca_dev, 0x05, 0x11, 0x0f);
}

static void sd_pkt_scan(struct gspca_dev *gspca_dev,
			u8 *data,			/*             */
			int len)			/*                   */
{
	switch (data[0]) {
	case 0:				/*                */
		gspca_frame_add(gspca_dev, LAST_PACKET, NULL, 0);
		data += SPCA50X_OFFSET_DATA;
		len -= SPCA50X_OFFSET_DATA;
		gspca_frame_add(gspca_dev, FIRST_PACKET, data, len);
		break;
	case 0xff:			/*      */
		break;
	default:
		data += 1;
		len -= 1;
		gspca_frame_add(gspca_dev, INTER_PACKET, data, len);
		break;
	}
}

static int sd_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct gspca_dev *gspca_dev =
		container_of(ctrl->handler, struct gspca_dev, ctrl_handler);

	gspca_dev->usb_err = 0;

	if (!gspca_dev->streaming)
		return 0;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		setbrightness(gspca_dev, ctrl->val);
		break;
	}
	return gspca_dev->usb_err;
}

static const struct v4l2_ctrl_ops sd_ctrl_ops = {
	.s_ctrl = sd_s_ctrl,
};

static int sd_init_controls(struct gspca_dev *gspca_dev)
{
	struct v4l2_ctrl_handler *hdl = &gspca_dev->ctrl_handler;

	gspca_dev->vdev.ctrl_handler = hdl;
	v4l2_ctrl_handler_init(hdl, 5);
	v4l2_ctrl_new_std(hdl, &sd_ctrl_ops,
			V4L2_CID_BRIGHTNESS, 0, 255, 1, 127);

	if (hdl->error) {
		pr_err("Could not initialize controls\n");
		return hdl->error;
	}
	return 0;
}

/*                        */
static const struct sd_desc sd_desc = {
	.name = MODULE_NAME,
	.config = sd_config,
	.init_controls = sd_init_controls,
	.init = sd_init,
	.start = sd_start,
	.stopN = sd_stopN,
	.stop0 = sd_stop0,
	.pkt_scan = sd_pkt_scan,
};

/*                             */
static const struct usb_device_id device_table[] = {
	{USB_DEVICE(0x041e, 0x401d), .driver_info = Nxultra},
	{USB_DEVICE(0x0733, 0x0430), .driver_info = IntelPCCameraPro},
/*                                                            */
	{}
};
MODULE_DEVICE_TABLE(usb, device_table);

/*                      */
static int sd_probe(struct usb_interface *intf,
			const struct usb_device_id *id)
{
	return gspca_dev_probe(intf, id, &sd_desc, sizeof(struct sd),
				THIS_MODULE);
}

static struct usb_driver sd_driver = {
	.name = MODULE_NAME,
	.id_table = device_table,
	.probe = sd_probe,
	.disconnect = gspca_disconnect,
#ifdef CONFIG_PM
	.suspend = gspca_suspend,
	.resume = gspca_resume,
	.reset_resume = gspca_resume,
#endif
};

module_usb_driver(sd_driver);
