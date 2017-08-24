/*
 * Samsung Exynos4 SoC series FIMC-IS slave interface driver
 *
 * FIMC-IS error code definition
 *
 * Copyright (C) 2011 - 2013 Samsung Electronics Co., Ltd.
 *
 * Authors: Younghwan Joo <yhwan.joo@samsung.com>
 *          Sylwester Nawrocki <s.nawrocki@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef FIMC_IS_ERR_H_
#define FIMC_IS_ERR_H_

#define IS_ERROR_VER			011 /*                       */

enum {
	IS_ERROR_NONE,

	/*                */
	IS_ERROR_INVALID_COMMAND,
	IS_ERROR_REQUEST_FAIL,
	IS_ERROR_INVALID_SCENARIO,
	IS_ERROR_INVALID_SENSORID,
	IS_ERROR_INVALID_MODE_CHANGE,
	IS_ERROR_INVALID_MAGIC_NUMBER,
	IS_ERROR_INVALID_SETFILE_HDR,
	IS_ERROR_BUSY,
	IS_ERROR_SET_PARAMETER,
	IS_ERROR_INVALID_PATH,
	IS_ERROR_OPEN_SENSOR_FAIL,
	IS_ERROR_ENTRY_MSG_THREAD_DOWN,
	IS_ERROR_ISP_FRAME_END_NOT_DONE,
	IS_ERROR_DRC_FRAME_END_NOT_DONE,
	IS_ERROR_SCALERC_FRAME_END_NOT_DONE,
	IS_ERROR_ODC_FRAME_END_NOT_DONE,
	IS_ERROR_DIS_FRAME_END_NOT_DONE,
	IS_ERROR_TDNR_FRAME_END_NOT_DONE,
	IS_ERROR_SCALERP_FRAME_END_NOT_DONE,
	IS_ERROR_WAIT_STREAM_OFF_NOT_DONE,
	IS_ERROR_NO_MSG_IS_RECEIVED,
	IS_ERROR_SENSOR_MSG_FAIL,
	IS_ERROR_ISP_MSG_FAIL,
	IS_ERROR_DRC_MSG_FAIL,
	IS_ERROR_SCALERC_MSG_FAIL,
	IS_ERROR_ODC_MSG_FAIL,
	IS_ERROR_DIS_MSG_FAIL,
	IS_ERROR_TDNR_MSG_FAIL,
	IS_ERROR_SCALERP_MSG_FAIL,
	IS_ERROR_LHFD_MSG_FAIL,
	IS_ERROR_LHFD_INTERNAL_STOP,

	/*                  */
	IS_ERROR_SENSOR_PWRDN_FAIL	= 100,
	IS_ERROR_SENSOR_STREAM_ON_FAIL,
	IS_ERROR_SENSOR_STREAM_OFF_FAIL,

	/*               */
	IS_ERROR_ISP_PWRDN_FAIL		= 200,
	IS_ERROR_ISP_MULTIPLE_INPUT,
	IS_ERROR_ISP_ABSENT_INPUT,
	IS_ERROR_ISP_ABSENT_OUTPUT,
	IS_ERROR_ISP_NONADJACENT_OUTPUT,
	IS_ERROR_ISP_FORMAT_MISMATCH,
	IS_ERROR_ISP_WIDTH_MISMATCH,
	IS_ERROR_ISP_HEIGHT_MISMATCH,
	IS_ERROR_ISP_BITWIDTH_MISMATCH,
	IS_ERROR_ISP_FRAME_END_TIME_OUT,

	/*               */
	IS_ERROR_DRC_PWRDN_FAIL		= 300,
	IS_ERROR_DRC_MULTIPLE_INPUT,
	IS_ERROR_DRC_ABSENT_INPUT,
	IS_ERROR_DRC_NONADJACENT_INPUT,
	IS_ERROR_DRC_ABSENT_OUTPUT,
	IS_ERROR_DRC_NONADJACENT_OUTPUT,
	IS_ERROR_DRC_FORMAT_MISMATCH,
	IS_ERROR_DRC_WIDTH_MISMATCH,
	IS_ERROR_DRC_HEIGHT_MISMATCH,
	IS_ERROR_DRC_BITWIDTH_MISMATCH,
	IS_ERROR_DRC_FRAME_END_TIME_OUT,

