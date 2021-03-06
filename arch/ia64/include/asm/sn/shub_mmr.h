/*
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (c) 2001-2005 Silicon Graphics, Inc.  All rights reserved.
 */

#ifndef _ASM_IA64_SN_SHUB_MMR_H
#define _ASM_IA64_SN_SHUB_MMR_H

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_IPI_INT			__IA64_UL_CONST(0x0000000110000380)
#define SH2_IPI_INT			__IA64_UL_CONST(0x0000000010000380)

/*                                                                      */
/*                                                                      */
#define SH_IPI_INT_TYPE_SHFT				0
#define SH_IPI_INT_TYPE_MASK		__IA64_UL_CONST(0x0000000000000007)

/*                                                                      */
/*                                                                      */
#define SH_IPI_INT_AGT_SHFT				3
#define SH_IPI_INT_AGT_MASK		__IA64_UL_CONST(0x0000000000000008)

/*                                                                      */
/*                                                                     */
#define SH_IPI_INT_PID_SHFT                      	4
#define SH_IPI_INT_PID_MASK		__IA64_UL_CONST(0x00000000000ffff0)

/*                                                                      */
/*                                                                      */
#define SH_IPI_INT_BASE_SHFT				21
#define SH_IPI_INT_BASE_MASK 		__IA64_UL_CONST(0x0003ffffffe00000)

/*                                                                      */
/*                                                                      */
#define SH_IPI_INT_IDX_SHFT				52
#define SH_IPI_INT_IDX_MASK		__IA64_UL_CONST(0x0ff0000000000000)

/*                                                                      */
/*                                                                      */
#define SH_IPI_INT_SEND_SHFT				63
#define SH_IPI_INT_SEND_MASK		__IA64_UL_CONST(0x8000000000000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_EVENT_OCCURRED		__IA64_UL_CONST(0x0000000110010000)
#define SH1_EVENT_OCCURRED_ALIAS	__IA64_UL_CONST(0x0000000110010008)
#define SH2_EVENT_OCCURRED		__IA64_UL_CONST(0x0000000010010000)
#define SH2_EVENT_OCCURRED_ALIAS 	__IA64_UL_CONST(0x0000000010010008)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_PI_CAM_CONTROL		__IA64_UL_CONST(0x0000000120050300)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_SHUB_ID			__IA64_UL_CONST(0x0000000110060580)
#define SH1_SHUB_ID_REVISION_SHFT			28
#define SH1_SHUB_ID_REVISION_MASK	__IA64_UL_CONST(0x00000000f0000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_RTC				__IA64_UL_CONST(0x00000001101c0000)
#define SH2_RTC				__IA64_UL_CONST(0x00000002101c0000)
#define SH_RTC_MASK			__IA64_UL_CONST(0x007fffffffffffff)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_PIO_WRITE_STATUS_0		__IA64_UL_CONST(0x0000000120070200)
#define SH1_PIO_WRITE_STATUS_1		__IA64_UL_CONST(0x0000000120070280)
#define SH2_PIO_WRITE_STATUS_0		__IA64_UL_CONST(0x0000000020070200)
#define SH2_PIO_WRITE_STATUS_1		__IA64_UL_CONST(0x0000000020070280)
#define SH2_PIO_WRITE_STATUS_2		__IA64_UL_CONST(0x0000000020070300)
#define SH2_PIO_WRITE_STATUS_3		__IA64_UL_CONST(0x0000000020070380)

/*                                                                      */
/*                                                                      */
#define SH_PIO_WRITE_STATUS_WRITE_DEADLOCK_SHFT		1
#define SH_PIO_WRITE_STATUS_WRITE_DEADLOCK_MASK \
					__IA64_UL_CONST(0x0000000000000002)

