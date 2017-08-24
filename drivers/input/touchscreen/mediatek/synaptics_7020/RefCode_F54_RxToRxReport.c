/*
   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2011 Synaptics, Inc.

   Permission is hereby granted, free of charge, to any person obtaining a copy of
   this software and associated documentation files (the "Software"), to deal in
   the Software without restriction, including without limitation the rights to use,
   copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

#include "RefCode.h"
#include "RefCode_PDTScan.h"

#ifdef _F54_TEST_

#ifdef F54_Porting
static unsigned char ImageBuffer[CFG_F54_TXCOUNT*CFG_F54_RXCOUNT*2];
static short ImageArray[CFG_F54_RXCOUNT][CFG_F54_RXCOUNT];
static char buf[3000] = {0};
static int ret = 0;
#endif

unsigned char F54_RxToRxReport(void)
{
#ifdef F54_Porting
#else
   unsigned char ImageBuffer[CFG_F54_TXCOUNT*CFG_F54_RXCOUNT*2];
   short ImageArray[CFG_F54_RXCOUNT][CFG_F54_RXCOUNT];
#endif
   //                                              
   int Result=0;

   short DiagonalLowerLimit = 900;
   short DiagonalUpperLimit = 1100;
   short OthersLowerLimit = -100;
   short OthersUpperLimit = 100;

   int i, j, k;
   int length;

   unsigned char command;

#ifdef F54_Porting
    memset(buf, 0, sizeof(buf));
    ret = sprintf(buf, "\nBin #: 7        Name: Receiver To Receiver Short Test\n");
    ret += sprintf(buf+ret, "\n\t");
#else
    printk("\nBin #: 7        Name: Receiver To Receiver Short Test\n");
    printk("\n\t");
#endif
    for (j = 0; j < numberOfRx; j++)
#ifdef F54_Porting
    ret += sprintf(buf+ret, "R%d\t", j);
#else
    printk("R%d\t", j);
#endif

#ifdef F54_Porting
    ret += sprintf(buf+ret, "\n");
#else
    printk("\n");
#endif

   length =  numberOfRx * numberOfTx*2;

   //                                         
   command = 0x07;
   writeRMI(F54_Data_Base, &command, 1);

   //            
   command = 0x00;
   writeRMI(F54_CBCSettings, &command, 1);

   //      
   command = 0x01;
   writeRMI(NoiseMitigation, &command, 1);

   //             
   command = 0x04;
   writeRMI(F54_Command_Base, &command, 1);

   do {
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   command = 0x02;
   writeRMI(F54_Command_Base, &command, 1);

   do {
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   //                 
   //                                  

   command = 0x00;
   writeRMI(F54_Data_LowIndex, &command, 1);
   writeRMI(F54_Data_HighIndex, &command, 1);

   //                                      
   command = 0x01;
   writeRMI(F54_Command_Base, &command, 1);

   //                                    
   do {
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   //                                                  
   longReadRMI(F54_Data_Buffer, &ImageBuffer[0], length);

   k = 0;
   for (i = 0; i < numberOfTx; i++)
   {
       for (j = 0; j < numberOfRx; j++)
       {
            ImageArray[i][j] = (ImageBuffer[k] | (ImageBuffer[k+1] << 8));
            k = k + 2;
       }
   }

   //                                         
   length = numberOfRx* (numberOfRx-numberOfTx) * 2;
   command = 0x11;
   writeRMI(F54_Data_Base, &command, 1);

   command = 0x00;
   writeRMI(F54_Data_LowIndex, &command, 1);
   writeRMI(F54_Data_HighIndex, &command, 1);

   //                                      
   command = 0x01;
   writeRMI(F54_Command_Base, &command, 1);

   //                                    
   do {
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   //                                                  
   longReadRMI(F54_Data_Buffer, &ImageBuffer[0], length);

   k = 0;
   for (i = 0; i < (numberOfRx-numberOfTx); i++)
   {
       for (j = 0; j < numberOfRx; j++)
       {
            ImageArray[numberOfTx+i][j] = ImageBuffer[k] | (ImageBuffer[k+1] << 8);
            k = k + 2;
       }
   }

   /*
                               
                                            
                                    
     
                                        
         
                       
             
                                                                                                        
                                              
                    
                                              
                                                  
             
                
             
                                                        
                                              
                    
                                              
             
                                            
         
                     
     
                 
    */

    for (i = 0; i < numberOfRx; i++)
    {
#ifdef F54_Porting
        ret += sprintf(buf+ret, "R%d\t", i);
#else
        printk("R%d\t", i);
#endif
        for (j = 0; j < numberOfRx; j++)
        {
            if (i == j)
            {
                if((ImageArray[i][j] <= DiagonalUpperLimit) && (ImageArray[i][j] >= DiagonalLowerLimit))
                {
                    Result++; //    
#ifdef F54_Porting
                    ret += sprintf(buf+ret, "%d\t", ImageArray[i][j]);
#else
                    printk("%d\t", ImageArray[i][j]);
#endif
                }
                else
                {
#ifdef F54_Porting
                    ret += sprintf(buf+ret, "%d(*)\t", ImageArray[i][j]);
#else
                    printk("%d(*)\t", ImageArray[i][j]);
#endif
                }
            }
            else
            {
                if((ImageArray[i][j] <= OthersUpperLimit) && (ImageArray[i][j] >= OthersLowerLimit))
                {
                    Result++; //    
#ifdef F54_Porting
                    ret += sprintf(buf+ret, "%d\t", ImageArray[i][j]);
#else
                    printk("%d\t", ImageArray[i][j]);
#endif
                }
                else
                {
#ifdef F54_Porting
                    ret += sprintf(buf+ret, "%d(*)\t", ImageArray[i][j]);
#else
                    printk("%d(*)\t", ImageArray[i][j]);
#endif
                }
            }
        }
#ifdef F54_Porting
        ret += sprintf(buf+ret, "\n");
#else
        printk("\n");
#endif
    }

   //                  
   command = 0x02;
   writeRMI(F54_Command_Base, &command, 1);

   do {
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
   } while (command != 0x00);

   //                         
