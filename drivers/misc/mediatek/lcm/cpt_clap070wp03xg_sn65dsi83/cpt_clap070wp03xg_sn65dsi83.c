#ifndef BUILD_LK
#include <linux/string.h>
#else
#include <string.h>
#endif 

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#elif (defined BUILD_UBOOT)
#include <asm/arch/mt6577_gpio.h>
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include "sn65dsi83_i2c.h"
#endif
#include "lcm_drv.h"


#define LVDS_PANEL_8BITS_SUPPORT
//                                                                            
//                 
//                                                                            

#define FRAME_WIDTH  (800)
#define FRAME_HEIGHT (1280) 

//                
#ifdef GPIO_LCM_PWR_EN
#define GPIO_LCD_PWR_EN      GPIO_LCM_PWR_EN
#else
#define GPIO_LCD_PWR_EN      0xFFFFFFFF
#endif

//                                        
#ifdef GPIO_LCM_PWR2_EN
#define GPIO_LCD_PWR2_EN      GPIO_LCM_PWR2_EN
#else
#define GPIO_LCD_PWR2_EN      0xFFFFFFFF
#endif

//                             
#ifdef GPIO_LCM_RST
#define GPIO_LCD_RST_EN      	GPIO_LCM_RST
#else
#define GPIO_LCD_RST_EN      	0xFFFFFFFF
#endif

//                                      
#ifdef GPIO_LCM_BRIDGE_EN
#define GPIO_LCD_BRIDGE_EN		GPIO_LCM_BRIDGE_EN
#else
#define GPIO_LCD_BRIDGE_EN		0xFFFFFFFF
#endif

//                                     
#ifdef GPIO_LCM_BL_EN
#define GPIO_LCD_BL_EN      	GPIO_LCM_BL_EN
#else
#define GPIO_LCD_BL_EN				0xFFFFFFFF
#endif

#ifdef GPIO_LCM_STB
#define GPIO_LCD_STB_EN      	GPIO_LCM_STB
#else
#define GPIO_LCD_STB_EN      	0xFFFFFFFF
#endif

#ifdef GPIO_LCM_LVL_SHIFT_EN
#define GPIO_SHIFT_EN      		GPIO_LCM_LVL_SHIFT_EN
#else
#define GPIO_SHIFT_EN      		0xFFFFFFFF
#endif

//                                                                    
//                                                                            
//                 
//                                                                            
static LCM_UTIL_FUNCS lcm_util = 
{
	.set_reset_pin	= NULL,
	.udelay 				= NULL,
	.mdelay 				= NULL,
};


#define SET_RESET_PIN(v)    (mt_set_reset_pin((v)))
#define UDELAY(n) 					(lcm_util.udelay(n))
#define MDELAY(n) 					(lcm_util.mdelay(n))
#define REGFLAG_DELAY 			0xAB

typedef unsigned char    kal_uint8;
typedef struct {
    unsigned char cmd;
    unsigned char data;
} sn65dsi8x_setting_table;

//                                                                            
//                 
//                                                                            
extern void DSI_clk_HS_mode(bool enter);

#ifdef BUILD_LK

#define sn65dsi83_SLAVE_ADDR_WRITE		0x58
#define SN65DSI83_I2C_ID							I2C3

static struct mt_i2c_t sn65dsi83_i2c;

/*                                                         
   
                                            
   
                                                          */
kal_uint32 sn65dsi83_write_byte(kal_uint8 addr, kal_uint8 value)
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint8 write_data[2];
    kal_uint16 len;

    write_data[0]= addr;
    write_data[1] = value;

    sn65dsi83_i2c.id = SN65DSI83_I2C_ID;
    /*                                                                              */
    sn65dsi83_i2c.addr = (sn65dsi83_SLAVE_ADDR_WRITE >> 1);
    sn65dsi83_i2c.mode = ST_MODE;
    sn65dsi83_i2c.speed = 100;
    len = 2;

    ret_code = i2c_write(&sn65dsi83_i2c, write_data, len);

    return ret_code;
}

kal_uint32 sn65dsi83_read_byte (kal_uint8 addr, kal_uint8 *dataBuffer) 
{
    kal_uint32 ret_code = I2C_OK;
    kal_uint16 len;
    
    *dataBuffer = addr;

    sn65dsi83_i2c.id = SN65DSI83_I2C_ID;
    /*                                                                              */
    sn65dsi83_i2c.addr = (sn65dsi83_SLAVE_ADDR_WRITE >> 1);
    sn65dsi83_i2c.mode = ST_MODE;
    sn65dsi83_i2c.speed = 100;
    len = 1;

    ret_code = i2c_write_read(&sn65dsi83_i2c, dataBuffer, len, len);

    return ret_code;
}

 /*                                                                             
                                                      
                                                                              */
