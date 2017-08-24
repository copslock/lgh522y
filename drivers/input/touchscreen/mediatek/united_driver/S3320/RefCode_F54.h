//                                                              
// Copyright ?2012 Synaptics Incorporated. All rights reserved.
//
//                                                             
//                                                             
//       
//
//                                                                 
//                                                             
//                                                             
// information in this file are not expressly or implicitly licensed
//                                                                
//                             
//                                                              

#include <linux/kernel.h>	//      
#include <linux/delay.h>	//      
#include <linux/time.h>		//                                            
#include <linux/string.h>	//      
#include <linux/i2c.h>

#include <linux/input/lgtp_common.h>
#include <linux/input/lgtp_common_driver.h>
#include <linux/input/lgtp_platform_api.h>
#include <linux/input/lgtp_model_config.h>
#include <linux/input/lgtp_device_s3320.h>

#if 0
extern const int DefaultTimeout;

int F54Test(int input);
bool switchPage(int page);
void RunQueries(void);
void DeltaImageReport(void);
void RawImageReport(void);
void SensorSpeed(void);
void ADCRange(void);
void TxTxTest(void);
int RxRxShortTest(void);
void HighResistanceTest(void);
void MaxMinTest(void);
int ImageTest(void);
void DeltaImageReport(void);

void SCAN_PDT(void);
#endif

#define TRX_max 32
#define CAP_FILE_PATH "/sns/touch/cap_diff_test.txt"
#define DS5_BUFFER_SIZE 6000

extern int UpperImage[32][32];
extern int LowerImage[32][32];
extern int SensorSpeedUpperImage[32][32];
extern int SensorSpeedLowerImage[32][32];
extern int ADCUpperImage[32][32];
extern int ADCLowerImage[32][32];
extern unsigned char RxChannelCount;
extern unsigned char TxChannelCount;
extern void SCAN_PDT(struct i2c_client *client);
extern void Reset(void);
extern int F54Test(int input, int mode, char *buf);//                                                                 
extern int GetImageReport(char *buf);
extern int diffnode(unsigned short *ImagepTest);
extern int write_file(char *filename, char *data);
extern int write_log(char *filename, char *data);
extern int Read8BitRegisters(unsigned short regAddr, unsigned char *data, int length);
extern int Write8BitRegisters(unsigned short regAddr, unsigned char *data, int length);

/*             */

