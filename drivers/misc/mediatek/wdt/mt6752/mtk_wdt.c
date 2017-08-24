#include <linux/init.h>        /*                      */
#include <linux/module.h>      /*                      */
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/watchdog.h>
#include <linux/platform_device.h>

#include <asm/uaccess.h>
#include <mach/irqs.h>
#include <mach/mt_reg_base.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_wdt.h>
#include <linux/delay.h>

#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <linux/aee.h>
#include <mach/sync_write.h>
#include <mach/ext_wd_drv.h>

#include <mach/wd_api.h>

#include <mach/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>

#include <mach/mt_pmic_wrap.h>

void __iomem *toprgu_base=0;
int 	wdt_irq_id=0;

static const struct of_device_id rgu_of_match[] = {
	{ .compatible = "mediatek,TOPRGU", },
	{},
};

extern void aee_wdt_printf(const char *fmt, ...);
extern S32 pwrap_write_nochk( U32  adr, U32  wdata);
extern S32 pwrap_read_nochk( U32  adr, U32 *rdata);

/*                                                                      
                            
                                                                        
 */
#define NO_DEBUG 1

/*                                                                      
            
                                                                      */
#define AP_RGU_WDT_IRQ_ID    wdt_irq_id

/* 
                      
 */
//                                    
//                                                                  
static DEFINE_SPINLOCK(rgu_reg_operation_spinlock);
static unsigned int timeout;

static volatile BOOL  rgu_wdt_intr_has_trigger; //             
static int g_last_time_time_out_value = 0;
static int g_wdt_enable= 1;

#ifndef __USING_DUMMY_WDT_DRV__ /*                         */
/*
                                        
                 
*/
void mtk_wdt_set_time_out_value(unsigned int value)
{
	/*
                           
                                 
  */	
	spin_lock(&rgu_reg_operation_spinlock);
	
	//                                                  
	//                   
	timeout = (unsigned int)(value * ( 1 << 6) );
	timeout = timeout << 5; 
	DRV_WriteReg32(MTK_WDT_LENGTH, (timeout | MTK_WDT_LENGTH_KEY) );

	spin_unlock(&rgu_reg_operation_spinlock);
}
/*
                  
                                           
                                                   
                                              
                                                 
                                      
*/
void mtk_wdt_mode_config(	BOOL dual_mode_en, 
					BOOL irq, 
					BOOL ext_en, 
					BOOL ext_pol, 
					BOOL wdt_en )
{
	unsigned int tmp; 

	spin_lock(&rgu_reg_operation_spinlock);

	//                                                                                            
	tmp = DRV_Reg32(MTK_WDT_MODE);
	tmp |= MTK_WDT_MODE_KEY;

	//                                       
	if(wdt_en == TRUE)
		tmp |= MTK_WDT_MODE_ENABLE;
	else
		tmp &= ~MTK_WDT_MODE_ENABLE;

	//                                                
	if(ext_pol == TRUE)
		tmp |= MTK_WDT_MODE_EXT_POL;
	else
		tmp &= ~MTK_WDT_MODE_EXT_POL;

	//                                             
	if(ext_en == TRUE)
		tmp |= MTK_WDT_MODE_EXTEN;
	else
		tmp &= ~MTK_WDT_MODE_EXTEN;

	//                                                             
	if(irq == TRUE)
		tmp |= MTK_WDT_MODE_IRQ;
	else
		tmp &= ~MTK_WDT_MODE_IRQ;

	//                                          
	if(dual_mode_en == TRUE)
		tmp |= MTK_WDT_MODE_DUAL_MODE;
	else
		tmp &= ~MTK_WDT_MODE_DUAL_MODE;

	//                                                                                    
	//                                                                   
	tmp |= MTK_WDT_MODE_AUTO_RESTART;

	DRV_WriteReg32(MTK_WDT_MODE,tmp);
	//                                
	//            
	printk(KERN_INFO " mtk_wdt_mode_config  mode value=%x, tmp:%x,pid=%d\n",DRV_Reg32(MTK_WDT_MODE), tmp,current->pid);

	spin_unlock(&rgu_reg_operation_spinlock);
}
//                                   

