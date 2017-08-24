#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <mach/mt_spm_sleep.h>
#include <mach/mt_gpio.h>
#include <mach/mt_clkbuf_ctl.h>
#include <mach/mt_clkmgr.h>

#include <mach/upmu_common.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>

#include "ccci_core.h"
#include "ccci_platform.h"
#include "modem_cldma.h"
#include "cldma_platform.h"
#include "cldma_reg.h"
#include "modem_reg_base.h"

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

extern unsigned long infra_ao_base;
extern void ccci_mem_dump(int md_id, void *start_addr, int len);

#define TAG "mcd"

int md_cd_get_modem_hw_info(struct platform_device *dev_ptr, struct ccci_dev_cfg *dev_cfg, struct md_hw_info *hw_info)
{
    struct device_node *node=NULL;
    memset(dev_cfg, 0, sizeof(struct ccci_dev_cfg));
    memset(hw_info, 0, sizeof(struct md_hw_info));

    #ifdef CONFIG_OF
    if(dev_ptr->dev.of_node == NULL) {
        CCCI_ERR_MSG(dev_cfg->index, TAG, "modem OF node NULL\n");
        return -1;
    }

    of_property_read_u32(dev_ptr->dev.of_node, "cell-index", &dev_cfg->index);
    CCCI_INF_MSG(dev_cfg->index, TAG, "modem hw info get idx:%d\n", dev_cfg->index);
    if(!get_modem_is_enabled(dev_cfg->index)) {
        CCCI_ERR_MSG(dev_cfg->index, TAG, "modem %d not enable, exit\n", dev_cfg->index + 1);
        return -1;
    }
    #else
    struct ccci_dev_cfg* dev_cfg_ptr = (struct ccci_dev_cfg*)dev->dev.platform_data;
    dev_cfg->index = dev_cfg_ptr->index;

    CCCI_INF_MSG(dev_cfg->index, TAG, "modem hw info get idx:%d\n", dev_cfg->index);
    if(!get_modem_is_enabled(dev_cfg->index)) {
        CCCI_ERR_MSG(dev_cfg->index, TAG, "modem %d not enable, exit\n", dev_cfg->index + 1);
        return -1;
    }
    #endif

    switch(dev_cfg->index) {
    case 0: //       
        #ifdef CONFIG_OF
        of_property_read_u32(dev_ptr->dev.of_node, "cldma,major", &dev_cfg->major);
        of_property_read_u32(dev_ptr->dev.of_node, "cldma,minor_base", &dev_cfg->minor_base);
        of_property_read_u32(dev_ptr->dev.of_node, "cldma,capability", &dev_cfg->capability);

        hw_info->cldma_ap_base = of_iomap(dev_ptr->dev.of_node, 0);
        hw_info->cldma_md_base = of_iomap(dev_ptr->dev.of_node, 1);
        hw_info->ap_ccif_base = of_iomap(dev_ptr->dev.of_node, 2);
        //                                                     
        node = of_find_compatible_node(NULL, NULL, "mediatek,MD_CCIF0");
        hw_info->md_ccif_base = of_iomap(node, 0);

        hw_info->cldma_irq_id = irq_of_parse_and_map(dev_ptr->dev.of_node, 0);
        hw_info->ap_ccif_irq_id = irq_of_parse_and_map(dev_ptr->dev.of_node, 1);
        hw_info->md_wdt_irq_id = irq_of_parse_and_map(dev_ptr->dev.of_node, 2);
        //                                      

        //                                                                                           
        hw_info->cldma_irq_flags = IRQF_TRIGGER_NONE;
        hw_info->ap_ccif_irq_flags = IRQF_TRIGGER_NONE;
        hw_info->md_wdt_irq_flags = IRQF_TRIGGER_NONE;
        hw_info->ap2md_bus_timeout_irq_flags = IRQF_TRIGGER_NONE;
        #else
        dev_cfg->major = dev_cfg_ptr->major;
        dev_cfg->minor_base = dev_cfg_ptr->minor_base;
        dev_cfg->capability = dev_cfg_ptr->capability;

        hw_info->cldma_ap_base = CLDMA_AP_BASE;
        hw_info->cldma_md_base = CLDMA_MD_BASE;
        hw_info->ap_ccif_base = AP_CCIF0_BASE;
        hw_info->md_ccif_base = hw_info->ap_ccif_base+0x1000;

        hw_info->cldma_irq_id = CLDMA_AP_IRQ;
        hw_info->ap_ccif_irq_id = CCIF0_AP_IRQ;
        hw_info->md_wdt_irq_id = MD_WDT_IRQ;
        //                                                          

        //                                                                                           
        hw_info->cldma_irq_flags = IRQF_TRIGGER_HIGH;
        hw_info->ap_ccif_irq_flags = IRQF_TRIGGER_LOW;
        hw_info->md_wdt_irq_flags = IRQF_TRIGGER_FALLING;
        //                                                         
        #endif

        hw_info->sram_size = CCIF_SRAM_SIZE;
        hw_info->md_rgu_base = MD_RGU_BASE;
        hw_info->md_boot_slave_Vector = MD_BOOT_VECTOR;
        hw_info->md_boot_slave_Key = MD_BOOT_VECTOR_KEY;
        hw_info->md_boot_slave_En = MD_BOOT_VECTOR_EN;
        
        break;
    default:
        return -1;
    }

    CCCI_INF_MSG(dev_cfg->index, TAG, "modem cldma of node get dev_major:%d\n", dev_cfg->major);
    CCCI_INF_MSG(dev_cfg->index, TAG, "modem cldma of node get minor_base:%d\n", dev_cfg->minor_base);
    CCCI_INF_MSG(dev_cfg->index, TAG, "modem cldma of node get capability:%d\n", dev_cfg->capability);

    CCCI_INF_MSG(dev_cfg->index, TAG, "ap_cldma_base:0x%p\n", (void*)hw_info->cldma_ap_base);
    CCCI_INF_MSG(dev_cfg->index, TAG, "md_cldma_base:0x%p\n",(void*) hw_info->cldma_md_base);
    CCCI_INF_MSG(dev_cfg->index, TAG, "ap_ccif_base:0x%p\n",(void*) hw_info->ap_ccif_base);
    CCCI_INF_MSG(dev_cfg->index, TAG, "cldma_irq_id:%d\n", hw_info->cldma_irq_id);
    CCCI_INF_MSG(dev_cfg->index, TAG, "ccif_irq_id:%d\n", hw_info->ap_ccif_irq_id);
    CCCI_INF_MSG(dev_cfg->index, TAG, "md_wdt_irq_id:%d\n", hw_info->md_wdt_irq_id);

    return 0;
}

