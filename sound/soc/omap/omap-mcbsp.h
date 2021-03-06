/*
 * omap-mcbsp.h
 *
 * Copyright (C) 2008 Nokia Corporation
 *
 * Contact: Jarkko Nikula <jarkko.nikula@bitmer.com>
 *          Peter Ujfalusi <peter.ujfalusi@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef __OMAP_I2S_H__
#define __OMAP_I2S_H__

/*                                               */
enum omap_mcbsp_clksrg_clk {
	OMAP_MCBSP_SYSCLK_CLKS_FCLK,	/*               */
	OMAP_MCBSP_SYSCLK_CLKS_EXT,	/*                   */
	OMAP_MCBSP_SYSCLK_CLK,		/*               */
	OMAP_MCBSP_SYSCLK_CLKX_EXT,	/*                   */
	OMAP_MCBSP_SYSCLK_CLKR_EXT,	/*                   */
};

/*                */
enum omap_mcbsp_div {
	OMAP_MCBSP_CLKGDV,		/*                               */
};

int omap_mcbsp_st_add_controls(struct snd_soc_pcm_runtime *rtd);

#endif
