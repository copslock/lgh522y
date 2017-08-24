/*                                                                                                
            
                                                         
                                          
          
                                                         
     
                                                                
                                                                     
                             
   
                                 
                                                     
                                                                                          
                                                                                                 */

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>  
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "imx135mipiraw_Sensor.h"

//                       

//                                                                         
//                                                                                                              
//                                                          
extern kal_uint16 read_cmos_sensor(kal_uint32 addr);
extern void write_cmos_sensor(kal_uint32 addr, kal_uint32 para);
extern void write_cmos_sensor_16(kal_uint16 addr,kal_uint16 para);
extern void write_cmos_sensor_8(kal_uint16 addr, kal_uint8 para);


#if 0
BYTE imx135_byteread_cmos_sensor(kal_uint32 addr)
{
    BYTE get_byte=0;
    char puSendCmd[2] = {(char)(addr >> 8) , (char)(addr & 0xFF) };
    iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,IMX135MIPI_WRITE_ID);
    return get_byte;
}
extern  void imx135_wordwrite_cmos_sensor(u16 addr, u32 para);
extern  void imx135_bytewrite_cmos_sensor(u16 addr, u32 para);
#endif


#define USHORT             unsigned short
#define BYTE               unsigned char
#define Sleep(ms) mdelay(ms)

#define TRULY_ID           0x02

enum LENS
{
    LARGEN_LENS = 1,
    KT_LENS,
    KM_LENS,
    GENIUS_LENS,
    SUNNY_LENS,
    OTHER_LENS,
};
enum DRIVER_IC
{
    DONGWOOK = 1,
    ADI,
    ASM,
    ROHM,
    OTHER_DRIVER,
};
enum VCM
{
    TDK = 1,
    MISTUMIS,
    SIKAO,
    MWT,
    ALPS,
    OTHER_VCM,
};
#define VALID_OTP          0x40

#define GAIN_DEFAULT       0x0100
#define GAIN_GREEN1_ADDR_H   0x020E
#define GAIN_GREEN1_ADDR_L   0x020F

#define GAIN_BLUE_ADDR_H     0x0212
#define GAIN_BLUE_ADDR_L     0x0213

#define GAIN_RED_ADDR_H      0x0210
#define GAIN_RED_ADDR_L      0x0211

#define GAIN_GREEN2_ADDR_H   0x0214
#define GAIN_GREEN2_ADDR_L   0x0214


USHORT golden_r;
USHORT golden_gr;
USHORT golden_gb;
USHORT golden_b;

USHORT current_r;
USHORT current_gr;
USHORT current_gb;
USHORT current_b;

kal_uint32 r_ratio;
kal_uint32 b_ratio;


//                                                                       
//                                                                           
/*                                                                                                
                               
                                                                  
                                                         
                                             
                                             
                                                                                                 */
bool start_read_otp(BYTE zone)
{
    BYTE val = 0;
    int i;
    write_cmos_sensor_8(0x0104, 0x01); 
    write_cmos_sensor(0x3B02, zone);   //    
    write_cmos_sensor(0x3B00, 0x01);
    write_cmos_sensor_8(0x0104, 0x00); 
    Sleep(5);
    for(i=0;i<100;i++)
    {
        val = read_cmos_sensor(0x3B01);
        if((val & 0x01) == 0x01)
            break;
        Sleep(2);
    }
    if(i == 100)
    {
        printk("Read Page %d Err!\n", zone); //          
        return 0;
    }
    return 1;
}


/*                                                                                                
                             
                                       
                                                         
                                                                                            
                                                                                                 */
BYTE get_otp_flag(BYTE zone)
{
    BYTE flag = 0;
    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }
    flag = read_cmos_sensor(0x3B04);
    flag = flag & 0xc0;
    printk("OTP Flag:0x%02x\n",flag );
    return flag;
}

/*                                                                                                
                             
                                       
                                                             
                                                                                                 */
bool get_otp_date(BYTE zone) 
{
    BYTE year  = 0;
    BYTE month = 0;
    BYTE day   = 0;
    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }
    year  = read_cmos_sensor(0x3B06);
    month = read_cmos_sensor(0x3B07);
    day   = read_cmos_sensor(0x3B08);
    printk("OTP date=%02d.%02d.%02d", year,month,day);
    return 1;
}