int md_cd_io_remap_md_side_register(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;

	//                                                                                              
	//                                                                                              
	md_ctrl->cldma_ap_base = (void __iomem *)(md_ctrl->hw_info->cldma_ap_base);
	md_ctrl->cldma_md_base = (void __iomem *)(md_ctrl->hw_info->cldma_md_base);
	md_ctrl->md_boot_slave_Vector = ioremap_nocache(md_ctrl->hw_info->md_boot_slave_Vector, 0x4);
	md_ctrl->md_boot_slave_Key = ioremap_nocache(md_ctrl->hw_info->md_boot_slave_Key, 0x4);
	md_ctrl->md_boot_slave_En = ioremap_nocache(md_ctrl->hw_info->md_boot_slave_En, 0x4);
	md_ctrl->md_rgu_base = ioremap_nocache(md_ctrl->hw_info->md_rgu_base, 0x40);
	md_ctrl->md_global_con0 = ioremap_nocache(MD_GLOBAL_CON0, 0x4);

	md_ctrl->md_bus_status = ioremap_nocache(MD_BUS_STATUS_BASE, MD_BUS_STATUS_LENGTH);
	md_ctrl->md_pc_monitor = ioremap_nocache(MD_PC_MONITOR_BASE, MD_PC_MONITOR_LENGTH);
	md_ctrl->md_topsm_status = ioremap_nocache(MD_TOPSM_STATUS_BASE, MD_TOPSM_STATUS_LENGTH);
	md_ctrl->md_ost_status = ioremap_nocache(MD_OST_STATUS_BASE, MD_OST_STATUS_LENGTH);
#ifdef MD_PEER_WAKEUP
	md_ctrl->md_peer_wakeup = ioremap_nocache(MD_PEER_WAKEUP, 0x4);
#endif
	return 0;
}

void md_cd_dump_debug_register(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	
        md_cd_lock_cldma_clock_src(1);
	CCCI_INF_MSG(md->index, TAG, "Dump MD Bus status %x\n", MD_BUS_STATUS_BASE);
	ccci_mem_dump(md->index,md_ctrl->md_bus_status, MD_BUS_STATUS_LENGTH);
	CCCI_INF_MSG(md->index, TAG, "Dump MD PC monitor %x\n", MD_PC_MONITOR_BASE);
	ccci_write32(md_ctrl->md_pc_monitor, 0, 0x80000000); //              
	ccci_mem_dump(md->index, md_ctrl->md_pc_monitor, MD_PC_MONITOR_LENGTH);
	ccci_write32(md_ctrl->md_pc_monitor, 0, 0x1); //                 
	CCCI_INF_MSG(md->index, TAG, "Dump MD TOPSM status %x\n", MD_TOPSM_STATUS_BASE);
	ccci_mem_dump(md->index, md_ctrl->md_topsm_status, MD_TOPSM_STATUS_LENGTH);
	CCCI_INF_MSG(md->index, TAG, "Dump MD OST status %x\n", MD_OST_STATUS_BASE);
	ccci_mem_dump(md->index, md_ctrl->md_ost_status, MD_OST_STATUS_LENGTH);
        md_cd_lock_cldma_clock_src(0);
}