int mtk_wdt_enable(enum wk_wdt_en en)
{
	unsigned int tmp =0;
    
	spin_lock(&rgu_reg_operation_spinlock);

	tmp = DRV_Reg32(MTK_WDT_MODE);
	
	tmp |= MTK_WDT_MODE_KEY;
	if(WK_WDT_EN == en)
	{
	  tmp |= MTK_WDT_MODE_ENABLE;
	  g_wdt_enable = 1;
	}
	if(WK_WDT_DIS == en)
	{
	  tmp &= ~MTK_WDT_MODE_ENABLE;
	  g_wdt_enable = 0;
	}
    printk("mtk_wdt_enable value=%x,pid=%d\n",tmp,current->pid);
	DRV_WriteReg32(MTK_WDT_MODE,tmp);

	spin_unlock(&rgu_reg_operation_spinlock);

	return 0;
}
int  mtk_wdt_confirm_hwreboot(void)
{
    //                                 
    //                                                     
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
	return 0;
}


void mtk_wdt_restart(enum wd_restart_type type)
{

    //                                                                      

	if(type == WD_TYPE_NORMAL) 
	{     
	    //                             
	    spin_lock(&rgu_reg_operation_spinlock); 

	    DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);

	    spin_unlock(&rgu_reg_operation_spinlock);
	}
	else if(type == WD_TYPE_NOLOCK) 
	{
	    *(volatile u32 *)( MTK_WDT_RESTART) =MTK_WDT_RESTART_KEY ;
	}
	else
	{
	    //                                                                       
	}
}

void wdt_dump_reg(void)
{
	printk("****************dump wdt reg start*************\n");
	printk("MTK_WDT_MODE:0x%x\n", DRV_Reg32(MTK_WDT_MODE));
	printk("MTK_WDT_LENGTH:0x%x\n", DRV_Reg32(MTK_WDT_LENGTH));
	printk("MTK_WDT_RESTART:0x%x\n", DRV_Reg32(MTK_WDT_RESTART));
	printk("MTK_WDT_STATUS:0x%x\n", DRV_Reg32(MTK_WDT_STATUS));
	printk("MTK_WDT_INTERVAL:0x%x\n", DRV_Reg32(MTK_WDT_INTERVAL));
	printk("MTK_WDT_SWRST:0x%x\n", DRV_Reg32(MTK_WDT_SWRST));
	printk("MTK_WDT_NONRST_REG:0x%x\n", DRV_Reg32(MTK_WDT_NONRST_REG));
	printk("MTK_WDT_NONRST_REG2:0x%x\n", DRV_Reg32(MTK_WDT_NONRST_REG2));
	printk("MTK_WDT_REQ_MODE:0x%x\n", DRV_Reg32(MTK_WDT_REQ_MODE));
	printk("MTK_WDT_REQ_IRQ_EN:0x%x\n", DRV_Reg32(MTK_WDT_REQ_IRQ_EN));
	printk("MTK_WDT_DRAMC_CTL:0x%x\n", DRV_Reg32(MTK_WDT_DRAMC_CTL));
	printk("****************dump wdt reg end*************\n");
	
}

static struct device_node *msdc_node = NULL;
static void __iomem *msdc_base = NULL;
#define REG_ADDR(x)             ((volatile unsigned int*)(msdc_base + OFFSET_##x))
#define OFFSET_EMMC_IOCON       (0x7c)
#define EMMC_IOCON              REG_ADDR(EMMC_IOCON)
#define MSDC_SET_BIT32(addr,mask)	\
	do { \
		(*(volatile unsigned int*)(addr) |= (mask)); \
	}while(0)
#define MSDC_CLR_BIT32(addr,mask)	\
	do { \
		(*(volatile unsigned int*)(addr) &= ~(mask)); \
	}while(0)