/*                                                                                                
                                  
                                   
                                                         
                                          
                                                                              
                                                                                                 */
BYTE get_otp_module_id(BYTE zone)
{
    BYTE module_id = 0;
    if(!start_read_otp(zone))
    {
        printk("OTP Start read Page %d Fail!\n", zone);
        return 0;
    }
    module_id = read_cmos_sensor(0x3B05);
    printk("OTP_Module ID: 0x%02x.\n",module_id);
    return module_id;
}


/*                                                                                                
                                
                                       
                                                         
                                          
                                                        
                                                                                                 */
BYTE get_otp_lens_id(BYTE zone)
{
    BYTE lens_id = 0;
    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }
    lens_id = read_cmos_sensor(0x3B09);
    printk("OTP_Lens ID: 0x%02x.\n",lens_id);
    return lens_id;
}


/*                                                                                                
                               
                                      
                                                         
                                          
                                                       
                                                                                                 */
BYTE get_otp_vcm_id(BYTE zone)
{
    BYTE vcm_id = 0;
    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }
    vcm_id = read_cmos_sensor(0x3B0A);
    printk("OTP_VCM ID: 0x%02x.\n",vcm_id);
    return vcm_id;  
}


/*                                                                                                
                                  
                                         
                                                         
                                          
                                                          
                                                                                                 */
BYTE get_otp_driver_id(BYTE zone)
{
    BYTE driver_id = 0;
    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }
    driver_id = read_cmos_sensor(0x3B0B);
    printk("OTP_Driver ID: 0x%02x.\n",driver_id);
    return driver_id;
}

/*                                                                                                
                             
                                                             
                                                         
                                          
                                                         
                                          
                                          
                                          
                                              
                                                                                                 */
BYTE get_light_id(BYTE zone)
{
    BYTE light_id = 0;
    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }
    light_id = read_cmos_sensor(0x3B0C);
    printk("OTP_Light ID: 0x%02x.\n",light_id);
    return light_id;
}

/*                                                                                                
                             
                                       
                                                                                            
                                                                                                 */
BYTE get_lsc_flag()
{
    BYTE flag = 0;
    if(!start_read_otp(0x0B))
    {
        printk("Start read Page 0x0B Fail!\n");
        return 0;
    }
    flag = read_cmos_sensor(0x3B43);
    flag = flag & 0xc0;
    printk("OTP Flag:0x%02x",flag );
    return flag;
}

/*                                                                                                
                                
                                        
                                          
                                                               
                                                                                                 */
bool otp_lenc_update()
{
    BYTE lsc_flag;
    int i, j;
    BYTE temp1, temp2;

    lsc_flag = get_lsc_flag();
    if(lsc_flag == 0xC0 || lsc_flag == 0x80)
    {
        printk("OTP lsc data invalid\n");
        return 0;
    }
    else if(lsc_flag == 0x00)
    {
        printk("OTP no lsc data\n");
        return 0;
    }
    BYTE lsc_data[64 * 8] ={0}; 

    for(i=0;i<8;i++)
    {
        if(!start_read_otp(0x04+i))
        {
            printk("OTP Start read Page %d Fail!\n", 0x04+i);
            return 0;
        }
        for(j=0;j<64;j++)
            lsc_data[i*64+j] = read_cmos_sensor(0x3B04+j);
    }
    for (i=0;i<504;i++)
        {
            printk("%0x  ",lsc_data[i]);
            if((i+1)%64==0)
                printk("\n");
        }

    write_cmos_sensor_8(0x0104, 0x01); 
    for(i=0;i<504;i++) //                     
        write_cmos_sensor(0x4800+i, lsc_data[i]);
    write_cmos_sensor_8(0x0104, 0x00); 

    //          
    temp1 = read_cmos_sensor(0x0700);
    temp2 = read_cmos_sensor(0x3A63);
    temp1 = temp1 | 0x01;
    temp2 = temp2 | 0x01;
    write_cmos_sensor_8(0x0104, 0x00); 
    write_cmos_sensor(0x0700, temp1);
    write_cmos_sensor(0x3A63, temp2);
    write_cmos_sensor_8(0x0104, 0x00); 

    printk("OTP Update lsc finished\n");
    
    return 1;
}

