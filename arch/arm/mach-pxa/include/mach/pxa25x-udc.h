#ifndef _ASM_ARCH_PXA25X_UDC_H
#define _ASM_ARCH_PXA25X_UDC_H

#ifdef _ASM_ARCH_PXA27X_UDC_H
#error "You can't include both PXA25x and PXA27x UDC support"
#endif

#define UDC_RES1	__REG(0x40600004)  /*                              */
#define UDC_RES2	__REG(0x40600008)  /*                              */
#define UDC_RES3	__REG(0x4060000C)  /*                              */

#define UDCCR		__REG(0x40600000)  /*                      */
#define UDCCR_UDE	(1 << 0)	/*            */
#define UDCCR_UDA	(1 << 1)	/*            */
#define UDCCR_RSM	(1 << 2)	/*               */
#define UDCCR_RESIR	(1 << 3)	/*                          */
#define UDCCR_SUSIR	(1 << 4)	/*                           */
#define UDCCR_SRM	(1 << 5)	/*                               */
#define UDCCR_RSTIR	(1 << 6)	/*                         */
#define UDCCR_REM	(1 << 7)	/*                      */

#define UDCCS0		__REG(0x40600010)  /*                                        */
#define UDCCS0_OPR	(1 << 0)	/*                  */
#define UDCCS0_IPR	(1 << 1)	/*                 */
#define UDCCS0_FTF	(1 << 2)	/*               */
#define UDCCS0_DRWF	(1 << 3)	/*                              */
#define UDCCS0_SST	(1 << 4)	/*            */
#define UDCCS0_FST	(1 << 5)	/*             */
#define UDCCS0_RNE	(1 << 6)	/*                       */
#define UDCCS0_SA	(1 << 7)	/*              */

/*                           */
#define UDCCS1		__REG(0x40600014)  /*                                             */
#define UDCCS6		__REG(0x40600028)  /*                                             */
#define UDCCS11		__REG(0x4060003C)  /*                                              */

#define UDCCS_BI_TFS	(1 << 0)	/*                       */
#define UDCCS_BI_TPC	(1 << 1)	/*                          */
#define UDCCS_BI_FTF	(1 << 2)	/*               */
#define UDCCS_BI_TUR	(1 << 3)	/*                        */
#define UDCCS_BI_SST	(1 << 4)	/*            */
#define UDCCS_BI_FST	(1 << 5)	/*             */
#define UDCCS_BI_TSP	(1 << 7)	/*                       */

/*                            */
#define UDCCS2		__REG(0x40600018)  /*                                              */
#define UDCCS7		__REG(0x4060002C)  /*                                              */
#define UDCCS12		__REG(0x40600040)  /*                                               */

#define UDCCS_BO_RFS	(1 << 0)	/*                      */
#define UDCCS_BO_RPC	(1 << 1)	/*                         */
#define UDCCS_BO_DME	(1 << 3)	/*            */
#define UDCCS_BO_SST	(1 << 4)	/*            */
#define UDCCS_BO_FST	(1 << 5)	/*             */
#define UDCCS_BO_RNE	(1 << 6)	/*                        */
#define UDCCS_BO_RSP	(1 << 7)	/*                      */

/*                                  */
#define UDCCS3		__REG(0x4060001C)  /*                                             */
#define UDCCS8		__REG(0x40600030)  /*                                             */
#define UDCCS13		__REG(0x40600044)  /*                                              */

#define UDCCS_II_TFS	(1 << 0)	/*                       */
#define UDCCS_II_TPC	(1 << 1)	/*                          */
#define UDCCS_II_FTF	(1 << 2)	/*               */
#define UDCCS_II_TUR	(1 << 3)	/*                        */
#define UDCCS_II_TSP	(1 << 7)	/*                       */

/*                                   */
#define UDCCS4		__REG(0x40600020)  /*                                              */
#define UDCCS9		__REG(0x40600034)  /*                                              */
#define UDCCS14		__REG(0x40600048)  /*                                               */

#define UDCCS_IO_RFS	(1 << 0)	/*                      */
#define UDCCS_IO_RPC	(1 << 1)	/*                         */
#define UDCCS_IO_ROF	(1 << 2)	/*                  */
#define UDCCS_IO_DME	(1 << 3)	/*            */
#define UDCCS_IO_RNE	(1 << 6)	/*                        */
#define UDCCS_IO_RSP	(1 << 7)	/*                      */