void wdt_arch_reset(char mode)
{
    unsigned int wdt_mode_val;
	struct device_node *np_rgu;
	U32 A4A_rdata=0x0;
	
	printk("wdt_arch_reset called@Kernel mode =%c\n",mode);
	np_rgu = of_find_compatible_node(NULL, NULL, rgu_of_match[0].compatible);
	
	if(!toprgu_base)
	{	
		toprgu_base = of_iomap(np_rgu, 0);
		if (!toprgu_base) {
			printk("RGU iomap failed\n");
		}
		printk("RGU base: 0x%p  RGU irq: %d\n", toprgu_base, wdt_irq_id);
	}
	
	if (msdc_base == NULL) {
		msdc_node = of_find_compatible_node(NULL, NULL, "mediatek,MSDC0");
		msdc_base = of_iomap(msdc_node, 0);
	}

	spin_lock(&rgu_reg_operation_spinlock);

	/*            */
	if (msdc_base != NULL) {
		MSDC_SET_BIT32(EMMC_IOCON, 1);
		udelay(100);
		MSDC_CLR_BIT32(EMMC_IOCON, 1);
	}

	//                                          
	pwrap_read_nochk(0xA4A, &A4A_rdata);
	pwrap_write_nochk(0xA4A, A4A_rdata | (1 << 9));
	pwrap_read_nochk(0xA4A, &A4A_rdata);
	//                                     
	udelay(50);

	printk("wdt_arch_reset read MT6325 0xA4A =%x \n",A4A_rdata);

	/*               */
	DRV_WriteReg32(MTK_WDT_RESTART, MTK_WDT_RESTART_KEY);
	wdt_mode_val = DRV_Reg32(MTK_WDT_MODE);
	printk("wdt_arch_reset called MTK_WDT_MODE =%x \n",wdt_mode_val);
	/*                                                                                 */
	wdt_mode_val &=(~MTK_WDT_MODE_AUTO_RESTART);
	/*                                                                */
	wdt_mode_val &=(~(MTK_WDT_MODE_IRQ|MTK_WDT_MODE_ENABLE | MTK_WDT_MODE_DUAL_MODE));
	if(mode)
	{
		/*                                                                                               */
		 wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN|MTK_WDT_MODE_AUTO_RESTART);
		 
	}else
	{
	       wdt_mode_val = wdt_mode_val | (MTK_WDT_MODE_KEY|MTK_WDT_MODE_EXTEN);
		 
	}

	DRV_WriteReg32(MTK_WDT_MODE,wdt_mode_val);
	printk("wdt_arch_reset called end  MTK_WDT_MODE =%x \n",wdt_mode_val);
	udelay(100);
	DRV_WriteReg32(MTK_WDT_SWRST, MTK_WDT_SWRST_KEY);
        printk("wdt_arch_reset: SW_reset happen\n");
	spin_unlock(&rgu_reg_operation_spinlock);

	while (1)
	{
	    wdt_dump_reg();
		printk("wdt_arch_reset error\n");
	}
	
}

int mtk_wdt_swsysret_config(int bit,int set_value)
{
    unsigned int wdt_sys_val;
	spin_lock(&rgu_reg_operation_spinlock);
	wdt_sys_val = DRV_Reg32(MTK_WDT_SWSYSRST);
	printk("fwq2 before set wdt_sys_val =%x\n",wdt_sys_val);
    wdt_sys_val |= MTK_WDT_SWSYS_RST_KEY;
	switch(bit)
	{
	  case MTK_WDT_SWSYS_RST_MD_RST:
	  	   if(1==set_value)
	  	   {
	  	     wdt_sys_val |= MTK_WDT_SWSYS_RST_MD_RST;
	  	   }
		   if(0==set_value)
		   {
		     wdt_sys_val &= ~MTK_WDT_SWSYS_RST_MD_RST;
		   }
	  	break;
	 case MTK_WDT_SWSYS_RST_MD_LITE_RST:
	  	   if(1==set_value)
	  	   {
	  	     wdt_sys_val |= MTK_WDT_SWSYS_RST_MD_LITE_RST;
	  	   }
		   if(0==set_value)
		   {
		     wdt_sys_val &= ~MTK_WDT_SWSYS_RST_MD_LITE_RST;
		   }
	  	break;
	  	   
	}
	DRV_WriteReg32(MTK_WDT_SWSYSRST,wdt_sys_val);
	spin_unlock(&rgu_reg_operation_spinlock);

	mdelay(10);
	printk("after set wdt_sys_val =%x,wdt_sys_val=%x\n",DRV_Reg32(MTK_WDT_SWSYSRST),wdt_sys_val);
    return 0;
}

