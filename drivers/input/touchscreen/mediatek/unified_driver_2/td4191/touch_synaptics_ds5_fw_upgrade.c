/*
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2012 Synaptics, Inc.

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to use,
   copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.


   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */
#define SYNA_F34_SAMPLE_CODE
#define SHOW_PROGRESS

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#define LGTP_MODULE "[TD4191]"

#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h>
#include <linux/input/unified_driver_2/lgtp_common.h>
#include <linux/input/unified_driver_2/lgtp_platform_api.h>
#include <linux/input/unified_driver_2/lgtp_device_td4191.h>
#include <linux/firmware.h>
unsigned short SynaF35QueryBase;
unsigned short SynaF35ControlBase;
unsigned short SynaF35DataBase;

/*                                 */
unsigned short SynaF34DataBase;
unsigned short SynaF34QueryBase;
unsigned short SynaF01DataBase;
unsigned short SynaF01CommandBase;
unsigned short SynaF01QueryBase;

unsigned short SynaF34Reflash_BlockNum;
unsigned short SynaF34Reflash_BlockData;
unsigned short SynaF34ReflashQuery_BootID;
unsigned short SynaF34ReflashQuery_FlashPropertyQuery;
unsigned short SynaF34ReflashQuery_BlockSize;
unsigned short SynaF34ReflashQuery_FirmwareBlockCount;
unsigned short SynaF34ReflashQuery_ConfigBlockCount;

unsigned char SynaF01Query43Length;

unsigned short SynaFirmwareBlockSize;
unsigned short SynaFirmwareBlockCount;
unsigned long SynaImageSize;

unsigned short SynaConfigBlockSize;
unsigned short SynaConfigBlockCount;
unsigned long SynaConfigImageSize;

//       
unsigned short SynaDisplayBlockSize;
unsigned short SynaDisplayBlockCount;
unsigned long SynaDisplayConfigImgStartAddr;

unsigned short SynaBootloadID;

unsigned short SynaF34_FlashControl;
unsigned short SynaF34_FlashStatus;

unsigned char *SynafirmwareImgData;
unsigned char *SynaconfigImgData;
unsigned char *SynalockImgData;
unsigned char *SynaDisplayConfigImgData;	//       
unsigned int SynafirmwareImgVersion;

unsigned char *my_image_bin;
unsigned long my_image_size;
u8	fw_image_config_id[5];

unsigned char *ConfigBlock;

void CompleteReflash(struct synaptics_ts_data *ts);
void FlashRecovery(struct synaptics_ts_data *ts);
void SynaInitialize(struct synaptics_ts_data *ts);
void SynaReadFirmwareInfo(struct synaptics_ts_data *ts);
void SynaEnableFlashing(struct synaptics_ts_data *ts);
void SynaProgramFirmware(struct synaptics_ts_data *ts);
void SynaFinalizeReflash(struct synaptics_ts_data *ts);
unsigned int SynaWaitForATTN(int time, struct synaptics_ts_data *ts);
bool CheckTouchControllerType(struct synaptics_ts_data *ts);
/*                                                         */
void eraseAllBlock(struct synaptics_ts_data *ts);
void SynaUpdateConfig(struct synaptics_ts_data *ts);
void EraseConfigBlock(struct synaptics_ts_data *ts);

enum FlashCommand {
	m_uF34ReflashCmd_FirmwareCrc        = 0x01,   //                        
	m_uF34ReflashCmd_FirmwareWrite      = 0x02,
	m_uF34ReflashCmd_EraseAll           = 0x03,
	m_uF34ReflashCmd_LockDown           = 0x04,   //                         
	m_uF34ReflashCmd_ConfigRead         = 0x05,
	m_uF34ReflashCmd_ConfigWrite        = 0x06,
	m_uF34ReflashCmd_EraseUIConfig      = 0x07,
	m_uF34ReflashCmd_Enable             = 0x0F,
	m_uF34ReflashCmd_QuerySensorID      = 0x08,
	m_uF34ReflashCmd_EraseBLConfig      = 0x09,
	m_uF34ReflashCmd_EraseDisplayConfig = 0x0A,
};

