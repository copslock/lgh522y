/*
                    
  
  
 */

#if 0
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "kd_camera_hw.h"
#include "eeprom.h"
#include "eeprom_define.h"
#include "GT24c32a.h"
#include <asm/system.h>  //        
#else

#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "kd_camera_hw.h"
#include "cam_cal.h"
#include "cam_cal_define.h"

#include "GT24c32a.h"
#include <asm/system.h>  //        
#endif
//                          
#define EEPROM_DEBUG
#ifdef EEPROM_DEBUG
#define EEPROMDB printk
#else
#define EEPROMDB(x,...)
#endif


static DEFINE_SPINLOCK(g_EEPROMLock); //        
#define EEPROM_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_eeprom_dev={ I2C_BOARD_INFO("CAM_CAL_DRV", 0xAA>>1)};

/*                                                                              
 
                                                                               */
#define EEPROM_ICS_REVISION 1 //             
/*                                                                              
 
                                                                               */
#define EEPROM_DRVNAME "CAM_CAL_DRV"
#define EEPROM_I2C_GROUP_ID 0

/*                                                                              
 
                                                                               */
//                                 
#define FM50AF_EEPROM_I2C_ID 0xA1


/*                                                                              
                                                     */
/*                                                                              */
#define SampleNum 221
#define Read_NUMofEEPROM 2
#define Boundary_Address 256
#define EEPROM_Address_Offset 0xC


/*                                                                              
 
                                                                               */
static struct i2c_client * g_pstI2Cclient = NULL;

//                         
static dev_t g_EEPROMdevno = MKDEV(EEPROM_DEV_MAJOR_NUMBER,0);
static struct cdev * g_pEEPROM_CharDrv = NULL;
//                               
//                         
//                           

static struct class *EEPROM_class = NULL;
static atomic_t g_EEPROMatomic;
//                                          
//                              
//                                

static unsigned short EEPROM_Address[2] = {0xA0,0xA0} ; //                           

/*                                                                              
 
                                                                               */
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iBurstReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);

/*                                                                              
 
                                                                               */
//                                                                                   
int iWriteEEPROM(u16 a_u2Addr  , u32 a_u4Bytes, u8 * puDataInBytes)
{
    int  i4RetValue = 0;
    u32 u4Index = 0;
    char puSendCmd[8] = {(char)(a_u2Addr >> 8) , (char)(a_u2Addr & 0xFF) ,
        0, 0, 0, 0, 0, 0};
    if(a_u4Bytes + 2 > 8)
    {
        EEPROMDB("[GT24C32A_eeprom] exceed I2c-mt65xx.c 8 bytes limitation (include address 2 Byte)\n");
        return -1;
    }

    for(u4Index = 0 ; u4Index < a_u4Bytes ; u4Index += 1 )
    {
        puSendCmd[(u4Index + 2)] = puDataInBytes[u4Index];
    }
    
    i4RetValue = i2c_master_send(g_pstI2Cclient, puSendCmd, (a_u4Bytes + 2));
    if (i4RetValue != (a_u4Bytes + 2))
    {
        EEPROMDB("[GT24C32A_eeprom] I2C write  failed!! \n");
        return -1;
    }
    mdelay(10); //                                                     
    
   //                                            
    return 0;
}


//                                                                                   
int iReadEEPROM(u16 a_u2Addr, u32 ui4_length, u8 * a_puBuff)
{
    int  i4RetValue = 0;
    char puReadCmd[2] = {(char)(a_u2Addr >> 8) , (char)(a_u2Addr & 0xFF)};

    //                                      

    if(ui4_length > 8)
    {
        EEPROMDB("[GT24C32A_eeprom] exceed I2c-mt65xx.c 8 bytes limitation\n");
        return -1;
    }
    spin_lock(&g_EEPROMLock); //       
    g_pstI2Cclient->addr = g_pstI2Cclient->addr & (I2C_MASK_FLAG | I2C_WR_FLAG);	
    spin_unlock(&g_EEPROMLock); //        
    
    //                                        
    i4RetValue = i2c_master_send(g_pstI2Cclient, puReadCmd, 2);
    if (i4RetValue != 2)
    {
        EEPROMDB("[GT24C32A_eeprom] I2C send read address failed!! \n");
        return -1;
    }

    //                                        
    i4RetValue = i2c_master_recv(g_pstI2Cclient, (char *)a_puBuff, ui4_length);
    if (i4RetValue != ui4_length)
    {
        EEPROMDB("[GT24C32A_eeprom] I2C read data failed!! \n");
        return -1;
    }
    spin_lock(&g_EEPROMLock); //       
    g_pstI2Cclient->addr = g_pstI2Cclient->addr & I2C_MASK_FLAG;    
    spin_unlock(&g_EEPROMLock); //            

    //                                           
    return 0;
}


