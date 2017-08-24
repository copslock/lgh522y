/*
 * apds993x.h - Linux kernel modules for ambient light + proximity sensor
 *
 * Copyright (C) 2012 Lee Kai Koon <kai-koon.lee@avagotech.com>
 * Copyright (C) 2012 Avago Technologies
 * Copyright (C) 2013 LGE Inc.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#if !defined (_APDS993X_H_)
#define _APDS993X_H_

#define APDS993X_DEV_NAME	"apds993x"
#define DRIVER_VERSION		"1.0.0"

#define APS_TAG					"[ALS/PS] "
#define APS_FUN(f)				printk(KERN_INFO APS_TAG"%s\n", __FUNCTION__)
#define APS_ERR(fmt, args...)	printk(KERN_ERR  APS_TAG"%s %d : "fmt, __FUNCTION__, __LINE__, ##args)
#define APS_LOG(fmt, args...)	printk(KERN_ERR APS_TAG fmt, ##args)
#define APS_DBG(fmt, args...)	printk(KERN_INFO APS_TAG fmt, ##args)

#define APDS993X_PS_DETECTION_THRESHOLD		800
#define APDS993X_PS_HSYTERESIS_THRESHOLD	700
#define APDS993X_PS_PULSE_NUMBER		8

#define APDS993X_GA	48	/*                           */
#define APDS993X_COE_B	223	/*                           */
#define APDS993X_COE_C	70	/*                           */
#define APDS993X_COE_D	142	/*                           */
#define APDS993X_DF	52

#define APDS993X_ALS_THRESHOLD_HSYTERESIS	20	/*   */

/*               
  
                                           
  
 */

#ifdef CONFIG_MACH_LGE
#define APDS993X_IOCTL_PS_ENABLE		ALSPS_SET_PS_MODE
#define APDS993X_IOCTL_PS_GET_ENABLE	ALSPS_GET_PS_MODE
#define APDS993X_IOCTL_PS_GET_PDATA		ALSPS_GET_PS_RAW_DATA
#define APDS993X_IOCTL_ALS_ENABLE		ALSPS_SET_ALS_MODE
#define APDS993X_IOCTL_ALS_GET_ENABLE	ALSPS_GET_ALS_MODE
//                                                      
//                                                      
//                                    
#define APDS993X_IOCTL_GET_CALI			ALSPS_IOCTL_GET_CALI
#define APDS993X_IOCTL_SET_CALI			ALSPS_IOCTL_SET_CALI
#else	//                       
#define APDS993X_IOCTL_PS_ENABLE		1
#define APDS993X_IOCTL_PS_GET_ENABLE	2
#define APDS993X_IOCTL_PS_GET_PDATA		3	/*       */
#define APDS993X_IOCTL_ALS_ENABLE		4
#define APDS993X_IOCTL_ALS_GET_ENABLE	5
#define APDS993X_IOCTL_ALS_GET_CH0DATA	6	/*         */
#define APDS993X_IOCTL_ALS_GET_CH1DATA	7	/*         */
#define APDS993X_IOCTL_ALS_DELAY		8
#endif

/*
          
 */
#define APDS993X_ENABLE_REG	0x00
#define APDS993X_ATIME_REG	0x01
#define APDS993X_PTIME_REG	0x02
#define APDS993X_WTIME_REG	0x03
#define APDS993X_AILTL_REG	0x04
#define APDS993X_AILTH_REG	0x05
#define APDS993X_AIHTL_REG	0x06
#define APDS993X_AIHTH_REG	0x07
#define APDS993X_PILTL_REG	0x08
#define APDS993X_PILTH_REG	0x09
#define APDS993X_PIHTL_REG	0x0A
#define APDS993X_PIHTH_REG	0x0B
#define APDS993X_PERS_REG	0x0C
#define APDS993X_CONFIG_REG	0x0D
#define APDS993X_PPCOUNT_REG	0x0E
#define APDS993X_CONTROL_REG	0x0F
#define APDS993X_REV_REG	0x11
#define APDS993X_ID_REG		0x12
#define APDS993X_STATUS_REG	0x13
#define APDS993X_CH0DATAL_REG	0x14
#define APDS993X_CH0DATAH_REG	0x15
#define APDS993X_CH1DATAL_REG	0x16
#define APDS993X_CH1DATAH_REG	0x17
#define APDS993X_PDATAL_REG	0x18
#define APDS993X_PDATAH_REG	0x19

#define CMD_BYTE		0x80
#define CMD_WORD		0xA0
#define CMD_SPECIAL		0xE0

#define CMD_CLR_PS_INT		0xE5
#define CMD_CLR_ALS_INT		0xE6
#define CMD_CLR_PS_ALS_INT	0xE7

/*                               */
#define APDS993X_100MS_ADC_TIME	0xDB  /*                           */
#define APDS993X_50MS_ADC_TIME	0xED  /*                          */
#define APDS993X_27MS_ADC_TIME	0xF6  /*                         */

/*                                 */
#define APDS993X_ALS_REDUCE	0x04  /*                                    */

/*                              */
#define APDS993X_PPERS_0	0x00  /*                           */
#define APDS993X_PPERS_1	0x10  /*                                            */
#define APDS993X_PPERS_2	0x20  /*                                            */
#define APDS993X_PPERS_3	0x30  /*                                            */
#define APDS993X_PPERS_4	0x40  /*                                            */
#define APDS993X_PPERS_5	0x50  /*                                            */
#define APDS993X_PPERS_6	0x60  /*                                            */
#define APDS993X_PPERS_7	0x70  /*                                            */
#define APDS993X_PPERS_8	0x80  /*                                            */
#define APDS993X_PPERS_9	0x90  /*                                            */
#define APDS993X_PPERS_10	0xA0  /*                                             */
#define APDS993X_PPERS_11	0xB0  /*                                             */
#define APDS993X_PPERS_12	0xC0  /*                                             */
#define APDS993X_PPERS_13	0xD0  /*                                             */
#define APDS993X_PPERS_14	0xE0  /*                                             */
#define APDS993X_PPERS_15	0xF0  /*                                             */

