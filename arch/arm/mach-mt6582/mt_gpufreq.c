/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <linux/jiffies.h>
#ifdef GPU_CLOCK_RATIO
#include <linux/time.h>
#endif

#include <asm/system.h>
#include <asm/uaccess.h>

#include "mach/mt_typedefs.h"
#include "mach/mt_clkmgr.h"
#include "mach/mt_gpufreq.h"
#include "mach/upmu_common.h"
#include "mach/sync_write.h"

/*                          
               
                           */
#define dprintk(fmt, args...)                                           \
do {                                                                    \
    if (mt_gpufreq_debug) {                                             \
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", fmt, ##args);   \
    }                                                                   \
} while(0)

#ifdef CONFIG_HAS_EARLYSUSPEND
static struct early_suspend mt_gpufreq_early_suspend_handler =
{
    .level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 200,
    .suspend = NULL,
    .resume  = NULL,
};
#endif

/*                          
                        
                           */
static struct mt_gpufreq_power_info mt_gpufreqs_golden_power[] = {
    {.gpufreq_khz = GPU_DVFS_F2, .gpufreq_power = 761},
    {.gpufreq_khz = GPU_DVFS_F3, .gpufreq_power = 625},
    {.gpufreq_khz = GPU_DVFS_F4, .gpufreq_power = 581},
    {.gpufreq_khz = GPU_DVFS_F5, .gpufreq_power = 456},
    {.gpufreq_khz = GPU_DVFS_F7, .gpufreq_power = 414},
    {.gpufreq_khz = GPU_DVFS_F8, .gpufreq_power = 336},
};

/*                         
                       
                          */
static int g_gpufreq_dvfs_disable_count = 0;

static unsigned int g_cur_freq = 286000;
static unsigned int g_cur_volt = 0;
static unsigned int g_cur_load = 0;
 
static unsigned int g_cur_freq_init_keep = 0;
static unsigned int g_volt_set_init_step_1 = 0;
static unsigned int g_volt_set_init_step_2 = 0;
static unsigned int g_volt_set_init_step_3 = 0;
static unsigned int g_freq_new_init_keep = 0;
static unsigned int g_volt_new_init_keep = 0;

/*                                                                                         */
static unsigned int g_gpufreq_max_id = 0;

/*                                                                   */
static unsigned int g_limited_max_id;
static unsigned int g_limited_min_id;

static bool mt_gpufreq_debug = false;
static bool mt_gpufreq_pause = false;
static bool mt_gpufreq_keep_max_frequency = false;
static bool mt_gpufreq_keep_specific_frequency = false;
static unsigned int mt_gpufreq_fixed_frequency = 0;
static unsigned int mt_gpufreq_fixed_voltage = 0;

static DEFINE_SPINLOCK(mt_gpufreq_lock);

static unsigned int mt_gpufreqs_num = 0;
static struct mt_gpufreq_info *mt_gpufreqs;
static struct mt_gpufreq_power_info *mt_gpufreqs_power;
static struct mt_gpufreq_power_info *mt_gpufreqs_default_power;

static bool mt_gpufreq_registered = false;
static bool mt_gpufreq_registered_statewrite = false;
static bool mt_gpufreq_already_non_registered = false;

#ifdef GPU_CLOCK_RATIO
//                                             
//                                                

static int mt_gpufreq_clock_on = 0;
static unsigned int mt_gpufreq_cur_wall_time = 0;
static unsigned int mt_gpufreq_prev_wall_time = 0;
static unsigned int mt_gpufreq_period_time = 0;
static unsigned int mt_gpu_clock_on_time_start = 0;
static unsigned int mt_gpu_clock_total_on_time = 0;
#endif

static unsigned int mt_gpufreq_enable_mainpll = 0;
static unsigned int mt_gpufreq_enable_mmpll = 0;

/*                             
                             
                              */
extern int mtk_gpufreq_register(struct mt_gpufreq_power_info *freqs, int num);

/*                         
                         
                          */
static struct hrtimer mt_gpufreq_timer;
struct task_struct *mt_gpufreq_thread = NULL;
static DECLARE_WAIT_QUEUE_HEAD(mt_gpufreq_timer_waiter);

static int mt_gpufreq_sample_s = 0;
static int mt_gpufreq_sample_ns = 30000000; //     
static int mt_gpufreq_timer_flag = 0;

/*                             
                             
                              */
extern int pmic_get_gpu_status_bit_info(void);

/*                             
                     
                              */
enum hrtimer_restart mt_gpufreq_timer_func(struct hrtimer *timer);
static int mt_gpufreq_thread_handler(void *unused);


/*                                                                
                                                                        
                                                                */
static void mt_gpufreq_check_freq_and_set_pll(void)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_check_freq_and_set_pll, g_cur_freq = %d\n", g_cur_freq);
	
    switch (g_cur_freq)
    {
        case GPU_MMPLL_D3: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
            break;
			
        case GPU_SYSPLL_D2: //       
            if(mt_gpufreq_enable_mainpll == 0)
            {
                enable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 1;
            }
            break;
			
        case GPU_MMPLL_D4: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
            break;
			
        case GPU_UNIVPLL1_D2: //       
            break;
			
        case GPU_MMPLL_D5: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
            break;
			
        case GPU_SYSPLL_D3: //       
            if(mt_gpufreq_enable_mainpll == 0)
            {
                enable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 1;
            }
            break;
			
        case GPU_MMPLL_D6: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
            break;
			
        case GPU_UNIVPLL1_D4: //       
            break;
			
        default:
            break;
    }
}

/*                                     
                                      
                                      */
static int mt_gpufreq_keep_max_freq(unsigned int freq_old, unsigned int freq_new)
{
    if (mt_gpufreq_keep_max_frequency == true)
        return 1;

    return 0;
}

/*                                                                
                                       
                                                                */
bool mt_gpufreq_is_registered_get(void)
{
    if((mt_gpufreq_registered == true) || (mt_gpufreq_already_non_registered == true))
        return true;
    else
        return false;
}
EXPORT_SYMBOL(mt_gpufreq_is_registered_get);

#ifdef GPU_CLOCK_RATIO
/*                                                                
                                                                   
                                             
                                                                */
static void mt_gpufreq_period_time_reset(void)
{
    struct timeval t1;
    do_gettimeofday(&t1);

    mt_gpufreq_prev_wall_time = (unsigned int)t1.tv_usec;
	mt_gpu_clock_total_on_time = 0;
	mt_gpufreq_period_time = 0;

	xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_period_time_reset(), mt_gpufreq_prev_wall_time = %d\n", mt_gpufreq_prev_wall_time);
}

/*                                                                
                                           
                                                                */
unsigned int mt_gpufreq_gpu_clock_ratio(enum mt_gpufreq_clock_ratio act)
{
    unsigned long flags;
	unsigned int load = 0;
	struct timeval t2;
	
    spin_lock_irqsave(&mt_gpufreq_lock, flags);

    do_gettimeofday(&t2);
	
    if(act == GPU_DVFS_CLOCK_RATIO_ON && mt_gpufreq_clock_on == 0)
    {
        /*                                  */
        mt_gpufreq_clock_on = 1;
        mt_gpu_clock_on_time_start = (unsigned int)t2.tv_usec;
        dprintk("ON = %d\n", mt_gpu_clock_on_time_start);
    }
    else if(act == GPU_DVFS_CLOCK_RATIO_OFF && mt_gpufreq_clock_on == 1)
    {
        /*                                                                   */
        mt_gpufreq_clock_on = 0;
        mt_gpufreq_cur_wall_time = (unsigned int)t2.tv_usec;

        if(mt_gpufreq_cur_wall_time >= mt_gpu_clock_on_time_start)
            mt_gpu_clock_total_on_time += (mt_gpufreq_cur_wall_time - mt_gpu_clock_on_time_start);
        else
            mt_gpu_clock_total_on_time += ((GPU_COUNT_OVERFLOW - mt_gpu_clock_on_time_start) + mt_gpufreq_cur_wall_time);

        dprintk("OFF = %d, mt_gpu_clock_total_on_time = %d\n", mt_gpufreq_cur_wall_time, mt_gpu_clock_total_on_time);
    }
    else if(act == GPU_DVFS_CLOCK_RATIO_GET)
    {
        mt_gpufreq_cur_wall_time = (unsigned int)t2.tv_usec;
        dprintk("GET 1, mt_gpufreq_cur_wall_time = %d\n", mt_gpufreq_cur_wall_time);

        /*                                        */
        if(mt_gpufreq_clock_on == 1)
        {
            /*                                                                   */
            if(mt_gpufreq_cur_wall_time >= mt_gpu_clock_on_time_start)
                mt_gpu_clock_total_on_time += (mt_gpufreq_cur_wall_time - mt_gpu_clock_on_time_start);
            else
                mt_gpu_clock_total_on_time += ((GPU_COUNT_OVERFLOW - mt_gpu_clock_on_time_start) + mt_gpufreq_cur_wall_time);
			
            /*                                                 */
            mt_gpu_clock_on_time_start = mt_gpufreq_cur_wall_time;
			dprintk("GET, clock is still ON!\n");
        }

        /*                        */
        if(mt_gpufreq_cur_wall_time >= mt_gpufreq_prev_wall_time)
            mt_gpufreq_period_time = mt_gpufreq_cur_wall_time - mt_gpufreq_prev_wall_time;
        else
            mt_gpufreq_period_time = (GPU_COUNT_OVERFLOW - mt_gpufreq_prev_wall_time) + mt_gpufreq_cur_wall_time;
        dprintk("GET 2, mt_gpu_clock_on_time_start = %d, mt_gpu_clock_total_on_time = %d\n", mt_gpu_clock_on_time_start, mt_gpu_clock_total_on_time);
        dprintk("GET 3, mt_gpufreq_prev_wall_time = %d, mt_gpufreq_period_time = %d\n", mt_gpufreq_prev_wall_time, mt_gpufreq_period_time);
        mt_gpufreq_prev_wall_time = mt_gpufreq_cur_wall_time;
		
        /*                  */
        load = 100 * mt_gpu_clock_total_on_time / mt_gpufreq_period_time;
        mt_gpu_clock_total_on_time = 0;
        mt_gpufreq_period_time = 0;
        dprintk("GET 4, ratio = %d\n", load);
    }
	
	spin_unlock_irqrestore(&mt_gpufreq_lock, flags);

	return load;
}
EXPORT_SYMBOL(mt_gpufreq_gpu_clock_ratio);
#endif

/*                                                    */
static void mt_setup_gpufreqs_default_power_table(int num)
{
    int j = 0;

    mt_gpufreqs_default_power = kzalloc((1) * sizeof(struct mt_gpufreq_power_info), GFP_KERNEL);
    if (mt_gpufreqs_default_power == NULL)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "GPU default power table memory allocation fail\n");
        return;
    }

    mt_gpufreqs_default_power[0].gpufreq_khz = g_cur_freq;

    for (j = 0; j < ARRAY_SIZE(mt_gpufreqs_golden_power); j++)
    {
        if (g_cur_freq == mt_gpufreqs_golden_power[j].gpufreq_khz)
        {
            mt_gpufreqs_default_power[0].gpufreq_power = mt_gpufreqs_golden_power[j].gpufreq_power;
            break;
        }
    }

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_default_power[0].gpufreq_khz = %u\n", mt_gpufreqs_default_power[0].gpufreq_khz);
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_default_power[0].gpufreq_power = %u\n", mt_gpufreqs_default_power[0].gpufreq_power);

    #ifdef CONFIG_THERMAL
    mtk_gpufreq_register(mt_gpufreqs_default_power, 1);
    #endif
}

