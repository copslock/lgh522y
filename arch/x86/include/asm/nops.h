#ifndef _ASM_X86_NOPS_H
#define _ASM_X86_NOPS_H

/*
                                                          
  
                                              
 */

#define NOP_DS_PREFIX 0x3e

/*                          
         
                                                          
                                            
                    
                          
                             
                                
                                   
*/
#define GENERIC_NOP1 0x90
#define GENERIC_NOP2 0x89,0xf6
#define GENERIC_NOP3 0x8d,0x76,0x00
#define GENERIC_NOP4 0x8d,0x74,0x26,0x00
#define GENERIC_NOP5 GENERIC_NOP1,GENERIC_NOP4
#define GENERIC_NOP6 0x8d,0xb6,0x00,0x00,0x00,0x00
#define GENERIC_NOP7 0x8d,0xb4,0x26,0x00,0x00,0x00,0x00
#define GENERIC_NOP8 GENERIC_NOP1,GENERIC_NOP7
#define GENERIC_NOP5_ATOMIC NOP_DS_PREFIX,GENERIC_NOP4

/*                   
         
             
                 
                     
*/
#define K8_NOP1 GENERIC_NOP1
#define K8_NOP2	0x66,K8_NOP1
#define K8_NOP3	0x66,K8_NOP2
#define K8_NOP4	0x66,K8_NOP3
#define K8_NOP5	K8_NOP3,K8_NOP2
#define K8_NOP6	K8_NOP3,K8_NOP3
#define K8_NOP7	K8_NOP4,K8_NOP3
#define K8_NOP8	K8_NOP4,K8_NOP4
#define K8_NOP5_ATOMIC 0x66,K8_NOP4

/*        
                                           
         
                    
                         
                             
                                
                                   
*/
#define K7_NOP1	GENERIC_NOP1
#define K7_NOP2	0x8b,0xc0
#define K7_NOP3	0x8d,0x04,0x20
#define K7_NOP4	0x8d,0x44,0x20,0x00
#define K7_NOP5	K7_NOP4,K7_NOP1
#define K7_NOP6	0x8d,0x80,0,0,0,0
#define K7_NOP7	0x8D,0x04,0x05,0,0,0,0
#define K7_NOP8	K7_NOP7,K7_NOP1
#define K7_NOP5_ATOMIC NOP_DS_PREFIX,K7_NOP4

/*        
                                                   
         
             
                 
                     
                            
                                
                           
                                  
                                                              
                                           
*/
#define P6_NOP1	GENERIC_NOP1
#define P6_NOP2	0x66,0x90
#define P6_NOP3	0x0f,0x1f,0x00
#define P6_NOP4	0x0f,0x1f,0x40,0
#define P6_NOP5	0x0f,0x1f,0x44,0x00,0
#define P6_NOP6	0x66,0x0f,0x1f,0x44,0x00,0
#define P6_NOP7	0x0f,0x1f,0x80,0,0,0,0
#define P6_NOP8	0x0f,0x1f,0x84,0x00,0,0,0,0
#define P6_NOP5_ATOMIC P6_NOP5

#ifdef __ASSEMBLY__
#define _ASM_MK_NOP(x) .byte x
#else
#define _ASM_MK_NOP(x) ".byte " __stringify(x) "\n"
#endif

#if defined(CONFIG_MK7)
#define ASM_NOP1 _ASM_MK_NOP(K7_NOP1)
#define ASM_NOP2 _ASM_MK_NOP(K7_NOP2)
#define ASM_NOP3 _ASM_MK_NOP(K7_NOP3)
#define ASM_NOP4 _ASM_MK_NOP(K7_NOP4)
#define ASM_NOP5 _ASM_MK_NOP(K7_NOP5)
#define ASM_NOP6 _ASM_MK_NOP(K7_NOP6)
#define ASM_NOP7 _ASM_MK_NOP(K7_NOP7)
#define ASM_NOP8 _ASM_MK_NOP(K7_NOP8)
#define ASM_NOP5_ATOMIC _ASM_MK_NOP(K7_NOP5_ATOMIC)
#elif defined(CONFIG_X86_P6_NOP)
#define ASM_NOP1 _ASM_MK_NOP(P6_NOP1)
#define ASM_NOP2 _ASM_MK_NOP(P6_NOP2)
#define ASM_NOP3 _ASM_MK_NOP(P6_NOP3)
#define ASM_NOP4 _ASM_MK_NOP(P6_NOP4)
#define ASM_NOP5 _ASM_MK_NOP(P6_NOP5)
#define ASM_NOP6 _ASM_MK_NOP(P6_NOP6)
#define ASM_NOP7 _ASM_MK_NOP(P6_NOP7)
#define ASM_NOP8 _ASM_MK_NOP(P6_NOP8)
#define ASM_NOP5_ATOMIC _ASM_MK_NOP(P6_NOP5_ATOMIC)
#elif defined(CONFIG_X86_64)
#define ASM_NOP1 _ASM_MK_NOP(K8_NOP1)
#define ASM_NOP2 _ASM_MK_NOP(K8_NOP2)
#define ASM_NOP3 _ASM_MK_NOP(K8_NOP3)
#define ASM_NOP4 _ASM_MK_NOP(K8_NOP4)
#define ASM_NOP5 _ASM_MK_NOP(K8_NOP5)
#define ASM_NOP6 _ASM_MK_NOP(K8_NOP6)
#define ASM_NOP7 _ASM_MK_NOP(K8_NOP7)
#define ASM_NOP8 _ASM_MK_NOP(K8_NOP8)
#define ASM_NOP5_ATOMIC _ASM_MK_NOP(K8_NOP5_ATOMIC)
#else
#define ASM_NOP1 _ASM_MK_NOP(GENERIC_NOP1)
#define ASM_NOP2 _ASM_MK_NOP(GENERIC_NOP2)
#define ASM_NOP3 _ASM_MK_NOP(GENERIC_NOP3)
#define ASM_NOP4 _ASM_MK_NOP(GENERIC_NOP4)
#define ASM_NOP5 _ASM_MK_NOP(GENERIC_NOP5)
#define ASM_NOP6 _ASM_MK_NOP(GENERIC_NOP6)
#define ASM_NOP7 _ASM_MK_NOP(GENERIC_NOP7)
#define ASM_NOP8 _ASM_MK_NOP(GENERIC_NOP8)
#define ASM_NOP5_ATOMIC _ASM_MK_NOP(GENERIC_NOP5_ATOMIC)
#endif

#define ASM_NOP_MAX 8
#define NOP_ATOMIC5 (ASM_NOP_MAX+1)	/*                                 */

#ifndef __ASSEMBLY__
extern const unsigned char * const *ideal_nops;
extern void arch_init_ideal_nops(void);
#endif

#endif /*                 */