#define APDS993X_APERS_0	0x00  /*                 */
#define APDS993X_APERS_1	0x01  /*                                            */
#define APDS993X_APERS_2	0x02  /*                                            */
#define APDS993X_APERS_3	0x03  /*                                            */
#define APDS993X_APERS_5	0x04  /*                                            */
#define APDS993X_APERS_10	0x05  /*                                             */
#define APDS993X_APERS_15	0x06  /*                                             */
#define APDS993X_APERS_20	0x07  /*                                             */
#define APDS993X_APERS_25	0x08  /*                                             */
#define APDS993X_APERS_30	0x09  /*                                             */
#define APDS993X_APERS_35	0x0A  /*                                             */
#define APDS993X_APERS_40	0x0B  /*                                             */
#define APDS993X_APERS_45	0x0C  /*                                             */
#define APDS993X_APERS_50	0x0D  /*                                             */
#define APDS993X_APERS_55	0x0E  /*                                             */
#define APDS993X_APERS_60	0x0F  /*                                             */

/*                                 */
#define APDS993X_AGAIN_1X	0x00  /*             */
#define APDS993X_AGAIN_8X	0x01  /*             */
#define APDS993X_AGAIN_16X	0x02  /*              */
#define APDS993X_AGAIN_120X	0x03  /*               */

#define APDS993X_PRX_IR_DIOD	0x20  /*                          */

#define APDS993X_PGAIN_1X	0x00  /*            */
#define APDS993X_PGAIN_2X	0x04  /*            */
#define APDS993X_PGAIN_4X	0x08  /*            */
#define APDS993X_PGAIN_8X	0x0C  /*            */

#define APDS993X_PDRVIE_100MA	0x00  /*                    */
#define APDS993X_PDRVIE_50MA	0x40  /*                   */
#define APDS993X_PDRVIE_25MA	0x80  /*                   */
#define APDS993X_PDRVIE_12_5MA	0xC0  /*                     */

/*           */
#define DEFAULT_CROSS_TALK	400
#define ADD_TO_CROSS_TALK	300
#define SUB_FROM_PS_THRESHOLD	100

#define APDS993X_HAL_USE_SYS_ENABLE

#define ALS_POLLING_ENABLED	/*                               */

extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);

static void apds993x_interrupt(void);

/*               */
static int apds993x_ps_detection_threshold = APDS993X_PS_DETECTION_THRESHOLD;
static int apds993x_ps_hsyteresis_threshold = APDS993X_PS_HSYTERESIS_THRESHOLD;
static int apds993x_ps_pulse_number = APDS993X_PS_PULSE_NUMBER;
static int apds993x_ps_pgain = 0;

typedef enum
{
	APDS993X_ALS_RES_10240 = 0,    /*                         */
	APDS993X_ALS_RES_19456 = 1,    /*                          */
	APDS993X_ALS_RES_37888 = 2     /*                           */
} apds993x_als_res_e;

typedef enum
{
	APDS993X_ALS_GAIN_1X    = 0,    /*          */
	APDS993X_ALS_GAIN_8X    = 1,    /*          */
	APDS993X_ALS_GAIN_16X   = 2,    /*           */
	APDS993X_ALS_GAIN_120X  = 3     /*            */
} apds993x_als_gain_e;

typedef enum {
	APDS_BIT_ALS = 1,
	APDS_BIT_PS = 2,
} APDS_BIT;

typedef enum {
	PS_NEAR = 0,
	PS_FAR = 1,
	PS_UNKNOWN = 2
} PS_STATUS;

/*
          
 */
struct apds993x_data {
	struct alsps_hw *hw;
	struct i2c_client *client;
	struct mutex update_lock;
	struct delayed_work dwork;		/*                  */
	struct delayed_work als_dwork;	/*                 */

	volatile long unsigned int enable;
	unsigned int atime;
	unsigned int ptime;
	unsigned int wtime;
	unsigned int ailt;
	unsigned int aiht;
	unsigned int pilt;
	unsigned int piht;
	unsigned int pers;
	unsigned int config;
	unsigned int ppcount;
	unsigned int control;

	u16 als;
	u16 ps;

	/*                       */
	unsigned int enable_ps_sensor;
	unsigned int enable_als_sensor;

	/*               */
	unsigned int ps_threshold;
	unsigned int ps_hysteresis_threshold; 	/*                                */
	unsigned int ps_detection;		/*                                  */
	unsigned int ps_data;			/*                  */

	/*           */
	unsigned int cross_talk;		/*                  */
	unsigned int avg_cross_talk;		/*                     */
	unsigned int ps_cal_result;		/*                      */

	/*                */
	unsigned int als_threshold_l;	/*               */
	unsigned int als_threshold_h;	/*                */
	unsigned int als_data;		/*                   */
	int als_prev_lux;		/*                             */
	int als_lux_value;		/*                         */

	unsigned int als_gain;		/*                            */
	unsigned int als_poll_delay;	/*                                                     */
	unsigned int als_atime_index;	/*                                   */
	unsigned int als_again_index;	/*                      */
	unsigned int als_reduce;	/*                                */

#if defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_drv;
#endif
};

#endif /*              */
