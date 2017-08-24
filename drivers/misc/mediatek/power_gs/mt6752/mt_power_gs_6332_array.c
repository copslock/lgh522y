//                                                

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <mach/mt_typedefs.h>
#include <mach/mt_power_gs.h>

const unsigned int MT6332_PMIC_REG_gs_early_suspend_deep_idle_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0400,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_early_suspend_deep_idle = MT6332_PMIC_REG_gs_early_suspend_deep_idle_data;

unsigned int MT6332_PMIC_REG_gs_early_suspend_deep_idle_len = 177;

const unsigned int MT6332_PMIC_REG_gs_datalink_3g_and_4g__data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0001,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0000,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_datalink_3g_and_4g_ = MT6332_PMIC_REG_gs_datalink_3g_and_4g__data;

unsigned int MT6332_PMIC_REG_gs_datalink_3g_and_4g__len = 177;

const unsigned int MT6332_PMIC_REG_gs_early_suspend_idle_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0001,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0400,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_early_suspend_idle = MT6332_PMIC_REG_gs_early_suspend_idle_data;

unsigned int MT6332_PMIC_REG_gs_early_suspend_idle_len = 177;

const unsigned int MT6332_PMIC_REG_gs_paging_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0001,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0000,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_paging = MT6332_PMIC_REG_gs_paging_data;

unsigned int MT6332_PMIC_REG_gs_paging_len = 177;

const unsigned int MT6332_PMIC_REG_gs_mp3_play_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0400,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_mp3_play = MT6332_PMIC_REG_gs_mp3_play_data;

unsigned int MT6332_PMIC_REG_gs_mp3_play_len = 177;

const unsigned int MT6332_PMIC_REG_gs_flightmode_suspend_mode_32k_removal_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0000,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_flightmode_suspend_mode_32k_removal = MT6332_PMIC_REG_gs_flightmode_suspend_mode_32k_removal_data;

unsigned int MT6332_PMIC_REG_gs_flightmode_suspend_mode_32k_removal_len = 177;

const unsigned int MT6332_PMIC_REG_gs_efuse_trimming_data[] = {
 //                                             
    0x0038, 0x0FC0, 0x0000,//          
    0x003C, 0x3F00, 0x0000,//          
    0x0042, 0x7FF0, 0x0000,//          
    0x0044, 0x07FC, 0x0000,//          
    0x0048, 0x00FF, 0x0000,//           
    0x004A, 0x03FF, 0x0000,//           
    0x004C, 0xFFFF, 0x0000,//           
    0x004E, 0x007C, 0x0000,//           
    0x0434, 0xF800, 0x0000,//               
    0x0436, 0x001F, 0x0000,//               
    0x0438, 0x3F00, 0x0000,//           
    0x043A, 0x000F, 0x0000,//           
    0x043C, 0x000F, 0x0000,//           
    0x0440, 0x0070, 0x0000,//           
    0x0462, 0x00E0, 0x0000,//            
    0x0464, 0x3F00, 0x0000,//            
    0x0466, 0x001F, 0x0000,//            
    0x0468, 0x001F, 0x0000,//            
    0x046C, 0x0070, 0x0000,//            
    0x047A, 0x007F, 0x0000,//             
    0x048E, 0x00E7, 0x0000,//             
    0x0558, 0x00FC, 0x0000,//            
    0x08A2, 0x0FFF, 0x0000,//             
    0x08A4, 0x07FF, 0x0000,//             
    0x08A6, 0x0FFF, 0x0000,//             
    0x08A8, 0x07FF, 0x0000,//             
    0x0C08, 0x001F, 0x0000,//           
    0x0C1E, 0x03E0, 0x0000,//            
    0x0CC2, 0xFE00, 0x0000,//         
    0x0CC4, 0x0060, 0x0000,//         
};

const unsigned int *MT6332_PMIC_REG_gs_efuse_trimming = MT6332_PMIC_REG_gs_efuse_trimming_data;

unsigned int MT6332_PMIC_REG_gs_efuse_trimming_len = 177;

const unsigned int MT6332_PMIC_REG_gs_talking_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0001,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0000,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_talking = MT6332_PMIC_REG_gs_talking_data;

unsigned int MT6332_PMIC_REG_gs_talking_len = 177;

const unsigned int MT6332_PMIC_REG_gs_video_play_and_3d_gaming_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0001,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0400,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_video_play_and_3d_gaming = MT6332_PMIC_REG_gs_video_play_and_3d_gaming_data;

unsigned int MT6332_PMIC_REG_gs_video_play_and_3d_gaming_len = 177;

const unsigned int MT6332_PMIC_REG_gs_video_record_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0001,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0400,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_video_record = MT6332_PMIC_REG_gs_video_record_data;

unsigned int MT6332_PMIC_REG_gs_video_record_len = 177;

const unsigned int MT6332_PMIC_REG_gs_flightmode_suspend_mode_data[] = {
 //                                             
    0x0004, 0x0300, 0x0100,//        
    0x0094, 0x7FFF, 0x7ECE,//               
    0x009A, 0xF3FF, 0x6027,//               
    0x00A0, 0xFFFF, 0x004A,//               
    0x00B2, 0x00FF, 0x00FF,//               
    0x0138, 0x0001, 0x0000,//          
    0x0446, 0x0001, 0x0000,//           
    0x044A, 0x0001, 0x0001,//           
    0x045C, 0x0100, 0x0100,//            
    0x0472, 0x0003, 0x0002,//            
    0x0476, 0x0001, 0x0000,//            
    0x0490, 0x0001, 0x0001,//             
    0x0498, 0x0100, 0x0000,//             
    0x04AA, 0x0003, 0x0001,//          
    0x04AC, 0x0003, 0x0000,//          
    0x04D6, 0x0003, 0x0001,//          
    0x04D8, 0x0003, 0x0001,//          
    0x0504, 0x0003, 0x0000,//         
    0x0508, 0x0001, 0x0000,//         
    0x053A, 0x0003, 0x0000,//           
    0x053E, 0x0001, 0x0000,//            
    0x0868, 0xE000, 0x0000,//            
    0x0C28, 0x0001, 0x0001,//           
    0x0C40, 0x0100, 0x0100,//            
    0x0CB6, 0x0C07, 0x0406,//         
    0x0CB8, 0x0C01, 0x0000,//         
    0x0CBA, 0x0403, 0x0402,//         
    0x0CBC, 0x0C03, 0x0000,//         
    0x0CCE, 0x8000, 0x0000 //           
};

const unsigned int *MT6332_PMIC_REG_gs_flightmode_suspend_mode = MT6332_PMIC_REG_gs_flightmode_suspend_mode_data;

unsigned int MT6332_PMIC_REG_gs_flightmode_suspend_mode_len = 177;

