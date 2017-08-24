/*                                                                         
                   
                                                                          */
#include "sec_boot_lib.h"

#define PART_PATH_PREFIX                   "/dev/"

/*                                                                         
                      
                                                                         */
extern SECURE_INFO                         sec_info;

/*                                                                         
                             
                                                                          */
bool sec_usif_enabled(void)
{
    return sec_info.bUsifEn;
}

/*                                                                         
                    
                                                                          */
void sec_usif_part_name (uint32 part_num, char* part_name)
{
    mcpy(part_name,pl2usif(mtd_part_map[part_num].name),strlen(pl2usif(mtd_part_map[part_num].name)));
}

/*                                                                         
                         
                                                                          */
void sec_usif_part_path(uint32 part_num, char* part_path, uint32 part_path_len)
{    
    memset(part_path,0x0,part_path_len);
    mcpy(part_path,PART_PATH_PREFIX,strlen(PART_PATH_PREFIX));
    sec_usif_part_name(part_num,part_path+strlen(PART_PATH_PREFIX));
    SMSG(TRUE,"usif part path %s\n",part_path);
    
}

/*                                                                         
                   
                                                                          */
char* usif2pl (char* part_name)
{
    /*                   */
    /*                   */
    /*                   */    
    if(0 == mcmp(part_name,USIF_SECCFG,strlen(USIF_SECCFG)))
    {   
        return (char*) PL_SECCFG;
    }
    /*                   */
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_UBOOT,strlen(USIF_UBOOT)))
    {   
        return (char*) PL_UBOOT;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_LOGO,strlen(USIF_LOGO)))
    {
        return (char*) PL_LOGO;
    }
    /*                   */
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_BOOTIMG,strlen(USIF_BOOTIMG)))
    {
        return (char*) PL_BOOTIMG;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_USER,strlen(USIF_USER)))
    {
        return (char*) PL_USER;               
    }   
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_ANDSYSIMG,strlen(USIF_ANDSYSIMG)))
    {
        return (char*) PL_ANDSYSIMG;
    }   
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_RECOVERY,strlen(USIF_RECOVERY)))
    {
        return (char*) PL_RECOVERY;
    }       
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,USIF_SECRO,strlen(USIF_SECRO)))
    {
        return (char*) PL_SECRO;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else
    {
        return part_name;
    }    
}

char* pl2usif (char* part_name)
{
    /*                   */
    /*                   */
    /*                   */    
    if(0 == mcmp(part_name,PL_SECCFG,strlen(PL_SECCFG)))
    {   
        return (char*) USIF_SECCFG;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_UBOOT,strlen(PL_UBOOT)))
    {   
        return (char*) USIF_UBOOT;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_LOGO,strlen(PL_LOGO)))
    {
        return (char*) USIF_LOGO;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_BOOTIMG,strlen(PL_BOOTIMG)))
    {
        return (char*) USIF_BOOTIMG;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_USER,strlen(PL_USER)))
    {
        return (char*) USIF_USER;               
    }   
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_ANDSYSIMG,strlen(PL_ANDSYSIMG)))
    {
        return (char*) USIF_ANDSYSIMG;
    }   
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_RECOVERY,strlen(PL_RECOVERY)))
    {
        return (char*) USIF_RECOVERY;
    }       
    /*                   */    
    /*                   */
    /*                   */    
    else if(0 == mcmp(part_name,PL_SECRO,strlen(PL_SECRO)))
    {
        return (char*) USIF_SECRO;
    }
    /*                   */    
    /*                   */
    /*                   */    
    else
    {
        return part_name;
    }    
}
