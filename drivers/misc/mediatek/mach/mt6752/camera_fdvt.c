#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/page.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/wait.h>
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <linux/xlog.h> //                   
#include <mach/sync_write.h>

#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

//                            
#include <mach/mt_typedefs.h>
#include <mach/camera_fdvt.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_clkmgr.h>
//                                  

//                            

#include <asm/cacheflush.h>
//                    

#ifdef CONFIG_COMPAT
//       
#include <linux/fs.h>
#include <linux/compat.h>
#endif


#define FDVT_DEVNAME     "camera-fdvt"

/*
                        
                       
                                                                 
     
                      
      

                       
                      
                                                                                       
     
                           
      
*/

#define LOG_VRB(format, args...)    xlog_printk(ANDROID_LOG_VERBOSE, "FDVT", "[%s] " format, __FUNCTION__, ##args)
#define LOG_DBG(format, args...)    xlog_printk(ANDROID_LOG_DEBUG  , "FDVT", "[%s] " format, __FUNCTION__, ##args)
#define LOG_INF(format, args...)    xlog_printk(ANDROID_LOG_INFO   , "FDVT", "[%s] " format, __FUNCTION__, ##args)
#define LOG_WRN(format, args...)    xlog_printk(ANDROID_LOG_WARN   , "FDVT", "[%s] WARNING: " format, __FUNCTION__, ##args)
#define LOG_ERR(format, args...)    xlog_printk(ANDROID_LOG_ERROR  , "FDVT", "[%s, line%04d] ERROR: " format, __FUNCTION__, __LINE__, ##args)
#define LOG_AST(format, args...)    xlog_printk(ANDROID_LOG_ASSERT , "FDVT", "[%s, line%04d] ASSERT: " format, __FUNCTION__, __LINE__, ##args)


static dev_t FDVT_devno;
static struct cdev * FDVT_cdev = NULL;
static struct class * FDVT_class = NULL;
static wait_queue_head_t g_MT6573FDVTWQ;
static u32 g_u4MT6573FDVTIRQ = 0 ,g_u4MT6573FDVTIRQMSK = 0x00000001;

static u8 *pBuff = NULL;
static u8 *pread_buf = NULL;
static u32 buf_size = 1024;

#define MT6573FDVT_DBUFFREGCNT 208

//                                                            
//                                                                              
//                                                                                 

#define FDVT_WR32(data, addr)    mt_reg_sync_writel(data, addr)

typedef struct 
{
    UINT32 u4Addr[MT6573FDVT_DBUFFREGCNT];
    UINT32 u4Data[MT6573FDVT_DBUFFREGCNT];
    UINT32 u4Counter;
} MT6573FDVTDBuffRegMap;

#ifdef CONFIG_OF

typedef enum
{
    FDVT_IRQ_IDX = 0,
    FDVT_IRQ_IDX_NUM
}FDVT_IRQ_ENUM;

typedef enum
{
    FDVT_BASE_ADDR = 0,
    FDVT_BASEADDR_NUM
}FDVT_BASEADDR_ENUM;

static unsigned long gFDVT_Irq[FDVT_IRQ_IDX_NUM];
static unsigned long gFDVT_Reg[FDVT_BASEADDR_NUM];

//                                     
//                                           
//                                               


#define FDVT_ADDR                        (gFDVT_Reg[FDVT_BASE_ADDR])

#else
#define FDVT_ADDR                        FDVT_BASE
#endif

static MT6573FDVTDBuffRegMap pMT6573FDVTRDBuff;
static MT6573FDVTDBuffRegMap pMT6573FDVTWRBuff;

//            
#define FDVT_START                 (FDVT_ADDR+0x0)
#define FDVT_ENABLE                (FDVT_ADDR+0x4)
#define FDVT_RS                    (FDVT_ADDR+0x8)
#define FDVT_RSCON_ADR             (FDVT_ADDR+0xC)
#define FDVT_RGB2Y0                (FDVT_ADDR+0x10)
#define FDVT_RGB2Y1                (FDVT_ADDR+0x14)
#define FDVT_INVG0                 (FDVT_ADDR+0x18)
#define FDVT_INVG1                 (FDVT_ADDR+0x1C)
#define FDVT_INVG2                 (FDVT_ADDR+0x20)
#define FD_FNUM_0                  (FDVT_ADDR+0x24)
#define FD_FNUM_1                  (FDVT_ADDR+0x28)
#define FD_FNUM_2                  (FDVT_ADDR+0x2C)
#define FD_FNUM_3                  (FDVT_ADDR+0x30)
#define VT_FNUM_0                  (FDVT_ADDR+0x34)
#define VT_FNUM_1                  (FDVT_ADDR+0x38)
#define VT_FNUM_2                  (FDVT_ADDR+0x3C)
#define VT_FNUM_3                  (FDVT_ADDR+0x40)
#define FE_FNUM_0                  (FDVT_ADDR+0x44)
#define FE_FNUM_1                  (FDVT_ADDR+0x48)
#define FE_FNUM_2                  (FDVT_ADDR+0x4C)
#define FE_FNUM_3                  (FDVT_ADDR+0x50)
#define LD_BADR_0                  (FDVT_ADDR+0x54)
#define LD_BADR_1                  (FDVT_ADDR+0x58)
#define LD_BADR_2                  (FDVT_ADDR+0x5C)
#define LD_BADR_3                  (FDVT_ADDR+0x60)
#define LD_BADR_4                  (FDVT_ADDR+0x64)
#define LD_BADR_5                  (FDVT_ADDR+0x68)
#define LD_BADR_6                  (FDVT_ADDR+0x6C)
#define LD_BADR_7                  (FDVT_ADDR+0x70)
#define FDVT_RMAP_0                (FDVT_ADDR+0x74)
#define FDVT_RMAP_1                (FDVT_ADDR+0x78)
#define FDVT_FD_SET                (FDVT_ADDR+0x7C)
#define FDVT_FD_CON_BASE_ADR       (FDVT_ADDR+0x80)
#define FDVT_GFD_SKIP              (FDVT_ADDR+0x84)
#define FDVT_LFD                   (FDVT_ADDR+0x88)
#define FDVT_GFD_POSITION_0        (FDVT_ADDR+0x8C)
#define FDVT_GFD_POSITION_1        (FDVT_ADDR+0x90)
#define FDVT_GFD_DET0              (FDVT_ADDR+0x94)
#define FDVT_GFD_DET1              (FDVT_ADDR+0x98)
#define FDVT_FD_RLT_BASE_ADR       (FDVT_ADDR+0x9C)