void md_cd_check_emi_state(struct ccci_modem *md, int polling)
{
}

int md_cd_power_on(struct ccci_modem *md)
{
    int ret = 0;
    unsigned int reg_value;
    struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
    //             
#ifdef FEATURE_VLTE_SUPPORT    
    if(!(mt6325_upmu_get_swcid()==PMIC6325_E1_CID_CODE ||
         mt6325_upmu_get_swcid()==PMIC6325_E2_CID_CODE))
    {
    CCCI_INF_MSG(md->index, CORE, "md_cd_power_on:set VLTE on,bit0,1\n");
    pmic_config_interface(0x0638, 0x1, 0x1, 0); //             
    udelay(200);
    //                                                  
    reg_value = ccci_read32(infra_ao_base,0xF00); 
    reg_value &= ~(0x10000); //             
    //                                                  
    ccci_write32(infra_ao_base,0xF00,reg_value);
    CCCI_INF_MSG(md->index, CORE, "md_cd_power_on: set infra_misc VLTE bit(0x1000_1F00)=0x%x, bit(16)=0x%x\n",ccci_read32(infra_ao_base,0xF00),(ccci_read32(infra_ao_base,0xF00)&0x10000));
    }
#endif
    //                   
    mutex_lock(&clk_buf_ctrl_lock); //                                            
    CCCI_INF_MSG(md->index, TAG, "clock buffer, BSI mode ignore\n"); 

    mt_set_gpio_mode(GPIO_RFIC0_BSI_CK,  GPIO_MODE_01); 
    mt_set_gpio_mode(GPIO_RFIC0_BSI_D0,  GPIO_MODE_01);
    mt_set_gpio_mode(GPIO_RFIC0_BSI_D1,  GPIO_MODE_01);
    mt_set_gpio_mode(GPIO_RFIC0_BSI_D2,  GPIO_MODE_01);
    mt_set_gpio_mode(GPIO_RFIC0_BSI_CS,  GPIO_MODE_01);
	//                                
    switch(md->index)
    {
        case MD_SYS1:
       	CCCI_INF_MSG(md->index, TAG, "Call start md_power_on()\n"); 
        ret = md_power_on(SYS_MD1);
        CCCI_INF_MSG(md->index, TAG, "Call end md_power_on() ret=%d\n",ret); 
        break;
    } 
	mutex_unlock(&clk_buf_ctrl_lock); //                       
	if(ret)
		return ret;
	//               
	cldma_write32(md_ctrl->md_rgu_base, WDT_MD_MODE, WDT_MD_MODE_KEY);
	return 0;
}

int md_cd_bootup_cleanup(struct ccci_modem *md, int success)
{
	int ret = 0;
#if 0    
	//                     
	if(success) {
		CCCI_INF_MSG(md->index, TAG, "clock buffer, GPIO mode\n"); 
	    mt_set_gpio_mode(GPIO_RFIC0_BSI_CK,  GPIO_MODE_GPIO); 
	    mt_set_gpio_mode(GPIO_RFIC0_BSI_D0,  GPIO_MODE_GPIO);
	    mt_set_gpio_mode(GPIO_RFIC0_BSI_D1,  GPIO_MODE_GPIO);
	    mt_set_gpio_mode(GPIO_RFIC0_BSI_D2,  GPIO_MODE_GPIO);
	    mt_set_gpio_mode(GPIO_RFIC0_BSI_CS,  GPIO_MODE_GPIO);
	} else {
		CCCI_INF_MSG(md->index, TAG, "clock buffer, unlock when bootup fail\n"); 
	}
	up(&clk_buf_ctrl_lock_2); //                                 
#endif
	return ret;
}

int md_cd_let_md_go(struct ccci_modem *md)
{
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	if(MD_IN_DEBUG(md))
		return -1;
	CCCI_INF_MSG(md->index, TAG, "set MD boot slave\n"); 
	//                                          
	cldma_write32(md_ctrl->md_boot_slave_Key, 0, 0x3567C766); //                              
	cldma_write32(md_ctrl->md_boot_slave_Vector, 0, 0x00000000); //                                                
	cldma_write32(md_ctrl->md_boot_slave_En, 0, 0xA3B66175); //                             
	return 0;
}