/*                                                                      */
/*                                                                      */
#define SH_PIO_WRITE_STATUS_PENDING_WRITE_COUNT_SHFT	56
#define SH_PIO_WRITE_STATUS_PENDING_WRITE_COUNT_MASK \
					__IA64_UL_CONST(0x3f00000000000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_PIO_WRITE_STATUS_0_ALIAS	__IA64_UL_CONST(0x0000000120070208)
#define SH2_PIO_WRITE_STATUS_0_ALIAS	__IA64_UL_CONST(0x0000000020070208)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_UART_INT_SHFT			20
#define SH_EVENT_OCCURRED_UART_INT_MASK	__IA64_UL_CONST(0x0000000000100000)

/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_IPI_INT_SHFT			28
#define SH_EVENT_OCCURRED_IPI_INT_MASK	__IA64_UL_CONST(0x0000000010000000)

/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_II_INT0_SHFT			29
#define SH_EVENT_OCCURRED_II_INT0_MASK	__IA64_UL_CONST(0x0000000020000000)

/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_II_INT1_SHFT			30
#define SH_EVENT_OCCURRED_II_INT1_MASK	__IA64_UL_CONST(0x0000000040000000)

/*                                                                      */
/*                                                                      */
#define SH2_EVENT_OCCURRED_EXTIO_INT2_SHFT		33
#define SH2_EVENT_OCCURRED_EXTIO_INT2_MASK __IA64_UL_CONST(0x0000000200000000)

/*                                                                      */
/*                                                                      */
#define SH2_EVENT_OCCURRED_EXTIO_INT3_SHFT		34
#define SH2_EVENT_OCCURRED_EXTIO_INT3_MASK __IA64_UL_CONST(0x0000000400000000)

#define SH_ALL_INT_MASK \
	(SH_EVENT_OCCURRED_UART_INT_MASK | SH_EVENT_OCCURRED_IPI_INT_MASK | \
	 SH_EVENT_OCCURRED_II_INT0_MASK | SH_EVENT_OCCURRED_II_INT1_MASK | \
	 SH_EVENT_OCCURRED_II_INT1_MASK | SH2_EVENT_OCCURRED_EXTIO_INT2_MASK | \
	 SH2_EVENT_OCCURRED_EXTIO_INT3_MASK)


/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_REAL_JUNK_BUS_LED0			0x7fed00000UL
#define SH1_REAL_JUNK_BUS_LED1			0x7fed10000UL
#define SH1_REAL_JUNK_BUS_LED2			0x7fed20000UL
#define SH1_REAL_JUNK_BUS_LED3			0x7fed30000UL

#define SH2_REAL_JUNK_BUS_LED0			0xf0000000UL
#define SH2_REAL_JUNK_BUS_LED1			0xf0010000UL
#define SH2_REAL_JUNK_BUS_LED2			0xf0020000UL
#define SH2_REAL_JUNK_BUS_LED3			0xf0030000UL

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_PTC_0			__IA64_UL_CONST(0x00000001101a0000)

/*                                                                      */
/*                                                                      */
#define SH1_PTC_0_A_SHFT				0

/*                                                                      */
/*                                                                      */
#define SH1_PTC_0_PS_SHFT				2

/*                                                                      */
/*                                                                      */
#define SH1_PTC_0_RID_SHFT				8

/*                                                                      */
/*                                                                      */
#define SH1_PTC_0_START_SHFT				63

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_PTC_1			__IA64_UL_CONST(0x00000001101a0080)

/*                                                                      */
/*                                                                      */
#define SH1_PTC_1_START_SHFT				63

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH2_PTC				__IA64_UL_CONST(0x0000000170000000)

/*                                                                      */
/*                                                                      */
#define SH2_PTC_A_SHFT					0

/*                                                                      */
/*                                                                      */
#define SH2_PTC_PS_SHFT					2

/*                                                                    */
/*                                                                      */
#define SH2_PTC_RID_SHFT				4

/*                                                                      */
/*                                                                      */
#define SH2_PTC_START_SHFT				63

/*                                                                      */
/*                                                                      */
#define SH2_PTC_ADDR_SHFT				4
#define SH2_PTC_ADDR_MASK		__IA64_UL_CONST(0x1ffffffffffff000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_RTC1_INT_CONFIG		__IA64_UL_CONST(0x0000000110001480)
#define SH2_RTC1_INT_CONFIG		__IA64_UL_CONST(0x0000000010001480)
#define SH_RTC1_INT_CONFIG_MASK		__IA64_UL_CONST(0x0ff3ffffffefffff)
#define SH_RTC1_INT_CONFIG_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_RTC1_INT_CONFIG_TYPE_SHFT			0
#define SH_RTC1_INT_CONFIG_TYPE_MASK	__IA64_UL_CONST(0x0000000000000007)

/*                                                                      */
/*                                                                      */
#define SH_RTC1_INT_CONFIG_AGT_SHFT			3
#define SH_RTC1_INT_CONFIG_AGT_MASK	__IA64_UL_CONST(0x0000000000000008)

/*                                                                      */
/*                                                                     */
#define SH_RTC1_INT_CONFIG_PID_SHFT			4
#define SH_RTC1_INT_CONFIG_PID_MASK	__IA64_UL_CONST(0x00000000000ffff0)

/*                                                                      */
/*                                                                      */
#define SH_RTC1_INT_CONFIG_BASE_SHFT			21
#define SH_RTC1_INT_CONFIG_BASE_MASK	__IA64_UL_CONST(0x0003ffffffe00000)

/*                                                                      */
/*                                                                      */
#define SH_RTC1_INT_CONFIG_IDX_SHFT			52
#define SH_RTC1_INT_CONFIG_IDX_MASK	__IA64_UL_CONST(0x0ff0000000000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_RTC1_INT_ENABLE		__IA64_UL_CONST(0x0000000110001500)
#define SH2_RTC1_INT_ENABLE		__IA64_UL_CONST(0x0000000010001500)
#define SH_RTC1_INT_ENABLE_MASK		__IA64_UL_CONST(0x0000000000000001)
#define SH_RTC1_INT_ENABLE_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_RTC1_INT_ENABLE_RTC1_ENABLE_SHFT		0
#define SH_RTC1_INT_ENABLE_RTC1_ENABLE_MASK \
					__IA64_UL_CONST(0x0000000000000001)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_RTC2_INT_CONFIG		__IA64_UL_CONST(0x0000000110001580)
#define SH2_RTC2_INT_CONFIG		__IA64_UL_CONST(0x0000000010001580)
#define SH_RTC2_INT_CONFIG_MASK		__IA64_UL_CONST(0x0ff3ffffffefffff)
#define SH_RTC2_INT_CONFIG_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_RTC2_INT_CONFIG_TYPE_SHFT			0
#define SH_RTC2_INT_CONFIG_TYPE_MASK	__IA64_UL_CONST(0x0000000000000007)

/*                                                                      */
/*                                                                      */
#define SH_RTC2_INT_CONFIG_AGT_SHFT			3
#define SH_RTC2_INT_CONFIG_AGT_MASK	__IA64_UL_CONST(0x0000000000000008)

/*                                                                      */
/*                                                                     */
#define SH_RTC2_INT_CONFIG_PID_SHFT			4
#define SH_RTC2_INT_CONFIG_PID_MASK	__IA64_UL_CONST(0x00000000000ffff0)

/*                                                                      */
/*                                                                      */
#define SH_RTC2_INT_CONFIG_BASE_SHFT			21
#define SH_RTC2_INT_CONFIG_BASE_MASK	__IA64_UL_CONST(0x0003ffffffe00000)

/*                                                                      */
/*                                                                      */
#define SH_RTC2_INT_CONFIG_IDX_SHFT			52
#define SH_RTC2_INT_CONFIG_IDX_MASK	__IA64_UL_CONST(0x0ff0000000000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_RTC2_INT_ENABLE		__IA64_UL_CONST(0x0000000110001600)
#define SH2_RTC2_INT_ENABLE		__IA64_UL_CONST(0x0000000010001600)
#define SH_RTC2_INT_ENABLE_MASK		__IA64_UL_CONST(0x0000000000000001)
#define SH_RTC2_INT_ENABLE_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_RTC2_INT_ENABLE_RTC2_ENABLE_SHFT		0
#define SH_RTC2_INT_ENABLE_RTC2_ENABLE_MASK \
					__IA64_UL_CONST(0x0000000000000001)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_RTC3_INT_CONFIG		__IA64_UL_CONST(0x0000000110001680)
#define SH2_RTC3_INT_CONFIG		__IA64_UL_CONST(0x0000000010001680)
#define SH_RTC3_INT_CONFIG_MASK		__IA64_UL_CONST(0x0ff3ffffffefffff)
#define SH_RTC3_INT_CONFIG_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_RTC3_INT_CONFIG_TYPE_SHFT			0
#define SH_RTC3_INT_CONFIG_TYPE_MASK	__IA64_UL_CONST(0x0000000000000007)

/*                                                                      */
/*                                                                      */
#define SH_RTC3_INT_CONFIG_AGT_SHFT			3
#define SH_RTC3_INT_CONFIG_AGT_MASK	__IA64_UL_CONST(0x0000000000000008)

/*                                                                      */
/*                                                                     */
#define SH_RTC3_INT_CONFIG_PID_SHFT			4
#define SH_RTC3_INT_CONFIG_PID_MASK	__IA64_UL_CONST(0x00000000000ffff0)

/*                                                                      */
/*                                                                      */
#define SH_RTC3_INT_CONFIG_BASE_SHFT			21
#define SH_RTC3_INT_CONFIG_BASE_MASK	__IA64_UL_CONST(0x0003ffffffe00000)

/*                                                                      */
/*                                                                      */
#define SH_RTC3_INT_CONFIG_IDX_SHFT			52
#define SH_RTC3_INT_CONFIG_IDX_MASK	__IA64_UL_CONST(0x0ff0000000000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_RTC3_INT_ENABLE		__IA64_UL_CONST(0x0000000110001700)
#define SH2_RTC3_INT_ENABLE		__IA64_UL_CONST(0x0000000010001700)
#define SH_RTC3_INT_ENABLE_MASK		__IA64_UL_CONST(0x0000000000000001)
#define SH_RTC3_INT_ENABLE_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_RTC3_INT_ENABLE_RTC3_ENABLE_SHFT		0
#define SH_RTC3_INT_ENABLE_RTC3_ENABLE_MASK \
					__IA64_UL_CONST(0x0000000000000001)

/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_RTC1_INT_SHFT			24
#define SH_EVENT_OCCURRED_RTC1_INT_MASK	__IA64_UL_CONST(0x0000000001000000)

/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_RTC2_INT_SHFT			25
#define SH_EVENT_OCCURRED_RTC2_INT_MASK	__IA64_UL_CONST(0x0000000002000000)

/*                                                                      */
/*                                                                      */
#define SH_EVENT_OCCURRED_RTC3_INT_SHFT			26
#define SH_EVENT_OCCURRED_RTC3_INT_MASK	__IA64_UL_CONST(0x0000000004000000)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_IPI_ACCESS			__IA64_UL_CONST(0x0000000110060480)
#define SH2_IPI_ACCESS0			__IA64_UL_CONST(0x0000000010060c00)
#define SH2_IPI_ACCESS1			__IA64_UL_CONST(0x0000000010060c80)
#define SH2_IPI_ACCESS2			__IA64_UL_CONST(0x0000000010060d00)
#define SH2_IPI_ACCESS3			__IA64_UL_CONST(0x0000000010060d80)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_INT_CMPB			__IA64_UL_CONST(0x00000001101b0080)
#define SH2_INT_CMPB			__IA64_UL_CONST(0x00000000101b0080)
#define SH_INT_CMPB_MASK		__IA64_UL_CONST(0x007fffffffffffff)
#define SH_INT_CMPB_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_INT_CMPB_REAL_TIME_CMPB_SHFT			0
#define SH_INT_CMPB_REAL_TIME_CMPB_MASK	__IA64_UL_CONST(0x007fffffffffffff)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_INT_CMPC			__IA64_UL_CONST(0x00000001101b0100)
#define SH2_INT_CMPC			__IA64_UL_CONST(0x00000000101b0100)
#define SH_INT_CMPC_MASK		__IA64_UL_CONST(0x007fffffffffffff)
#define SH_INT_CMPC_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_INT_CMPC_REAL_TIME_CMPC_SHFT			0
#define SH_INT_CMPC_REAL_TIME_CMPC_MASK	__IA64_UL_CONST(0x007fffffffffffff)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */

#define SH1_INT_CMPD			__IA64_UL_CONST(0x00000001101b0180)
#define SH2_INT_CMPD			__IA64_UL_CONST(0x00000000101b0180)
#define SH_INT_CMPD_MASK		__IA64_UL_CONST(0x007fffffffffffff)
#define SH_INT_CMPD_INIT		__IA64_UL_CONST(0x0000000000000000)

/*                                                                      */
/*                                                                      */
#define SH_INT_CMPD_REAL_TIME_CMPD_SHFT			0
#define SH_INT_CMPD_REAL_TIME_CMPD_MASK	__IA64_UL_CONST(0x007fffffffffffff)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_MD_DQLP_MMR_DIR_PRIVEC0	__IA64_UL_CONST(0x0000000100030300)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define SH1_MD_DQRP_MMR_DIR_PRIVEC0	__IA64_UL_CONST(0x0000000100050300)

/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
#define shubmmr(a,b) 		(is_shub2() ? a##2_##b : a##1_##b)

#define SH_REAL_JUNK_BUS_LED0	shubmmr(SH, REAL_JUNK_BUS_LED0)
#define SH_IPI_INT		shubmmr(SH, IPI_INT)
#define SH_EVENT_OCCURRED	shubmmr(SH, EVENT_OCCURRED)
#define SH_EVENT_OCCURRED_ALIAS	shubmmr(SH, EVENT_OCCURRED_ALIAS)
#define SH_RTC			shubmmr(SH, RTC)
#define SH_RTC1_INT_CONFIG	shubmmr(SH, RTC1_INT_CONFIG)
#define SH_RTC1_INT_ENABLE	shubmmr(SH, RTC1_INT_ENABLE)
#define SH_RTC2_INT_CONFIG	shubmmr(SH, RTC2_INT_CONFIG)
#define SH_RTC2_INT_ENABLE	shubmmr(SH, RTC2_INT_ENABLE)
#define SH_RTC3_INT_CONFIG	shubmmr(SH, RTC3_INT_CONFIG)
#define SH_RTC3_INT_ENABLE	shubmmr(SH, RTC3_INT_ENABLE)
#define SH_INT_CMPB		shubmmr(SH, INT_CMPB)
#define SH_INT_CMPC		shubmmr(SH, INT_CMPC)
#define SH_INT_CMPD		shubmmr(SH, INT_CMPD)

/*                                                                            */
/*                                                                            */
/*                                                                            */
/*                                                                            */

#define SH2_BT_ENG_CSR_0		__IA64_UL_CONST(0x0000000030040000)
#define SH2_BT_ENG_SRC_ADDR_0		__IA64_UL_CONST(0x0000000030040080)
#define SH2_BT_ENG_DEST_ADDR_0		__IA64_UL_CONST(0x0000000030040100)
#define SH2_BT_ENG_NOTIF_ADDR_0		__IA64_UL_CONST(0x0000000030040180)

/*                                                                            */
/*                                                                            */
/*                                                                            */

#define SH2_BT_ENG_CSR_1		__IA64_UL_CONST(0x0000000030050000)
#define SH2_BT_ENG_CSR_2		__IA64_UL_CONST(0x0000000030060000)
#define SH2_BT_ENG_CSR_3		__IA64_UL_CONST(0x0000000030070000)

#endif /*                         */