#define LFD_CTRL_0                 (FDVT_ADDR+0xA4)
#define LFD_TL_0                   (FDVT_ADDR+0xA8)
#define LFD_BR_0                   (FDVT_ADDR+0xAC)
#define LFD_CTRL_1                 (FDVT_ADDR+0xB0)
#define LFD_TL_1                   (FDVT_ADDR+0xB4)
#define LFD_BR_1                   (FDVT_ADDR+0xB8)
#define LFD_CTRL_2                 (FDVT_ADDR+0xBC)
#define LFD_TL_2                   (FDVT_ADDR+0xC0)
#define LFD_BR_2                   (FDVT_ADDR+0xC4)
#define LFD_CTRL_3                 (FDVT_ADDR+0xC8)
#define LFD_TL_3                   (FDVT_ADDR+0xCC)
#define LFD_BR_3                   (FDVT_ADDR+0xD0)
#define LFD_CTRL_4                 (FDVT_ADDR+0xD4)
#define LFD_TL_4                   (FDVT_ADDR+0xD8)
#define LFD_BR_4                   (FDVT_ADDR+0xDC)
#define LFD_CTRL_5                 (FDVT_ADDR+0xE0)
#define LFD_TL_5                   (FDVT_ADDR+0xE4)
#define LFD_BR_5                   (FDVT_ADDR+0xE8)
#define LFD_CTRL_6                 (FDVT_ADDR+0xEC)
#define LFD_TL_6                   (FDVT_ADDR+0xF0)
#define LFD_BR_6                   (FDVT_ADDR+0xF4)
#define LFD_CTRL_7                 (FDVT_ADDR+0xF8)
#define LFD_TL_7                   (FDVT_ADDR+0xFC)
#define LFD_BR_7                   (FDVT_ADDR+0x100)
#define LFD_CTRL_8                 (FDVT_ADDR+0x104)
#define LFD_TL_8                   (FDVT_ADDR+0x108)
#define LFD_BR_8                   (FDVT_ADDR+0x10C)
#define LFD_CTRL_9                 (FDVT_ADDR+0x110)
#define LFD_TL_9                   (FDVT_ADDR+0x114)
#define LFD_BR_9                   (FDVT_ADDR+0x118)
#define LFD_CTRL_10                (FDVT_ADDR+0x11C)
#define LFD_TL_10                  (FDVT_ADDR+0x120)
#define LFD_BR_10                  (FDVT_ADDR+0x124)
#define LFD_CTRL_11                (FDVT_ADDR+0x128)
#define LFD_TL_11                  (FDVT_ADDR+0x12C)
#define LFD_BR_11                  (FDVT_ADDR+0x130)
#define LFD_CTRL_12                (FDVT_ADDR+0x134)
#define LFD_TL_12                  (FDVT_ADDR+0x138)
#define LFD_BR_12                  (FDVT_ADDR+0x13C)
#define LFD_CTRL_13                (FDVT_ADDR+0x140)
#define LFD_TL_13                  (FDVT_ADDR+0x144)
#define LFD_BR_13                  (FDVT_ADDR+0x148)
#define LFD_CTRL_14                (FDVT_ADDR+0x14C)
#define LFD_TL_14                  (FDVT_ADDR+0x150)
#define LFD_BR_14                  (FDVT_ADDR+0x154)
#define FDVT_RESULT                (FDVT_ADDR+0x158)
#define FDVT_INT_EN                (FDVT_ADDR+0x15C)
#define FDVT_SRC_WD_HT             (FDVT_ADDR+0x160)
//                                                    
#define FDVT_INT                   (FDVT_ADDR+0x168)
#define DEBUG_INFO_1               (FDVT_ADDR+0x16C)
#define DEBUG_INFO_2               (FDVT_ADDR+0x170)
#define DEBUG_INFO_3               (FDVT_ADDR+0x174)
#define FDVT_RESULTNUM             (FDVT_ADDR+0x178)
#define FDVT_MAX_OFFSET            0x178

#ifdef CONFIG_OF
struct fdvt_device{
    void __iomem *regs[FDVT_BASEADDR_NUM];
    struct device *dev;
    int irq[FDVT_IRQ_IDX_NUM];
};

