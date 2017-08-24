/*
 * Copyright (C) 2012 Texas Instruments Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Contributors:
 *      Manjunath Hadli <manjunath.hadli@ti.com>
 *      Prabhakar Lad <prabhakar.lad@ti.com>
 */

#ifndef _DAVINCI_VPFE_DM365_IPIPEIF_USER_H
#define _DAVINCI_VPFE_DM365_IPIPEIF_USER_H

/*                        */
struct ipipeif_5_1_clkdiv {
	unsigned char m;
	unsigned char n;
};

enum ipipeif_decimation {
	IPIPEIF_DECIMATION_OFF,
	IPIPEIF_DECIMATION_ON
};

/*                             */
struct ipipeif_dpc {
	/*                         */
	unsigned char en;
	/*           */
	unsigned short thr;
};

enum ipipeif_clock {
	IPIPEIF_PIXCEL_CLK,
	IPIPEIF_SDRAM_CLK
};

enum  ipipeif_avg_filter {
	IPIPEIF_AVG_OFF,
	IPIPEIF_AVG_ON
};

struct ipipeif_5_1 {
	struct ipipeif_5_1_clkdiv clk_div;
	/*                         */
	struct ipipeif_dpc dpc;
	/*                       */
	unsigned short clip;
	/*                                    */
	unsigned char align_sync;
	/*                        */
	unsigned int rsz_start;
	/*                */
	unsigned char df_gain_en;
	/*               */
	unsigned short df_gain;
	/*                         */
	unsigned short df_gain_thr;
};

struct ipipeif_params {
	enum ipipeif_clock clock_select;
	unsigned int ppln;
	unsigned int lpfr;
	unsigned char rsz;
	enum ipipeif_decimation decimation;
	enum ipipeif_avg_filter avg_filter;
	/*           */
	struct ipipeif_5_1 if_5_1;
};

/*
                
                                                         
                                                         
 */
#define VIDIOC_VPFE_IPIPEIF_S_CONFIG \
	_IOWR('I', BASE_VIDIOC_PRIVATE + 1, struct ipipeif_params)
#define VIDIOC_VPFE_IPIPEIF_G_CONFIG \
	_IOWR('I', BASE_VIDIOC_PRIVATE + 2, struct ipipeif_params)

#endif		/*                                    */