	/*                   */
	IS_ERROR_SCALERC_PWRDN_FAIL	= 400,

	/*               */
	IS_ERROR_ODC_PWRDN_FAIL		= 500,

	/*               */
	IS_ERROR_DIS_PWRDN_FAIL		= 600,

	/*                */
	IS_ERROR_TDNR_PWRDN_FAIL	= 700,

	/*                   */
	IS_ERROR_SCALERP_PWRDN_FAIL	= 800,

	/*              */
	IS_ERROR_FD_PWRDN_FAIL		= 900,
	IS_ERROR_FD_MULTIPLE_INPUT,
	IS_ERROR_FD_ABSENT_INPUT,
	IS_ERROR_FD_NONADJACENT_INPUT,
	IS_ERROR_LHFD_FRAME_END_TIME_OUT,

	IS_ERROR_UNKNOWN		= 1000,
};

#define IS_ERROR_TIME_OUT_FLAG	0x80000000

/*                          */
enum fimc_is_error {
	/*                     */
	ERROR_COMMON_NONE		= 0,
	ERROR_COMMON_CMD		= 1,	/*                 */
	ERROR_COMMON_PARAMETER		= 2,	/*                   */
	/*                                        */
	ERROR_COMMON_SETFILE_LOAD	= 3,
	/*                                        */
	ERROR_COMMON_SETFILE_ADJUST	= 4,
	/*                                                     */
	ERROR_COMMON_SETFILE_INDEX	= 5,
	/*                                                */
	ERROR_COMMON_INPUT_PATH		= 6,
	/*                                           */
	ERROR_COMMON_INPUT_INIT		= 7,
	/*                                                  */
	ERROR_COMMON_OUTPUT_PATH	= 8,
	/*                                            */
	ERROR_COMMON_OUTPUT_INIT	= 9,

	ERROR_CONTROL_NONE		= ERROR_COMMON_NONE,
	ERROR_CONTROL_BYPASS		= 11,	/*                   */

	ERROR_OTF_INPUT_NONE		= ERROR_COMMON_NONE,
	ERROR_OTF_INPUT_CMD		= 21,
	/*                                                     */
	ERROR_OTF_INPUT_FORMAT		= 22,
	/*                                            */
	ERROR_OTF_INPUT_WIDTH		= 23,
	/*                                            */
	ERROR_OTF_INPUT_HEIGHT		= 24,
	/*                                             */
	ERROR_OTF_INPUT_BIT_WIDTH	= 25,
	/*                           */
	ERROR_OTF_INPUT_USER_FRAMETIIME	= 26,

	ERROR_DMA_INPUT_NONE		= ERROR_COMMON_NONE,
	/*                                            */
	ERROR_DMA_INPUT_WIDTH		= 31,
	/*                                            */
	ERROR_DMA_INPUT_HEIGHT		= 32,
	/*                                                              */
	ERROR_DMA_INPUT_FORMAT		= 33,
	/*                                            */
	ERROR_DMA_INPUT_BIT_WIDTH	= 34,
	/*                                                                   */
	ERROR_DMA_INPUT_ORDER		= 35,
	/*                                     */
	ERROR_DMA_INPUT_PLANE		= 36,

	ERROR_OTF_OUTPUT_NONE		= ERROR_COMMON_NONE,
	/*                               */
	ERROR_OTF_OUTPUT_WIDTH		= 41,
	/*                               */
	ERROR_OTF_OUTPUT_HEIGHT		= 42,
	/*                              */
	ERROR_OTF_OUTPUT_FORMAT		= 43,
	/*                                   */
	ERROR_OTF_OUTPUT_BIT_WIDTH	= 44,

