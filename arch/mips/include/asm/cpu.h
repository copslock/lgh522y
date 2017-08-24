/*
 * cpu.h: Values of the PRId register used to match up
 *	  various MIPS cpu types.
 *
 * Copyright (C) 1996 David S. Miller (davem@davemloft.net)
 * Copyright (C) 2004  Maciej W. Rozycki
 */
#ifndef _ASM_CPU_H
#define _ASM_CPU_H

/*                                                            
                                                                       
                                                                   
       

                                                                       
                                                                       
                                                                       
                               

                                                                          
                                                                           
        
*/

#define PRID_COMP_LEGACY	0x000000
#define PRID_COMP_MIPS		0x010000
#define PRID_COMP_BROADCOM	0x020000
#define PRID_COMP_ALCHEMY	0x030000
#define PRID_COMP_SIBYTE	0x040000
#define PRID_COMP_SANDCRAFT	0x050000
#define PRID_COMP_NXP		0x060000
#define PRID_COMP_TOSHIBA	0x070000
#define PRID_COMP_LSI		0x080000
#define PRID_COMP_LEXRA		0x0b0000
#define PRID_COMP_NETLOGIC	0x0c0000
#define PRID_COMP_CAVIUM	0x0d0000
#define PRID_COMP_INGENIC	0xd00000

/*
                                                                     
                                                                       
                                                               
 */
#define PRID_IMP_R2000		0x0100
#define PRID_IMP_AU1_REV1	0x0100
#define PRID_IMP_AU1_REV2	0x0200
#define PRID_IMP_R3000		0x0200		/*                 */
#define PRID_IMP_R6000		0x0300		/*                 */
#define PRID_IMP_R4000		0x0400
#define PRID_IMP_R6000A		0x0600
#define PRID_IMP_R10000		0x0900
#define PRID_IMP_R4300		0x0b00
#define PRID_IMP_VR41XX		0x0c00
#define PRID_IMP_R12000		0x0e00
#define PRID_IMP_R14000		0x0f00
#define PRID_IMP_R8000		0x1000
#define PRID_IMP_PR4450		0x1200
#define PRID_IMP_R4600		0x2000
#define PRID_IMP_R4700		0x2100
#define PRID_IMP_TX39		0x2200
#define PRID_IMP_R4640		0x2200
#define PRID_IMP_R4650		0x2200		/*               */
#define PRID_IMP_R5000		0x2300
#define PRID_IMP_TX49		0x2d00
#define PRID_IMP_SONIC		0x2400
#define PRID_IMP_MAGIC		0x2500
#define PRID_IMP_RM7000		0x2700
#define PRID_IMP_NEVADA		0x2800		/*            */
#define PRID_IMP_RM9000		0x3400
#define PRID_IMP_LOONGSON1	0x4200
#define PRID_IMP_R5432		0x5400
#define PRID_IMP_R5500		0x5500
#define PRID_IMP_LOONGSON2	0x6300

#define PRID_IMP_UNKNOWN	0xff00

/*
                                                        
 */

#define PRID_IMP_4KC		0x8000
#define PRID_IMP_5KC		0x8100
#define PRID_IMP_20KC		0x8200
#define PRID_IMP_4KEC		0x8400
#define PRID_IMP_4KSC		0x8600
#define PRID_IMP_25KF		0x8800
#define PRID_IMP_5KE		0x8900
#define PRID_IMP_4KECR2		0x9000
#define PRID_IMP_4KEMPR2	0x9100
#define PRID_IMP_4KSD		0x9200
#define PRID_IMP_24K		0x9300
#define PRID_IMP_34K		0x9500
#define PRID_IMP_24KE		0x9600
#define PRID_IMP_74K		0x9700
#define PRID_IMP_1004K		0x9900
#define PRID_IMP_1074K		0x9a00
#define PRID_IMP_M14KC		0x9c00
#define PRID_IMP_M14KEC		0x9e00

/*
                                                          
 */

#define PRID_IMP_SB1		0x0100
#define PRID_IMP_SB1A		0x1100

/*
                                                             
 */

#define PRID_IMP_SR71000	0x0400

/*
                                                            
 */

#define PRID_IMP_BMIPS32_REV4	0x4000
#define PRID_IMP_BMIPS32_REV8	0x8000
#define PRID_IMP_BMIPS3300	0x9000
#define PRID_IMP_BMIPS3300_ALT	0x9100
#define PRID_IMP_BMIPS3300_BUG	0x0000
#define PRID_IMP_BMIPS43XX	0xa000
#define PRID_IMP_BMIPS5000	0x5a00

#define PRID_REV_BMIPS4380_LO	0x0040
#define PRID_REV_BMIPS4380_HI	0x006f

/*
                                                          
 */

