#ifndef SPI_ADIS16204_H_
#define SPI_ADIS16204_H_

#define ADIS16204_STARTUP_DELAY	220 /*    */

#define ADIS16204_FLASH_CNT      0x00 /*                          */
#define ADIS16204_SUPPLY_OUT     0x02 /*                      */
#define ADIS16204_XACCL_OUT      0x04 /*                              */
#define ADIS16204_YACCL_OUT      0x06 /*                              */
#define ADIS16204_AUX_ADC        0x08 /*                             */
#define ADIS16204_TEMP_OUT       0x0A /*                     */
#define ADIS16204_X_PEAK_OUT     0x0C /*                 */
#define ADIS16204_Y_PEAK_OUT     0x0E /*                 */
#define ADIS16204_XACCL_NULL     0x10 /*                                              */
#define ADIS16204_YACCL_NULL     0x12 /*                                              */
#define ADIS16204_XACCL_SCALE    0x14 /*                                          */
#define ADIS16204_YACCL_SCALE    0x16 /*                                          */
#define ADIS16204_XY_RSS_OUT     0x18 /*                                */
#define ADIS16204_XY_PEAK_OUT    0x1A /*                                */
#define ADIS16204_CAP_BUF_1      0x1C /*                                  */
#define ADIS16204_CAP_BUF_2      0x1E /*                                  */
#define ADIS16204_ALM_MAG1       0x20 /*                             */
#define ADIS16204_ALM_MAG2       0x22 /*                             */
#define ADIS16204_ALM_CTRL       0x28 /*               */
#define ADIS16204_CAPT_PNTR      0x2A /*                                  */
#define ADIS16204_AUX_DAC        0x30 /*                    */
#define ADIS16204_GPIO_CTRL      0x32 /*                                              */
#define ADIS16204_MSC_CTRL       0x34 /*                       */
#define ADIS16204_SMPL_PRD       0x36 /*                                       */
#define ADIS16204_AVG_CNT        0x38 /*                                 */
#define ADIS16204_SLP_CNT        0x3A /*                               */
#define ADIS16204_DIAG_STAT      0x3C /*                                     */
#define ADIS16204_GLOB_CMD       0x3E /*                                    */

/*          */
#define ADIS16204_MSC_CTRL_PWRUP_SELF_TEST	(1 << 10) /*                                                  */
#define ADIS16204_MSC_CTRL_SELF_TEST_EN	        (1 << 8)  /*                  */
#define ADIS16204_MSC_CTRL_DATA_RDY_EN	        (1 << 2)  /*                                              */
#define ADIS16204_MSC_CTRL_ACTIVE_HIGH	        (1 << 1)  /*                                                      */
#define ADIS16204_MSC_CTRL_DATA_RDY_DIO2	(1 << 0)  /*                                               */

/*           */
#define ADIS16204_DIAG_STAT_ALARM2        (1<<9) /*                                                      */
#define ADIS16204_DIAG_STAT_ALARM1        (1<<8) /*                                                      */
#define ADIS16204_DIAG_STAT_SELFTEST_FAIL_BIT 5 /*                                                      
                           */
#define ADIS16204_DIAG_STAT_SPI_FAIL_BIT      3 /*                            */
#define ADIS16204_DIAG_STAT_FLASH_UPT_BIT     2 /*                      */
#define ADIS16204_DIAG_STAT_POWER_HIGH_BIT    1 /*                            */
#define ADIS16204_DIAG_STAT_POWER_LOW_BIT     0 /*                            */

/*          */
#define ADIS16204_GLOB_CMD_SW_RESET	(1<<7)
#define ADIS16204_GLOB_CMD_CLEAR_STAT	(1<<4)
#define ADIS16204_GLOB_CMD_FACTORY_CAL	(1<<1)

#define ADIS16204_ERROR_ACTIVE          (1<<14)

enum adis16204_scan {
	ADIS16204_SCAN_ACC_X,
	ADIS16204_SCAN_ACC_Y,
	ADIS16204_SCAN_ACC_XY,
	ADIS16204_SCAN_SUPPLY,
	ADIS16204_SCAN_AUX_ADC,
	ADIS16204_SCAN_TEMP,
};

#endif /*                  */