//                 
   //     
   command= 0x01;
   writeRMI(F01_Cmd_Base, &command, 1);
   delayMS(200);
   readRMI(F01_Data_Base+1, &command, 1); //                                                             

   //                                                                    
   if(Result == numberOfRx * numberOfRx)
    {
#ifdef F54_Porting
        ret += sprintf(buf+ret, "Test Result: Pass\n");
        write_log(buf);
#else
        printk("Test Result: Pass\n");
#endif
        return 1; //    
    }
   else
    {
#ifdef F54_Porting
        ret += sprintf(buf+ret, "Test Result: Fail\n");
        write_log(buf);
#else
        printk("Test Result: Fail\n");
#endif
        return 0; //    
    }
}

int F54_GetRxToRxReport(char *buf)
{
    int Result=0;
    int ret = 0;

    short DiagonalLowerLimit = 900;
    short DiagonalUpperLimit = 1100;
    short OthersLowerLimit = -100;
    short OthersUpperLimit = 100;

    int i, j, k;
    int length;

    unsigned char command;
    int waitcount;

    ret += sprintf(buf+ret, "Info: Rx=%d\n", numberOfRx);

    length =  numberOfRx * numberOfTx*2;

    //                                         
    command = 0x07;
    writeRMI(F54_Data_Base, &command, 1);

    //            
    command = 0x00;
    writeRMI(F54_CBCSettings, &command, 1);

    //      
    command = 0x01;
    writeRMI(NoiseMitigation, &command, 1);

    //             
    command = 0x04;
    writeRMI(F54_Command_Base, &command, 1);

    waitcount = 0;
    do {
        if(++waitcount > 500)
        {
            pr_info("%s[%d], command = %d\n", __func__, __LINE__, command);
            return ret;
        }
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
    } while (command != 0x00);

    command = 0x02;
    writeRMI(F54_Command_Base, &command, 1);

    waitcount = 0;
    do {
        if(++waitcount > 1000)
        {
            pr_info("%s[%d], command = %d\n", __func__, __LINE__, command);
            return ret;
        }
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
    } while (command != 0x00);

    //                 
    //                                  

    command = 0x00;
    writeRMI(F54_Data_LowIndex, &command, 1);
    writeRMI(F54_Data_HighIndex, &command, 1);

    //                                      
    command = 0x01;
    writeRMI(F54_Command_Base, &command, 1);

    //                                    
    waitcount = 0;
    do {
        if(++waitcount > 500)
        {
            pr_info("%s[%d], command = %d\n", __func__, __LINE__, command);
            return ret;
        }
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
    } while (command != 0x00);

    //                                                  
    longReadRMI(F54_Data_Buffer, &ImageBuffer[0], length);

    k = 0;
    for (i = 0; i < numberOfTx; i++)
    {
        for (j = 0; j < numberOfRx; j++)
        {
            ImageArray[i][j] = (ImageBuffer[k] | (ImageBuffer[k+1] << 8));
            k = k + 2;
        }
    }

    //                                         
    length = numberOfRx* (numberOfRx-numberOfTx) * 2;
    command = 0x11;
    writeRMI(F54_Data_Base, &command, 1);

    command = 0x00;
    writeRMI(F54_Data_LowIndex, &command, 1);
    writeRMI(F54_Data_HighIndex, &command, 1);

    //                                      
    command = 0x01;
    writeRMI(F54_Command_Base, &command, 1);

    //                                    
    waitcount = 0;
    do {
        if(++waitcount > 500)
        {
            pr_info("%s[%d], command = %d\n", __func__, __LINE__, command);
            return ret;
        }
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
    } while (command != 0x00);

    //                                                  
    longReadRMI(F54_Data_Buffer, &ImageBuffer[0], length);

    k = 0;
    for (i = 0; i < (numberOfRx-numberOfTx); i++)
    {
        for (j = 0; j < numberOfRx; j++)
        {
            ImageArray[numberOfTx+i][j] = ImageBuffer[k] | (ImageBuffer[k+1] << 8);
            k = k + 2;
        }
    }

    /*
                                
                                             
                                    
     
                                    
     
               
     
                                                                                            
                              
        
                              
                                      
     
        
     
                                            
                              
        
                              
     
                                    
     
                 
     
                 
     */

    for (i = 0; i < numberOfRx; i++)
    {
        for (j = 0; j < numberOfRx; j++)
        {
            if (i == j)
            {
                if((ImageArray[i][j] <= DiagonalUpperLimit) && (ImageArray[i][j] >= DiagonalLowerLimit))
                {
                    Result++; //    
                    ret += sprintf(buf+ret, "%d", ImageArray[i][j]);
                }
                else
                {
                    ret += sprintf(buf+ret, "%d(*)", ImageArray[i][j]);
                }
            }
            else
            {
                if((ImageArray[i][j] <= OthersUpperLimit) && (ImageArray[i][j] >= OthersLowerLimit))
                {
                    Result++; //    
                    ret += sprintf(buf+ret, "%d", ImageArray[i][j]);
                }
                else
                {
                    ret += sprintf(buf+ret, "%d(*)", ImageArray[i][j]);
                }
            }

            if(j < (numberOfRx-1))
                ret += sprintf(buf+ret, " ");
        }
        ret += sprintf(buf+ret, "\n");
    }

    //                  
    command = 0x02;
    writeRMI(F54_Command_Base, &command, 1);

    waitcount = 0;
    do {
        if(++waitcount > 500)
        {
            pr_info("%s[%d], command = %d\n", __func__, __LINE__, command);
            break;
        }
        delayMS(1); //        
        readRMI(F54_Command_Base, &command, 1);
    } while (command != 0x00);


    //                                                                    
    if(Result == numberOfRx * numberOfRx)
    {
        ret += sprintf(buf+ret, "RESULT: Pass\n");
    }
    else
    {
        ret += sprintf(buf+ret, "RESULT: Fail\n");
    }

    //                         
    //                 
    //     
    command= 0x01;
    writeRMI(F01_Cmd_Base, &command, 1);
    delayMS(200);
    readRMI(F01_Data_Base+1, &command, 1); //                                                             
    return ret;
}
#endif

