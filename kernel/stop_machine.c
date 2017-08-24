/*
 * kernel/stop_machine.c
 *
 * Copyright (C) 2008, 2005	IBM Corporation.
 * Copyright (C) 2008, 2005	Rusty Russell rusty@rustcorp.com.au
 * Copyright (C) 2010		SUSE Linux Products GmbH
 * Copyright (C) 2010		Tejun Heo <tj@kernel.org>
 *
 * This file is released under the GPLv2 and any later version.
 */
#include <linux/completion.h>
#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/export.h>
#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/stop_machine.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/smpboot.h>
#include <linux/atomic.h>

/*
                                                                      
                                        
 */
struct cpu_stop_done {
	atomic_t		nr_todo;	/*                    */
	bool			executed;	/*                    */
	int			ret;		/*                        */
	struct completion	completion;	/*                            */
};

/*                                                                        */
struct cpu_stopper {
	spinlock_t		lock;
	bool			enabled;	/*                          */
	struct list_head	works;		/*                       */
};

static DEFINE_PER_CPU(struct cpu_stopper, cpu_stopper);
static DEFINE_PER_CPU(struct task_struct *, cpu_stopper_task);
static bool stop_machine_initialized = false;

static void cpu_stop_init_done(struct cpu_stop_done *done, unsigned int nr_todo)
{
	memset(done, 0, sizeof(*done));
	atomic_set(&done->nr_todo, nr_todo);
	init_completion(&done->completion);
}

/*                                        */
static void cpu_stop_signal_done(struct cpu_stop_done *done, bool executed)
{
	if (done) {
		if (executed)
			done->executed = true;
		if (atomic_dec_and_test(&done->nr_todo))
			complete(&done->completion);
	}
}

/*                                                                      */
static void cpu_stop_queue_work(unsigned int cpu, struct cpu_stop_work *work)
{
	struct cpu_stopper *stopper = &per_cpu(cpu_stopper, cpu);
	struct task_struct *p = per_cpu(cpu_stopper_task, cpu);

	unsigned long flags;

	spin_lock_irqsave(&stopper->lock, flags);

	if (stopper->enabled) {
		list_add_tail(&work->list, &stopper->works);
		wake_up_process(p);
	} else
		cpu_stop_signal_done(work->done, false);

	spin_unlock_irqrestore(&stopper->lock, flags);
}

/* 
                            
                    
                           
                        
  
                                                                   
                                                          
                                                                 
            
  
                                                             
                                                                    
                                                                    
                                                                    
                           
  
           
               
  
           
                                                                  
                                      
 */
int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg)
{
	struct cpu_stop_done done;
	struct cpu_stop_work work = { .fn = fn, .arg = arg, .done = &done };

	cpu_stop_init_done(&done, 1);
	cpu_stop_queue_work(cpu, &work);
	wait_for_completion(&done.completion);
	return done.executed ? done.ret : -ENOENT;
}

/* 
                                                                 
                    
                           
                        
  
                                                                  
                                                                   
                                                                
  
           
              
 */
void stop_one_cpu_nowait(unsigned int cpu, cpu_stop_fn_t fn, void *arg,
			struct cpu_stop_work *work_buf)
{
	*work_buf = (struct cpu_stop_work){ .fn = fn, .arg = arg, };
	cpu_stop_queue_work(cpu, work_buf);
}

/*                           */
static DEFINE_MUTEX(stop_cpus_mutex);
static DEFINE_PER_CPU(struct cpu_stop_work, stop_cpus_work);

static void queue_stop_cpus_work(const struct cpumask *cpumask,
				 cpu_stop_fn_t fn, void *arg,
				 struct cpu_stop_done *done)
{
	struct cpu_stop_work *work;
	unsigned int cpu;

	/*                           */
	for_each_cpu(cpu, cpumask) {
		work = &per_cpu(stop_cpus_work, cpu);
		work->fn = fn;
		work->arg = arg;
		work->done = done;
	}

	/*
                                                      
                                                              
                                            
  */
	preempt_disable();
	for_each_cpu(cpu, cpumask)
		cpu_stop_queue_work(cpu, &per_cpu(stop_cpus_work, cpu));
	preempt_enable();
}

static int __stop_cpus(const struct cpumask *cpumask,
		       cpu_stop_fn_t fn, void *arg)
{
	struct cpu_stop_done done;

	cpu_stop_init_done(&done, cpumask_weight(cpumask));
	queue_stop_cpus_work(cpumask, fn, arg, &done);
	wait_for_completion(&done.completion);
	return done.executed ? done.ret : -ENOENT;
}