static void mt_setup_gpufreqs_power_table(int num)
{
    int i = 0, j = 0;

    mt_gpufreqs_power = kzalloc((num) * sizeof(struct mt_gpufreq_power_info), GFP_KERNEL);
    if (mt_gpufreqs_power == NULL)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "GPU power table memory allocation fail\n");
        return;
    }

    for (i = 0; i < num; i++) {
        mt_gpufreqs_power[i].gpufreq_khz = mt_gpufreqs[i].gpufreq_khz;

        for (j = 0; j < ARRAY_SIZE(mt_gpufreqs_golden_power); j++)
        {
            if (mt_gpufreqs[i].gpufreq_khz == mt_gpufreqs_golden_power[j].gpufreq_khz)
            {
                mt_gpufreqs_power[i].gpufreq_power = mt_gpufreqs_golden_power[j].gpufreq_power;
                break;
            }
        }

        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_power[%d].gpufreq_khz = %u\n", i, mt_gpufreqs_power[i].gpufreq_khz);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreqs_power[%d].gpufreq_power = %u\n", i, mt_gpufreqs_power[i].gpufreq_power);
    }

    #ifdef CONFIG_THERMAL
    mtk_gpufreq_register(mt_gpufreqs_power, num);
    #endif
}

/*                                              
                                               
                                               */
