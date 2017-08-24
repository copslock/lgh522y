/*
                                                            
                                   
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/sched.h>

/* 
                                                                     
  
                                                                          
 */
void dump_stack(void)
{
	dump_stack_print_info(KERN_DEFAULT);
	show_stack(NULL, NULL);
}
EXPORT_SYMBOL(dump_stack);
