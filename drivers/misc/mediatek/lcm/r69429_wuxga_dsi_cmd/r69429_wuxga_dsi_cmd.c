#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif
//                                                                            
//                 
//                                                                            

#define FRAME_WIDTH  (1200)
#define FRAME_HEIGHT (1920)


//                                                                            
//                 
//                                                                            

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


//                                                                            
//                 
//                                                                            

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#if 0

#define   LCM_DSI_CMD_MODE	1


static void init_lcm_registers(void)
{
	unsigned int data_array[16];
#if 0	
	data_array[0] = 0x04B02300;                          
	dsi_set_cmdq(data_array, 1, 1); 

	data_array[0] = 0x00351500;                          
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00290500;                          
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0] = 0x00110500;                          
	dsi_set_cmdq(data_array, 1, 1);
#endif 


    data_array[0] = 0x00010500;  //                 
    dsi_set_cmdq(data_array, 1, 1); 

    MDELAY(5);
	
   //                                      
   //                                 
    
	data_array[0] = 0x00B02300;  //           
    dsi_set_cmdq(data_array, 1, 1); 

    data_array[0] = 0x00062902;  //                  
    data_array[1] = 0x000804B3;  //       
	data_array[2] = 0x00000022;  	
    dsi_set_cmdq(data_array, 3, 1); 

    data_array[0] = 0x00022902;  //                    
    data_array[1] = 0x00000CB4;  
    dsi_set_cmdq(data_array, 2, 1); 

	data_array[0] = 0x00032902;  //           
    data_array[1] = 0x00D33AB6;   
    dsi_set_cmdq(data_array, 2, 1); 

    data_array[0] = 0x773A1500;  //                  
    dsi_set_cmdq(data_array, 1, 1); 

    data_array[0] = 0x00053902;  //                    
	data_array[1] = 0x0400002A;  
    data_array[2] = 0x000000AF;  
    dsi_set_cmdq(data_array, 3, 1); 

	
	data_array[0] = 0x00053902;  //                  
	data_array[1] = 0x0700002B;  
	data_array[2] = 0x0000007F;  
	dsi_set_cmdq(data_array, 3, 1); 
	
	data_array[0] = 0x00033902;
	data_array[1] = (((FRAME_HEIGHT/2)&0xFF) << 16) | (((FRAME_HEIGHT/2)>>8) << 8) | 0x44;
	dsi_set_cmdq(data_array, 2, 1);
    
	data_array[0] = 0x00351500;  //      
    dsi_set_cmdq(data_array, 1, 1); 
	
	//           

    //                                                  
	//                                


    data_array[0] = 0x00290500;  //                    
    dsi_set_cmdq(data_array, 1, 1); 

     MDELAY(10); //            

    data_array[0] = 0x00110500;  //                      
    dsi_set_cmdq(data_array, 1, 1); 

    MDELAY(120);



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

		//                    
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE;  //                       
        #endif
	
		//    
		/*                      */
		//                         
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//                                                                
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		//                                         
		//                      
		params->dsi.packet_size=256;

		//                     
		params->dsi.intermediat_buffer_num = 0;//                                                                                                          

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=FRAME_WIDTH*3;	

		//                             
		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch					= 4;  
		params->dsi.vertical_frontporch					= 8;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 16;
		params->dsi.horizontal_backporch				= 45;  //                  
		params->dsi.horizontal_frontporch				= 144;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.pll_select=1;	//                        
		//                     

        //                  
		//                                                                                                                        
		//                  
		params->dsi.PLL_CLOCK = 550;
		params->dsi.pll_div1=0;		//                                                         
		params->dsi.pll_div2=1;		//                                
	    params->dsi.fbk_div =19;    //                                                           

}

static void lcm_init(void)
{


	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	
	MDELAY(10);
	
	SET_RESET_PIN(1);
	MDELAY(50); 

    init_lcm_registers();


#if 0
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20);      

	init_lcm_registers();
#endif

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; //            
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(20);
	
	data_array[0] = 0x00100500; //         
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(80);
	SET_RESET_PIN(0);  //        
}


static void lcm_resume(void)
{
#if 0
	unsigned int data_array[16];
	//                       
	
	data_array[0] = 0x00110500; //          
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(120);

	data_array[0] = 0x00290500; //           
	dsi_set_cmdq(data_array, 1, 1);
#else
    lcm_init();
#endif
}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	//                                                               
	//                               
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif
#if 0
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20); 

	array[0] = 0x00023700;//                                       
	dsi_set_cmdq(array, 1, 1);
	
	read_reg_v2(0xF4, buffer, 2);
	id = buffer[0]; //               
    #ifdef BUILD_LK
		printf("%s, LK nt35590 debug: nt35590 id = 0x%08x\n", __func__, id);
    #else
		printk("%s, kernel nt35590 horse debug: nt35590 id = 0x%08x\n", __func__, id);
    #endif

    if(id == LCM_ID_NT35590)
    	return 1;
    else
        return 0;


}
#endif