enum F35RecoveryCommand {
	CMD_F35_IDLE = 0x0,
	CMD_F35_RESERVED = 0x1,
	CMD_F35_WRITE_CHUNK = 0x2,
	CMD_F35_ERASE_ALL = 0x3,
	CMD_F35_RESET = 0x10,
};

char SynaFlashCommandStr[0x0C][0x20] = {
	"",
	"FirmwareCrc",
	"FirmwareWrite",
	"EraseAll",
	"LockDown",
	"ConfigRead",
	"ConfigWrite",
	"EraseUIConfig",
	"Enable",
	"QuerySensorID",
	"EraseBLConfig",
	"EraseDisplayConfig",
};

int FirmwareUpgrade (struct synaptics_ts_data *ts, const char *fw_path) {

	int ret = 0;
	const struct firmware *fw_entry = NULL;

	if ((ret = request_firmware(&fw_entry, fw_path,
					&ts->client->dev)) != 0) {
		TOUCH_ERR("request_firmware() failed %d\n", ret);
		goto error;
	}

	my_image_size = fw_entry->size;
	my_image_bin = kzalloc(sizeof(char) * (my_image_size + 1), GFP_KERNEL);
	if (my_image_bin == NULL) {
		TOUCH_ERR("Can not allocate  memory\n");
		ret = -ENOMEM;
		goto error;
	}

	memcpy(my_image_bin, fw_entry->data, my_image_size);

	/*              */
	*(my_image_bin+my_image_size) = 0xFF;

	strncpy(ts->fw_info.fw_image_product_id, &my_image_bin[0x0010], 6);
	strncpy(ts->fw_info.fw_image_version, &my_image_bin[0x11100], 4);

	ts->fw_info.fw_start = (unsigned char *)&my_image_bin[0];
	ts->fw_info.fw_size = my_image_size;

	CompleteReflash(ts);
	kfree(my_image_bin);
	return ret;
error:
	memset(&fw_entry, 0, sizeof(fw_entry));
	kfree(my_image_bin);
	return ret;
}

int FirmwareRecovery(struct synaptics_ts_data *ts, const char *fw_path)
{
	int ret = 0;
	const struct firmware *fw_entry = NULL;

	if ((ret = request_firmware(&fw_entry, fw_path,
			&ts->client->dev)) != 0) {
		TOUCH_ERR("request_firmware() failed %d\n", ret);
		goto error;
	}

	my_image_size = fw_entry->size;
	my_image_bin = kzalloc(sizeof(char) * (my_image_size + 1), GFP_KERNEL);
	if (my_image_bin == NULL) {
		TOUCH_ERR("Memory allocation failed\n");
		ret = -ENOMEM;
		goto error;
	}

	memcpy(my_image_bin, fw_entry->data, my_image_size);

	ts->fw_info.fw_start = (unsigned char *)&my_image_bin[0];
	ts->fw_info.fw_size = my_image_size;

	FlashRecovery(ts);
	kfree(my_image_bin);
	return ret;

error:
	memset(&fw_entry, 0, sizeof(fw_entry));
	kfree(my_image_bin);
	return ret;
}

static int writeRMI(struct i2c_client *client,
		u8 uRmiAddress, u8 *data, unsigned int length)
{
	return Touch_I2C_Write(client, uRmiAddress, data, length);
}

static int readRMI(struct i2c_client *client,
		u8 uRmiAddress, u8 *data, unsigned int length)
{
	return Touch_I2C_Read(client, uRmiAddress, data, length);
}

static void touch_i2c_check_addr(struct i2c_client *client)
{
    TOUCH_ERR("0 touch_i2c_check_addr : %x, \n",client->addr);
	if (client->addr == 0x220){
        TOUCH_ERR("1 touch_i2c_check_addr : %x, \n",client->addr);
		client->addr = 0x2C;
	}else{
	    TOUCH_ERR("2 touch_i2c_check_addr : %x, \n",client->addr);
		client->addr = 0x20;
	}
	return;
}