static int mt_setup_gpufreqs_table(struct mt_gpufreq_info *freqs, int num)
{
    int i = 0;

    mt_gpufreqs = kzalloc((num) * sizeof(*freqs), GFP_KERNEL);
    if (mt_gpufreqs == NULL)
        return -ENOMEM;

    for (i = 0; i < num; i++) {
        mt_gpufreqs[i].gpufreq_khz = freqs[i].gpufreq_khz;
        mt_gpufreqs[i].gpufreq_lower_bound = freqs[i].gpufreq_lower_bound;
        mt_gpufreqs[i].gpufreq_upper_bound = freqs[i].gpufreq_upper_bound;
        mt_gpufreqs[i].gpufreq_volt = freqs[i].gpufreq_volt;
        mt_gpufreqs[i].gpufreq_remap = freqs[i].gpufreq_remap;

        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[%d].gpufreq_khz = %u\n", i, freqs[i].gpufreq_khz);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[%d].gpufreq_lower_bound = %u\n", i, freqs[i].gpufreq_lower_bound);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[%d].gpufreq_upper_bound = %u\n", i, freqs[i].gpufreq_upper_bound);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[%d].gpufreq_volt = %u\n", i, freqs[i].gpufreq_volt);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[%d].gpufreq_remap = %u\n", i, freqs[i].gpufreq_remap);
    }

    mt_gpufreqs_num = num;

    /*                                                                           */
    #if 0// 
    g_cur_freq = freqs[0].gpufreq_khz;
    g_cur_volt = freqs[0].gpufreq_volt;
    #endif
    g_limited_max_id = 0;
    g_limited_min_id = mt_gpufreqs_num - 1;

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_setup_gpufreqs_table, g_cur_freq = %d, g_cur_volt = %d\n", g_cur_freq, g_cur_volt);
	
    mt_setup_gpufreqs_power_table(num);

    return 0;
}

/*                            
                     
                             */
int mt_gpufreq_state_set(int enabled)
{
    ktime_t ktime = ktime_set(mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

    if (enabled)
    {
        if (!mt_gpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "gpufreq already enabled\n");
            return 0;
        }

        /*                
                         
                         */
        g_gpufreq_dvfs_disable_count--;
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "enable GPU DVFS: g_gpufreq_dvfs_disable_count = %d\n", g_gpufreq_dvfs_disable_count);

        /*                                              
                                                       
                                                       */
        if (g_gpufreq_dvfs_disable_count <= 0)
        {
            mt_gpufreq_pause = false;
            hrtimer_start(&mt_gpufreq_timer, ktime, HRTIMER_MODE_REL);
            #ifdef GPU_CLOCK_RATIO
            mt_gpufreq_period_time_reset();
            #endif
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "someone still disable gpufreq, cannot enable it\n");
        }
    }
    else
    {
        /*                 
                          
                          */
        g_gpufreq_dvfs_disable_count++;
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "disable GPU DVFS: g_gpufreq_dvfs_disable_count = %d\n", g_gpufreq_dvfs_disable_count);

        if (mt_gpufreq_pause)
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "gpufreq already disabled\n");
            return 0;
        }

        mt_gpufreq_pause = true;
        hrtimer_cancel(&mt_gpufreq_timer);
    }

    return 0;
}
EXPORT_SYMBOL(mt_gpufreq_state_set);