#else
extern int sn65dsi83_read_byte(kal_uint8 cmd, kal_uint8 *returnData);
extern int sn65dsi83_write_byte(kal_uint8 cmd, kal_uint8 Data);
#endif
//                         
static sn65dsi8x_setting_table sn65dis83_init_table[]=
{
	#if defined(LVDS_PANEL_8BITS_SUPPORT)
		{0x09,            0x00},
		{0x0A,            0x05},
		{0x0B,            0x10},
		{0x0D,            0x00},
		{0x10,            0x26},
		{0x11,            0x00},
		{0x12,            0x2c},
		{0x13,            0x00},
		{0x18,            0x78},
		{0x19,            0x00},
		{0x1A,            0x03},
		{0x1B,            0x00},
		{0x20,            0x20},
		{0x21,            0x03},
		{0x22,            0x00},
		{0x23,            0x00},
		{0x24,            0x00},
		{0x25,            0x00},
		{0x26,            0x00},
		{0x27,            0x00},
		{0x28,            0x21},
		{0x29,            0x00},
		{0x2A,            0x00},
		{0x2B,            0x00},
		{0x2C,            0x0a},
		{0x2D,            0x00},
		{0x2E,            0x00},
		{0x2F,            0x00},
		{0x30,            0x02},
		{0x31,            0x00},
		{0x32,            0x00},
		{0x33,            0x00},
		{0x34,           	0x18},
		{0x35,            0x00},
		{0x36,            0x00},
		{0x37,            0x00},
		{0x38,            0x00},
		{0x39,            0x00},
		{0x3A,            0x00},
		{0x3B,            0x00},
		{0x3C,            0x00},
		{0x3D,           	0x00},
		{0x3E,           	0x00},
		{0x0D, 		  			0x01},
		{REGFLAG_DELAY,		0x0a},
		{0x09,						0x01},
		{0xFF,						0x00},
	#endif
};
static void push_table(sn65dsi8x_setting_table *table, unsigned int count)
{
		unsigned int i;

    for(i = 0; i < count; i++)
		{
        unsigned cmd;
				
        cmd = table[i].cmd;
        switch (cmd)
				{	
            case REGFLAG_DELAY:
            		MDELAY(table[i].data);
                break;
								
            case 0xFF:
                break;
				
            default:
								#ifdef BUILD_LK
								sn65dsi83_write_byte(cmd, table[i].data);
								#else
								sn65dsi83_write_byte(cmd, table[i].data);
								#endif
       	}
    }
	
}
#ifdef SN65DSI_DEBUG
static void dump_reg_table(sn65dsi8x_setting_table *table, unsigned int count)
{
		unsigned int i;
		unsigned char data;
	
    for(i = 0; i < count; i++)
		{		
        unsigned cmd;

				cmd = table[i].cmd;
        switch(cmd)
				{	
            case REGFLAG_DELAY:
            		MDELAY(table[i].data);
                break;
								
            case 0xFF:
                break;
				
            default:
								#ifdef BUILD_LK
								sn65dsi83_read_byte(cmd,&data);	
								printf("dump cmd=0x%x  data=0x%x \n",cmd,data);
								#else
								sn65dsi83_read_byte(cmd,&data);
								printk("dump cmd=0x%x  data=0x%x \n",cmd,data);
								#endif
       	}
    }
}
#endif

void init_sn65dsi8x(void)
{
		#ifdef SN65DSI_DEBUG
		unsigned char data;
		#endif

		push_table(sn65dis83_init_table, sizeof(sn65dis83_init_table)/sizeof(sn65dsi8x_setting_table));
		
		#ifdef SN65DSI_DEBUG
		sn65dsi83_write_byte(0xe0,1);
		sn65dsi83_write_byte(0xe1,0xff);
		MDELAY(5);
		
		sn65dsi83_read_byte(0xe5, &data);
		#ifdef BUILD_LK
		printf("dump cmd=0xe5  data=0x%x \n",data);
		#else
		printk("dump cmd=0xe5  data=0x%x \n",data);
		#endif
		dump_reg_table(sn65dis83_init_table, sizeof(sn65dis83_init_table)/sizeof(sn65dsi8x_setting_table));
		#endif
}

//                                                                            
//                            
//                                                                            
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{	
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		params->dsi.mode   = SYNC_EVENT_VDO_MODE;
	
		//    
		/*                      */  
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		
		//                                                                
		#if defined(LVDS_PANEL_8BITS_SUPPORT)
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
		#else
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666;
		#endif

  	params->dsi.word_count=FRAME_WIDTH*3;

		params->dsi.vertical_sync_active	= 2;
		params->dsi.vertical_backporch		= 2;
		params->dsi.vertical_frontporch		= 4;
		params->dsi.vertical_active_line	= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active	=10;
		params->dsi.horizontal_backporch		= 24;
		params->dsi.horizontal_frontporch		= 30;
		params->dsi.horizontal_active_pixel	= FRAME_WIDTH;

		#if defined(LVDS_PANEL_8BITS_SUPPORT)
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		#else
		params->dsi.PS=LCM_PACKED_PS_18BIT_RGB666;
		#endif
		
		params->dsi.pll_select 	= 0;		//                        
		params->dsi.PLL_CLOCK 	= 224;
		params->dsi.cont_clock 	= 1;
}