static void readRMI_Recovery(struct i2c_client *client,
		u8 uRmiAddress, u8 *data, unsigned int length)
{
    int retry,ret;
    for (retry = 0; retry < 6; retry++) {
        if (Touch_I2C_Read(client, uRmiAddress, data, length)< 0) {
             if (printk_ratelimit())
                 TOUCH_ERR("transfer error, "
                                "retry (%d)times\n", retry + 1);
                 msleep(20);
             } else
                 break;
                
             if (retry == 3 ) {
                   TOUCH_ERR("before client address : %x, \n",client->addr);
                   touch_i2c_check_addr(client);
                   TOUCH_ERR("after client address : %x, \n",client->addr);
             }
	}
	return;
}

bool CheckFlashStatus(struct synaptics_ts_data *ts,
		enum FlashCommand command)//      
{
	unsigned char uData = 0;
	//                                                            
	//                        
	//            
	readRMI(ts->client, SynaF34_FlashStatus, &uData, 1);
	//                                                         
	//                             

	//                        
	//                                                             
	//                                            
	return !(uData & 0x3F);
}

void SynaImageParser(struct synaptics_ts_data *ts)//      
{

	//                 
	SynaImageSize = ((unsigned int)my_image_bin[0x08] |
			(unsigned int)my_image_bin[0x09] << 8 |
			(unsigned int)my_image_bin[0x0A] << 16 |
			(unsigned int)my_image_bin[0x0B] << 24);

	SynafirmwareImgData = (unsigned char *)((&my_image_bin[0]) + 0x100);

	//       
	SynaDisplayConfigImgStartAddr = ((unsigned int)my_image_bin[0x40] |
			(unsigned int)my_image_bin[0x41] << 8 |
			(unsigned int)my_image_bin[0x42] << 16 |
			(unsigned int)my_image_bin[0x43] << 24);

	SynaDisplayConfigImgData = (unsigned char *)((&my_image_bin[0]) + SynaDisplayConfigImgStartAddr);

	//              
	TOUCH_DBG("%s: Retrieve SynaDisplayConfigImgData from image offset at 0x%s\n",
		__func__, SynaDisplayConfigImgData );

	SynaconfigImgData  =
		(unsigned char *)(SynafirmwareImgData + SynaImageSize);
	SynafirmwareImgVersion = (unsigned int)(my_image_bin[7]);

	switch (SynafirmwareImgVersion) {
	case 2:
		SynalockImgData = (unsigned char *)((&my_image_bin[0]) + 0xD0);
		break;
	case 3:
	case 4:
		SynalockImgData = (unsigned char *)((&my_image_bin[0]) + 0xC0);
		break;
	case 5:
	case 6:
		SynalockImgData = (unsigned char *)((&my_image_bin[0]) + 0xB0);
	default:
		break;
	}
}

void SynaBootloaderLock(struct synaptics_ts_data *ts)//      
{
	unsigned short lockBlockCount;
	unsigned char uData[2] = {0};
	unsigned short uBlockNum;
	enum FlashCommand cmd;

	if (my_image_bin[0x1E] == 0) {
		TOUCH_ERR("Skip lockdown process with this .img\n");
		return;
	}
	//                                     
	readRMI(ts->client, (SynaF34QueryBase + 1), &uData[0], 1);

	//                  
	if (uData[0] & 0x02) {
		TOUCH_ERR("Device unlocked. Lock it first...\n");
		//                                                       
		//                      
		//                                                         
		//                
		switch (SynafirmwareImgVersion) {
		case 2:
			lockBlockCount = 3;
			break;
		case 3:
		case 4:
			lockBlockCount = 4;
			break;
		case 5:
		case 6:
			lockBlockCount = 5;
			break;
		default:
			lockBlockCount = 0;
			break;
		}

		//                                       
		//                                                       
		//                       
		//                                                        
		//                        
		//                                                       
		//                           
		//                                      
		for (uBlockNum = 0; uBlockNum < lockBlockCount; ++uBlockNum) {
			uData[0] = uBlockNum & 0xff;
			uData[1] = (uBlockNum & 0xff00) >> 8;

			/*                    */
			writeRMI(ts->client,
					SynaF34Reflash_BlockNum, &uData[0], 2);

			/*                  */
			writeRMI(ts->client, SynaF34Reflash_BlockData,
					SynalockImgData, SynaFirmwareBlockSize);

			/*                         */
			SynalockImgData += SynaFirmwareBlockSize;

			/*                                    */
			cmd = m_uF34ReflashCmd_LockDown;
			writeRMI(ts->client, SynaF34_FlashControl,
					(unsigned char *)&cmd, 1);

			/*                                                 
                                 */
			SynaWaitForATTN(1000, ts);
			CheckFlashStatus(ts, cmd);
		}

		//                                                     
		//                                                            
		//                
		//                                                  
		//                                
		SynaEnableFlashing(ts);
	} else
		TOUCH_ERR("Device already locked.\n");
}