static void mt_gpu_clock_switch(unsigned int sel)
{
    unsigned int clk_cfg_0 = 0, clk_cfg_4 = 0;

    clk_cfg_0 = DRV_Reg32(CLK_CFG_0);
    clk_cfg_4 = DRV_Reg32(CLK_CFG_4);

    switch (sel)
    {
        case GPU_MMPLL_D3: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
			
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x5 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_MMPLL_D3\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x5);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_MMPLL_D3\n");

            if(mt_gpufreq_enable_mainpll == 1)
            {
                disable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 0;
            }
            break;
        case GPU_SYSPLL_D2: //       
            if(mt_gpufreq_enable_mainpll == 0)
            {
                enable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 1;
            }
			
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x2 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_SYSPLL_D2\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x2);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_SYSPLL_D2\n");

            if(mt_gpufreq_enable_mmpll == 1)
            {
                disable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 0;
            }
            break;
        case GPU_MMPLL_D4: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
			
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x6 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_MMPLL_D4\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x6);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_MMPLL_D4\n");

            if(mt_gpufreq_enable_mainpll == 1)
            {
                disable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 0;
            }
            break;
        case GPU_UNIVPLL1_D2: //       
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x4 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_UNIVPLL1_D2\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x4);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_UNIVPLL1_D2\n");
			
            if(mt_gpufreq_enable_mainpll == 1)
            {
                disable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 0;
            }
			
            if(mt_gpufreq_enable_mmpll == 1)
            {
                disable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 0;
            }
            break;
        case GPU_MMPLL_D5: //       
           if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
			
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x7 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_MMPLL_D5\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x7);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_MMPLL_D5\n");

            if(mt_gpufreq_enable_mainpll == 1)
            {
                disable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 0;
            }
            break;
        case GPU_SYSPLL_D3: //       
            if(mt_gpufreq_enable_mainpll == 0)
            {
                enable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 1;
            }
			
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x3 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_SYSPLL_D3\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x7);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_MMPLL_D5\n");

            if(mt_gpufreq_enable_mmpll == 1)
            {
                disable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 0;
            }
            break;
        case GPU_MMPLL_D6: //       
            if(mt_gpufreq_enable_mmpll == 0)
            {
                enable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 1;
            }
			
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x1 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_MMPLL_D6\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x7);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_MMPLL_D5\n");

            if(mt_gpufreq_enable_mainpll == 1)
            {
                disable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 0;
            }
            break;
        case GPU_UNIVPLL1_D4: //       
            clk_cfg_0 = (clk_cfg_0 & 0xFFF8FFFF) | (0x0 << 16);
            mt65xx_reg_sync_writel(clk_cfg_0, CLK_CFG_0);
            dprintk("mt_gpu_clock_switch: switch MFG clock to GPU_UNIVPLL1_D4\n");

            clk_cfg_4 = (clk_cfg_4 & 0xFFFFFFF8) | (0x7);
            mt65xx_reg_sync_writel(clk_cfg_4, CLK_CFG_4);
            dprintk("mt_gpu_clock_switch: switch HYD clock to GPU_MMPLL_D5\n");

            if(mt_gpufreq_enable_mainpll == 1)
            {
                disable_pll(MAINPLL, "GPU_DVFS");
                mt_gpufreq_enable_mainpll = 0;
            }
			
            if(mt_gpufreq_enable_mmpll == 1)
            {
                disable_pll(MMPLL, "GPU_DVFS");
                mt_gpufreq_enable_mmpll = 0;
            }
            break;
        default:
            dprintk("mt_gpu_clock_switch: do not support specified clock (%d)\n", sel);
            break;
    }

    dprintk("CLK_CFG_0 = 0x%x, CLK_CFG_4 = 0x%x\n", DRV_Reg32(CLK_CFG_0), DRV_Reg32(CLK_CFG_4));
}

static void mt_gpu_volt_switch(unsigned int volt_old, unsigned int volt_new)
{
    int i = 0;

    upmu_set_vrf18_2_vosel_ctrl(0); //                

    if (volt_new == GPU_POWER_VRF18_1_15V)
    {
        dprintk("mt_gpu_volt_switch: switch MFG power to GPU_POWER_VRF18_1_15V\n");
    }
    else if (volt_new == GPU_POWER_VRF18_1_10V)
    {
        dprintk("mt_gpu_volt_switch: switch MFG power to GPU_POWER_VRF18_1_10V\n");
    }
    else if (volt_new == GPU_POWER_VRF18_1_05V)
    {
        dprintk("mt_gpu_volt_switch: switch MFG power to GPU_POWER_VRF18_1_05V\n");
    }
    else if (volt_new == GPU_POWER_VCORE_1_05V)
    {
        dprintk("mt_gpu_volt_switch: use VCORE, not support voltage scaling\n");
        return;
    }
    else
    {
        dprintk("mt_gpu_volt_switch: do not support specified volt (%d)\n", volt_new);
        return;
    }

    if (volt_old < volt_new)
    {
        for (i = ((int)volt_old + 1); i <= (int)volt_new; i++)
        {
            dprintk("mt_gpu_volt_switch: volt_old < volt_new, i=(%d)\n", i);
            upmu_set_vrf18_2_vosel(i);
            udelay(PMIC_SETTLE_TIME);
        }
    }
    else
    {
        for (i = ((int)volt_old - 1); i >= (int)volt_new ; i--)
        {
            dprintk("mt_gpu_volt_switch: volt_old > volt_new, i=(%d)\n", i);
            upmu_set_vrf18_2_vosel(i);
            udelay(PMIC_SETTLE_TIME);
        }
    }
}

static void mt_gpu_volt_switch_initial(unsigned int volt_new)
{
    upmu_set_vrf18_2_vosel_ctrl(0); //                

    if (volt_new == GPU_POWER_VRF18_1_15V)
    {
        upmu_set_vrf18_2_vosel(4);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpu_volt_switch_initial: switch MFG power to GPU_POWER_VRF18_1_15V\n");
    }
    else if (volt_new == GPU_POWER_VRF18_1_10V)
    {
        upmu_set_vrf18_2_vosel(2);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpu_volt_switch_initial: switch MFG power to GPU_POWER_VRF18_1_10V\n");
    }
    else if (volt_new == GPU_POWER_VRF18_1_05V)
    {
        upmu_set_vrf18_2_vosel(0);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpu_volt_switch_initial: switch MFG power to GPU_POWER_VRF18_1_05V\n");
    }
    else if (volt_new == GPU_POWER_VCORE_1_05V)
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpu_volt_switch_initial: use VCORE, not support voltage scaling\n");
        return;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpu_volt_switch_initial: do not support specified volt (%d)\n");
        return;
    }

}

/*                                                          
                                                                     
                                                            
                                                           */
void mt_gpufreq_set_initial(unsigned int freq_new, unsigned int volt_new)
{
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_set_initial, freq_new = %d, volt_new = %d, g_cur_freq = %d\n", freq_new, volt_new, g_cur_freq);
    g_freq_new_init_keep = freq_new;
    g_volt_new_init_keep = volt_new;

    mt_gpufreq_check_freq_and_set_pll();
		
    if(freq_new == g_cur_freq)
    {
        g_volt_set_init_step_1 = 1;
        return;
    }
	
    if (freq_new > g_cur_freq)
    {
        #ifdef MT_BUCK_ADJUST
        if (pmic_get_gpu_status_bit_info() == 0) //                     
        { 
            g_volt_set_init_step_2 = 1;
            mt_gpu_volt_switch_initial(volt_new);
            udelay(PMIC_SETTLE_TIME);
        }
        #endif

        mt_gpu_clock_switch(freq_new);
    }
    else
    {
        mt_gpu_clock_switch(freq_new);

        #ifdef MT_BUCK_ADJUST
        if (pmic_get_gpu_status_bit_info() == 0) //                     
        { 
            g_volt_set_init_step_3 = 1;
            mt_gpu_volt_switch_initial(volt_new);
        }
        #endif
    }

    g_cur_freq = freq_new;
    g_cur_volt = volt_new;
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_set_initial, g_cur_freq = %d, g_cur_volt = %d\n", g_cur_freq, g_cur_volt);
}
EXPORT_SYMBOL(mt_gpufreq_set_initial);