int iWriteData(unsigned int  ui4_offset, unsigned int  ui4_length, unsigned char * pinputdata)
{
   int  i4RetValue = 0;
   int  i4ResidueDataLength;
   u32 u4IncOffset = 0;
   u32 u4CurrentOffset;
   u8 * pBuff;

   EEPROMDB("[GT24C32A_eeprom] iWriteData\n" );

#if 0
   if (ui4_offset + ui4_length >= 0x2000)
   {
      EEPROMDB("[S24EEPROM] Write Error!! S-24CS64A not supprt address >= 0x2000!! \n" );
      return -1;
   }

   i4ResidueDataLength = (int)ui4_length;
   u4CurrentOffset = ui4_offset;
   pBuff = pinputdata;   

   EEPROMDB("[S24EEPROM] iWriteData u4CurrentOffset is %d \n",u4CurrentOffset);   

   do 
   {
       if(i4ResidueDataLength >= 6)
       {
           //                                                          
           if (i4RetValue != 0)
           {
                EEPROMDB("[EEPROM] I2C iWriteData failed!! \n");
                return -1;
           }           
           u4IncOffset += 6;
           i4ResidueDataLength -= 6;
           u4CurrentOffset = ui4_offset + u4IncOffset;
           pBuff = pinputdata + u4IncOffset;
       }
       else
       {
           //                                                                            
           if (i4RetValue != 0)
           {
                EEPROMDB("[EEPROM] I2C iWriteData failed!! \n");
                return -1;
           }            
           u4IncOffset += 6;
           i4ResidueDataLength -= 6;
           u4CurrentOffset = ui4_offset + u4IncOffset;
           pBuff = pinputdata + u4IncOffset;
           //      
       }
   }while (i4ResidueDataLength > 0);
#endif
   EEPROMDB("[GT24C32A_eeprom] iWriteData done\n" );
 
   return 0;
}