static struct fdvt_device *fdvt_devs;
static int nr_fdvt_devs = 0;
#endif

void FDVT_basic_config(void)
{
    FDVT_WR32(0x00000111,FDVT_ENABLE);
    FDVT_WR32(0x0000040B,FDVT_RS               );
    FDVT_WR32(0x00000000,FDVT_RSCON_ADR        );
    FDVT_WR32(0x02590132,FDVT_RGB2Y0           );
    FDVT_WR32(0x00000075,FDVT_RGB2Y1           );
    FDVT_WR32(0x66553520,FDVT_INVG0            );
    FDVT_WR32(0xB8A28D79,FDVT_INVG1            );
    FDVT_WR32(0xFFF4E7CF,FDVT_INVG2            );
    FDVT_WR32(0x01D10203,FD_FNUM_0             );
    FDVT_WR32(0x02A10213,FD_FNUM_1             );
    FDVT_WR32(0x03F102B1,FD_FNUM_2             );
    FDVT_WR32(0x046103C1,FD_FNUM_3             );
    FDVT_WR32(0x01450168,VT_FNUM_0             );
    FDVT_WR32(0x01D70173,VT_FNUM_1             );
    FDVT_WR32(0x02C201E2,VT_FNUM_2             );
    FDVT_WR32(0x031002A0,VT_FNUM_3             );
    FDVT_WR32(0x00600060,FE_FNUM_0             );
    FDVT_WR32(0x00000000,FE_FNUM_1             );
    FDVT_WR32(0x00000000,FE_FNUM_2             );
    FDVT_WR32(0x00000000,FE_FNUM_3             );
    FDVT_WR32(0x00000000,LD_BADR_0             );
    FDVT_WR32(0x00000000,LD_BADR_1             );
    FDVT_WR32(0x00000000,LD_BADR_2             );
    FDVT_WR32(0x00000000,LD_BADR_3             );
    FDVT_WR32(0x00000000,LD_BADR_4             );
    FDVT_WR32(0x00000000,LD_BADR_5             );
    FDVT_WR32(0x00000000,LD_BADR_6             );
    FDVT_WR32(0x00000000,LD_BADR_7             );
    FDVT_WR32(0x3F000000,FDVT_RMAP_0           );
    FDVT_WR32(0x01230101,FDVT_RMAP_1           );
    FDVT_WR32(0x000F010B,FDVT_FD_SET           );
    FDVT_WR32(0x00000000,FDVT_FD_CON_BASE_ADR  );
    FDVT_WR32(0x00000011,FDVT_GFD_SKIP         );
    FDVT_WR32(0x01130000,FDVT_LFD              );
    FDVT_WR32(0x00000000,FDVT_GFD_POSITION_0   );
    FDVT_WR32(0x00F00140,FDVT_GFD_POSITION_1   );
    FDVT_WR32(0x00000001,FDVT_GFD_DET0         );
    FDVT_WR32(0x00000000,FDVT_GFD_DET1         );
    FDVT_WR32(0x00000000,FDVT_FD_RLT_BASE_ADR  );
    FDVT_WR32(0x00000000,LFD_CTRL_0            );
    FDVT_WR32(0x00000000,LFD_TL_0              );
    FDVT_WR32(0x00000000,LFD_BR_0              );
    FDVT_WR32(0x00000000,LFD_CTRL_1            );
    FDVT_WR32(0x00000000,LFD_TL_1              );
    FDVT_WR32(0x00000000,LFD_BR_1              );
    FDVT_WR32(0x00000000,LFD_CTRL_2            );
    FDVT_WR32(0x00000000,LFD_TL_2              );
    FDVT_WR32(0x00000000,LFD_BR_2              );
    FDVT_WR32(0x00000000,LFD_CTRL_3            );
    FDVT_WR32(0x00000000,LFD_TL_3              );
    FDVT_WR32(0x00000000,LFD_BR_3              );
    FDVT_WR32(0x00000000,LFD_CTRL_4            );
    FDVT_WR32(0x00000000,LFD_TL_4              );
    FDVT_WR32(0x00000000,LFD_BR_4              );
    FDVT_WR32(0x00000000,LFD_CTRL_5            );
    FDVT_WR32(0x00000000,LFD_TL_5              );
    FDVT_WR32(0x00000000,LFD_BR_5              );
    FDVT_WR32(0x00000000,LFD_CTRL_6            );
    FDVT_WR32(0x00000000,LFD_TL_6              );
    FDVT_WR32(0x00000000,LFD_BR_6              );
    FDVT_WR32(0x00000000,LFD_CTRL_7            );
    FDVT_WR32(0x00000000,LFD_TL_7              );
    FDVT_WR32(0x00000000,LFD_BR_7              );
    FDVT_WR32(0x00000000,LFD_CTRL_8            );
    FDVT_WR32(0x00000000,LFD_TL_8              );
    FDVT_WR32(0x00000000,LFD_BR_8              );
    FDVT_WR32(0x00000000,LFD_CTRL_9            );
    FDVT_WR32(0x00000000,LFD_TL_9              );
    FDVT_WR32(0x00000000,LFD_BR_9              );
    FDVT_WR32(0x00000000,LFD_CTRL_10           );
    FDVT_WR32(0x00000000,LFD_TL_10             );
    FDVT_WR32(0x00000000,LFD_BR_10             );
    FDVT_WR32(0x00000000,LFD_CTRL_11           );
    FDVT_WR32(0x00000000,LFD_TL_11             );
    FDVT_WR32(0x00000000,LFD_BR_11             );
    FDVT_WR32(0x00000000,LFD_CTRL_12           );
    FDVT_WR32(0x00000000,LFD_TL_12             );
    FDVT_WR32(0x00000000,LFD_BR_12             );
    FDVT_WR32(0x00000000,LFD_CTRL_13           );
    FDVT_WR32(0x00000000,LFD_TL_13             );
    FDVT_WR32(0x00000000,LFD_BR_13             );
    FDVT_WR32(0x00000000,LFD_CTRL_14           );
    FDVT_WR32(0x00000000,LFD_TL_14             );
    FDVT_WR32(0x00000000,LFD_BR_14             );
    FDVT_WR32(0x00000000,FDVT_INT_EN           );
    FDVT_WR32(0x014000F0,FDVT_SRC_WD_HT        );
}