	ERROR_DMA_OUTPUT_NONE		= ERROR_COMMON_NONE,
	ERROR_DMA_OUTPUT_WIDTH		= 51,	/*               */
	ERROR_DMA_OUTPUT_HEIGHT		= 52,	/*                */
	ERROR_DMA_OUTPUT_FORMAT		= 53,	/*                */
	ERROR_DMA_OUTPUT_BIT_WIDTH	= 54,	/*                   */
	ERROR_DMA_OUTPUT_PLANE		= 55,	/*               */
	ERROR_DMA_OUTPUT_ORDER		= 56,	/*               */

	ERROR_GLOBAL_SHOTMODE_NONE	= ERROR_COMMON_NONE,

	/*                       */
	ERROR_SENSOR_NONE		= ERROR_COMMON_NONE,
	ERROR_SENSOR_I2C_FAIL		= 101,
	ERROR_SENSOR_INVALID_FRAMERATE,
	ERROR_SENSOR_INVALID_EXPOSURETIME,
	ERROR_SENSOR_INVALID_SIZE,
	ERROR_SENSOR_INVALID_SETTING,
	ERROR_SENSOR_ACTURATOR_INIT_FAIL,
	ERROR_SENSOR_INVALID_AF_POS,
	ERROR_SENSOR_UNSUPPORT_FUNC,
	ERROR_SENSOR_UNSUPPORT_PERI,
	ERROR_SENSOR_UNSUPPORT_AF,

	/*                     */
	ERROR_ISP_AF_NONE		= ERROR_COMMON_NONE,
	ERROR_ISP_AF_BUSY		= 201,
	ERROR_ISP_AF_INVALID_COMMAND	= 202,
	ERROR_ISP_AF_INVALID_MODE	= 203,
	ERROR_ISP_FLASH_NONE		= ERROR_COMMON_NONE,
	ERROR_ISP_AWB_NONE		= ERROR_COMMON_NONE,
	ERROR_ISP_IMAGE_EFFECT_NONE	= ERROR_COMMON_NONE,
	ERROR_ISP_ISO_NONE		= ERROR_COMMON_NONE,
	ERROR_ISP_ADJUST_NONE		= ERROR_COMMON_NONE,
	ERROR_ISP_METERING_NONE		= ERROR_COMMON_NONE,
	ERROR_ISP_AFC_NONE		= ERROR_COMMON_NONE,

	/*                     */

	/*                     */
	ERROR_FD_NONE					= ERROR_COMMON_NONE,
	/*                           */
	ERROR_FD_CONFIG_MAX_NUMBER_STATE		= 401,
	ERROR_FD_CONFIG_MAX_NUMBER_INVALID		= 402,
	ERROR_FD_CONFIG_YAW_ANGLE_STATE			= 403,
	ERROR_FD_CONFIG_YAW_ANGLE_INVALID		= 404,
	ERROR_FD_CONFIG_ROLL_ANGLE_STATE		= 405,
	ERROR_FD_CONFIG_ROLL_ANGLE_INVALID		= 406,
	ERROR_FD_CONFIG_SMILE_MODE_INVALID		= 407,
	ERROR_FD_CONFIG_BLINK_MODE_INVALID		= 408,
	ERROR_FD_CONFIG_EYES_DETECT_INVALID		= 409,
	ERROR_FD_CONFIG_MOUTH_DETECT_INVALID		= 410,
	ERROR_FD_CONFIG_ORIENTATION_STATE		= 411,
	ERROR_FD_CONFIG_ORIENTATION_INVALID		= 412,
	ERROR_FD_CONFIG_ORIENTATION_VALUE_INVALID	= 413,
	/*                                                                    */
	ERROR_FD_RESULT					= 414,
	/*                                                                  */
	ERROR_FD_MODE					= 415,
	/*                           */
	ERROR_SCALER_NO_NONE				= ERROR_COMMON_NONE,
	ERROR_SCALER_DMA_OUTSEL				= 501,
	ERROR_SCALER_H_RATIO				= 502,
	ERROR_SCALER_V_RATIO				= 503,

	ERROR_SCALER_IMAGE_EFFECT			= 510,

	ERROR_SCALER_ROTATE				= 520,
	ERROR_SCALER_FLIP				= 521,
};

const char * const fimc_is_strerr(unsigned int error);
const char * const fimc_is_param_strerr(unsigned int error);

#endif /*                */
