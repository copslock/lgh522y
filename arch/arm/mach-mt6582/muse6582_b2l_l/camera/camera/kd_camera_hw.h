#ifndef _KD_CAMERA_HW_H_
#define _KD_CAMERA_HW_H_
 

#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>




//
//      
#define CAMERA_POWER_VCAM_A MT6323_POWER_LDO_VCAMA
#define CAMERA_POWER_VCAM_D MT6323_POWER_LDO_VCAMD
#define CAMERA_POWER_VCAM_A2 MT6323_POWER_LDO_VCAM_AF
#define CAMERA_POWER_VCAM_D2 MT6323_POWER_LDO_VCAM_IO

#ifndef GPIO_CAMERA_CMPDN_PIN 
#define GPIO_CAMERA_CMPDN_PIN GPIO10
#endif 

#define GPIO_CAMERA_CMRST_PIN_MAIN (GPIO120 | 0x80000000)
#define GPIO_CAMERA_CMRST_PIN_MAIN_M_GPIO GPIO_MODE_00


//                   
#define GPIO_CAMERA_8M_EXT_LDO_DVDD (GPIO118 | 0x80000000)
#define GPIO_CAMERA_8M_EXT_LDO_DVDD_M_GPIO GPIO_MODE_00
//                   


#ifndef GPIO_CAMERA_CMPDN_PIN_M_GPIO
#define GPIO_CAMERA_CMPDN_PIN_M_GPIO  GPIO_MODE_00 
#endif 

#ifndef GPIO_CAMERA_CMPDN1_PIN
#define GPIO_CAMERA_CMPDN1_PIN GPIO4
#endif
//
#ifndef GPIO_CAMERA_CMPDN1_PIN_M_GPIO
#define GPIO_CAMERA_CMPDN1_PIN_M_GPIO GPIO_MODE_00
#endif


//                                  
//
/*
                               
                                     
       
  
                              
                                   
       
  
                                    
                                                 
       
  
                              
                                    
       
  
                                     
                                                  
       
  
                                    
                                                   
       
  
                              
                                    
      
  
                                     
                                                  
      
  
                              
                                    
      
  
                                     
                                                  
      
*/


//                                                               
//                                 

//                              
//                             



#endif 