/*                                        
                                         
                                         */
/*                                                          
        
                                                  
                                                           
                                                           */
static void mt_gpufreq_set(unsigned int freq_old, unsigned int freq_new, unsigned int volt_old, unsigned int volt_new)
{
    if (freq_new > freq_old)
    {
        #ifdef MT_BUCK_ADJUST
        if (pmic_get_gpu_status_bit_info() == 0) //                     
        {
            if (volt_old != volt_new)
            {
                mt_gpu_volt_switch(volt_old, volt_new);
            }
        }
        #endif

        mt_gpu_clock_switch(freq_new);
    }
    else
    {
        mt_gpu_clock_switch(freq_new);

        #ifdef MT_BUCK_ADJUST
        if (pmic_get_gpu_status_bit_info() == 0) //                     
        {
            if (volt_old != volt_new)
            {
                mt_gpu_volt_switch(volt_old, volt_new);
            }
        }
        #endif
    }

    g_cur_freq = freq_new;
    g_cur_volt = volt_new;
}

static int mt_gpufreq_look_up(unsigned int load)
{
    int i = 0, remap = 100;

    /*                         
                             
                              */
    for (i = 0; i < mt_gpufreqs_num; i++)
    {
        if (mt_gpufreqs[i].gpufreq_khz == g_cur_freq)
        {
            remap = mt_gpufreqs[i].gpufreq_remap;
            break;
        }
    }

    load = (load * remap) / 100;
    g_cur_load = load;
    dprintk("GPU Loading = %d\n", load);

    /*                             
                                  
                                  */
    for (i = 0; i < mt_gpufreqs_num; i++)
    {
        if (load > mt_gpufreqs[i].gpufreq_lower_bound && load <= mt_gpufreqs[i].gpufreq_upper_bound)
        {
            return i;
        }
    }

    return (mt_gpufreqs_num - 1);
}

/*                                 
                                  
                                  */
/*                                                
        
                                    
                                                
                                                 */
static int mt_gpufreq_target(int idx)
{
    unsigned long flags, target_freq, target_volt;

    spin_lock_irqsave(&mt_gpufreq_lock, flags);

    /*                                 
                                    
                                      */
    target_freq = mt_gpufreqs[idx].gpufreq_khz;
    target_volt = mt_gpufreqs[idx].gpufreq_volt;

    /*                                 
                                         
                                      */
    if (mt_gpufreq_keep_max_freq(g_cur_freq, target_freq))
    {
        target_freq = mt_gpufreqs[g_gpufreq_max_id].gpufreq_khz;
        target_volt = mt_gpufreqs[g_gpufreq_max_id].gpufreq_volt;
        dprintk("Keep MAX frequency %d !\n", target_freq);
    }    

    /*                                                   
                                                        
                                                        */
    if(target_freq > g_cur_freq)
    {
        target_freq = mt_gpufreqs[g_gpufreq_max_id].gpufreq_khz;
        target_volt = mt_gpufreqs[g_gpufreq_max_id].gpufreq_volt;
        dprintk("Need to raise frequency, raise to MAX frequency %d !\n", target_freq);
    }
	
    if (target_freq > mt_gpufreqs[g_limited_max_id].gpufreq_khz)
    {
        /*                                            
                                                    
                                                     */
        target_freq = mt_gpufreqs[g_limited_max_id].gpufreq_khz;
        target_volt = mt_gpufreqs[g_limited_max_id].gpufreq_volt;
		dprintk("Limit! Target freq %d > Thermal limit frequency %d\n", target_freq, mt_gpufreqs[g_limited_max_id].gpufreq_khz);
    }

    /*                                               
                                         
                                                    */
    if(mt_gpufreq_keep_specific_frequency == true)
    {
        target_freq = mt_gpufreq_fixed_frequency;
        target_volt = mt_gpufreq_fixed_voltage;
        dprintk("Fixed! fixed frequency %d, fixed voltage %d\n", target_freq, target_volt);
    }
	
    /*                                               
                                                    
                                                    */
    if (g_cur_freq == target_freq)
    {
        spin_unlock_irqrestore(&mt_gpufreq_lock, flags);
        dprintk("GPU frequency from %d MHz to %d MHz (skipped) due to same frequency\n", g_cur_freq / 1000, target_freq / 1000);
        return 0;
    }

    dprintk("GPU current frequency %d MHz, target frequency %d MHz\n", g_cur_freq / 1000, target_freq / 1000);
	
    /*                             
                                 
                                  */
    mt_gpufreq_set(g_cur_freq, target_freq, g_cur_volt, target_volt);

    spin_unlock_irqrestore(&mt_gpufreq_lock, flags);

    return 0;
}

/*                                
                                 
                                 */
void mt_gpufreq_early_suspend(struct early_suspend *h)
{
    mt_gpufreq_state_set(0);

    if(mt_gpufreq_enable_mainpll == 1)
    {
        disable_pll(MAINPLL, "GPU_DVFS");
        mt_gpufreq_enable_mainpll = 0;
    }
	
    if(mt_gpufreq_enable_mmpll == 1)
    {
        disable_pll(MMPLL, "GPU_DVFS");
        mt_gpufreq_enable_mmpll = 0;
    }
}

/*                              
                               
                               */
void mt_gpufreq_late_resume(struct early_suspend *h)
{
    mt_gpufreq_check_freq_and_set_pll();
	
    mt_gpufreq_state_set(1);
}

/*                                               
                                                
                                                */
/*                                                     
                         
                                                      */
