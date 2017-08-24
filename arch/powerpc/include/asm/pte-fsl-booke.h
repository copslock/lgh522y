#ifndef _ASM_POWERPC_PTE_FSL_BOOKE_H
#define _ASM_POWERPC_PTE_FSL_BOOKE_H
#ifdef __KERNEL__

/*                                                                
             
  
                         

                                                                
                                                                

                                                                  
                                 

                                                               
                                 
*/

/*                                  */
#define _PAGE_PRESENT	0x00001	/*                               */
#define _PAGE_USER	0x00002	/*                           */
#define _PAGE_FILE	0x00002	/*                                          */
#define _PAGE_RW	0x00004	/*                          */
#define _PAGE_DIRTY	0x00008	/*               */
#define _PAGE_EXEC	0x00010	/*                  */
#define _PAGE_ACCESSED	0x00020	/*                    */

#define _PAGE_ENDIAN	0x00040	/*          */
#define _PAGE_GUARDED	0x00080	/*          */
#define _PAGE_COHERENT	0x00100	/*          */
#define _PAGE_NO_CACHE	0x00200	/*          */
#define _PAGE_WRITETHRU	0x00400	/*          */
#define _PAGE_SPECIAL	0x00800 /*                 */

#define _PMD_PRESENT	0
#define _PMD_PRESENT_MASK (PAGE_MASK)
#define _PMD_BAD	(~PAGE_MASK)

#endif /*            */
#endif /*                               */