int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en)
{
    int res=0;
    unsigned int tmp;
	spin_lock(&rgu_reg_operation_spinlock);
	tmp = DRV_Reg32(MTK_WDT_REQ_MODE);
	tmp |=  MTK_WDT_REQ_MODE_KEY;
	
    if(MTK_WDT_REQ_MODE_SPM_SCPSYS == mark_bit)
    {
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_SPM_SCPSYS);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_SPM_SCPSYS);
		}
    }
	else if(MTK_WDT_REQ_MODE_SPM_THERMAL == mark_bit)
	{
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_SPM_THERMAL);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_SPM_THERMAL);
		}
	}
	else if ( MTK_WDT_REQ_MODE_THERMAL == mark_bit)
	{
	 	if(WD_REQ_EN == en){
            tmp |= (MTK_WDT_REQ_MODE_THERMAL);
	 	}
		if(WD_REQ_DIS == en){
			tmp &=~(MTK_WDT_REQ_MODE_THERMAL);
		}
	}
	else
	{
		    res =-1; 
	}
      
   DRV_WriteReg32(MTK_WDT_REQ_MODE,tmp);
   spin_unlock(&rgu_reg_operation_spinlock);
   return res;
   
}

int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode)
{
    int res=0;
    unsigned int tmp;
	spin_lock(&rgu_reg_operation_spinlock);
	tmp = DRV_Reg32(MTK_WDT_REQ_IRQ_EN);
	tmp |=  MTK_WDT_REQ_IRQ_KEY;
    if(MTK_WDT_REQ_MODE_SPM_SCPSYS == mark_bit)
    {
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_SPM_SCPSYS_EN);
		}
    }
	else if ( MTK_WDT_REQ_MODE_SPM_THERMAL == mark_bit)
	{
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_SPM_THERMAL_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_SPM_THERMAL_EN);
		}
	}
	else if ( MTK_WDT_REQ_MODE_THERMAL == mark_bit)
	{
	 	if(WD_REQ_IRQ_MODE == mode){
            tmp |= (MTK_WDT_REQ_IRQ_THERMAL_EN);
	 	}
		if(WD_REQ_RST_MODE == mode){
			tmp &=~(MTK_WDT_REQ_IRQ_THERMAL_EN);
		}
	}
	else
	{
		res =-1;
    }
  
   DRV_WriteReg32(MTK_WDT_REQ_IRQ_EN,tmp);
   spin_unlock(&rgu_reg_operation_spinlock);
   return res;
}

#else 
//                                                                                                 
//                     
//                                                                                                 
void mtk_wdt_set_time_out_value(unsigned int value){}
static void mtk_wdt_set_reset_length(unsigned int value){}
void mtk_wdt_mode_config(BOOL dual_mode_en,BOOL irq,	BOOL ext_en, BOOL ext_pol, BOOL wdt_en){}
int mtk_wdt_enable(enum wk_wdt_en en){ return 0;}
void mtk_wdt_restart(enum wd_restart_type type){}
static void mtk_wdt_sw_trigger(void){}
static unsigned char mtk_wdt_check_status(void){ return 0;}
void wdt_arch_reset(char mode){}
int  mtk_wdt_confirm_hwreboot(void){return 0;}
void mtk_wd_suspend(void){}
void mtk_wd_resume(void){}
void wdt_dump_reg(void){}
int mtk_wdt_swsysret_config(int bit,int set_value){ return 0;}
int mtk_wdt_request_mode_set(int mark_bit,WD_REQ_MODE mode){return 0;}
int mtk_wdt_request_en_set(int mark_bit,WD_REQ_CTL en){return 0;}








#endif //                               