/* 
                                 
                         
                           
                        
  
                                                                     
                                                            
                                                                     
                                             
  
                                                                   
                                                                     
                                                                   
                                                                   
                                                      
  
                                                                      
                                      
  
           
               
  
           
                                                                   
                                                               
                                                                  
 */
int stop_cpus(const struct cpumask *cpumask, cpu_stop_fn_t fn, void *arg)
{
	int ret;

	/*                                                      */
	mutex_lock(&stop_cpus_mutex);
	ret = __stop_cpus(cpumask, fn, arg);
	mutex_unlock(&stop_cpus_mutex);
	return ret;
}

/* 
                                            
                         
                           
                        
  
                                                                
                                              
  
           
               
  
           
                                                               
                                                                      
                                                                     
                                              
 */
int try_stop_cpus(const struct cpumask *cpumask, cpu_stop_fn_t fn, void *arg)
{
	int ret;

	/*                                                      */
	if (!mutex_trylock(&stop_cpus_mutex))
		return -EAGAIN;
	ret = __stop_cpus(cpumask, fn, arg);
	mutex_unlock(&stop_cpus_mutex);
	return ret;
}

static int cpu_stop_should_run(unsigned int cpu)
{
	struct cpu_stopper *stopper = &per_cpu(cpu_stopper, cpu);
	unsigned long flags;
	int run;

	spin_lock_irqsave(&stopper->lock, flags);
	run = !list_empty(&stopper->works);
	spin_unlock_irqrestore(&stopper->lock, flags);
	return run;
}

static void cpu_stopper_thread(unsigned int cpu)
{
	struct cpu_stopper *stopper = &per_cpu(cpu_stopper, cpu);
	struct cpu_stop_work *work;
	int ret;

repeat:
	work = NULL;
	spin_lock_irq(&stopper->lock);
	if (!list_empty(&stopper->works)) {
		work = list_first_entry(&stopper->works,
					struct cpu_stop_work, list);
		list_del_init(&work->list);
	}
	spin_unlock_irq(&stopper->lock);

	if (work) {
		cpu_stop_fn_t fn = work->fn;
		void *arg = work->arg;
		struct cpu_stop_done *done = work->done;
		char ksym_buf[KSYM_NAME_LEN] __maybe_unused;

		/*                                             */
		preempt_disable();

		ret = fn(arg);
		if (ret)
			done->ret = ret;

		/*                                                  */
		preempt_enable();
		WARN_ONCE(preempt_count(),
			  "cpu_stop: %s(%p) leaked preempt count\n",
			  kallsyms_lookup((unsigned long)fn, NULL, NULL, NULL,
					  ksym_buf), arg);

		cpu_stop_signal_done(done, true);
		goto repeat;
	}
}

extern void sched_set_stop_task(int cpu, struct task_struct *stop);

static void cpu_stop_create(unsigned int cpu)
{
	sched_set_stop_task(cpu, per_cpu(cpu_stopper_task, cpu));
}

static void cpu_stop_park(unsigned int cpu)
{
	struct cpu_stopper *stopper = &per_cpu(cpu_stopper, cpu);
	struct cpu_stop_work *work;
	unsigned long flags;

	/*                       */
	spin_lock_irqsave(&stopper->lock, flags);
	list_for_each_entry(work, &stopper->works, list)
		cpu_stop_signal_done(work->done, false);
	stopper->enabled = false;
	spin_unlock_irqrestore(&stopper->lock, flags);
}

static void cpu_stop_unpark(unsigned int cpu)
{
	struct cpu_stopper *stopper = &per_cpu(cpu_stopper, cpu);

	spin_lock_irq(&stopper->lock);
	stopper->enabled = true;
	spin_unlock_irq(&stopper->lock);
}

static struct smp_hotplug_thread cpu_stop_threads = {
	.store			= &cpu_stopper_task,
	.thread_should_run	= cpu_stop_should_run,
	.thread_fn		= cpu_stopper_thread,
	.thread_comm		= "migration/%u",
	.create			= cpu_stop_create,
	.setup			= cpu_stop_unpark,
	.park			= cpu_stop_park,
	.pre_unpark		= cpu_stop_unpark,
	.selfparking		= true,
};

static int __init cpu_stop_init(void)
{
	unsigned int cpu;

	for_each_possible_cpu(cpu) {
		struct cpu_stopper *stopper = &per_cpu(cpu_stopper, cpu);

		spin_lock_init(&stopper->lock);
		INIT_LIST_HEAD(&stopper->works);
	}

	BUG_ON(smpboot_register_percpu_thread(&cpu_stop_threads));
	stop_machine_initialized = true;
	return 0;
}
early_initcall(cpu_stop_init);