/*                                                                              
              
                                                                               */
static unsigned long ms_to_jiffies(unsigned long ms)
{
    return ((ms * HZ + 512) >> 10);
}
/*                                                                       */
//                                    
/*                                                                       */
static int mt_fdvt_clk_ctrl(int en)
{
    //
    if (en) {
        enable_clock(MT_CG_DISP0_SMI_COMMON, "CAMERA");
        enable_clock(MT_CG_IMAGE_FD, "FDVT");
        //                             
    }
    else {
        disable_clock(MT_CG_IMAGE_FD, "FDVT");
        disable_clock(MT_CG_DISP0_SMI_COMMON, "CAMERA");
        //                             
    } 
    return 0;
}

/*                                                                       */
//                                
/*                                                                       */

void FaceDetecteConfig(void)
{
    FDVT_WR32(0x01D10203,FD_FNUM_0             );
    FDVT_WR32(0x02A10213,FD_FNUM_1             );
    FDVT_WR32(0x03F102B1,FD_FNUM_2             );
    FDVT_WR32(0x046103C1,FD_FNUM_3             );
    FDVT_WR32(0x01450168,VT_FNUM_0             );
    FDVT_WR32(0x01D70173,VT_FNUM_1             );
    FDVT_WR32(0x02C201E2,VT_FNUM_2             );
    FDVT_WR32(0x031002A0,VT_FNUM_3             );
    FDVT_WR32(0x00600060,FE_FNUM_0             );
    FDVT_WR32(0x00000000,FE_FNUM_1             );
    FDVT_WR32(0x00000000,FE_FNUM_2             );
    FDVT_WR32(0x00000000,FE_FNUM_3             );
    //                                                              
}
/*                                                                       */
//                                
/*                                                                       */
void SmileDetecteConfig(void)
{      
    FDVT_WR32(0x01210171,FD_FNUM_0             );
    FDVT_WR32(0x00D10081,FD_FNUM_1             );
    FDVT_WR32(0x00F100D1,FD_FNUM_2             );
    FDVT_WR32(0x00F100D1,FD_FNUM_3             );
    FDVT_WR32(0x00E70127,VT_FNUM_0             );
    FDVT_WR32(0x00A70067,VT_FNUM_1             );
    FDVT_WR32(0x00C000A7,VT_FNUM_2             );
    FDVT_WR32(0x00C000A7,VT_FNUM_3             );
    FDVT_WR32(0x00180018,FE_FNUM_0             );
    FDVT_WR32(0x00180018,FE_FNUM_1             );
    FDVT_WR32(0x00180018,FE_FNUM_2             );
    FDVT_WR32(0x00180018,FE_FNUM_3             );
    FDVT_WR32(0x000F010B,FDVT_FD_SET           );
}

/*                                                                       */
//                           
/*                                                                       */
void MT6573FDVT_DUMPREG(void)
{
    UINT32 u4RegValue = 0;
    UINT32 u4Index = 0;
    LOG_DBG("FDVT REG:\n ********************\n");

    //                                                 
    for(u4Index = 0x158; u4Index < 0x180; u4Index += 4) {
        u4RegValue = ioread32((void *)(FDVT_ADDR + u4Index));
        LOG_DBG("+0x%x 0x%x\n", u4Index,u4RegValue);
    }
}

/*                                                                       */
//                                  
/*                                                                       */
static int MT6573FDVT_SetRegHW(MT6573FDVTRegIO * a_pstCfg)
{
    MT6573FDVTRegIO *pREGIO = NULL;
    u32 i=0;
    if (NULL == a_pstCfg) {
        LOG_DBG("Null input argrment \n"); 
        return -EINVAL; 
    }

    pREGIO = (MT6573FDVTRegIO*)a_pstCfg;

    if(copy_from_user((void*)pMT6573FDVTWRBuff.u4Addr, (void *) pREGIO->pAddr, pREGIO->u4Count * sizeof(u32))) {
        LOG_DBG("ioctl copy from user failed\n");
        return -EFAULT;
    }

    if(copy_from_user((void*)pMT6573FDVTWRBuff.u4Data, (void *) pREGIO->pData, pREGIO->u4Count * sizeof(u32))) {
        LOG_DBG("ioctl copy from user failed\n");
        return -EFAULT;
    }

    //                                            
    //                                          

    for( i = 0; i < pREGIO->u4Count; i++ ) {
        if ((FDVT_ADDR + pMT6573FDVTWRBuff.u4Addr[i]) >= FDVT_ADDR && (FDVT_ADDR + pMT6573FDVTWRBuff.u4Addr[i]) <= (FDVT_ADDR + FDVT_MAX_OFFSET))
        {
            //                                                                                                                        
            FDVT_WR32(pMT6573FDVTWRBuff.u4Data[i], FDVT_ADDR + pMT6573FDVTWRBuff.u4Addr[i] );
        }
        else
        {
            LOG_DBG("Error: Writing Memory(0x%8x) Excess FDVT Range!\n", FDVT_ADDR + pMT6573FDVTWRBuff.u4Addr[i]);
        }
    }

    return 0;
}

