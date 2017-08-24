/*
                                  
  
                                                                              
                                                                          
                                                                       
                                                                             
                 
  
                         
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irqflags.h>
#include <linux/cpumask.h>

#include <asm/r4k-timer.h>
#include <linux/atomic.h>
#include <asm/barrier.h>
#include <asm/mipsregs.h>

static atomic_t __cpuinitdata count_start_flag = ATOMIC_INIT(0);
static atomic_t __cpuinitdata count_count_start = ATOMIC_INIT(0);
static atomic_t __cpuinitdata count_count_stop = ATOMIC_INIT(0);
static atomic_t __cpuinitdata count_reference = ATOMIC_INIT(0);

#define COUNTON 100
#define NR_LOOPS 5

void __cpuinit synchronise_count_master(int cpu)
{
	int i;
	unsigned long flags;
	unsigned int initcount;

#ifdef CONFIG_MIPS_MT_SMTC
	/*
                                                  
                  
  */
	return;
#endif

	printk(KERN_INFO "Synchronize counters for CPU %u: ", cpu);

	local_irq_save(flags);

	/*
                                             
  */
	atomic_set(&count_reference, read_c0_count());
	atomic_set(&count_start_flag, cpu);
	smp_wmb();

	/*                                                          */
	initcount = read_c0_count();

	/*
                                                          
                                                       
                                                                  
                                                                       
                                                           
                                                         
                                                           
             
  */

	for (i = 0; i < NR_LOOPS; i++) {
		/*                       */
		while (atomic_read(&count_count_start) != 1)
			mb();
		atomic_set(&count_count_stop, 0);
		smp_wmb();

		/*                                                 */
		atomic_inc(&count_count_start);

		/*
                                                 
   */
		if (i == NR_LOOPS-1)
			write_c0_count(initcount);

		/*
                                                            
   */
		while (atomic_read(&count_count_stop) != 1)
			mb();
		atomic_set(&count_count_start, 0);
		smp_wmb();
		atomic_inc(&count_count_stop);
	}
	/*                                           */
	write_c0_compare(read_c0_count() + COUNTON);
	atomic_set(&count_start_flag, 0);

	local_irq_restore(flags);

	/*
                                             
                                                     
                                  
  */
	printk("done.\n");
}

void __cpuinit synchronise_count_slave(int cpu)
{
	int i;
	unsigned int initcount;

#ifdef CONFIG_MIPS_MT_SMTC
	/*
                                                  
                  
  */
	return;
#endif

	/*
                                                         
                                                            
  */

	while (atomic_read(&count_start_flag) != cpu)
		mb();

	/*                                                        */
	initcount = atomic_read(&count_reference);

	for (i = 0; i < NR_LOOPS; i++) {
		atomic_inc(&count_count_start);
		while (atomic_read(&count_count_start) != 2)
			mb();

		/*
                                                 
   */
		if (i == NR_LOOPS-1)
			write_c0_count(initcount);

		atomic_inc(&count_count_stop);
		while (atomic_read(&count_count_stop) != 2)
			mb();
	}
	/*                                           */
	write_c0_compare(read_c0_count() + COUNTON);
}
#undef NR_LOOPS