void mt_gpufreq_thermal_protect(unsigned int limited_power)
{
    int i = 0;
    unsigned int limited_freq = 0;

    if (mt_gpufreqs_num == 0)
        return;

    if (limited_power == 0)
    {
        g_limited_max_id = 0;
    }
    else
    {
        g_limited_max_id = mt_gpufreqs_num - 1;

        for (i = 0; i < ARRAY_SIZE(mt_gpufreqs_golden_power); i++)
        {
            if (mt_gpufreqs_golden_power[i].gpufreq_power <= limited_power)
            {
                limited_freq = mt_gpufreqs_golden_power[i].gpufreq_khz;
                break;
            }
        }

        for (i = 0; i < mt_gpufreqs_num; i++)
        {
            if (mt_gpufreqs[i].gpufreq_khz <= limited_freq)
            {
                g_limited_max_id = i;
                break;
            }
        }
    }

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "limit frequency upper bound to id = %d, frequency = %d\n", g_limited_max_id, mt_gpufreqs[g_limited_max_id].gpufreq_khz);
    mt_gpufreq_target(g_limited_max_id);

    return;
}
EXPORT_SYMBOL(mt_gpufreq_thermal_protect);

/*                                               
                              
                                                */
unsigned int mt_gpufreq_cur_freq(void)
{
    dprintk("current GPU frequency is %d MHz\n", g_cur_freq / 1000);
    return g_cur_freq;
}
EXPORT_SYMBOL(mt_gpufreq_cur_freq);

/*                                               
                            
                                                */
unsigned int mt_gpufreq_cur_load(void)
{
    dprintk("current GPU load is %d\n", g_cur_load);
    return g_cur_load;
}
EXPORT_SYMBOL(mt_gpufreq_cur_load);

/*                             
                              
                              */
static int mt_gpufreq_state_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (!mt_gpufreq_pause)
        p += sprintf(p, "GPU DVFS enabled\n");
    else
        p += sprintf(p, "GPU DVFS disabled\n");

    len = p - buf;
    return len;
}

/*                                       
                                        
                                        */
static ssize_t mt_gpufreq_state_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enabled = 0;

    /*                                                                 */
    if((mt_gpufreq_registered == false) && (mt_gpufreq_registered_statewrite == false))
    {
        mt_gpufreq_pause = true;
		
        mt_gpufreqs = kzalloc((1) * sizeof(struct mt_gpufreq_info), GFP_KERNEL);
        mt_gpufreqs[0].gpufreq_khz = g_cur_freq;
        mt_gpufreqs[0].gpufreq_lower_bound = 0;
        mt_gpufreqs[0].gpufreq_upper_bound = 100;
        mt_gpufreqs[0].gpufreq_volt = 0;
        mt_gpufreqs[0].gpufreq_remap = 100;
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[0].gpufreq_khz = %u\n", mt_gpufreqs[0].gpufreq_khz);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[0].gpufreq_lower_bound = %u\n", mt_gpufreqs[0].gpufreq_lower_bound);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[0].gpufreq_upper_bound = %u\n", mt_gpufreqs[0].gpufreq_upper_bound);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[0].gpufreq_volt = %u\n", mt_gpufreqs[0].gpufreq_volt);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "freqs[0].gpufreq_remap = %u\n", mt_gpufreqs[0].gpufreq_remap);
	
        mt_gpufreq_timer.function = mt_gpufreq_timer_func;
        mt_gpufreq_thread = kthread_run(mt_gpufreq_thread_handler, 0, "mt_gpufreq");
        if (IS_ERR(mt_gpufreq_thread))
        {
            printk("[%s]: failed to create mt_gpufreq thread\n", __FUNCTION__);
        }
        mt_gpufreq_registered_statewrite = true;
    }
	
    if (sscanf(buffer, "%d", &enabled) == 1)
    {
        if (enabled == 1)
        {
            mt_gpufreq_keep_max_frequency = false;
            mt_gpufreq_state_set(1);
        }
        else if (enabled == 0)
        {
            /*                                            */
            mt_gpufreq_keep_max_frequency = true;
            mt_gpufreq_target(g_gpufreq_max_id);
            mt_gpufreq_state_set(0);
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! argument should be \"1\" or \"0\"\n");
    }

    return count;
}

/*                           
                            
                            */
static int mt_gpufreq_limited_power_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "g_limited_max_id = %d, frequency = %d\n", g_limited_max_id, mt_gpufreqs[g_limited_max_id].gpufreq_khz);

    len = p - buf;
    return len;
}

/*                                 
                                   
                                  */
static ssize_t mt_gpufreq_limited_power_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int power = 0;

    if (sscanf(buffer, "%u", &power) == 1)
    {
        mt_gpufreq_thermal_protect(power);
        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! please provide the maximum limited power\n");
    }

    return -EINVAL;
}

/*                          
                           
                           */
static int mt_gpufreq_debug_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_gpufreq_debug)
        p += sprintf(p, "gpufreq debug enabled\n");
    else
        p += sprintf(p, "gpufreq debug disabled\n");

    len = p - buf;
    return len;
}

/*                      
                      
                       */
static ssize_t mt_gpufreq_debug_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int debug = 0;

    if (sscanf(buffer, "%d", &debug) == 1)
    {
        if (debug == 0) 
        {
            mt_gpufreq_debug = 0;
            return count;
        }
        else if (debug == 1)
        {
            mt_gpufreq_debug = 1;
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be 0 or 1 [0: disable, 1: enable]\n");
    }

    return -EINVAL;
}

/*                   
                    
                    */
static int mt_gpufreq_sampling_rate_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    p += sprintf(p, "get sampling rate = %d (s) %d (ns)\n", mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

    len = p - buf;
    return len;
}

/*                   
                   
                    */
static ssize_t mt_gpufreq_sampling_rate_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int len = 0, s = 0, ns = 0;
    char desc[32];

    len = (count < (sizeof(desc) - 1)) ? count : (sizeof(desc) - 1);
    if (copy_from_user(desc, buffer, len))
    {
        return 0;
    }
    desc[len] = '\0';

    if (sscanf(desc, "%d %d", &s, &ns) == 2)
    {
        printk("[%s]: set sampling rate = %d (s), %d (ns)\n", __FUNCTION__, s, ns);
        mt_gpufreq_sample_s = s;
        mt_gpufreq_sample_ns = ns;
        return count;
    }
    else
    {
        printk("[%s]: bad argument!! should be \"[s]\" or \"[ns]\"\n", __FUNCTION__);
    }

    return -EINVAL;
}