/*                                                                              
  
                                                                               */
static int MT6573FDVT_ReadRegHW(MT6573FDVTRegIO * a_pstCfg)
{
    int ret = 0;
    int size = a_pstCfg->u4Count * 4;
    int i;

    if (size > buf_size) {
        LOG_DBG("size too big \n");
    }
    //
    if (copy_from_user(pMT6573FDVTRDBuff.u4Addr,  a_pstCfg->pAddr, size) != 0) {
        LOG_DBG("copy_from_user failed \n");
        ret = -EFAULT;
        goto mt_FDVT_read_reg_exit;
    }
    //
    for (i = 0; i < a_pstCfg->u4Count; i++) {
        if ((FDVT_ADDR + pMT6573FDVTRDBuff.u4Addr[i]) >= FDVT_ADDR && (FDVT_ADDR + pMT6573FDVTRDBuff.u4Addr[i]) <= (FDVT_ADDR + FDVT_MAX_OFFSET))
        {
            pMT6573FDVTRDBuff.u4Data[i] = ioread32((void *)(FDVT_ADDR + pMT6573FDVTRDBuff.u4Addr[i]));
            //                                                                                                                                 
        }
        else
        {
            LOG_DBG("Error: Reading Memory(0x%8x) Excess FDVT Range!\n", FDVT_ADDR + pMT6573FDVTRDBuff.u4Addr[i]);
            ret = -EFAULT;
            goto mt_FDVT_read_reg_exit; 
        }
    }
    //
    if (copy_to_user( a_pstCfg->pData, pMT6573FDVTRDBuff.u4Data, size) != 0) {
        LOG_DBG("copy_to_user failed \n");
        ret = -EFAULT;
        goto mt_FDVT_read_reg_exit;
    }
mt_FDVT_read_reg_exit:

    return ret;
}

/*                                                                       */
//                                                           
//                        
/*                                                                       */
static int MT6573FDVT_WaitIRQ(u32 * u4IRQMask)
{
    int timeout;
    timeout=wait_event_interruptible_timeout(g_MT6573FDVTWQ, (g_u4MT6573FDVTIRQMSK & g_u4MT6573FDVTIRQ) , ms_to_jiffies(500));
    
    if (timeout == 0) {
    	LOG_DBG("wait_event_interruptible_timeout timeout, %d, %d \n", g_u4MT6573FDVTIRQMSK, g_u4MT6573FDVTIRQ);
        FDVT_WR32(0x00030000, FDVT_START);  //            
        FDVT_WR32(0x00000000, FDVT_START);  //            
        return -EAGAIN;
    }
    
    *u4IRQMask = g_u4MT6573FDVTIRQ;
    //                                                   

    if (! (g_u4MT6573FDVTIRQMSK & g_u4MT6573FDVTIRQ)) { 
        LOG_DBG("wait_event_interruptible Not FDVT, %d, %d \n", g_u4MT6573FDVTIRQMSK, g_u4MT6573FDVTIRQ); 
        MT6573FDVT_DUMPREG(); 
        return -1; 
    }
    
    g_u4MT6573FDVTIRQ = 0;

    return 0;
}

static irqreturn_t MT6573FDVT_irq(int irq, void *dev_id)
{   
    //                                    
    //           
    g_u4MT6573FDVTIRQ=ioread32((void *)FDVT_INT);
    wake_up_interruptible(&g_MT6573FDVTWQ);
    
    return IRQ_HANDLED;
}