int iReadDataFromGT24c32a(unsigned int  ui4_offset, unsigned int  ui4_length, unsigned char * pinputdata)
{
        char puSendCmd[2];//                                
        unsigned short SampleOffset = (unsigned short)((ui4_offset) & (0x0000FFFF)) ;
        //                                                
        //                                                                                                                  
        short loop[2], loopCount;
        unsigned short SampleCount;
        u8 * pBuff ;
        u32 u4IncOffset = 0;
        int  i4RetValue = 0;

        pBuff = pinputdata;
        
        EEPROMDB("[GT24C32A_eeprom] ui4_offset=%x ui4_offset(80)=%x ui4_offset(8)=%x\n",ui4_offset , (unsigned short)( (ui4_offset>>8) & 0x0000FFFF),SampleOffset);
        
        //                                             
       
#if 0
        EEPROM_Address[0] = ( (0 < address_offset)? (EEPROM_Address[0] |(address_offset-1) ) : EEPROM_Address[0]);
        EEPROM_Address[1] = ( (EEPROM_Address[0] & 0xF0) |(address_offset) );

        EEPROM_Address[0] = EEPROM_Address[0] << 1;
        EEPROM_Address[1] = EEPROM_Address[1] << 1;
#endif
        
        EEPROMDB("[GT24C32A_eeprom] EEPROM_Address[0]=%x EEPROM_Address[1]=%x\n",(EEPROM_Address[0]>>1) ,(EEPROM_Address[1]>>1));

        //                                                                                                    
        loop[0] = ((ui4_length>>4)<<4);

        loop[1] = ui4_length - loop[0] ;

        
        EEPROMDB("[GT24C32A_eeprom] loop[0]=%d loop[1]=%d\n",(loop[0]) ,(loop[1]));

        puSendCmd[0] = (char)( ((SampleOffset+u4IncOffset)>>8) & 0xFF) ;
        puSendCmd[1] = (char)( (SampleOffset+u4IncOffset) & 0xFF) ;

        for(loopCount=0; loopCount < Read_NUMofEEPROM; loopCount++)
        {
               do 
               {
                   if( 16 <= loop[loopCount])
                   {
                       EEPROMDB("[GT24C32A_eeprom]1 loopCount=%d loop[loopCount]=%d puSendCmd[0]=%x puSendCmd[1]=%x, EEPROM(%x)\n",loopCount ,loop[loopCount],puSendCmd[0],puSendCmd[1],EEPROM_Address[loopCount] );
                       //                                                                    
                       i4RetValue=iBurstReadRegI2C(puSendCmd , 2, (u8*)pBuff,16,EEPROM_Address[loopCount]);
                       if (i4RetValue != 0)
                       {
                            EEPROMDB("[GT24C32A_eeprom] I2C iReadData failed!! \n");
                            return -1;
                       }           
                       u4IncOffset += 16;
                       loop[loopCount] -= 16;
                       //                                                         
                       puSendCmd[0] = (char)( ((SampleOffset+u4IncOffset)>>8) & 0xFF) ;
                       puSendCmd[1] = (char)( (SampleOffset+u4IncOffset) & 0xFF) ;
                       pBuff = pinputdata + u4IncOffset;
                   }
                   else if(0 < loop[loopCount])
                   {
                       EEPROMDB("[GT24C32A_eeprom]2 loopCount=%d loop[loopCount]=%d puSendCmd[0]=%x puSendCmd[1]=%x \n",loopCount ,loop[loopCount],puSendCmd[0],puSendCmd[1] );
                       //                                                                                 
                       i4RetValue=iBurstReadRegI2C(puSendCmd , 2, (u8*)pBuff,16,EEPROM_Address[loopCount]);
                       if (i4RetValue != 0)
                       {
                            EEPROMDB("[GT24C32A_eeprom] I2C iReadData failed!! \n");
                            return -1;
                       }  
                       u4IncOffset += loop[loopCount];
                       loop[loopCount] -= loop[loopCount];
                       //                                                         
                       puSendCmd[0] = (char)( ((SampleOffset+u4IncOffset)>>8) & 0xFF) ;
                       puSendCmd[1] = (char)( (SampleOffset+u4IncOffset) & 0xFF) ;
                       pBuff = pinputdata + u4IncOffset;
                   }
               }while (loop[loopCount] > 0);
        }

   return 0;
}

int iReadData(unsigned int  ui4_offset, unsigned int  ui4_length, unsigned char * pinputdata)
{
   int  i4RetValue = 0;
   int  i4ResidueDataLength;
   u32 u4IncOffset = 0;
   u32 u4CurrentOffset;
   u8 * pBuff;
//                                         
   
   if (ui4_offset + ui4_length >= 0x2000)
   {
      EEPROMDB("[GT24C32A_eeprom] Read Error!! S-24CS64A not supprt address >= 0x2000!! \n" );
      return -1;
   }

   i4ResidueDataLength = (int)ui4_length;
   u4CurrentOffset = ui4_offset;
   pBuff = pinputdata;
   do 
   {
       if(i4ResidueDataLength >= 8)
       {
           i4RetValue = iReadEEPROM((u16)u4CurrentOffset, 8, pBuff);
           if (i4RetValue != 0)
           {
                EEPROMDB("[GT24C32A_eeprom] I2C iReadData failed!! \n");
                return -1;
           }           
           u4IncOffset += 8;
           i4ResidueDataLength -= 8;
           u4CurrentOffset = ui4_offset + u4IncOffset;
           pBuff = pinputdata + u4IncOffset;
       }
       else
       {
           i4RetValue = iReadEEPROM((u16)u4CurrentOffset, i4ResidueDataLength, pBuff);
           if (i4RetValue != 0)
           {
                EEPROMDB("[GT24C32A_eeprom] I2C iReadData failed!! \n");
                return -1;
           }  
           u4IncOffset += 8;
           i4ResidueDataLength -= 8;
           u4CurrentOffset = ui4_offset + u4IncOffset;
           pBuff = pinputdata + u4IncOffset;
           //      
       }
   }while (i4ResidueDataLength > 0);
//                                                                                                                                               
//                                             
   return 0;
}