#ifdef CONFIG_STOP_MACHINE

/*                                        */
enum stopmachine_state {
	/*                                  */
	STOPMACHINE_NONE,
	/*                                    */
	STOPMACHINE_PREPARE,
	/*                     */
	STOPMACHINE_DISABLE_IRQ,
	/*                  */
	STOPMACHINE_RUN,
	/*      */
	STOPMACHINE_EXIT,
};

struct stop_machine_data {
	int			(*fn)(void *);
	void			*data;
	/*                                                                   */
	unsigned int		num_threads;
	const struct cpumask	*active_cpus;

	enum stopmachine_state	state;
	atomic_t		thread_ack;
};

static void set_state(struct stop_machine_data *smdata,
		      enum stopmachine_state newstate)
{
	/*                    */
	atomic_set(&smdata->thread_ack, smdata->num_threads);
	smp_wmb();
	smdata->state = newstate;
}

/*                                                  */
static void ack_state(struct stop_machine_data *smdata)
{
	if (atomic_dec_and_test(&smdata->thread_ack))
		set_state(smdata, smdata->state + 1);
}

/*                                                    */
static int stop_machine_cpu_stop(void *data)
{
	struct stop_machine_data *smdata = data;
	enum stopmachine_state curstate = STOPMACHINE_NONE;
	int cpu = smp_processor_id(), err = 0;
	unsigned long flags;
	bool is_active;

	/*
                                                                
                                                                
  */
	local_save_flags(flags);

	if (!smdata->active_cpus)
		is_active = cpu == cpumask_first(cpu_online_mask);
	else
		is_active = cpumask_test_cpu(cpu, smdata->active_cpus);

	/*                      */
	do {
		/*                                                    */
		cpu_relax();
		if (smdata->state != curstate) {
			curstate = smdata->state;
			switch (curstate) {
			case STOPMACHINE_DISABLE_IRQ:
				local_irq_disable();
				hard_irq_disable();
				break;
			case STOPMACHINE_RUN:
				if (is_active)
					err = smdata->fn(smdata->data);
				break;
			default:
				break;
			}
			ack_state(smdata);
		}
	} while (curstate != STOPMACHINE_EXIT);

	local_irq_restore(flags);
	return err;
}

int __stop_machine(int (*fn)(void *), void *data, const struct cpumask *cpus)
{
	struct stop_machine_data smdata = { .fn = fn, .data = data,
					    .num_threads = num_online_cpus(),
					    .active_cpus = cpus };

	if (!stop_machine_initialized) {
		/*
                                                   
                                                 
                 
   */
		unsigned long flags;
		int ret;

		WARN_ON_ONCE(smdata.num_threads != 1);

		local_irq_save(flags);
		hard_irq_disable();
		ret = (*fn)(data);
		local_irq_restore(flags);

		return ret;
	}

	/*                                                 */
	set_state(&smdata, STOPMACHINE_PREPARE);
	return stop_cpus(cpu_online_mask, stop_machine_cpu_stop, &smdata);
}

int stop_machine(int (*fn)(void *), void *data, const struct cpumask *cpus)
{
	int ret;

	/*                                          */
	get_online_cpus();
	ret = __stop_machine(fn, data, cpus);
	put_online_cpus();
	return ret;
}
EXPORT_SYMBOL_GPL(stop_machine);

/* 
                                                                    
                           
                                    
                                                              
  
                                                                         
                                                                          
                                                                       
                    
  
                                                                        
                                                                           
       
  
           
                                                             
  
           
                                                                          
                     
 */
int stop_machine_from_inactive_cpu(int (*fn)(void *), void *data,
				  const struct cpumask *cpus)
{
	struct stop_machine_data smdata = { .fn = fn, .data = data,
					    .active_cpus = cpus };
	struct cpu_stop_done done;
	int ret;

	/*                                                         */
	BUG_ON(cpu_active(raw_smp_processor_id()));
	smdata.num_threads = num_active_cpus() + 1;	/*              */

	/*                                                                  */
	while (!mutex_trylock(&stop_cpus_mutex))
		cpu_relax();

	/*                                                                */
	set_state(&smdata, STOPMACHINE_PREPARE);
	cpu_stop_init_done(&done, num_active_cpus());
	queue_stop_cpus_work(cpu_active_mask, stop_machine_cpu_stop, &smdata,
			     &done);
	ret = stop_machine_cpu_stop(&smdata);

	/*                           */
	while (!completion_done(&done.completion))
		cpu_relax();

	mutex_unlock(&stop_cpus_mutex);
	return ret ?: done.ret;
}

#endif	/*                     */
