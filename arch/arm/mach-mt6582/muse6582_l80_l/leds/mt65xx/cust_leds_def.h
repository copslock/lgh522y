#ifndef _CUST_LEDS_DEF_H
#define _CUST_LEDS_DEF_H

//                                                    
//                                                  
enum mt65xx_led_mode
{
	MT65XX_LED_MODE_NONE,
	MT65XX_LED_MODE_PWM,
	MT65XX_LED_MODE_GPIO,
	MT65XX_LED_MODE_PMIC,
	//                     
	MT65XX_LED_MODE_CUST_LCM,	
	MT65XX_LED_MODE_CUST_BLS_PWM
};

enum mt65xx_led_pmic
{
	MT65XX_LED_PMIC_LCD_ISINK=0,
	MT65XX_LED_PMIC_NLED_ISINK0,
	MT65XX_LED_PMIC_NLED_ISINK1,
	MT65XX_LED_PMIC_NLED_ISINK2,
	MT65XX_LED_PMIC_NLED_ISINK3
};
struct PWM_config
{
	int clock_source;
	int div;
	int low_duration;
	int High_duration;
	BOOL pmic_pad;
};
/*
                       
                   
*/
struct cust_leds_param {
	const char	*name;
	const void	*args;
};

struct cust_leds_param* get_cust_leds_param(void);
typedef int (*cust_brightness_set)(int level, int div);
typedef int (*cust_set_brightness)(int level);

/*
                                     
                      
         
                      
                   
                                
                                                  
*/
struct cust_mt65xx_led {
	char                 *name;
	enum mt65xx_led_mode  mode;
	int                   data;
 struct PWM_config config_data;
};

extern struct cust_mt65xx_led *get_cust_led_list(void);
#endif /*                  */