/*                                                                              
 
                                                                               */
#define NEW_UNLOCK_IOCTL
#ifndef NEW_UNLOCK_IOCTL
static int EEPROM_Ioctl(struct inode * a_pstInode,
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
#else 
static long EEPROM_Ioctl(
    struct file *file, 
    unsigned int a_u4Command, 
    unsigned long a_u4Param
)
#endif
{
    int i4RetValue = 0;
    u8 * pBuff = NULL;
    u8 * pWorkingBuff = NULL;
    stCAM_CAL_INFO_STRUCT *ptempbuf;
    ssize_t writeSize;
    u8 readTryagain=0;

#ifdef EEPROMGETDLT_DEBUG
    struct timeval ktv1, ktv2;
    unsigned long TimeIntervalUS;
#endif

    if(_IOC_NONE == _IOC_DIR(a_u4Command))
    {
    }
    else
    {
        pBuff = (u8 *)kmalloc(sizeof(stCAM_CAL_INFO_STRUCT),GFP_KERNEL);

        if(NULL == pBuff)
        {
            EEPROMDB("[GT24C32A_eeprom] ioctl allocate mem failed\n");
            return -ENOMEM;
        }

        if(_IOC_WRITE & _IOC_DIR(a_u4Command))
        {
            if(copy_from_user((u8 *) pBuff , (u8 *) a_u4Param, sizeof(stCAM_CAL_INFO_STRUCT)))
            {    //                           
                kfree(pBuff);
                EEPROMDB("[GT24C32A_eeprom] ioctl copy from user failed\n");
                return -EFAULT;
            }
        }
    }

    ptempbuf = (stCAM_CAL_INFO_STRUCT *)pBuff;
    pWorkingBuff = (u8*)kmalloc(ptempbuf->u4Length,GFP_KERNEL); 
    if(NULL == pWorkingBuff)
    {
        kfree(pBuff);
        EEPROMDB("[GT24C32A_eeprom] ioctl allocate mem failed\n");
        return -ENOMEM;
    }
     EEPROMDB("[GT24C32A_eeprom] init Working buffer address 0x%8x  command is 0x%8x\n", (u32)pWorkingBuff, (u32)a_u4Command);

 
    if(copy_from_user((u8*)pWorkingBuff ,  (u8*)ptempbuf->pu1Params, ptempbuf->u4Length))
    {
        kfree(pBuff);
        kfree(pWorkingBuff);
        EEPROMDB("[GT24C32A_eeprom] ioctl copy from user failed\n");
        return -EFAULT;
    } 
    
    switch(a_u4Command)
    {
        case CAM_CALIOC_S_WRITE:    
            EEPROMDB("[GT24C32A_eeprom] Write CMD \n");
#ifdef EEPROMGETDLT_DEBUG
            do_gettimeofday(&ktv1);
#endif            
            i4RetValue = iWriteData((u16)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
#ifdef EEPROMGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Write data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif            
            break;
        case CAM_CALIOC_G_READ:
            EEPROMDB("[GT24C32A_eeprom] Read CMD \n");
#ifdef EEPROMGETDLT_DEBUG            
            do_gettimeofday(&ktv1);
#endif 
            EEPROMDB("[GT24C32A_eeprom] offset %x \n", ptempbuf->u4Offset);
            EEPROMDB("[GT24C32A_eeprom] length %x \n", ptempbuf->u4Length);
            EEPROMDB("[GT24C32A_eeprom] Before read Working buffer address 0x%8x \n", (u32)pWorkingBuff);
			
            if(ptempbuf->u4Offset == 0x0024C32a)
            {
                *(u32 *)pWorkingBuff = 0x0124C32a;
            }
            else if(ptempbuf->u4Offset == 0x00a0C32a)
            {
                *(u32 *)pWorkingBuff = 0x0124C32a;
                EEPROM_Address[0] = (ptempbuf->u4Offset & 0xffff0000) >> 16;
                EEPROM_Address[1] = (ptempbuf->u4Offset & 0xffff0000) >> 16;				
            }
            else if(ptempbuf->u4Offset == 0x00a6C32a)
            {
                *(u32 *)pWorkingBuff = 0x0124C32a;
                EEPROM_Address[0] = (ptempbuf->u4Offset & 0xffff0000) >> 16;
                EEPROM_Address[1] = (ptempbuf->u4Offset & 0xffff0000) >> 16;				
            }			
            else
            {
               readTryagain=3;
               while(0 < readTryagain)
               {               	 
                   i4RetValue =  iReadDataFromGT24c32a((u16)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
                   EEPROMDB("[GT24C32A_eeprom] error (%d) Read retry (%d) \n", i4RetValue,readTryagain);
                   if(i4RetValue != 0)
                   {
                       readTryagain--;
                   }
                   else
                   {
                       readTryagain = 0;
                   }
               }
            }            
            EEPROMDB("[GT24C32A_eeprom] After read Working buffer data  0x%4x \n", *pWorkingBuff);

#ifdef EEPROMGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Read data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif            
            break;
        default :
      	     EEPROMDB("[GT24C32A_eeprom] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    if(_IOC_READ & _IOC_DIR(a_u4Command))
    {
        //                                                                    
        EEPROMDB("[GT24C32A_eeprom] to user length %d \n", ptempbuf->u4Length);
        EEPROMDB("[GT24C32A_eeprom] to user  Working buffer address 0x%8x \n", (u32)pWorkingBuff);
        if(copy_to_user((u8 __user *) ptempbuf->pu1Params , (u8 *)pWorkingBuff , ptempbuf->u4Length))
        {
            kfree(pBuff);
            kfree(pWorkingBuff);
            EEPROMDB("[GT24C32A_eeprom] ioctl copy to user failed\n");
            return -EFAULT;
        }
    }

    kfree(pBuff);
    kfree(pWorkingBuff);
    return i4RetValue;
}


static u32 g_u4Opened = 0;
//       
//          
//                                                       
//                                                            
static int EEPROM_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    EEPROMDB("[GT24C32A_eeprom] EEPROM_Open\n");
    spin_lock(&g_EEPROMLock);
    if(g_u4Opened)
    {
        spin_unlock(&g_EEPROMLock);
        return -EBUSY;
    }
    else
    {
        g_u4Opened = 1;
        atomic_set(&g_EEPROMatomic,0);
    }
    spin_unlock(&g_EEPROMLock);

    return 0;
}

//          
//                                                             
//                                      
//                                 
//                                 
static int EEPROM_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    spin_lock(&g_EEPROMLock);

    g_u4Opened = 0;

    atomic_set(&g_EEPROMatomic,0);

    spin_unlock(&g_EEPROMLock);

    return 0;
}

static const struct file_operations g_stEEPROM_fops =
{
    .owner = THIS_MODULE,
    .open = EEPROM_Open,
    .release = EEPROM_Release,
    //                     
    .unlocked_ioctl = EEPROM_Ioctl
};

#define EEPROM_DYNAMIC_ALLOCATE_DEVNO 1
inline static int RegisterEEPROMCharDrv(void)
{
    struct device* EEPROM_device = NULL;

#if EEPROM_DYNAMIC_ALLOCATE_DEVNO
    if( alloc_chrdev_region(&g_EEPROMdevno, 0, 1,EEPROM_DRVNAME) )
    {
        EEPROMDB("[GT24C32A_eeprom] Allocate device no failed\n");

        return -EAGAIN;
    }
#else
    if( register_chrdev_region(  g_EEPROMdevno , 1 , EEPROM_DRVNAME) )
    {
        EEPROMDB("[GT24C32A_eeprom] Register device no failed\n");

        return -EAGAIN;
    }
#endif

    //               
    g_pEEPROM_CharDrv = cdev_alloc();

    if(NULL == g_pEEPROM_CharDrv)
    {
        unregister_chrdev_region(g_EEPROMdevno, 1);

        EEPROMDB("[GT24C32A_eeprom] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //                       
    cdev_init(g_pEEPROM_CharDrv, &g_stEEPROM_fops);

    g_pEEPROM_CharDrv->owner = THIS_MODULE;

    //             
    if(cdev_add(g_pEEPROM_CharDrv, g_EEPROMdevno, 1))
    {
        EEPROMDB("[GT24C32A_eeprom] Attatch file operation failed\n");

        unregister_chrdev_region(g_EEPROMdevno, 1);

        return -EAGAIN;
    }

    EEPROM_class = class_create(THIS_MODULE, "EEPROMdrv");
    if (IS_ERR(EEPROM_class)) {
        int ret = PTR_ERR(EEPROM_class);
        EEPROMDB("[GT24C32A_eeprom] Unable to create class, err = %d\n", ret);
        return ret;
    }
    EEPROM_device = device_create(EEPROM_class, NULL, g_EEPROMdevno, NULL, EEPROM_DRVNAME);

    return 0;
}

inline static void UnregisterEEPROMCharDrv(void)
{
    //                   
    cdev_del(g_pEEPROM_CharDrv);

    unregister_chrdev_region(g_EEPROMdevno, 1);

    device_destroy(EEPROM_class, g_EEPROMdevno);
    class_destroy(EEPROM_class);
}


//                                                                    
#ifndef EEPROM_ICS_REVISION
static int EEPROM_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
#elif 0
static int EEPROM_i2c_detect(struct i2c_client *client, struct i2c_board_info *info);
#else
#endif
static int EEPROM_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int EEPROM_i2c_remove(struct i2c_client *);

static const struct i2c_device_id EEPROM_i2c_id[] = {{EEPROM_DRVNAME,0},{}};   
#if 0 //                                                     
static unsigned short force[] = {EEPROM_I2C_GROUP_ID, S24CS64A_DEVICE_ID, I2C_CLIENT_END, I2C_CLIENT_END};   
#else
//                                                                                                                 
#endif
//                                                                             
//                                                                        


static struct i2c_driver EEPROM_i2c_driver = {
    .probe = EEPROM_i2c_probe,                                   
    .remove = EEPROM_i2c_remove,                           
//                                                          
    .driver.name = EEPROM_DRVNAME,
    .id_table = EEPROM_i2c_id,                             
};

#ifndef EEPROM_ICS_REVISION
static int EEPROM_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, EEPROM_DRVNAME);
    return 0;
}
#endif
static int EEPROM_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id) {             
int i4RetValue = 0;
    EEPROMDB("[GT24C32A_eeprom] Attach I2C \n");
//                                  

    //                     
    spin_lock(&g_EEPROMLock); //       
    g_pstI2Cclient = client;
    g_pstI2Cclient->addr = S24CS64A_DEVICE_ID>>1;
    spin_unlock(&g_EEPROMLock); //            
    
    EEPROMDB("[GT24C32A_eeprom] g_pstI2Cclient->addr = 0x%8x \n",g_pstI2Cclient->addr);
    //                    
    i4RetValue = RegisterEEPROMCharDrv();

    if(i4RetValue){
        EEPROMDB("[GT24C32A_eeprom] register char device failed!\n");
        return i4RetValue;
    }


    EEPROMDB("[GT24C32A_eeprom] Attached!! \n");
    return 0;                                                                                       
} 

static int EEPROM_i2c_remove(struct i2c_client *client)
{
    return 0;
}

static int EEPROM_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&EEPROM_i2c_driver);
}