/*                                                                                                
                            
                                                            
                                                                           
                                                                           
                                        
                                                      
                                                                                                 */

bool wb_gain_set()
{
    USHORT R_GAIN;
    USHORT B_GAIN;
    USHORT Gr_GAIN;
    USHORT Gb_GAIN;
    USHORT G_GAIN;
        
    if(!r_ratio || !b_ratio)
    {
        printk("OTP WB ratio Data Err!\n");
        return 0;
    }

    if(r_ratio >= 512 )
    {
        if(b_ratio>=512) 
        {
            R_GAIN = (USHORT)(GAIN_DEFAULT * r_ratio / 512);                        
            G_GAIN = GAIN_DEFAULT;
            B_GAIN = (USHORT)(GAIN_DEFAULT * b_ratio / 512);
        }
        else
        {
            R_GAIN =  (USHORT)(GAIN_DEFAULT*r_ratio / b_ratio );
            G_GAIN = (USHORT)(GAIN_DEFAULT*512 / b_ratio );
            B_GAIN = GAIN_DEFAULT; 
        }
    }
    else                      
    {
        if(b_ratio >= 512)
        {
            R_GAIN = GAIN_DEFAULT;
            G_GAIN = (USHORT)(GAIN_DEFAULT*512 /r_ratio);       
            B_GAIN =  (USHORT)(GAIN_DEFAULT*b_ratio / r_ratio );
        } 
        else 
        {
            Gr_GAIN = (USHORT)(GAIN_DEFAULT*512/ r_ratio );                     
            Gb_GAIN = (USHORT)(GAIN_DEFAULT*512/b_ratio );                      
            if(Gr_GAIN >= Gb_GAIN)                      
            {                       
                R_GAIN = GAIN_DEFAULT;                      
                G_GAIN = (USHORT)(GAIN_DEFAULT *512/ r_ratio );                     
                B_GAIN =  (USHORT)(GAIN_DEFAULT*b_ratio / r_ratio );                        
            } 
            else
            {                       
                R_GAIN =  (USHORT)(GAIN_DEFAULT*r_ratio  / b_ratio);                        
                G_GAIN = (USHORT)(GAIN_DEFAULT*512 / b_ratio );                     
                B_GAIN = GAIN_DEFAULT;
            }
        }        
    }

    //                                                                         
    
    printk("OTP_golden_r=%d,golden_gr=%d,golden_gb=%d,golden_b=%d \n",golden_r,golden_gr,golden_gb,golden_b);
    printk("OTP_current_r=%d,current_gr=%d,current_gb=%d,current_b=%d \n",current_r,current_gr,current_gb,current_b);
    printk("OTP_r_ratio=%d,b_ratio=%d \n",r_ratio,b_ratio);
    printk("R_GAIN=0x%0x,G_GAIN=0x%0x,B_GAIN=0x%0x.\n",R_GAIN,G_GAIN,B_GAIN);
#if 0
    IMX135MIPI_write_cmos_sensor(GAIN_RED_ADDR_H, (R_GAIN>>8)&0xff);
    IMX135MIPI_write_cmos_sensor(GAIN_RED_ADDR_L, (R_GAIN)&0xff);   
    IMX135MIPI_write_cmos_sensor(GAIN_BLUE_ADDR_H, (B_GAIN>>8)&0xff);
    IMX135MIPI_write_cmos_sensor(GAIN_BLUE_ADDR_L, (B_GAIN)&0xff);  
    IMX135MIPI_write_cmos_sensor(GAIN_GREEN1_ADDR_H, (G_GAIN>>8)&0xff); //                          
    IMX135MIPI_write_cmos_sensor(GAIN_GREEN1_ADDR_L, (G_GAIN)&0xff); 
    IMX135MIPI_write_cmos_sensor(GAIN_GREEN2_ADDR_H, (G_GAIN>>8)&0xff);//                       
    IMX135MIPI_write_cmos_sensor(GAIN_GREEN2_ADDR_H, (G_GAIN)&0xff); //                       
#endif
    
    write_cmos_sensor_8(0x0104, 0x01); 
    write_cmos_sensor_16(GAIN_RED_ADDR_H,R_GAIN);
    write_cmos_sensor_16(GAIN_BLUE_ADDR_H,B_GAIN);
    write_cmos_sensor_16(GAIN_GREEN1_ADDR_H,G_GAIN);
    write_cmos_sensor_16(GAIN_GREEN2_ADDR_H,G_GAIN);
    write_cmos_sensor_8(0x0104, 0x00); 

    printk("OTP WB Update Finished! \n");
    return 1;
}