#define PRID_IMP_CAVIUM_CN38XX 0x0000
#define PRID_IMP_CAVIUM_CN31XX 0x0100
#define PRID_IMP_CAVIUM_CN30XX 0x0200
#define PRID_IMP_CAVIUM_CN58XX 0x0300
#define PRID_IMP_CAVIUM_CN56XX 0x0400
#define PRID_IMP_CAVIUM_CN50XX 0x0600
#define PRID_IMP_CAVIUM_CN52XX 0x0700
#define PRID_IMP_CAVIUM_CN63XX 0x9000
#define PRID_IMP_CAVIUM_CN68XX 0x9100
#define PRID_IMP_CAVIUM_CN66XX 0x9200
#define PRID_IMP_CAVIUM_CN61XX 0x9300

/*
                                                           
 */

#define PRID_IMP_JZRISC	       0x0200

/*
                                                            
 */
#define PRID_IMP_NETLOGIC_XLR732	0x0000
#define PRID_IMP_NETLOGIC_XLR716	0x0200
#define PRID_IMP_NETLOGIC_XLR532	0x0900
#define PRID_IMP_NETLOGIC_XLR308	0x0600
#define PRID_IMP_NETLOGIC_XLR532C	0x0800
#define PRID_IMP_NETLOGIC_XLR516C	0x0a00
#define PRID_IMP_NETLOGIC_XLR508C	0x0b00
#define PRID_IMP_NETLOGIC_XLR308C	0x0f00
#define PRID_IMP_NETLOGIC_XLS608	0x8000
#define PRID_IMP_NETLOGIC_XLS408	0x8800
#define PRID_IMP_NETLOGIC_XLS404	0x8c00
#define PRID_IMP_NETLOGIC_XLS208	0x8e00
#define PRID_IMP_NETLOGIC_XLS204	0x8f00
#define PRID_IMP_NETLOGIC_XLS108	0xce00
#define PRID_IMP_NETLOGIC_XLS104	0xcf00
#define PRID_IMP_NETLOGIC_XLS616B	0x4000
#define PRID_IMP_NETLOGIC_XLS608B	0x4a00
#define PRID_IMP_NETLOGIC_XLS416B	0x4400
#define PRID_IMP_NETLOGIC_XLS412B	0x4c00
#define PRID_IMP_NETLOGIC_XLS408B	0x4e00
#define PRID_IMP_NETLOGIC_XLS404B	0x4f00
#define PRID_IMP_NETLOGIC_AU13XX	0x8000

#define PRID_IMP_NETLOGIC_XLP8XX	0x1000
#define PRID_IMP_NETLOGIC_XLP3XX	0x1100

/*
                                           
 */

#define PRID_REV_MASK		0x00ff

#define PRID_REV_TX4927		0x0022
#define PRID_REV_TX4937		0x0030
#define PRID_REV_R4400		0x0040
#define PRID_REV_R3000A		0x0030
#define PRID_REV_R3000		0x0020
#define PRID_REV_R2000A		0x0010
#define PRID_REV_TX3912		0x0010
#define PRID_REV_TX3922		0x0030
#define PRID_REV_TX3927		0x0040
#define PRID_REV_VR4111		0x0050
#define PRID_REV_VR4181		0x0050	/*                */
#define PRID_REV_VR4121		0x0060
#define PRID_REV_VR4122		0x0070
#define PRID_REV_VR4181A	0x0070	/*                */
#define PRID_REV_VR4130		0x0080
#define PRID_REV_34K_V1_0_2	0x0022
#define PRID_REV_LOONGSON1B	0x0020
#define PRID_REV_LOONGSON2E	0x0002
#define PRID_REV_LOONGSON2F	0x0003

/*
                                                                        
                                                                            
                                                                           
                            
 */
#define PRID_REV_ENCODE_44(ver, rev)					\
	((ver) << 4 | (rev))
#define PRID_REV_ENCODE_332(ver, rev, patch)				\
	((ver) << 5 | (rev) << 2 | (patch))

/*
                                                                 
  
                                                                        
                                                  
                                                                        
                                   
 */

#define FPIR_IMP_NONE		0x0000

enum cpu_type_enum {
	CPU_UNKNOWN,

	/*
                          
  */
	CPU_R2000, CPU_R3000, CPU_R3000A, CPU_R3041, CPU_R3051, CPU_R3052,
	CPU_R3081, CPU_R3081E,

	/*
                          
  */
	CPU_R6000, CPU_R6000A,

	/*
                          
  */
	CPU_R4000PC, CPU_R4000SC, CPU_R4000MC, CPU_R4200, CPU_R4300, CPU_R4310,
	CPU_R4400PC, CPU_R4400SC, CPU_R4400MC, CPU_R4600, CPU_R4640, CPU_R4650,
	CPU_R4700, CPU_R5000, CPU_R5500, CPU_NEVADA, CPU_R5432, CPU_R10000,
	CPU_R12000, CPU_R14000, CPU_VR41XX, CPU_VR4111, CPU_VR4121, CPU_VR4122,
	CPU_VR4131, CPU_VR4133, CPU_VR4181, CPU_VR4181A, CPU_RM7000,
	CPU_SR71000, CPU_RM9000, CPU_TX49XX,