static int EEPROM_remove(struct platform_device *pdev)
{
    i2c_del_driver(&EEPROM_i2c_driver);
    return 0;
}

//                   
static struct platform_driver g_stEEPROM_Driver = {
    .probe		= EEPROM_probe,
    .remove	= EEPROM_remove,
    .driver		= {
        .name	= EEPROM_DRVNAME,
        .owner	= THIS_MODULE,
    }
};


static struct platform_device g_stEEPROM_Device = {
    .name = EEPROM_DRVNAME,
    .id = 0,
    .dev = {
    }
};

static int __init EEPROM_i2C_init(void)
{
    i2c_register_board_info(EEPROM_I2C_BUSNUM, &kd_eeprom_dev, 1);
    if(platform_driver_register(&g_stEEPROM_Driver)){
        EEPROMDB("failed to register GT24C32A_eeprom driver\n");
        return -ENODEV;
    }

    if (platform_device_register(&g_stEEPROM_Device))
    {
        EEPROMDB("failed to register GT24C32A_eeprom driver, 2nd time\n");
        return -ENODEV;
    }	

    return 0;
}

static void __exit EEPROM_i2C_exit(void)
{
	platform_driver_unregister(&g_stEEPROM_Driver);
}

module_init(EEPROM_i2C_init);
module_exit(EEPROM_i2C_exit);

MODULE_DESCRIPTION("EEPROM driver");
MODULE_AUTHOR("Sean Lin <Sean.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");