LCM_DRIVER r69429_wuxga_dsi_cmd_lcm_drv = 
{
    .name			= "r69429_wuxga_dsi_cmd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//                                 
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };

#else
#define   LCM_DSI_CMD_MODE	0


static void init_lcm_registers(void)
{
	unsigned int data_array[16];

    data_array[0] = 0x00010500;  //                 
    dsi_set_cmdq(data_array, 1, 1); 

    MDELAY(5);
	data_array[0] = 0x00110500;  //                      
    dsi_set_cmdq(data_array, 1, 1); 

    MDELAY(120);

  
	data_array[0] = 0x04B02300;  //           
    dsi_set_cmdq(data_array, 1, 1); 

	//                                        
    //                                

    data_array[0] = 0x00062902;  //                  
    data_array[1] = 0x000814B3;  //                                   
	data_array[2] = 0x00000022;  	
    dsi_set_cmdq(data_array, 3, 1); 

    data_array[0] = 0x00022902;  //                    
    data_array[1] = 0x00000CB4;  
    dsi_set_cmdq(data_array, 2, 1); 

	data_array[0] = 0x00032902;  //           
    data_array[1] = 0x00D33AB6;     //  
    dsi_set_cmdq(data_array, 2, 1); 

#ifdef _VENDOR_TEST_PATTERN__
	data_array[0] = 0x00062902;  //            
    data_array[1] = 0x3F0001DE;  //
	data_array[2] = 0x000010FF;  	
    dsi_set_cmdq(data_array, 3, 1); 
#endif
    data_array[0] = 0x773A1500;  //                  
    dsi_set_cmdq(data_array, 1, 1); 

    /*                                                   
                              
                                 
                                    

 
                                                  
                              
                              
                                   */
	
   //                                      
    //                                
	
	//           

    //                                                  
	//                                


    //                                                   
   //                                 


   //                           



    data_array[0] = 0x00290500;  //                    
    dsi_set_cmdq(data_array, 1, 1); 
	MDELAY(20);


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

		//                    
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_EVENT_VDO_MODE; //                                          
        #endif
	
		//    
		/*                      */
		//                         
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//                                                                
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		//                                         
		//                      
		//                            

		//                     
		//                                                                                                                                                   

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		//                                      

		//                             
		params->dsi.vertical_sync_active				= 4; //   
		params->dsi.vertical_backporch					= 4; //     
		params->dsi.vertical_frontporch					= 8; //   
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 16; //   
		params->dsi.horizontal_backporch				= 45; //                         
		params->dsi.horizontal_frontporch				= 144; //    
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.pll_select=1;	//                        
		//                     

		//                                                                                                                       
		//                  
		params->dsi.PLL_CLOCK = 480;
		params->dsi.pll_div1=0;		//                                                         
		params->dsi.pll_div2=1;		//                                
		params->dsi.fbk_div =0x12;    //                                                           

}

static void lcm_init(void)
{
#if 0
    //                                          
	lcm_util.set_gpio_mode(GPIO24,GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO24,GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO24,1);
	MDELAY(50);  //                             



    //                                          
	lcm_util.set_gpio_mode(GPIO77,GPIO_MODE_00);
	lcm_util.set_gpio_dir(GPIO77,GPIO_DIR_OUT);
	lcm_util.set_gpio_out(GPIO77,1);
	MDELAY(10);
    lcm_util.set_gpio_out(GPIO77,0);
	MDELAY(30);  //              
	lcm_util.set_gpio_out(GPIO77,1);
	MDELAY(200);  //                             
#endif
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	
	MDELAY(10);
	
	SET_RESET_PIN(1);
	MDELAY(50); 


    init_lcm_registers();


#if 0
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	
	SET_RESET_PIN(1);
	MDELAY(20);      

	init_lcm_registers();
#endif

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; //            
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(20);
	
	data_array[0] = 0x00100500; //         
	dsi_set_cmdq(data_array, 1, 1);

	MDELAY(80);

   //                                
	SET_RESET_PIN(0);  //        
}


static void lcm_resume(void)
{
	unsigned int data_array[16];
	lcm_init();  //        
	
	data_array[0] = 0x00110500; //          
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(120);

	data_array[0] = 0x00290500; //           
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(40);

}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	//                                                               
	//                               
	
	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

LCM_DRIVER r69429_wuxga_dsi_cmd_lcm_drv = 
{
    .name			= "r69429_wuxga_dsi_cmd",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//                                 
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };


#endif