	/*
                          
  */
	CPU_R8000,

	/*
                           
  */
	CPU_TX3912, CPU_TX3922, CPU_TX3927,

	/*
                           
  */
	CPU_4KC, CPU_4KEC, CPU_4KSC, CPU_24K, CPU_34K, CPU_1004K, CPU_74K,
	CPU_ALCHEMY, CPU_PR4450, CPU_BMIPS32, CPU_BMIPS3300, CPU_BMIPS4350,
	CPU_BMIPS4380, CPU_BMIPS5000, CPU_JZRISC, CPU_LOONGSON1, CPU_M14KC,
	CPU_M14KEC,

	/*
                           
  */
	CPU_5KC, CPU_5KE, CPU_20KC, CPU_25KF, CPU_SB1, CPU_SB1A, CPU_LOONGSON2,
	CPU_CAVIUM_OCTEON, CPU_CAVIUM_OCTEON_PLUS, CPU_CAVIUM_OCTEON2,
	CPU_XLR, CPU_XLP,

	CPU_LAST
};


/*
                      
  
 */
#define MIPS_CPU_ISA_I		0x00000001
#define MIPS_CPU_ISA_II		0x00000002
#define MIPS_CPU_ISA_III	0x00000004
#define MIPS_CPU_ISA_IV		0x00000008
#define MIPS_CPU_ISA_V		0x00000010
#define MIPS_CPU_ISA_M32R1	0x00000020
#define MIPS_CPU_ISA_M32R2	0x00000040
#define MIPS_CPU_ISA_M64R1	0x00000080
#define MIPS_CPU_ISA_M64R2	0x00000100

#define MIPS_CPU_ISA_32BIT (MIPS_CPU_ISA_I | MIPS_CPU_ISA_II | \
	MIPS_CPU_ISA_M32R1 | MIPS_CPU_ISA_M32R2)
#define MIPS_CPU_ISA_64BIT (MIPS_CPU_ISA_III | MIPS_CPU_ISA_IV | \
	MIPS_CPU_ISA_V | MIPS_CPU_ISA_M64R1 | MIPS_CPU_ISA_M64R2)

/*
                       
 */
#define MIPS_CPU_TLB		0x00000001 /*             */
#define MIPS_CPU_4KEX		0x00000002 /*                       */
#define MIPS_CPU_3K_CACHE	0x00000004 /*                    */
#define MIPS_CPU_4K_CACHE	0x00000008 /*                    */
#define MIPS_CPU_TX39_CACHE	0x00000010 /*                     */
#define MIPS_CPU_FPU		0x00000020 /*             */
#define MIPS_CPU_32FPR		0x00000040 /*                            */
#define MIPS_CPU_COUNTER	0x00000080 /*                     */
#define MIPS_CPU_WATCH		0x00000100 /*                      */
#define MIPS_CPU_DIVEC		0x00000200 /*                            */
#define MIPS_CPU_VCE		0x00000400 /*                                   */
#define MIPS_CPU_CACHE_CDEX_P	0x00000800 /*                                 */
#define MIPS_CPU_CACHE_CDEX_S	0x00001000 /*                                 */
#define MIPS_CPU_MCHECK		0x00002000 /*                         */
#define MIPS_CPU_EJTAG		0x00004000 /*                 */
#define MIPS_CPU_NOFPUEX	0x00008000 /*                  */
#define MIPS_CPU_LLSC		0x00010000 /*                            */
#define MIPS_CPU_INCLUSIVE_CACHES	0x00020000 /*                         */
#define MIPS_CPU_PREFETCH	0x00040000 /*                         */
#define MIPS_CPU_VINT		0x00080000 /*                                         */
#define MIPS_CPU_VEIC		0x00100000 /*                                                        */
#define MIPS_CPU_ULRI		0x00200000 /*                      */
#define MIPS_CPU_PCI		0x00400000 /*                                */
#define MIPS_CPU_RIXI		0x00800000 /*                               */
#define MIPS_CPU_MICROMIPS	0x01000000 /*                              */

/*
                    
 */
#define MIPS_ASE_MIPS16		0x00000001 /*                  */
#define MIPS_ASE_MDMX		0x00000002 /*                              */
#define MIPS_ASE_MIPS3D		0x00000004 /*         */
#define MIPS_ASE_SMARTMIPS	0x00000008 /*           */
#define MIPS_ASE_DSP		0x00000010 /*                       */
#define MIPS_ASE_MIPSMT		0x00000020 /*                      */
#define MIPS_ASE_DSP2P		0x00000040 /*                             */
#define MIPS_ASE_VZ		0x00000080 /*                    */

#endif /*            */