static long FDVT_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{   
    int ret = 0;

    if (_IOC_NONE != _IOC_DIR(cmd)) {                
        //                 
        if(_IOC_WRITE & _IOC_DIR(cmd)) {               
            if(copy_from_user(pBuff , (void *) arg, _IOC_SIZE(cmd))) {
                LOG_DBG(" ioctl copy from user failed\n");
                return -EFAULT;
            }            
        }
        //    
            //                                      
    }
    //    
        //                                    

    switch(cmd)
    {  
        case MT6573FDVT_INIT_SETPARA_CMD:
            LOG_DBG("[FDVT] FDVT_INIT_CMD\n");
            FDVT_basic_config();            
            break;
        case MT6573FDVTIOC_STARTFD_CMD:   
            LOG_DBG("[FDVT] MT6573FDVTIOC_STARTFD_CMD \n");   
            FDVT_WR32(0x00000001, FDVT_INT_EN);
            FDVT_WR32(0x00000000, FDVT_START);
            FDVT_WR32(0x00000001, FDVT_START);
            FDVT_WR32(0x00000000, FDVT_START);
            //                     
            break;  
        case MT6573FDVTIOC_G_WAITIRQ:
            //                                        
            ret = MT6573FDVT_WaitIRQ((UINT32 *)pBuff);
            FDVT_WR32(0x00000000, FDVT_INT_EN);  //                                 
            break;
        case MT6573FDVTIOC_T_SET_FDCONF_CMD:  
            //                                                          
            FaceDetecteConfig();  //                                                                      
            MT6573FDVT_SetRegHW((MT6573FDVTRegIO*)pBuff);
            break;
        case MT6573FDVTIOC_T_SET_SDCONF_CMD:  
            //                                                          
            SmileDetecteConfig();
            MT6573FDVT_SetRegHW((MT6573FDVTRegIO*)pBuff);
            break;
        case MT6573FDVTIOC_G_READ_FDREG_CMD:    
            //                                                 
            MT6573FDVT_ReadRegHW((MT6573FDVTRegIO*)pBuff);
            break;                 
        case MT6573FDVTIOC_T_DUMPREG :
            LOG_DBG("[FDVT] FDVT_DUMPREG \n");
            MT6573FDVT_DUMPREG();
            break;
        default:
            LOG_DBG("[FDVT][ERROR] FDVT_ioctl default case\n");
            break;
    }
    //                    
    //                                            
    if (_IOC_READ & _IOC_DIR(cmd)) {
        if (copy_to_user((void __user *) arg, pBuff , _IOC_SIZE(cmd)) != 0) {
            LOG_DBG("copy_to_user failed \n");
            return -EFAULT;
        }
    }
    
    return ret;
}

#ifdef CONFIG_COMPAT

/*                                                                              
 
                                                                               */

static int compat_FD_get_register_data(
            compat_MT6573FDVTRegIO __user *data32,
            MT6573FDVTRegIO __user *data)
{
    compat_uint_t count;
    compat_uptr_t uptr_Addr;
    compat_uptr_t uptr_Data;
    int err;

    err = get_user(uptr_Addr, &data32->pAddr);
    err |= put_user(compat_ptr(uptr_Addr), &data->pAddr);
    err |= get_user(uptr_Data, &data32->pData);
    err |= put_user(compat_ptr(uptr_Data), &data->pData);
    err |= get_user(count, &data32->u4Count);
    err |= put_user(count, &data->u4Count);
    
    return err;   
}

static int compat_FD_put_register_data(
            compat_MT6573FDVTRegIO __user *data32,
            MT6573FDVTRegIO __user *data)
{
    compat_uint_t count;
    compat_uptr_t uptr_Addr;
    compat_uptr_t uptr_Data;
    int err;

    //                                 
    //                                        
    //                                                      
    //                                         
    //                                                       
    err = get_user(count, &data->u4Count);
    err |= put_user(count, &data32->u4Count);
    
    return err;
}

static long compat_FD_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret;

    if (!file->f_op || !file->f_op->unlocked_ioctl)
        return -ENOTTY;

    switch (cmd) {
    case COMPAT_MT6573FDVT_INIT_SETPARA_CMD:
    {
        return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
    }
    case COMPAT_MT6573FDVTIOC_STARTFD_CMD:
    {
        return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
    }
    case COMPAT_MT6573FDVTIOC_G_WAITIRQ:
    {
        compat_MT6573FDVTRegIO __user *data32;
        MT6573FDVTRegIO __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_FD_get_register_data(data32, data);
        if (err)
            return err;
        ret = file->f_op->unlocked_ioctl(file, MT6573FDVTIOC_G_WAITIRQ,(unsigned long)data);
        err = compat_FD_put_register_data(data32, data);
        return ret ? ret : err;
    }
    case COMPAT_MT6573FDVTIOC_T_SET_FDCONF_CMD:
    {
        compat_MT6573FDVTRegIO __user *data32;
        MT6573FDVTRegIO __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_FD_get_register_data(data32, data);
        if (err)
            return err;
        ret = file->f_op->unlocked_ioctl(file, MT6573FDVTIOC_T_SET_FDCONF_CMD,(unsigned long)data);
        return ret ? ret : err;
    }
    case COMPAT_MT6573FDVTIOC_G_READ_FDREG_CMD:
    {
        compat_MT6573FDVTRegIO __user *data32;
        MT6573FDVTRegIO __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_FD_get_register_data(data32, data);
        if (err)
            return err;
        ret = file->f_op->unlocked_ioctl(file, MT6573FDVTIOC_G_READ_FDREG_CMD,(unsigned long)data);
        err = compat_FD_put_register_data(data32, data);
        return ret ? ret : err;

    }
    case COMPAT_MT6573FDVTIOC_T_SET_SDCONF_CMD:
    {
        compat_MT6573FDVTRegIO __user *data32;
        MT6573FDVTRegIO __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_FD_get_register_data(data32, data);
        if (err)
            return err;
        ret = file->f_op->unlocked_ioctl(file, MT6573FDVTIOC_T_SET_SDCONF_CMD,(unsigned long)data);
        return ret ? ret : err;
    }
    case COMPAT_MT6573FDVTIOC_T_DUMPREG:
    {
        return file->f_op->unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
    }
    default:
        return -ENOIOCTLCMD;    
    //                                  
    }
}

#endif