/*                                                                                                
                           
                                
                                                               
                                                                                                 */
bool get_otp_wb(BYTE zone)
{
    BYTE temph = 0;
    BYTE templ = 0;
    golden_r = 0, golden_gr = 0, golden_gb = 0, golden_b = 0;
    current_r = 0, current_gr = 0, current_gb = 0, current_b = 0;

    if(!start_read_otp(zone))
    {
        printk("Start read Page %d Fail!\n", zone);
        return 0;
    }

    temph = read_cmos_sensor(0x3B18);  
    templ = read_cmos_sensor(0x3B19);   
    golden_r  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B1A);
    templ = read_cmos_sensor(0x3B1B);
    golden_gr  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B1C);
    templ = read_cmos_sensor(0x3B1D);
    golden_gb  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B1E);
    templ = read_cmos_sensor(0x3B1F);
    golden_b  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B10);  
    templ = read_cmos_sensor(0x3B11);   
    current_r  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B12);
    templ = read_cmos_sensor(0x3B13);
    current_gr  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B14);
    templ = read_cmos_sensor(0x3B15);
    current_gb  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    temph = read_cmos_sensor(0x3B16);
    templ = read_cmos_sensor(0x3B17);
    current_b  = (USHORT)templ + (((USHORT)temph & 0x03) << 8);

    return 1;
}


/*                                                                                                
                              
                                      
                                          
                                                             
                                                                                                 */
bool otp_wb_update(BYTE zone)
{
    USHORT golden_g, current_g;


    if(!get_otp_wb(zone))  //                     
    {
        printk("Get OTP WB data Err!\n");
        return 0;
    }

    golden_g = (golden_gr + golden_gb) / 2;
    current_g = (current_gr + current_gb) / 2;

    if(!golden_g || !current_g || !golden_r || !golden_b || !current_r || !current_b)
    {
        printk("WB update Err !\n");
        return 0;
    }

    r_ratio = 512 * golden_r * current_g /( golden_g * current_r );
    b_ratio = 512 * golden_b * current_g /( golden_g * current_b );

    wb_gain_set();

    printk("OTP WB update finished! \n");

    return 1;
}

/*                                                                                                
                             
                                                                  
                                                           
                                       
                                          
                                                                                                 */
bool otp_update(BYTE update_sensor_otp_awb, BYTE update_sensor_otp_lsc)
{
    BYTE zone = 0x01;
    BYTE FLG  = 0x00;
    BYTE MID = 0x00, LENS_ID= 0x00, VCM_ID= 0x00;
    int i;

    printk("update_sensor_otp_awb: %d, update_sensor_otp_lsc: %d\n", update_sensor_otp_awb, update_sensor_otp_lsc );

    for(i=0;i<3;i++)
    {
        FLG = get_otp_flag(zone);
        if(FLG == VALID_OTP)
            break;
        else
            zone++;
    }
    if(i==3)
    {
        printk("No OTP Data or OTP data is invalid!!!\n");
        return 0;
    }

    MID =     get_otp_module_id(zone);
    LENS_ID = get_otp_lens_id(zone);
    VCM_ID =  get_otp_vcm_id(zone);

    if(MID != TRULY_ID) //       
    {
        printk("No Truly Module !!!!\n");
        return 0;
    }

    if(0 != update_sensor_otp_awb)
        otp_wb_update(zone);


    if(0 != update_sensor_otp_lsc)
    {
        if(!otp_lenc_update())
        {
            printk("OTP Update LSC Err\n");
            return 0;
        }
    }

    return 1;   
}