//                                                                            
//                                
bool CheckTouchControllerType(struct synaptics_ts_data *ts)
{
	int ID;
	char buffer[5] = {0};
	char controllerType[20] = {0};
	unsigned char uData[4] = {0};

	readRMI(ts->client, (SynaF01QueryBase + 22),
			&SynaF01Query43Length, 1); //  

	if ((SynaF01Query43Length & 0x0f) > 0) {
		readRMI(ts->client, (SynaF01QueryBase + 23), &uData[0], 1);
		if (uData[0] & 0x01) {
			readRMI(ts->client, (SynaF01QueryBase + 17),
					&uData[0], 2);

			ID = ((int)uData[0] | ((int)uData[1] << 8));

			if (strstr(controllerType, buffer) != 0)
				return true;
			return false;
		} else
			return false;
	} else
		return false;
}

/*                                                        
   
                          
                           
        
                                                                     
                 

                         
   
                          
   
                               
   
   

                                         
              
               
   */


/*                                                   
                                      
                                                                             
              
                                                           
 */
void SynaScanPDT(struct synaptics_ts_data *ts) //                
{
	unsigned char address;
	unsigned char uData[2] = {0}; //      
	unsigned char buffer[6] = {0};

	ts->ubootloader_mode = false;

	for (address = 0xe9; address > 0xc0; address = address - 6) {
		readRMI_Recovery(ts->client, address, buffer, 6);

		if (!buffer[5])
			continue;
		switch (buffer[5]) {
		case 0x35:
			SynaF35QueryBase = buffer[0];
			SynaF35ControlBase = buffer[2];
			SynaF35DataBase = buffer[3];
			ts->ubootloader_mode = true;
			return;
		case 0x34:
			SynaF34DataBase = buffer[3];
			SynaF34QueryBase = buffer[0];
			break;
		case 0x01:
			SynaF01DataBase = buffer[3];
			SynaF01CommandBase = buffer[1];
			SynaF01QueryBase = buffer[0];//      
			break;
		}
	}

	SynaF34Reflash_BlockNum = SynaF34DataBase;
	SynaF34Reflash_BlockData = SynaF34DataBase + 1; //  
	SynaF34ReflashQuery_BootID = SynaF34QueryBase;
	SynaF34ReflashQuery_FlashPropertyQuery = SynaF34QueryBase + 1;//  
	SynaF34ReflashQuery_BlockSize = SynaF34QueryBase + 2;//  
	SynaF34ReflashQuery_FirmwareBlockCount = SynaF34QueryBase + 3;//  
	//                                                          
	SynaF34_FlashControl = SynaF34DataBase + 2;
	SynaF34_FlashStatus = SynaF34DataBase + 3;//      

	//                                                                                    
	readRMI_Recovery(ts->client, SynaF34ReflashQuery_FirmwareBlockCount, buffer, 6);
	SynaFirmwareBlockCount  = buffer[0] | (buffer[1] << 8);
	SynaConfigBlockCount    = buffer[2] | (buffer[3] << 8);
	SynaDisplayBlockCount   = buffer[4] | (buffer[5] << 8);

	TOUCH_DBG("%s : SynaFirmwareBlockCount = 0x%04x, SynaConfigBlockCount = 0x%04x, SynaDisplayBlockCount = 0x%04x\n",
			__func__,SynaFirmwareBlockCount, SynaConfigBlockCount,SynaDisplayBlockCount);

	readRMI_Recovery(ts->client, SynaF34ReflashQuery_BlockSize, &uData[0], 2);

	SynaConfigBlockSize = uData[0] | (uData[1] << 8);
	SynaFirmwareBlockSize = uData[0] | (uData[1] << 8);
	SynaDisplayBlockSize = uData[0] | (uData[1] << 8);
	//                                                       
	TOUCH_DBG( "%s : SynaDisplayBlockSize = 0x%04x\n",__func__, SynaDisplayBlockSize);


	//          
	readRMI_Recovery(ts->client, (SynaF01DataBase + 1), buffer, 1);
}