static void lcm_set_gpio_output(unsigned int GPIO, unsigned int output)
{
    if(GPIO == 0xFFFFFFFF)
		{
			#ifdef BUILD_LK	 
			printf("[LK/LCM] GPIO_LCD_PWR_EN =   0x%x\n",GPIO_LCD_PWR_EN);
			printf("[LK/LCM] GPIO_LCD_PWR2_EN =  0x%x \n",GPIO_LCD_PWR2_EN);
			printf("[LK/LCM] GPIO_LCD_RST_EN =  0x%x \n",GPIO_LCD_RST_EN);
			printf("[LK/LCM] GPIO_LCD_STB_EN =   0x%x\n",GPIO_LCD_STB_EN);
			printf("[LK/LCM] GPIO_SHIFT_EN =   0x%x\n",GPIO_SHIFT_EN);
			printf("[LK/LCM] GPIO_LCD_BL_EN =   0x%x\n",GPIO_LCD_BL_EN);
			printf("[LK/LCM] GPIO_LCD_BRIDGE_EN =  0x%x \n",GPIO_LCD_BRIDGE_EN);
			#elif (defined BUILD_UBOOT)         //                     
			#else
			#endif
			
			return;
		}	
		
		mt_set_gpio_mode(GPIO, GPIO_MODE_00);
		mt_set_gpio_dir(GPIO, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO, (output>0)? GPIO_OUT_ONE: GPIO_OUT_ZERO);
}

static void lcm_init(void)
{
#ifdef BUILD_LK
   	printf("[LK/LCM]lcm_init \n");
   	
		//                                 
    lcm_set_gpio_output(GPIO_LCD_BRIDGE_EN, GPIO_OUT_ONE);
		MDELAY(20);
		
    lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(5);
		
    lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
    MDELAY(20);
		
    lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
    MDELAY(50);
	
		//                   
		MDELAY(5);
		
   	init_sn65dsi8x();
		MDELAY(10);
		
		//                     
    lcm_set_gpio_output(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);		//                        
    MDELAY(50);
    
    lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ONE); //                                       
    MDELAY(50);
		
#elif (defined BUILD_UBOOT)
#else
		printk("[Kernel/LCM]lcm_init \n");
	  
	  //                   
		 
#endif    
}


static void lcm_suspend(void)
{
#ifdef BUILD_LK
		printf("[LK/LCM]lcm_suspend enter \n");
		
#else
		unsigned char temp;

		printk("[Kernel/LCM]lcm_suspend enter \n");

		//                           
		lcm_set_gpio_output(GPIO_LCD_BRIDGE_EN, GPIO_OUT_ZERO);
		MDELAY(10);

		lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ZERO); 	 //            
		MDELAY(50);

		lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ZERO); //                   
		MDELAY(30); //                          

		lcm_set_gpio_output(GPIO_LCD_PWR_EN, GPIO_OUT_ZERO); 	 //                        

		//                        
		sn65dsi83_read_byte(0x0a,&temp);	//                                    
		printk("lcm_suspend  0x0a  value=0x%x \n",temp);

		sn65dsi83_read_byte(0x0d,&temp);
		printk("lcm_suspend  0x0d  value=0x%x \n",temp);

		sn65dsi83_write_byte(0x0d, (temp&0xfe));	//           

		//                      
		//                     

#endif
}


static void lcm_resume(void)
{    
#ifdef BUILD_LK
		printf("[LK/LCM]lcm_resume enter\n");

#else
		#ifdef SN65DSI_DEBUG
		unsigned char temp;
		#endif
		
		printk("[Kernel/LCM]lcm_resume enter \n");

		//                   
		MDELAY(50);

		//                       
		lcm_set_gpio_output(GPIO_LCD_BRIDGE_EN, GPIO_OUT_ONE);
		MDELAY(10);

		lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
		MDELAY(5);

		lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ZERO);
		MDELAY(20);

		lcm_set_gpio_output(GPIO_LCD_RST_EN, GPIO_OUT_ONE);
		MDELAY(10);

		init_sn65dsi8x();
		MDELAY(10);

#ifdef SN65DSI_DEBUG
		sn65dsi83_read_byte(0x0a,&temp);
		printk("lcm_resume cmd-- 0x0a=0x%x \n",temp);

		sn65dsi83_read_byte(0x0d,&temp);
		printk("lcm_resume cmd-- 0x0d=0x%x \n",temp);

		sn65dsi83_read_byte(0x09,&temp);
		printk("lcm_resume cmd-- 0x09=0x%x \n",temp);
#endif

		//                    
		lcm_set_gpio_output(GPIO_LCD_PWR_EN, GPIO_OUT_ONE);
		MDELAY(50);

		lcm_set_gpio_output(GPIO_LCD_PWR2_EN, GPIO_OUT_ONE);
		MDELAY(50);

#endif
}
static unsigned int lcm_compare_id(void)
{
#if defined(BUILD_LK)
		printf("Sn65dsi83 lcm_compare_id enter \n");
#endif

    return 1;
}

LCM_DRIVER cpt_clap070wp03xg_sn65dsi83_lcm_drv = 
{
  .name		    = "cpt_clap070wp03xg_sn65dsi83",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