static int FDVT_open(struct inode *inode, struct file *file)
{
    //                
    INT32 ret=0;
    
    LOG_DBG("[FDVT_DEBUG] FDVT_open\n");
    mt_fdvt_clk_ctrl(1); //               
    if (pBuff != NULL) {
        LOG_DBG("pBuff is not null \n");
    }
    if (pread_buf != NULL) {
        LOG_DBG("pread_buf is not null \n");
    }
    
    pBuff = kmalloc(buf_size,GFP_KERNEL);

    if(NULL == pBuff) {
        LOG_DBG(" ioctl allocate mem failed\n");
        ret = -ENOMEM;
    }
    else
        LOG_DBG(" ioctl allocate mem ok\n");

    pread_buf = (u8 *) kmalloc(buf_size, GFP_KERNEL);
    if (pread_buf == NULL) {    
        LOG_DBG(" ioctl allocate mem failed\n");
        ret = -ENOMEM;
    }
    else
        LOG_DBG(" ioctl allocate mem ok\n");
    
    if (ret < 0) {
        if (pBuff) {
            kfree(pBuff);
            pBuff = NULL;
        }
        if (pread_buf) {
            kfree(pread_buf);
            pread_buf = NULL;
        }
    }
    
    return 0;
}
/*
                                                       
 
                                         
    
             
 
*/
static int FDVT_release(struct inode *inode, struct file *file)
{
    LOG_DBG("[FDVT_DEBUG] FDVT_release\n");
    if (pBuff) {
        kfree(pBuff);
        pBuff = NULL;
    }
    if (pread_buf) {
        kfree(pread_buf);
        pread_buf = NULL;
    }
    
    FDVT_WR32(0x00000000, FDVT_INT_EN);  //                                 
    mb();
    g_u4MT6573FDVTIRQ=ioread32((void *)FDVT_INT); //                                
    mb();
    mt_fdvt_clk_ctrl(0); //                
    return 0;
}   
    
static struct file_operations FDVT_fops = {
    .owner               = THIS_MODULE,
    .unlocked_ioctl      = FDVT_ioctl,
    .open                = FDVT_open,
    .release             = FDVT_release,
#ifdef CONFIG_COMPAT
    .compat_ioctl        = compat_FD_ioctl,
#endif

};

static int FDVT_probe(struct platform_device *dev)
{
    struct class_device;
    
    int ret;
    int i=0;
    int new_count;
    struct class_device *class_dev = NULL;

#ifdef CONFIG_OF
		struct fdvt_device *fdvt_dev;

        LOG_DBG("[FDVT_DEBUG] FDVT_probe\n");
	
		if(dev == NULL)
		{
			dev_err(&dev->dev, "dev is NULL");
			return -ENXIO;
		}
	
		new_count = nr_fdvt_devs + 1;
		fdvt_devs = krealloc(fdvt_devs, 
			sizeof(struct fdvt_device) * new_count, GFP_KERNEL);
		if (!fdvt_devs) {
			dev_err(&dev->dev, "Unable to allocate fdvt_devs\n");
			return -ENOMEM;
		}
	
		fdvt_dev = &(fdvt_devs[nr_fdvt_devs]);
		fdvt_dev->dev = &dev->dev;
	
		/*                        */
		for(i=0;i<FDVT_BASEADDR_NUM;i++)
		{
			fdvt_dev->regs[i] = of_iomap(dev->dev.of_node, i);
			if (!fdvt_dev->regs[i]) {
				dev_err(&dev->dev, "Unable to ioremap registers, of_iomap fail, i=%d \n", i);
				return -ENOMEM;
			}
	
			gFDVT_Reg[i] = (unsigned long)fdvt_dev->regs[i];
			LOG_INF("DT, i=%d, map_addr=0x%lx\n", i, gFDVT_Reg[i]);
		}

		/*                            */
        
		for(i=0;i<FDVT_IRQ_IDX_NUM;i++)
		{
            fdvt_dev->irq[i] = irq_of_parse_and_map(dev->dev.of_node, i);
            gFDVT_Irq[i] = fdvt_dev->irq[i];
            if (FDVT_IRQ_IDX == i)
            {
        	    ret = request_irq(fdvt_dev->irq[i], (irq_handler_t)MT6573FDVT_irq, IRQF_TRIGGER_NONE, FDVT_DEVNAME,  NULL);  //                                                                               
        	    //                                                                                               
            }
	     	if (ret) {
	            dev_err(&dev->dev, "Unable to request IRQ, request_irq fail, i=%d, irq=%d \n", i, fdvt_dev->irq[i]);
		        return ret;
		 	}
		    LOG_INF("DT, i=%d, map_irq=%d\n", i, fdvt_dev->irq[i]);
        }
                
	    nr_fdvt_devs = new_count;
        if(dev == NULL)
        {
            dev_err(&dev->dev, "dev is NULL");
            return -ENXIO;
        }
/* 
                         
                              
   
                                              
              
   
                          
     
                                                                                                                                
   
                                               
                                                        
                   
    
                                                       
                  
              
    
                                                                                  
                   
    
                                                    
                  
              
    
   
      

                          
                                                  
                       
   
                            
                                                    
              
   
                          
                                                              
*/	
#endif

    ret = alloc_chrdev_region(&FDVT_devno, 0, 1, FDVT_DEVNAME);
        
    if(ret)
    {
        LOG_DBG("[FDVT_DEBUG][ERROR] Can't Get Major number for FDVT Device\n");
    }
    
    FDVT_cdev = cdev_alloc();
    
    FDVT_cdev->owner = THIS_MODULE;
    FDVT_cdev->ops = &FDVT_fops;

    cdev_init(FDVT_cdev, &FDVT_fops);
    
    if ( (ret = cdev_add(FDVT_cdev, FDVT_devno, 1)) < 0) {
        LOG_DBG("[FDVT_DEBUG] Attatch file operation failed \n");
        return -EFAULT;
    }

#ifndef CONFIG_OF
    //                   

    //                                                                                                
//                                                                                                         
    if (request_irq(FD_IRQ_BIT_ID, (irq_handler_t)MT6573FDVT_irq, IRQF_TRIGGER_LOW, FDVT_DEVNAME, NULL) < 0)
    {
       LOG_DBG("[FDVT_DEBUG][ERROR] error to request FDVT irq\n"); 
    }
    else
    {
       LOG_DBG("[FDVT_DEBUG] success to request FDVT irq\n");
    }
    //                                                                        
    //                                                        
    //                                              
    //                                       
#endif


    FDVT_class = class_create(THIS_MODULE, FDVT_DEVNAME);
    class_dev = (struct class_device *)device_create(FDVT_class,
                                                     NULL,
                                                     FDVT_devno,
                                                     NULL,
                                                     FDVT_DEVNAME
                                                     );
    //                    
    init_waitqueue_head(&g_MT6573FDVTWQ);
    

    //                          
    //                    
    
   
    LOG_DBG("[FDVT_DEBUG] FDVT_probe Done\n");
    
    return 0;
}