/*                   
                    
                    */
static int mt_gpufreq_opp_dump_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int i = 0, j = 0, len = 0;
    char *p = buf;

    for (i = 0; i < mt_gpufreqs_num; i++)
    {
        p += sprintf(p, "[%d] ", i);
        p += sprintf(p, "freq = %d, ", mt_gpufreqs[i].gpufreq_khz);
        p += sprintf(p, "lower_bound = %d, ", mt_gpufreqs[i].gpufreq_lower_bound);
        p += sprintf(p, "upper_bound = %d, ", mt_gpufreqs[i].gpufreq_upper_bound);
        p += sprintf(p, "volt = %d, ", mt_gpufreqs[i].gpufreq_volt);
        p += sprintf(p, "remap = %d, ", mt_gpufreqs[i].gpufreq_remap);

        for (j = 0; j < ARRAY_SIZE(mt_gpufreqs_golden_power); j++)
        {
            if (mt_gpufreqs_golden_power[j].gpufreq_khz == mt_gpufreqs[i].gpufreq_khz)
            {
                p += sprintf(p, "power = %d\n", mt_gpufreqs_golden_power[j].gpufreq_power);
                break;
            }
        }
    }

    len = p - buf;
    return len;
}

/*                          
                                        
                           */
static int mt_gpufreq_fixed_frequency_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    if (mt_gpufreq_keep_specific_frequency)
        p += sprintf(p, "gpufreq fixed frequency enabled\n");
    else
        p += sprintf(p, "gpufreq fixed frequency disabled\n");

    len = p - buf;
    return len;
}

/*                      
                           
                       */
static ssize_t mt_gpufreq_fixed_frequency_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    int enable = 0;
    int fixed_freq = 0;
    int fixed_volt = 0;

    if (sscanf(buffer, "%d %d %d", &enable, &fixed_freq, &fixed_volt) == 3)
    {
        if (enable == 0) 
        {
            mt_gpufreq_keep_specific_frequency = false;
            return count;
        }
        else if (enable == 1)
        {
            mt_gpufreq_keep_specific_frequency = true;
            mt_gpufreq_fixed_frequency = fixed_freq;
            mt_gpufreq_fixed_voltage = fixed_volt;
            return count;
        }
        else
        {
            xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be [enable fixed_freq fixed_volt]\n");
        }
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! should be [enable fixed_freq fixed_volt]\n");
    }
	
    return -EINVAL;
}

/*                   
                    
                    */
static int mt_gpufreq_var_dump(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *p = buf;

    unsigned int clk_cfg_0 = 0;
    clk_cfg_0 = DRV_Reg32(CLK_CFG_0);
    clk_cfg_0 = (clk_cfg_0 & 0x00070000) >> 16;
    p += sprintf(p, "clk_cfg_0 = %d\n", clk_cfg_0);
    p += sprintf(p, "pmic_get_gpu_status_bit_info() = %d\n", pmic_get_gpu_status_bit_info());
    p += sprintf(p, "mt_gpufreq_enable_mainpll = %d\n", mt_gpufreq_enable_mainpll);
    p += sprintf(p, "mt_gpufreq_enable_mmpll = %d\n", mt_gpufreq_enable_mmpll);
	
    p += sprintf(p, "g_cur_freq_init_keep = %d\n", g_cur_freq_init_keep);
    p += sprintf(p, "g_volt_set_init_step_1 = %d, g_volt_set_init_step_2 = %d, g_volt_set_init_step_3 = %d\n", g_volt_set_init_step_1, g_volt_set_init_step_2, g_volt_set_init_step_3);
    p += sprintf(p, "g_freq_new_init_keep = %d, g_volt_new_init_keep = %d\n", g_freq_new_init_keep, g_volt_new_init_keep);
    p += sprintf(p, "mt_gpufreq_registered = %d, mt_gpufreq_already_non_registered = %d\n", mt_gpufreq_registered, mt_gpufreq_already_non_registered);
	
    len = p - buf;
    return len;
}

#ifdef GPU_CLOCK_RATIO
/*                                 
                            
                                  */
static ssize_t mt_gpufreq_clock_on_ratio_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    unsigned int delay = 0;

    if (sscanf(buffer, "%u", &delay) == 1)
    {
        mt_gpufreq_gpu_clock_ratio(GPU_DVFS_CLOCK_RATIO_ON);
		mdelay(delay);
		mt_gpufreq_gpu_clock_ratio(GPU_DVFS_CLOCK_RATIO_OFF);

        return count;
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "bad argument!! please provide proper mdelay time\n");
    }

    return -EINVAL;
}
#endif

enum hrtimer_restart mt_gpufreq_timer_func(struct hrtimer *timer)
{
    mt_gpufreq_timer_flag = 1; wake_up_interruptible(&mt_gpufreq_timer_waiter);

    return HRTIMER_NORESTART;
}

static int mt_gpufreq_thread_handler(void *unused)
{
    int idx = 0;
    unsigned int load = 0;

    do
    {
        ktime_t ktime = ktime_set(mt_gpufreq_sample_s, mt_gpufreq_sample_ns);

        wait_event_interruptible(mt_gpufreq_timer_waiter, mt_gpufreq_timer_flag != 0);
        mt_gpufreq_timer_flag = 0;

        /*                                 
                         
                                          */
        #ifdef GPU_CLOCK_RATIO
        load = mt_gpufreq_gpu_clock_ratio(GPU_DVFS_CLOCK_RATIO_GET);
        #endif

        idx = mt_gpufreq_look_up(load);

        mt_gpufreq_target(idx);

        if(mt_gpufreq_pause == false)
            hrtimer_start(&mt_gpufreq_timer, ktime, HRTIMER_MODE_REL);

    } while (!kthread_should_stop());

    return 0;
}