/*                                           
 */
void SynaInitialize(struct synaptics_ts_data *ts)
{
	u8 data;

	TOUCH_ERR("Initializing Reflash Process...\n");

	data = 0x00;
	writeRMI(ts->client, 0xff, &data, 1);

	SynaScanPDT(ts);	//            

	if (!ts->ubootloader_mode)
		SynaImageParser(ts); //      
}

/*                                                                           
                 
                                                       
 */
void SynaReadFirmwareInfo(struct synaptics_ts_data *ts)
{
	unsigned char uData[3] = {0};
	unsigned char product_id[11];
	int firmware_version;

	TOUCH_LOG("%s\n", __FUNCTION__);


	readRMI(ts->client, SynaF01QueryBase + 11, product_id, 10);
	product_id[10] = '\0';
	TOUCH_ERR("Read Product ID %s\n", product_id);

	readRMI(ts->client, SynaF01QueryBase + 18, uData, 3);
	firmware_version = uData[2] << 16 | uData[1] << 8 | uData[0];
	TOUCH_ERR("Read Firmware Info %d\n", firmware_version);

	CheckTouchControllerType(ts);
	/*                          */
}
//                             
/*                                                               
                                    
 */
void SynaReadBootloadID(struct synaptics_ts_data *ts)
{
	unsigned char uData[2] = {0};

	readRMI(ts->client, SynaF34ReflashQuery_BootID, &uData[0], 2);
	SynaBootloadID = uData[0] | (uData[1] << 8);
	TOUCH_ERR("SynaBootloadID = %d\n", SynaBootloadID);
}

/*                                                                      
                                
 */
void SynaWriteBootloadID(struct synaptics_ts_data *ts)
{
	unsigned char uData[2];

	uData[0] = SynaBootloadID % 0x100;
	uData[1] = SynaBootloadID / 0x100;

	TOUCH_ERR("uData[0] = %x uData[0] = %x\n", uData[0], uData[1]);
	writeRMI(ts->client, SynaF34Reflash_BlockData, &uData[0], 2);
}

/*                                                 
 */
void SynaEnableFlashing(struct synaptics_ts_data *ts)
{
	//            
	unsigned char uStatus = 0;
	enum FlashCommand cmd;
	unsigned char uData[3] = {0};
	int firmware_version;

	TOUCH_LOG("%s\n", __FUNCTION__);

	TOUCH_LOG("Enable Reflash...\n");
	readRMI (ts->client, SynaF01DataBase, &uStatus, 1);

	if ((uStatus & 0x40) == 0) {
		//                                                      
		//                                    
		SynaReadBootloadID(ts);
		SynaWriteBootloadID(ts);

		//                                            
		//                        
		//                                       
		cmd = m_uF34ReflashCmd_Enable;
		writeRMI(ts->client, SynaF34_FlashControl,
				(unsigned char *)&cmd, 1);
		SynaWaitForATTN(1000, ts);

		//                     
		//                      

		//                                                              
		SynaScanPDT(ts);

		readRMI(ts->client, SynaF01QueryBase + 18, uData, 3);
		firmware_version = uData[2] << 16 | uData[1] << 8 | uData[0];

		//                                                            
		//                        
		//            
		CheckFlashStatus(ts, cmd);
	}
}