#ifndef CONFIG_FIQ_GLUE
static void wdt_report_info (void)
{
    //                                  
    struct task_struct *task ;
    task = &init_task ;
    
    printk ("Qwdt: -- watchdog time out\n") ;
    for_each_process (task)
    {
        if (task->state == 0)
        {
            printk ("PID: %d, name: %s\n backtrace:\n", task->pid, task->comm) ;
            show_stack (task, NULL) ;
            printk ("\n") ;
        }
    }
    
    
    printk ("backtrace of current task:\n") ;
    show_stack (NULL, NULL) ;
    
       
    printk ("Qwdt: -- watchdog time out\n") ;    
}
#endif


#ifdef CONFIG_FIQ_GLUE
static void wdt_fiq(void *arg, void *regs, void *svc_sp)
{
	unsigned int wdt_mode_val;
	struct wd_api*wd_api = NULL;
    get_wd_api(&wd_api);
	wdt_mode_val = DRV_Reg32(MTK_WDT_STATUS);
	DRV_WriteReg32(MTK_WDT_NONRST_REG, wdt_mode_val);
    #ifdef	CONFIG_MTK_WD_KICKER
	aee_wdt_printf("kick=0x%08x,check=0x%08x",wd_api->wd_get_kick_bit(),wd_api->wd_get_check_bit());
    #endif 

     aee_wdt_fiq_info(arg, regs, svc_sp);
#if 0
     asm volatile("mov %0, %1\n\t"
                  "mov fp, %2\n\t"
                 : "=r" (sp)
                 : "r" (svc_sp), "r" (preg[11])
                 );

     *((volatile unsigned int*)(0x00000000)); /*                   */
#endif
}
#else //               
static irqreturn_t mtk_wdt_isr(int irq, void *dev_id)

{
    printk("fwq mtk_wdt_isr\n" );
    aee_wdt_irq_info();
    
#ifndef __USING_DUMMY_WDT_DRV__ /*                         */
	//                                   
	rgu_wdt_intr_has_trigger = 1;
	wdt_report_info () ;
	BUG();

#endif	
	return IRQ_HANDLED;
}
#endif //               

/* 
                    
 */
static int mtk_wdt_probe(struct platform_device *dev)
{
	int ret=0;
	unsigned int interval_val;
	
	printk("******** MTK WDT driver probe!! ********\n" );

	if(!toprgu_base)
	{	
	toprgu_base = of_iomap(dev->dev.of_node, 0);
	if (!toprgu_base) {
		printk("RGU iomap failed\n");
		return -ENODEV;
		}
	}

	if (msdc_base == NULL) {
		msdc_node = of_find_compatible_node(NULL, NULL, "mediatek,MSDC0");
		msdc_base = of_iomap(msdc_node, 0);
	}

	if(!wdt_irq_id)
	{	
	wdt_irq_id = irq_of_parse_and_map(dev->dev.of_node, 0);
	if (!wdt_irq_id) {
		printk("RGU get IRQ ID failed\n");
		return -ENODEV;
	}
	}
	printk("RGU base: 0x%p  RGU irq: %d\n", toprgu_base, wdt_irq_id);


#ifndef __USING_DUMMY_WDT_DRV__ /*                         */

#ifndef CONFIG_FIQ_GLUE	
    printk("******** MTK WDT register irq ********\n" );
		ret = request_irq(AP_RGU_WDT_IRQ_ID, (irq_handler_t)mtk_wdt_isr, IRQF_TRIGGER_FALLING, "mtk_watchdog", NULL);
#else
    printk("******** MTK WDT register fiq ********\n" );
		ret = request_fiq(AP_RGU_WDT_IRQ_ID, wdt_fiq, IRQF_TRIGGER_FALLING, NULL);
#endif	

    if(ret != 0)
	{
		printk( "mtk_wdt_probe : failed to request irq (%d)\n", ret);
		return ret;
	}
	printk("mtk_wdt_probe : Success to request irq\n");
	
	/*                                      */
	g_last_time_time_out_value=30;
	mtk_wdt_set_time_out_value(g_last_time_time_out_value);
		
	mtk_wdt_restart(WD_TYPE_NORMAL);

	/* 
                                                          
                                                                          
                                                                           
                                           
                                       
                                       
                                                                                      
  */
	#define POWER_OFF_ON_MAGIC	(0x3)
	#define PRE_LOADER_MAGIC	(0x0)
    #define U_BOOT_MAGIC		(0x1)
	#define KERNEL_MAGIC		(0x2)
	#define MAGIC_NUM_MASK		(0x3)


    #ifdef  CONFIG_MTK_WD_KICKER	//                        
	printk("mtk_wdt_probe : Initialize to dual mode \n");
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
	#else				//                          
	printk("mtk_wdt_probe : Initialize to disable wdt \n");
	mtk_wdt_mode_config(FALSE, FALSE, TRUE, FALSE, FALSE);
	g_wdt_enable =0;
	#endif


	/*                                                      */
	interval_val = DRV_Reg32(MTK_WDT_INTERVAL);
	interval_val &= ~(MAGIC_NUM_MASK);
	interval_val |= (KERNEL_MAGIC);
	/*                         */
	DRV_WriteReg32(MTK_WDT_INTERVAL, interval_val);
#endif
   udelay(100);
   printk("mtk_wdt_probe : done WDT_MODE(%x)\n",DRV_Reg32(MTK_WDT_MODE));
	 printk("mtk_wdt_probe : done MTK_WDT_REQ_MODE(%x)\n",DRV_Reg32(MTK_WDT_REQ_MODE));
	 printk("mtk_wdt_probe : done MTK_WDT_REQ_IRQ_EN(%x)\n",DRV_Reg32(MTK_WDT_REQ_IRQ_EN));

	return ret;
}

