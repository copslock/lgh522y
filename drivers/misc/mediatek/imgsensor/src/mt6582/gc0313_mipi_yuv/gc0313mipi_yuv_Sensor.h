/*                                                                            
  
            
            
                            
  
           
           
         
  
               
               
                                                                     
  
          
          
          
  
                                                               
                      
                                                                       
                                                                                
        
                                       
  
  
                                                                                
                                                                       
                                                               
                                                                              */
 
#ifndef __GC0313MIPI_SENSOR_H
#define __GC0313MIPI_SENSOR_H


#define VGA_PERIOD_PIXEL_NUMS						694
#define VGA_PERIOD_LINE_NUMS						488

#define IMAGE_SENSOR_VGA_GRAB_PIXELS			0
#define IMAGE_SENSOR_VGA_GRAB_LINES			1

#define IMAGE_SENSOR_VGA_WIDTH					(640)
#define IMAGE_SENSOR_VGA_HEIGHT					(480)

#define IMAGE_SENSOR_PV_WIDTH					(IMAGE_SENSOR_VGA_WIDTH - 8)
#define IMAGE_SENSOR_PV_HEIGHT					(IMAGE_SENSOR_VGA_HEIGHT - 6)

#define IMAGE_SENSOR_FULL_WIDTH					(IMAGE_SENSOR_VGA_WIDTH - 8)
#define IMAGE_SENSOR_FULL_HEIGHT					(IMAGE_SENSOR_VGA_HEIGHT - 6)

#define GC0313MIPI_WRITE_ID							0x42
#define GC0313MIPI_READ_ID								0x43

//                                

typedef enum
{
	GC0313MIPI_RGB_Gamma_m1 = 0,
	GC0313MIPI_RGB_Gamma_m2,
	GC0313MIPI_RGB_Gamma_m3,
	GC0313MIPI_RGB_Gamma_m4,
	GC0313MIPI_RGB_Gamma_m5,
	GC0313MIPI_RGB_Gamma_m6,
	GC0313MIPI_RGB_Gamma_night
}GC0313MIPI_GAMMA_TAG;

typedef enum
{
	CHT_806C_2 = 1,
	CHT_808C_2,
	LY_982A_H114,
	XY_046A,
	XY_0620,
	XY_078V,
	YG1001A_F
}GC0313MIPI_LENS_TAG;

UINT32 GC0313MIPIOpen(void);
UINT32 GC0313MIPIControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC0313MIPIFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 GC0313MIPIGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 GC0313MIPIGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 GC0313MIPIClose(void);

#endif /*            */