/*                                                                    
             
 */
unsigned int SynaWaitForATTN(int timeout, struct synaptics_ts_data *ts)
{
	unsigned char uStatus;
	//                  
	//                             
	//              

	int trial_us = 0;
#ifdef POLLING
	do {
		uStatus = 0x00;
		readRMI(ts->client, (SynaF01DataBase + 1), &uStatus, 1);
		if (uStatus != 0)
			break;
		Sleep(duration);
		times++;
	} while (times < retry);

	if (times == retry)
		return -1;
#else
	/*                                                    
    
             
    */
	while ((mt_get_gpio_in(GPIO_TOUCH_INT)  != 0)
			&& (trial_us < (timeout * 1000))) {
		udelay(1);
		trial_us++;
	}
	if (mt_get_gpio_in(GPIO_TOUCH_INT)  != 0) {
		TOUCH_ERR("interrupt pin is busy...\n");
		return -1;
	}

	readRMI(ts->client, (SynaF01DataBase + 1), &uStatus, 1);
#endif
	return 0;
}
/*                                                  
 */
void SynaFinalizeReflash(struct synaptics_ts_data *ts)
{
	unsigned char uData;

	TOUCH_LOG("%s\n", __FUNCTION__);

	TOUCH_LOG("Finalizing Reflash...\n");

	//                                                                    
	//                                                              
	//                
	uData = 1;
	writeRMI(ts->client, SynaF01CommandBase, &uData, 1);

	//                                                              
	//                                            
	msleep(150);
	SynaWaitForATTN(1000, ts);

	SynaScanPDT(ts);

	readRMI(ts->client, SynaF01DataBase, &uData, 1);
}

/*                                                   
                              
 */
void SynaFlashFirmwareWrite(struct synaptics_ts_data *ts)
{
	//                                                                      
	unsigned char *puFirmwareData = SynafirmwareImgData;
	unsigned char uData[2];
	unsigned short blockNum;
	enum FlashCommand cmd;

	for (blockNum = 0; blockNum < SynaFirmwareBlockCount; ++blockNum) {
		if (blockNum == 0) {

			//                                               
			//                                       
			uData[0] = blockNum & 0xff;
			uData[1] = (blockNum & 0xff00) >> 8;
			writeRMI(ts->client, SynaF34Reflash_BlockNum,
					&uData[0], 2);
		}

		writeRMI(ts->client, SynaF34Reflash_BlockData, puFirmwareData,
				SynaFirmwareBlockSize);
		puFirmwareData += SynaFirmwareBlockSize;

		//                                         
		cmd = m_uF34ReflashCmd_FirmwareWrite;
		writeRMI(ts->client, SynaF34_FlashControl,
				(unsigned char *)&cmd, 1);

		//                         
		CheckFlashStatus(ts, cmd);
		//                                   
		//                                             
		//                                  
#ifdef SHOW_PROGRESS
		if (blockNum % 100 == 0)
			TOUCH_ERR("blk %d / %d\n",
					blockNum, SynaFirmwareBlockCount);
#endif
	}
#ifdef SHOW_PROGRESS
	TOUCH_ERR("blk %d / %d\n",
			SynaFirmwareBlockCount, SynaFirmwareBlockCount);
#endif
}

/*                                                   
                              
 */