/*                                
                               
                                 */
int mt_gpufreq_register(struct mt_gpufreq_info *freqs, int num)
{
    if(mt_gpufreq_registered == false)
    {
        ktime_t ktime = ktime_set(mt_gpufreq_sample_s, mt_gpufreq_sample_ns);
    
        mt_gpufreq_registered = true;
    	
        #ifdef CONFIG_HAS_EARLYSUSPEND
        mt_gpufreq_early_suspend_handler.suspend = mt_gpufreq_early_suspend;
        mt_gpufreq_early_suspend_handler.resume = mt_gpufreq_late_resume;
        register_early_suspend(&mt_gpufreq_early_suspend_handler);
        #endif
    
        /*                     
                             
                              */
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "setup gpufreqs table\n");
    
        mt_setup_gpufreqs_table(freqs, num);
    
        /*                                   
                                            
                                            */
        hrtimer_init(&mt_gpufreq_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        mt_gpufreq_timer.function = mt_gpufreq_timer_func;
    
        mt_gpufreq_thread = kthread_run(mt_gpufreq_thread_handler, 0, "mt_gpufreq");
        if (IS_ERR(mt_gpufreq_thread))
        {
            printk("[%s]: failed to create mt_gpufreq thread\n", __FUNCTION__);
        }

        #ifdef GPU_DVFS_DEFAULT_DISABLED
        mt_gpufreq_state_set(0);
        #endif
	
        if(mt_gpufreq_pause == false)
        {
            hrtimer_start(&mt_gpufreq_timer, ktime, HRTIMER_MODE_REL);
            #ifdef GPU_CLOCK_RATIO
            mt_gpufreq_period_time_reset();
            #endif
        }

        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mediatek gpufreq registration done\n");
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mediatek gpufreq already registered !\n");
    }
    return 0;
}
EXPORT_SYMBOL(mt_gpufreq_register);

/*                                
                                   
                                 */
int mt_gpufreq_non_register(void)
{
    if(mt_gpufreq_already_non_registered == false)
    {
        mt_gpufreq_already_non_registered = true;
        mt_setup_gpufreqs_default_power_table(1);
		
        hrtimer_init(&mt_gpufreq_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        mt_gpufreq_state_set(0);
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_non_register() done\n");
    }
    else
    {
        xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_non_register() already called !\n");
    }
    return 0;
}
EXPORT_SYMBOL(mt_gpufreq_non_register);

/*                                 
                                 
                                  */
static int __init mt_gpufreq_init(void)
{
    struct proc_dir_entry *mt_entry = NULL;
    struct proc_dir_entry *mt_gpufreq_dir = NULL;
    unsigned int clk_cfg_0 = 0;

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_init\n");
		
    mt_gpufreq_dir = proc_mkdir("gpufreq", NULL);
    if (!mt_gpufreq_dir)
    {
        pr_err("[%s]: mkdir /proc/gpufreq failed\n", __FUNCTION__);
    }
    else
    {
        mt_entry = create_proc_entry("gpufreq_debug", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_debug_read;
            mt_entry->write_proc = mt_gpufreq_debug_write;
        }

        mt_entry = create_proc_entry("gpufreq_limited_power", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_limited_power_read;
            mt_entry->write_proc = mt_gpufreq_limited_power_write;
        }

        mt_entry = create_proc_entry("gpufreq_state", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_state_read;
            mt_entry->write_proc = mt_gpufreq_state_write;
        }

        mt_entry = create_proc_entry("gpufreq_sampling_rate", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_sampling_rate_read;
            mt_entry->write_proc = mt_gpufreq_sampling_rate_write;
        }

        mt_entry = create_proc_entry("gpufreq_opp_dump", S_IRUGO, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_opp_dump_read;
        }
	
        #ifdef GPU_CLOCK_RATIO
        mt_entry = create_proc_entry("gpufreq_clock_on", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->write_proc = mt_gpufreq_clock_on_ratio_write;
        }
        #endif

        mt_entry = create_proc_entry("gpufreq_fix_frequency", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_fixed_frequency_read;
            mt_entry->write_proc = mt_gpufreq_fixed_frequency_write;
        }

        mt_entry = create_proc_entry("gpufreq_var_dump", S_IRUGO | S_IWUSR | S_IWGRP, mt_gpufreq_dir);
        if (mt_entry)
        {
            mt_entry->read_proc = mt_gpufreq_var_dump;
        }
    }
	
    clk_cfg_0 = DRV_Reg32(CLK_CFG_0);
    clk_cfg_0 = (clk_cfg_0 & 0x00070000) >> 16;

    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_init, clk_cfg_0 = %d\n", clk_cfg_0);
	
    switch (clk_cfg_0)
    {
        case 0x5: //       
            g_cur_freq = GPU_MMPLL_D3;
            break;
        case 0x2: //       
            g_cur_freq = GPU_SYSPLL_D2;
            break;
        case 0x6: //       
            g_cur_freq = GPU_MMPLL_D4;
            break;
        case 0x4: //       
            g_cur_freq = GPU_UNIVPLL1_D2;
            break;
        case 0x7: //       
            g_cur_freq = GPU_MMPLL_D5;
            break;
        case 0x3: //       
            g_cur_freq = GPU_SYSPLL_D3;
            break;
        case 0x1: //       
            g_cur_freq = GPU_MMPLL_D6;
            break;
        case 0x0: //       
            g_cur_freq = GPU_UNIVPLL1_D4;
            break;
        default:
            break;
    }

    g_cur_freq_init_keep = g_cur_freq;
    xlog_printk(ANDROID_LOG_INFO, "Power/GPU_DVFS", "mt_gpufreq_init, g_cur_freq_init_keep = %d\n", g_cur_freq_init_keep);

    return 0;
}

static void __exit mt_gpufreq_exit(void)
{

}

module_init(mt_gpufreq_init);
module_exit(mt_gpufreq_exit);

MODULE_DESCRIPTION("MediaTek GPU Frequency Scaling driver");
MODULE_LICENSE("GPL");