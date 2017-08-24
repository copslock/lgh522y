#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/aee.h>
#include <linux/xlog.h>
#include <linux/proc_fs.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/writeback.h>
#include <linux/earlysuspend.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h>

#include <mach/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_pmic_wrap.h>
#include <mach/mt_gpio.h>
#include <mach/mtk_rtc.h>
#include <mach/mt_spm_mtcmos.h>

#include <mach/battery_common.h>
#include <linux/time.h>

//                                                                
//               
//                                                                
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern void Charger_Detect_Init(void);
extern void Charger_Detect_Release(void);
//                         
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)

int hw_charging_get_charger_type(void)
{
    return STANDARD_HOST;
}

#else

static void hw_bc11_dump_register(void)
{
    battery_xlog_printk(BAT_LOG_FULL, "Reg[0x%x]=0x%x,Reg[0x%x]=0x%x\n", 
        MT6325_CHR_CON20, upmu_get_reg_value(MT6325_CHR_CON20),
        MT6325_CHR_CON21, upmu_get_reg_value(MT6325_CHR_CON21)
        );
}

static void hw_bc11_init(void)
{
    msleep(200);
    Charger_Detect_Init();
        
    //                     
    mt6325_upmu_set_rg_bc11_bias_en(0x1);
    //                       
    mt6325_upmu_set_rg_bc11_vsrc_en(0x0);
    //                           
    mt6325_upmu_set_rg_bc11_vref_vth(0x0);
    //                        
    mt6325_upmu_set_rg_bc11_cmp_en(0x0);
    //                        
    mt6325_upmu_set_rg_bc11_ipu_en(0x0);
    //                        
    mt6325_upmu_set_rg_bc11_ipd_en(0x0);
    //          
    mt6325_upmu_set_rg_bc11_rst(0x1);
    //              
    mt6325_upmu_set_rg_bc11_bb_ctrl(0x1);

    msleep(50);
    //           
    
    if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
        battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_init() \r\n");
        hw_bc11_dump_register();
    }    

}
 
 
static U32 hw_bc11_DCD(void)
{
    U32 wChargerAvail = 0;

    //                        
    mt6325_upmu_set_rg_bc11_ipu_en(0x2);    
    //                        
    mt6325_upmu_set_rg_bc11_ipd_en(0x1);
    //                           
    mt6325_upmu_set_rg_bc11_vref_vth(0x1);    
    //                        
    mt6325_upmu_set_rg_bc11_cmp_en(0x2);

    msleep(80);
    //           

    wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();
    
    if(Enable_BATDRV_LOG == BAT_LOG_FULL)
    {
        battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_DCD() \r\n");
        hw_bc11_dump_register();
    }
    
    //                        
    mt6325_upmu_set_rg_bc11_ipu_en(0x0);
    //                        
    mt6325_upmu_set_rg_bc11_ipd_en(0x0);
    //                        
    mt6325_upmu_set_rg_bc11_cmp_en(0x0);
    //                           
    mt6325_upmu_set_rg_bc11_vref_vth(0x0);

    return wChargerAvail;
}
 
 
static U32 hw_bc11_stepA1(void)
{
   U32 wChargerAvail = 0;
     
   //                        
   mt6325_upmu_set_rg_bc11_ipd_en(0x1);   
   //                           
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);   
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x1);

   msleep(80);
   //           

   wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();

   if(Enable_BATDRV_LOG == BAT_LOG_FULL)
   {
       battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepA1() \r\n");
       hw_bc11_dump_register();
   }

   //                        
   mt6325_upmu_set_rg_bc11_ipd_en(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);

   return  wChargerAvail;
}
 
 
static U32 hw_bc11_stepA2(void)
{
   U32 wChargerAvail = 0;
     
   //                          
   mt6325_upmu_set_rg_bc11_vsrc_en(0x2);   
   //                        
   mt6325_upmu_set_rg_bc11_ipd_en(0x1);   
   //                           
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);   
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x1);

   msleep(80);
   //           

   wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();

   if(Enable_BATDRV_LOG == BAT_LOG_FULL)
   {
       battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepA2() \r\n");
       hw_bc11_dump_register();
   }

   //                       
   mt6325_upmu_set_rg_bc11_vsrc_en(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_ipd_en(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);

   return  wChargerAvail;
}
 
 
static U32 hw_bc11_stepB2(void)
{
   U32 wChargerAvail = 0;

   //                      
   mt6325_upmu_set_rg_bc11_ipu_en(0x2);   
   //                           
   mt6325_upmu_set_rg_bc11_vref_vth(0x1);   
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x1);

   msleep(80);
   //           

   wChargerAvail = mt6325_upmu_get_rgs_bc11_cmp_out();

   if(Enable_BATDRV_LOG == BAT_LOG_FULL)
   {
       battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_stepB2() \r\n");
       hw_bc11_dump_register();
   }

   
   if (!wChargerAvail) {
       //                          
       mt6325_upmu_set_rg_bc11_vsrc_en(0x2);
   }
   //                        
   mt6325_upmu_set_rg_bc11_ipu_en(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);
   //                           
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);

   return  wChargerAvail;
}
 
 
static void hw_bc11_done(void)
{
   //                       
   mt6325_upmu_set_rg_bc11_vsrc_en(0x0);
   //                          
   mt6325_upmu_set_rg_bc11_vref_vth(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_cmp_en(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_ipu_en(0x0);
   //                        
   mt6325_upmu_set_rg_bc11_ipd_en(0x0);
   //                 
   mt6325_upmu_set_rg_bc11_bias_en(0x0); 

  //                  
   Charger_Detect_Release();

   if(Enable_BATDRV_LOG == BAT_LOG_FULL)
   {
       battery_xlog_printk(BAT_LOG_FULL, "hw_bc11_done() \r\n");
       hw_bc11_dump_register();
   }
   
}

int hw_charging_get_charger_type(void)
{
#if 0
    return STANDARD_HOST;
    //                                  
#else
    CHARGER_TYPE CHR_Type_num = CHARGER_UNKNOWN;
    
    
    /*                                     */         
    hw_bc11_init();
 
    /*                                */  
    if(1 == hw_bc11_DCD())
    {
         /*                               */
         if(1 == hw_bc11_stepA1())
         {             
             CHR_Type_num = APPLE_2_1A_CHARGER;
             battery_xlog_printk(BAT_LOG_CRTI, "step A1 : Apple 2.1A CHARGER!\r\n");
         }
         else
         {
             CHR_Type_num = NONSTANDARD_CHARGER;
             battery_xlog_printk(BAT_LOG_CRTI, "step A1 : Non STANDARD CHARGER!\r\n");
         }
    }
    else
    {
         /*                               */
         if(1 == hw_bc11_stepA2())
         {
             /*                               */
             if(1 == hw_bc11_stepB2())
             {
                //                    
                 CHR_Type_num = STANDARD_CHARGER;
                 battery_xlog_printk(BAT_LOG_CRTI, "step B2 : STANDARD CHARGER!\r\n");
             }
             else
             {
                 CHR_Type_num = CHARGING_HOST;
                 battery_xlog_printk(BAT_LOG_CRTI, "step B2 :  Charging Host!\r\n");
             }
         }
         else
         {
             CHR_Type_num = STANDARD_HOST;
             battery_xlog_printk(BAT_LOG_CRTI, "step A2 : Standard USB Host!\r\n");
         }
 
    }
 
    /*                                                       */
    hw_bc11_done();

    return CHR_Type_num;
#endif    
}
#endif
