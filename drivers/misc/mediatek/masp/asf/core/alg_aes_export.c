#include "sec_osal_light.h"

/*                                                                         
           
                                                                          */
typedef unsigned int uint32;
typedef unsigned char uchar;


/*                                                                         
                
                                                                          */
#include "sec_aes.h"
/*                                      */
#include "aes_legacy.h"
/*                                                   */
#include "aes_so.h"

/*                                                                         
               
                                                                          */
#define MOD                             "AES_EXPORT"

/*                                                                         
         
                                                                          */
#define SMSG printk

/*                                                                         
                   
                                                                          */
/*                                       */
AES_VER g_ver = AES_VER_LEGACY; 

/*                                                                         
                                        
                                                                          */
int lib_aes_enc(uchar* input_buf,  uint32 input_len, uchar* output_buf, uint32 output_len)
{
    
    switch (g_ver)
    {
        case AES_VER_LEGACY:
            if(0 != aes_legacy_enc(input_buf,input_len,output_buf,output_len))
            {
                goto _err;
            }
            break;
            
        case AES_VER_SO:
            if(0 != aes_so_enc(input_buf,input_len,output_buf,output_len))
            {
                goto _err;
            }
            break;
            
        default:
            SMSG("[%s] Invalid Ver\n",MOD);
            goto _err;
    }    
    
    return 0;

_err:

    return -1;    

}

/*                                                                         
                                        
                                                                          */
int lib_aes_dec(uchar* input_buf,  uint32 input_len, uchar* output_buf, uint32 output_len)
{
    switch (g_ver)
    {
        case AES_VER_LEGACY:
            if(0 != aes_legacy_dec(input_buf,input_len,output_buf,output_len))
            {
                goto _err;
            }
            break;
            
        case AES_VER_SO:
            if(0 != aes_so_dec(input_buf,input_len,output_buf,output_len))
            {
                goto _err;
            }
            break;
            
        default:
            SMSG("[%s] Invalid Ver\n",MOD);
            goto _err;
    }    
    
    return 0;

_err:

    return -1;    

}

/*                                                                         
                                                
                                                                          */
int lib_aes_init_key(uchar* key_buf,  uint32 key_len, AES_VER ver)
{
    switch (ver)
    {
        case AES_VER_LEGACY:
            g_ver = AES_VER_LEGACY;
            SMSG("\n[%s] Legacy\n",MOD);
            if(0 != aes_legacy_init_key(key_buf,key_len))
            {
                goto _err;
            }
            break;
            
        case AES_VER_SO:
            g_ver = AES_VER_SO;
            SMSG("\n[%s] SO\n",MOD);
            if(0 != aes_so_init_key(key_buf,key_len))
            {
                goto _err;            
            }
            break;
            
        default:
            SMSG("\n[%s] Invalid Ver\n",MOD);
            goto _err;
    }
    
    return 0;

_err:

    return -1;    
}

int lib_aes_init_vector(AES_VER ver)
{
    switch (ver)
    {
        case AES_VER_LEGACY:
            g_ver = AES_VER_LEGACY;
            SMSG("[%s] Legacy(V)\n",MOD);
            if(0 != aes_legacy_init_vector())
            {
                goto _err;
            }
            break;
            
        case AES_VER_SO:
            g_ver = AES_VER_SO;
            SMSG("[%s] SO(V)\n",MOD);
            if(0 != aes_so_init_vector())
            {
                goto _err;            
            }
            break;
            
        default:
            SMSG("[%s] Invalid Ver(V)\n",MOD);
            goto _err;
    }
    
    return 0;

_err:
    return -1;
}