/*                                 */
#define UDCCS5		__REG(0x40600024)  /*                                                    */
#define UDCCS10		__REG(0x40600038)  /*                                                     */
#define UDCCS15		__REG(0x4060004C)  /*                                                     */

#define UDCCS_INT_TFS	(1 << 0)	/*                       */
#define UDCCS_INT_TPC	(1 << 1)	/*                          */
#define UDCCS_INT_FTF	(1 << 2)	/*               */
#define UDCCS_INT_TUR	(1 << 3)	/*                        */
#define UDCCS_INT_SST	(1 << 4)	/*            */
#define UDCCS_INT_FST	(1 << 5)	/*             */
#define UDCCS_INT_TSP	(1 << 7)	/*                       */

#define UFNRH		__REG(0x40600060)  /*                                */
#define UFNRL		__REG(0x40600064)  /*                               */
#define UBCR2		__REG(0x40600068)  /*                      */
#define UBCR4		__REG(0x4060006c)  /*                      */
#define UBCR7		__REG(0x40600070)  /*                      */
#define UBCR9		__REG(0x40600074)  /*                      */
#define UBCR12		__REG(0x40600078)  /*                       */
#define UBCR14		__REG(0x4060007c)  /*                       */
#define UDDR0		__REG(0x40600080)  /*                              */
#define UDDR1		__REG(0x40600100)  /*                              */
#define UDDR2		__REG(0x40600180)  /*                              */
#define UDDR3		__REG(0x40600200)  /*                              */
#define UDDR4		__REG(0x40600400)  /*                              */
#define UDDR5		__REG(0x406000A0)  /*                              */
#define UDDR6		__REG(0x40600600)  /*                              */
#define UDDR7		__REG(0x40600680)  /*                              */
#define UDDR8		__REG(0x40600700)  /*                              */
#define UDDR9		__REG(0x40600900)  /*                              */
#define UDDR10		__REG(0x406000C0)  /*                               */
#define UDDR11		__REG(0x40600B00)  /*                               */
#define UDDR12		__REG(0x40600B80)  /*                               */
#define UDDR13		__REG(0x40600C00)  /*                               */
#define UDDR14		__REG(0x40600E00)  /*                               */
#define UDDR15		__REG(0x406000E0)  /*                               */

#define UICR0		__REG(0x40600050)  /*                                  */

#define UICR0_IM0	(1 << 0)	/*                     */
#define UICR0_IM1	(1 << 1)	/*                     */
#define UICR0_IM2	(1 << 2)	/*                     */
#define UICR0_IM3	(1 << 3)	/*                     */
#define UICR0_IM4	(1 << 4)	/*                     */
#define UICR0_IM5	(1 << 5)	/*                     */
#define UICR0_IM6	(1 << 6)	/*                     */
#define UICR0_IM7	(1 << 7)	/*                     */

#define UICR1		__REG(0x40600054)  /*                                  */

#define UICR1_IM8	(1 << 0)	/*                     */
#define UICR1_IM9	(1 << 1)	/*                     */
#define UICR1_IM10	(1 << 2)	/*                      */
#define UICR1_IM11	(1 << 3)	/*                      */
#define UICR1_IM12	(1 << 4)	/*                      */
#define UICR1_IM13	(1 << 5)	/*                      */
#define UICR1_IM14	(1 << 6)	/*                      */
#define UICR1_IM15	(1 << 7)	/*                      */

#define USIR0		__REG(0x40600058)  /*                                 */

#define USIR0_IR0	(1 << 0)	/*                        */
#define USIR0_IR1	(1 << 1)	/*                        */
#define USIR0_IR2	(1 << 2)	/*                        */
#define USIR0_IR3	(1 << 3)	/*                        */
#define USIR0_IR4	(1 << 4)	/*                        */
#define USIR0_IR5	(1 << 5)	/*                        */
#define USIR0_IR6	(1 << 6)	/*                        */
#define USIR0_IR7	(1 << 7)	/*                        */

#define USIR1		__REG(0x4060005C)  /*                                 */

#define USIR1_IR8	(1 << 0)	/*                        */
#define USIR1_IR9	(1 << 1)	/*                        */
#define USIR1_IR10	(1 << 2)	/*                         */
#define USIR1_IR11	(1 << 3)	/*                         */
#define USIR1_IR12	(1 << 4)	/*                         */
#define USIR1_IR13	(1 << 5)	/*                         */
#define USIR1_IR14	(1 << 6)	/*                         */
#define USIR1_IR15	(1 << 7)	/*                         */

#endif