int md_cd_power_off(struct ccci_modem *md, unsigned int timeout)
{
    int ret = 0;
    unsigned int reg_value;
    mutex_lock(&clk_buf_ctrl_lock); //                                            
    //                                 
    switch(md->index)
    {
        case MD_SYS1:
        ret = md_power_off(SYS_MD1, timeout);
        break;
    }
    //                     
    CCCI_INF_MSG(md->index, TAG, "clock buffer, GPIO mode\n"); 
    mt_set_gpio_mode(GPIO_RFIC0_BSI_CK,  GPIO_MODE_GPIO); 
    mt_set_gpio_mode(GPIO_RFIC0_BSI_D0,  GPIO_MODE_GPIO);
    mt_set_gpio_mode(GPIO_RFIC0_BSI_D1,  GPIO_MODE_GPIO);
    mt_set_gpio_mode(GPIO_RFIC0_BSI_D2,  GPIO_MODE_GPIO);
    mt_set_gpio_mode(GPIO_RFIC0_BSI_CS,  GPIO_MODE_GPIO);
	mutex_unlock(&clk_buf_ctrl_lock); //                                          
	
#ifdef FEATURE_VLTE_SUPPORT
    //              
    if(!(mt6325_upmu_get_swcid()==PMIC6325_E1_CID_CODE ||
         mt6325_upmu_get_swcid()==PMIC6325_E2_CID_CODE))
    {    
    //                                                  
    reg_value = ccci_read32(infra_ao_base,0xF00); 
    reg_value &= ~(0x10000); //             
    reg_value |= 0x10000;//             
    //                                                  
    ccci_write32(infra_ao_base,0xF00,reg_value);
    CCCI_INF_MSG(md->index, CORE, "md_cd_power_off: set SRCLKEN infra_misc(0x1000_1F00)=0x%x, bit(16)=0x%x\n",ccci_read32(infra_ao_base,0xF00),(ccci_read32(infra_ao_base,0xF00)&0x10000));
    
    CCCI_INF_MSG(md->index, CORE, "md_cd_power_off:set VLTE on,bit0=0\n");
    pmic_config_interface(0x0638, 0x0, 0x1, 0); //            
    }
#endif
    return ret;
}

void md_cd_lock_cldma_clock_src(int locked)
{
	spm_ap_mdsrc_req(locked);
}

int ccci_modem_remove(struct platform_device *dev)
{
	return 0;
}

void ccci_modem_shutdown(struct platform_device *dev)
{
}

int ccci_modem_suspend(struct platform_device *dev, pm_message_t state)
{
	struct ccci_modem *md = (struct ccci_modem *)dev->dev.platform_data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;

	CCCI_INF_MSG(md->index, TAG, "AP_BUSY(%p)=%x\n", md_ctrl->ap_ccif_base+APCCIF_BUSY, cldma_read32(md_ctrl->ap_ccif_base, APCCIF_BUSY));
	CCCI_INF_MSG(md->index, TAG, "MD_BUSY(%p)=%x\n", md_ctrl->md_ccif_base+APCCIF_BUSY, cldma_read32(md_ctrl->md_ccif_base, APCCIF_BUSY));

	return 0;
}

int ccci_modem_resume(struct platform_device *dev)
{
	struct ccci_modem *md = (struct ccci_modem *)dev->dev.platform_data;
	struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	cldma_write32(md_ctrl->ap_ccif_base, APCCIF_CON, 0x01); //            
	return 0;
}

int ccci_modem_pm_suspend(struct device *device)
{
    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    return ccci_modem_suspend(pdev, PMSG_SUSPEND);
}

int ccci_modem_pm_resume(struct device *device)
{
    struct platform_device *pdev = to_platform_device(device);
    BUG_ON(pdev == NULL);

    return ccci_modem_resume(pdev);
}

int ccci_modem_pm_restore_noirq(struct device *device)
{
	struct ccci_modem *md = (struct ccci_modem *)device->platform_data;
    struct md_cd_ctrl *md_ctrl = (struct md_cd_ctrl *)md->private_data;
	//      
    //            
#ifdef FEATURE_PM_IPO_H
    irq_set_irq_type(md_ctrl->cldma_irq_id, IRQF_TRIGGER_HIGH); 
    irq_set_irq_type(md_ctrl->md_wdt_irq_id, IRQF_TRIGGER_FALLING); 
#endif
	//                           
	md->config.setting |= MD_SETTING_RELOAD;
	md->config.setting |= MD_SETTING_FIRST_BOOT;
    return 0;
}