void SynaFlashConfigWrite(struct synaptics_ts_data *ts)
{
	//                                                                    
	unsigned char *puConfigData = SynaconfigImgData;
	unsigned char uData[2];
	unsigned short blockNum;
	enum FlashCommand cmd;

	for (blockNum = 0; blockNum < SynaConfigBlockCount; ++blockNum)	{
		//                                               
		//                                       
		uData[0] = blockNum & 0xff;
		uData[1] = (blockNum & 0xff00) >> 8;
		writeRMI(ts->client, SynaF34Reflash_BlockNum, &uData[0], 2);

		writeRMI(ts->client, SynaF34Reflash_BlockData,
				puConfigData, SynaConfigBlockSize);
		puConfigData += SynaConfigBlockSize;

		//                                       
		cmd = m_uF34ReflashCmd_ConfigWrite;
		writeRMI(ts->client, SynaF34_FlashControl,
				(unsigned char *)&cmd, 1);

		SynaWaitForATTN(100, ts);
		CheckFlashStatus(ts, cmd);
#ifdef SHOW_PROGRESS
		if (blockNum % 100 == 0)
			TOUCH_ERR("blk %d / %d\n",
					blockNum, SynaConfigBlockCount);
#endif
	}
#ifdef SHOW_PROGRESS
	TOUCH_ERR("blk %d / %d\n",
			SynaConfigBlockCount, SynaConfigBlockCount);
#endif
}

/*
        
                                                         
                                                                         
*/
void SynaFlashDispConfigWrite(struct synaptics_ts_data *ts)
{
	unsigned char *dispConfigData = SynaDisplayConfigImgData;
	unsigned char uData[2];
	unsigned short blockNum;
	enum FlashCommand cmd;

	for (blockNum = 0; blockNum < SynaDisplayBlockCount; ++blockNum)	{
		//                                               
		//                                       
		uData[0] = blockNum & 0xff;
		uData[1] = (blockNum & 0xff00) >> 8;
		uData[1] |= 0x60; //                              
		writeRMI(ts->client, SynaF34Reflash_BlockNum, &uData[0], 2);

		writeRMI(ts->client, SynaF34Reflash_BlockData,
				dispConfigData, SynaDisplayBlockSize);
		dispConfigData += SynaDisplayBlockSize;

		//                                       
		cmd = m_uF34ReflashCmd_ConfigWrite;
		writeRMI(ts->client, SynaF34_FlashControl,
				(unsigned char *)&cmd, 1);

		SynaWaitForATTN(100, ts);
		CheckFlashStatus(ts, cmd);
#ifdef SHOW_PROGRESS
		if (blockNum % 100 == 0)
			TOUCH_ERR("blk %d / %d\n",
					blockNum, SynaDisplayBlockCount);
#endif
	}
#ifdef SHOW_PROGRESS
	TOUCH_ERR("blk %d / %d\n",
			SynaDisplayBlockCount, SynaDisplayBlockCount);
#endif
}

/*                                         
 */
void eraseAllBlock(struct synaptics_ts_data *ts)
{
	enum FlashCommand cmd;

	//                                                                     
	SynaReadBootloadID(ts);
	SynaWriteBootloadID(ts);

	//                                
	cmd = m_uF34ReflashCmd_EraseAll;
	writeRMI(ts->client, SynaF34_FlashControl, (unsigned char *)&cmd, 1);

	SynaWaitForATTN(6000, ts);
	CheckFlashStatus(ts, cmd);
}

/*                                                          
 */
void SynaProgramFirmware(struct synaptics_ts_data *ts)
{
	TOUCH_ERR("\nProgram Firmware Section...\n");

	eraseAllBlock(ts);

	SynaFlashFirmwareWrite(ts);

	SynaFlashConfigWrite(ts);

	SynaFlashDispConfigWrite(ts);	//      

}

/*                                                          
 */
void SynaUpdateConfig(struct synaptics_ts_data *ts)
{
	TOUCH_ERR("\nUpdate Config Section...\n");

	EraseConfigBlock(ts);

	SynaFlashConfigWrite(ts);

	SynaFlashDispConfigWrite(ts);
}



/*                                         
 */
void EraseConfigBlock(struct synaptics_ts_data *ts)
{
	enum FlashCommand cmd;

	//                                                                     
	SynaReadBootloadID(ts);
	SynaWriteBootloadID(ts);

	//                                
	cmd = m_uF34ReflashCmd_EraseUIConfig;
	writeRMI(ts->client, SynaF34_FlashControl, (unsigned char *)&cmd, 1);

	SynaWaitForATTN(2000, ts);
	CheckFlashStatus(ts, cmd);
}