static int FDVT_remove(struct platform_device *dev)
{    
    int i4IRQ = 0;
    LOG_DBG("[FDVT_DEBUG] FDVT_driver_exit\n");
    FDVT_WR32(0x00000000, FDVT_INT_EN);  //                                 
    g_u4MT6573FDVTIRQ=ioread32((void *)FDVT_INT); //                                
    mt_fdvt_clk_ctrl(0); //                
    device_destroy(FDVT_class, FDVT_devno);
    class_destroy(FDVT_class);
   
    cdev_del(FDVT_cdev);
    unregister_chrdev_region(FDVT_devno, 1);            
    
    i4IRQ = platform_get_irq(dev, 0);
    free_irq(i4IRQ , NULL);
    
    if (pBuff) {
        kfree(pBuff);
        pBuff = NULL;
    }
    if (pread_buf) {
        kfree(pread_buf);
        pread_buf = NULL;
    }
    
    
    //                                         
    //                                         
    return 0;
}

static int FDVT_suspend(struct platform_device *dev, pm_message_t state)
{    
    //              
    //                                       
    //                    
    /*
                                
     
                                                             
                                                             
     
    */    
    //                      
    return 0;
}

static int FDVT_resume(struct platform_device *dev)
{   
    //          
    //                                      
    //                    
    /*
                                
     
                                                            
     
    */
    //                     
    return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id fdvt_of_ids[] = {
    { .compatible = "mediatek,FD", },
    {}
};
#endif

static struct platform_driver FDVT_driver = {
    .driver      = {
    .name        = FDVT_DEVNAME,
    .owner       = THIS_MODULE,
#ifdef CONFIG_OF
       .of_match_table = fdvt_of_ids,
#endif
    },
    .probe       = FDVT_probe,
    .remove      = FDVT_remove,
    .suspend     = FDVT_suspend,
    .resume      = FDVT_resume,
};

/*                                               
                                             
                             
                  
  
*/

#ifdef CONFIG_HAS_EARLYSUSPEND
static void FDVT_early_suspend(struct early_suspend *h)
{
    //                                       
    //                    

}

static void FDVT_early_resume(struct early_suspend *h)
{
    //                                       
    //                    
}

static struct early_suspend FDVT_early_suspend_desc = {
	.level		= EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1,
	.suspend	= FDVT_early_suspend,
	.resume		= FDVT_early_resume,
};
#endif

static int __init FDVT_driver_init(void)
{
    int ret;

    LOG_DBG("[FDVT_DEBUG] FDVT_driver_init\n");

	/*
                                                  
                                                                          
                        
                     
       
      */
    
    if (platform_driver_register(&FDVT_driver)){
        LOG_DBG("[FDVT_DEBUG][ERROR] failed to register FDVT Driver\n");
        //                                         
        ret = -ENODEV;
        return ret;
    }
    #ifdef CONFIG_HAS_EARLYSUSPEND
    register_early_suspend(&FDVT_early_suspend_desc);
    #endif

    LOG_DBG("[FDVT_DEBUG] FDVT_driver_init Done\n");
    
    return 0;
}

static void __exit FDVT_driver_exit(void)
{
    LOG_DBG("[FDVT_DEBUG] FDVT_driver_exit\n");

    device_destroy(FDVT_class, FDVT_devno);
    class_destroy(FDVT_class);
   
    cdev_del(FDVT_cdev);
    unregister_chrdev_region(FDVT_devno, 1);            
       
    platform_driver_unregister(&FDVT_driver);
    //                                         
    
}


module_init(FDVT_driver_init);
module_exit(FDVT_driver_exit);
MODULE_AUTHOR("WCD/OSS9/ME3");
MODULE_DESCRIPTION("FDVT Driver");
MODULE_LICENSE("GPL");
  