static int mtk_wdt_remove(struct platform_device *dev)
{
	printk("******** MTK wdt driver remove!! ********\n" );

#ifndef __USING_DUMMY_WDT_DRV__ /*                         */
	free_irq(AP_RGU_WDT_IRQ_ID, NULL);
#endif
	return 0;
}

static void mtk_wdt_shutdown(struct platform_device *dev)
{	
	printk("******** MTK WDT driver shutdown!! ********\n" );

	//                                                       
	//                 
	//                                                      

	mtk_wdt_restart(WD_TYPE_NORMAL);

   printk("******** MTK WDT driver shutdown done ********\n" );
}

void mtk_wd_suspend(void)
{
	//                                                       
	//                                             
	mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, FALSE);

	mtk_wdt_restart(WD_TYPE_NORMAL);

	aee_sram_printk("[WDT] suspend\n");
	printk("[WDT] suspend\n");
}

void mtk_wd_resume(void)
{
	
	if ( g_wdt_enable == 1 ) 
	{
		mtk_wdt_set_time_out_value(g_last_time_time_out_value);
		mtk_wdt_mode_config(TRUE, TRUE, TRUE, FALSE, TRUE);
		mtk_wdt_restart(WD_TYPE_NORMAL);
		
	}

	aee_sram_printk("[WDT] resume(%d)\n", g_wdt_enable);
	printk("[WDT] resume(%d)\n", g_wdt_enable);
}
	


static struct platform_driver mtk_wdt_driver =
{
	.driver     = {
		.name	= "mtk-wdt",
		.of_match_table = rgu_of_match,		
	},
	.probe	= mtk_wdt_probe,
	.remove	= mtk_wdt_remove,
	.shutdown	= mtk_wdt_shutdown,
//                            
//                          
};

/*
                         
 */
static int __init mtk_wdt_init(void)
{

	int ret;

	ret = platform_driver_register(&mtk_wdt_driver);
	if (ret) {
		printk("****[mtk_wdt_driver] Unable to register driver (%d)\n", ret);
		return ret;
	}
    printk("mtk_wdt_init ok\n");
	return 0;	
}

static void __exit mtk_wdt_exit (void)
{
}

arch_initcall(mtk_wdt_init);
//                          

MODULE_AUTHOR("MTK");
MODULE_DESCRIPTION("MT6582 Watchdog Device Driver");
MODULE_LICENSE("GPL");