void SynaCheckFlashStatus(struct synaptics_ts_data *ts)
{
	unsigned char status;

	readRMI(ts->client, SynaF35DataBase + F35_ERROR_CODE_OFFSET, &status, 1);

	status = status & 0x7f;

	if (status != 0x00)
		TOUCH_ERR("Recovery mode error code = 0x%02x\n", status);

	return;
}

void SynaEraseFlash(struct synaptics_ts_data *ts)
{
	enum F35RecoveryCommand command = CMD_F35_ERASE_ALL;

	writeRMI(ts->client, SynaF35ControlBase + F35_CHUNK_COMMAND_OFFSET,
			(unsigned char *)&command, 1);

	msleep(F35_ERASE_ALL_WAIT_MS);

	SynaCheckFlashStatus(ts);

	return;
}

void SynaWriteChunkData(struct synaptics_ts_data *ts)
{
	unsigned char chunk_number[] = {0, 0};
	unsigned char chunk_spare;
	unsigned char chunk_size;
	unsigned char buf[F35_CHUNK_SIZE + 1];
	unsigned short chunk;
	unsigned short chunk_total;
	unsigned char *chunk_ptr = (unsigned char *)ts->fw_info.fw_start;

	writeRMI(ts->client, SynaF35ControlBase + F35_CHUNK_NUM_LSB_OFFSET,
			chunk_number, sizeof(chunk_number));

	chunk_total = ts->fw_info.fw_size / F35_CHUNK_SIZE;
	chunk_spare = ts->fw_info.fw_size % F35_CHUNK_SIZE;
	if (chunk_spare)
		chunk_total++;

	buf[sizeof(buf) - 1] = CMD_F35_WRITE_CHUNK;

	for (chunk = 0; chunk < chunk_total; chunk++) {
		if (chunk_spare && chunk == (chunk_total - 1))
			chunk_size = chunk_spare;
		else
			chunk_size = F35_CHUNK_SIZE;

		memset(buf, 0x00, F35_CHUNK_SIZE);
		memcpy(buf, chunk_ptr, chunk_size);

		writeRMI(ts->client, SynaF35ControlBase + F35_CHUNK_DATA_OFFSET,
				buf, sizeof(buf));

		chunk_ptr += chunk_size;
#ifdef SHOW_PROGRESS
		if (chunk % 100 == 0)
			TOUCH_ERR("[Recovery] %d / %d\n", chunk, chunk_total);
#endif
	}
#ifdef SHOW_PROGRESS
	TOUCH_ERR("[Recovery] %d / %d\n", chunk, chunk_total);
#endif
	SynaCheckFlashStatus(ts);

	return;
}

void SynaFinalizeRecovery(struct synaptics_ts_data *ts)
{
	unsigned char uData;
	enum F35RecoveryCommand command = CMD_F35_RESET;

	writeRMI(ts->client, SynaF35ControlBase + F35_CHUNK_COMMAND_OFFSET,
			(unsigned char *)&command, 1);

	msleep(F35_RESET_WAIT_MS);

	SynaWaitForATTN(1000, ts);

	SynaScanPDT(ts);

	readRMI(ts->client, SynaF01DataBase, &uData, 1);
	TOUCH_ERR("[%s] FW Recovery Finished!!!!", __func__);
	return;
}

/*                                                 
                                                 
 */
void CompleteReflash(struct synaptics_ts_data *ts)
{
	bool bFlashAll = true;

	SynaInitialize(ts);

	SynaReadFirmwareInfo(ts);

	SynaEnableFlashing(ts);

	SynaBootloaderLock(ts);

	if (bFlashAll)
		SynaProgramFirmware(ts);
	else
		SynaUpdateConfig(ts);

	SynaFinalizeReflash(ts);
}

void FlashRecovery(struct synaptics_ts_data *ts)
{
	SynaInitialize(ts);

	SynaEraseFlash(ts);

	SynaWriteChunkData(ts);

	SynaFinalizeRecovery(ts);

	return;
}
