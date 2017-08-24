/*
 * Copyright (C) 2013 Advanced Micro Devices, Inc.
 *
 * Author: Jacob Shin <jacob.shin@amd.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/perf_event.h>
#include <linux/percpu.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>

#include <asm/cpufeature.h>
#include <asm/perf_event.h>
#include <asm/msr.h>

#define NUM_COUNTERS_NB		4
#define NUM_COUNTERS_L2		4
#define MAX_COUNTERS		NUM_COUNTERS_NB

#define RDPMC_BASE_NB		6
#define RDPMC_BASE_L2		10

#define COUNTER_SHIFT		16

struct amd_uncore {
	int id;
	int refcnt;
	int cpu;
	int num_counters;
	int rdpmc_base;
	u32 msr_base;
	cpumask_t *active_mask;
	struct pmu *pmu;
	struct perf_event *events[MAX_COUNTERS];
	struct amd_uncore *free_when_cpu_online;
};

static struct amd_uncore * __percpu *amd_uncore_nb;
static struct amd_uncore * __percpu *amd_uncore_l2;

static struct pmu amd_nb_pmu;
static struct pmu amd_l2_pmu;

static cpumask_t amd_nb_active_mask;
static cpumask_t amd_l2_active_mask;

static bool is_nb_event(struct perf_event *event)
{
	return event->pmu->type == amd_nb_pmu.type;
}

static bool is_l2_event(struct perf_event *event)
{
	return event->pmu->type == amd_l2_pmu.type;
}

static struct amd_uncore *event_to_amd_uncore(struct perf_event *event)
{
	if (is_nb_event(event) && amd_uncore_nb)
		return *per_cpu_ptr(amd_uncore_nb, event->cpu);
	else if (is_l2_event(event) && amd_uncore_l2)
		return *per_cpu_ptr(amd_uncore_l2, event->cpu);

	return NULL;
}

static void amd_uncore_read(struct perf_event *event)
{
	struct hw_perf_event *hwc = &event->hw;
	u64 prev, new;
	s64 delta;

	/*
                                                       
                                                           
  */

	prev = local64_read(&hwc->prev_count);
	rdpmcl(hwc->event_base_rdpmc, new);
	local64_set(&hwc->prev_count, new);
	delta = (new << COUNTER_SHIFT) - (prev << COUNTER_SHIFT);
	delta >>= COUNTER_SHIFT;
	local64_add(delta, &event->count);
}

static void amd_uncore_start(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;

	if (flags & PERF_EF_RELOAD)
		wrmsrl(hwc->event_base, (u64)local64_read(&hwc->prev_count));

	hwc->state = 0;
	wrmsrl(hwc->config_base, (hwc->config | ARCH_PERFMON_EVENTSEL_ENABLE));
	perf_event_update_userpage(event);
}

static void amd_uncore_stop(struct perf_event *event, int flags)
{
	struct hw_perf_event *hwc = &event->hw;

	wrmsrl(hwc->config_base, hwc->config);
	hwc->state |= PERF_HES_STOPPED;

	if ((flags & PERF_EF_UPDATE) && !(hwc->state & PERF_HES_UPTODATE)) {
		amd_uncore_read(event);
		hwc->state |= PERF_HES_UPTODATE;
	}
}

static int amd_uncore_add(struct perf_event *event, int flags)
{
	int i;
	struct amd_uncore *uncore = event_to_amd_uncore(event);
	struct hw_perf_event *hwc = &event->hw;

	/*                          */
	if (hwc->idx != -1 && uncore->events[hwc->idx] == event)
		goto out;

	for (i = 0; i < uncore->num_counters; i++) {
		if (uncore->events[i] == event) {
			hwc->idx = i;
			goto out;
		}
	}

	/*                                          */
	hwc->idx = -1;
	for (i = 0; i < uncore->num_counters; i++) {
		if (cmpxchg(&uncore->events[i], NULL, event) == NULL) {
			hwc->idx = i;
			break;
		}
	}

out:
	if (hwc->idx == -1)
		return -EBUSY;

	hwc->config_base = uncore->msr_base + (2 * hwc->idx);
	hwc->event_base = uncore->msr_base + 1 + (2 * hwc->idx);
	hwc->event_base_rdpmc = uncore->rdpmc_base + hwc->idx;
	hwc->state = PERF_HES_UPTODATE | PERF_HES_STOPPED;

	if (flags & PERF_EF_START)
		amd_uncore_start(event, PERF_EF_RELOAD);

	return 0;
}

static void amd_uncore_del(struct perf_event *event, int flags)
{
	int i;
	struct amd_uncore *uncore = event_to_amd_uncore(event);
	struct hw_perf_event *hwc = &event->hw;

	amd_uncore_stop(event, PERF_EF_UPDATE);

	for (i = 0; i < uncore->num_counters; i++) {
		if (cmpxchg(&uncore->events[i], event, NULL) == event)
			break;
	}

	hwc->idx = -1;
}

static int amd_uncore_event_init(struct perf_event *event)
{
	struct amd_uncore *uncore;
	struct hw_perf_event *hwc = &event->hw;

	if (event->attr.type != event->pmu->type)
		return -ENOENT;

	/*
                                                                        
                                                                     
                                                                       
                                                                 
                      
  */
	if (is_sampling_event(event) || event->attach_state & PERF_ATTACH_TASK)
		return -EINVAL;

	/*                                                       */
	if (event->attr.exclude_user || event->attr.exclude_kernel ||
	    event->attr.exclude_host || event->attr.exclude_guest)
		return -EINVAL;

	/*                                                  */
	hwc->config = event->attr.config & AMD64_RAW_EVENT_MASK_NB;
	hwc->idx = -1;

	if (event->cpu < 0)
		return -EINVAL;

	uncore = event_to_amd_uncore(event);
	if (!uncore)
		return -ENODEV;

	/*
                                                                       
                           
  */
	event->cpu = uncore->cpu;

	return 0;
}

static ssize_t amd_uncore_attr_show_cpumask(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	int n;
	cpumask_t *active_mask;
	struct pmu *pmu = dev_get_drvdata(dev);

	if (pmu->type == amd_nb_pmu.type)
		active_mask = &amd_nb_active_mask;
	else if (pmu->type == amd_l2_pmu.type)
		active_mask = &amd_l2_active_mask;
	else
		return 0;

	n = cpulist_scnprintf(buf, PAGE_SIZE - 2, active_mask);
	buf[n++] = '\n';
	buf[n] = '\0';
	return n;
}
static DEVICE_ATTR(cpumask, S_IRUGO, amd_uncore_attr_show_cpumask, NULL);

static struct attribute *amd_uncore_attrs[] = {
	&dev_attr_cpumask.attr,
	NULL,
};

static struct attribute_group amd_uncore_attr_group = {
	.attrs = amd_uncore_attrs,
};

PMU_FORMAT_ATTR(event, "config:0-7,32-35");
PMU_FORMAT_ATTR(umask, "config:8-15");

static struct attribute *amd_uncore_format_attr[] = {
	&format_attr_event.attr,
	&format_attr_umask.attr,
	NULL,
};

static struct attribute_group amd_uncore_format_group = {
	.name = "format",
	.attrs = amd_uncore_format_attr,
};

static const struct attribute_group *amd_uncore_attr_groups[] = {
	&amd_uncore_attr_group,
	&amd_uncore_format_group,
	NULL,
};

static struct pmu amd_nb_pmu = {
	.attr_groups	= amd_uncore_attr_groups,
	.name		= "amd_nb",
	.event_init	= amd_uncore_event_init,
	.add		= amd_uncore_add,
	.del		= amd_uncore_del,
	.start		= amd_uncore_start,
	.stop		= amd_uncore_stop,
	.read		= amd_uncore_read,
};

static struct pmu amd_l2_pmu = {
	.attr_groups	= amd_uncore_attr_groups,
	.name		= "amd_l2",
	.event_init	= amd_uncore_event_init,
	.add		= amd_uncore_add,
	.del		= amd_uncore_del,
	.start		= amd_uncore_start,
	.stop		= amd_uncore_stop,
	.read		= amd_uncore_read,
};

static struct amd_uncore * __cpuinit amd_uncore_alloc(unsigned int cpu)
{
	return kzalloc_node(sizeof(struct amd_uncore), GFP_KERNEL,
			cpu_to_node(cpu));
}

static void __cpuinit amd_uncore_cpu_up_prepare(unsigned int cpu)
{
	struct amd_uncore *uncore;

	if (amd_uncore_nb) {
		uncore = amd_uncore_alloc(cpu);
		uncore->cpu = cpu;
		uncore->num_counters = NUM_COUNTERS_NB;
		uncore->rdpmc_base = RDPMC_BASE_NB;
		uncore->msr_base = MSR_F15H_NB_PERF_CTL;
		uncore->active_mask = &amd_nb_active_mask;
		uncore->pmu = &amd_nb_pmu;
		*per_cpu_ptr(amd_uncore_nb, cpu) = uncore;
	}

	if (amd_uncore_l2) {
		uncore = amd_uncore_alloc(cpu);
		uncore->cpu = cpu;
		uncore->num_counters = NUM_COUNTERS_L2;
		uncore->rdpmc_base = RDPMC_BASE_L2;
		uncore->msr_base = MSR_F16H_L2I_PERF_CTL;
		uncore->active_mask = &amd_l2_active_mask;
		uncore->pmu = &amd_l2_pmu;
		*per_cpu_ptr(amd_uncore_l2, cpu) = uncore;
	}
}

static struct amd_uncore *
__cpuinit amd_uncore_find_online_sibling(struct amd_uncore *this,
					 struct amd_uncore * __percpu *uncores)
{
	unsigned int cpu;
	struct amd_uncore *that;

	for_each_online_cpu(cpu) {
		that = *per_cpu_ptr(uncores, cpu);

		if (!that)
			continue;

		if (this == that)
			continue;

		if (this->id == that->id) {
			that->free_when_cpu_online = this;
			this = that;
			break;
		}
	}

	this->refcnt++;
	return this;
}

static void __cpuinit amd_uncore_cpu_starting(unsigned int cpu)
{
	unsigned int eax, ebx, ecx, edx;
	struct amd_uncore *uncore;

	if (amd_uncore_nb) {
		uncore = *per_cpu_ptr(amd_uncore_nb, cpu);
		cpuid(0x8000001e, &eax, &ebx, &ecx, &edx);
		uncore->id = ecx & 0xff;

		uncore = amd_uncore_find_online_sibling(uncore, amd_uncore_nb);
		*per_cpu_ptr(amd_uncore_nb, cpu) = uncore;
	}

	if (amd_uncore_l2) {
		unsigned int apicid = cpu_data(cpu).apicid;
		unsigned int nshared;

		uncore = *per_cpu_ptr(amd_uncore_l2, cpu);
		cpuid_count(0x8000001d, 2, &eax, &ebx, &ecx, &edx);
		nshared = ((eax >> 14) & 0xfff) + 1;
		uncore->id = apicid - (apicid % nshared);

		uncore = amd_uncore_find_online_sibling(uncore, amd_uncore_l2);
		*per_cpu_ptr(amd_uncore_l2, cpu) = uncore;
	}
}

static void __cpuinit uncore_online(unsigned int cpu,
				    struct amd_uncore * __percpu *uncores)
{
	struct amd_uncore *uncore = *per_cpu_ptr(uncores, cpu);

	kfree(uncore->free_when_cpu_online);
	uncore->free_when_cpu_online = NULL;

	if (cpu == uncore->cpu)
		cpumask_set_cpu(cpu, uncore->active_mask);
}

static void __cpuinit amd_uncore_cpu_online(unsigned int cpu)
{
	if (amd_uncore_nb)
		uncore_online(cpu, amd_uncore_nb);

	if (amd_uncore_l2)
		uncore_online(cpu, amd_uncore_l2);
}

static void __cpuinit uncore_down_prepare(unsigned int cpu,
					  struct amd_uncore * __percpu *uncores)
{
	unsigned int i;
	struct amd_uncore *this = *per_cpu_ptr(uncores, cpu);

	if (this->cpu != cpu)
		return;

	/*                                                                 */
	for_each_online_cpu(i) {
		struct amd_uncore *that = *per_cpu_ptr(uncores, i);

		if (cpu == i)
			continue;

		if (this == that) {
			perf_pmu_migrate_context(this->pmu, cpu, i);
			cpumask_clear_cpu(cpu, that->active_mask);
			cpumask_set_cpu(i, that->active_mask);
			that->cpu = i;
			break;
		}
	}
}

static void __cpuinit amd_uncore_cpu_down_prepare(unsigned int cpu)
{
	if (amd_uncore_nb)
		uncore_down_prepare(cpu, amd_uncore_nb);

	if (amd_uncore_l2)
		uncore_down_prepare(cpu, amd_uncore_l2);
}

static void __cpuinit uncore_dead(unsigned int cpu,
				  struct amd_uncore * __percpu *uncores)
{
	struct amd_uncore *uncore = *per_cpu_ptr(uncores, cpu);

	if (cpu == uncore->cpu)
		cpumask_clear_cpu(cpu, uncore->active_mask);

	if (!--uncore->refcnt)
		kfree(uncore);
	*per_cpu_ptr(amd_uncore_nb, cpu) = NULL;
}

static void __cpuinit amd_uncore_cpu_dead(unsigned int cpu)
{
	if (amd_uncore_nb)
		uncore_dead(cpu, amd_uncore_nb);

	if (amd_uncore_l2)
		uncore_dead(cpu, amd_uncore_l2);
}

static int __cpuinit
amd_uncore_cpu_notifier(struct notifier_block *self, unsigned long action,
			void *hcpu)
{
	unsigned int cpu = (long)hcpu;

	switch (action & ~CPU_TASKS_FROZEN) {
	case CPU_UP_PREPARE:
		amd_uncore_cpu_up_prepare(cpu);
		break;

	case CPU_STARTING:
		amd_uncore_cpu_starting(cpu);
		break;

	case CPU_ONLINE:
		amd_uncore_cpu_online(cpu);
		break;

	case CPU_DOWN_PREPARE:
		amd_uncore_cpu_down_prepare(cpu);
		break;

	case CPU_UP_CANCELED:
	case CPU_DEAD:
		amd_uncore_cpu_dead(cpu);
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block amd_uncore_cpu_notifier_block __cpuinitdata = {
	.notifier_call	= amd_uncore_cpu_notifier,
	.priority	= CPU_PRI_PERF + 1,
};

static void __init init_cpu_already_online(void *dummy)
{
	unsigned int cpu = smp_processor_id();

	amd_uncore_cpu_starting(cpu);
	amd_uncore_cpu_online(cpu);
}

static int __init amd_uncore_init(void)
{
	unsigned int cpu;
	int ret = -ENODEV;

	if (boot_cpu_data.x86_vendor != X86_VENDOR_AMD)
		return -ENODEV;

	if (!cpu_has_topoext)
		return -ENODEV;

	if (cpu_has_perfctr_nb) {
		amd_uncore_nb = alloc_percpu(struct amd_uncore *);
		perf_pmu_register(&amd_nb_pmu, amd_nb_pmu.name, -1);

		printk(KERN_INFO "perf: AMD NB counters detected\n");
		ret = 0;
	}

	if (cpu_has_perfctr_l2) {
		amd_uncore_l2 = alloc_percpu(struct amd_uncore *);
		perf_pmu_register(&amd_l2_pmu, amd_l2_pmu.name, -1);

		printk(KERN_INFO "perf: AMD L2I counters detected\n");
		ret = 0;
	}

	if (ret)
		return -ENODEV;

	get_online_cpus();
	/*                                                                  */
	for_each_online_cpu(cpu) {
		amd_uncore_cpu_up_prepare(cpu);
		smp_call_function_single(cpu, init_cpu_already_online, NULL, 1);
	}

	register_cpu_notifier(&amd_uncore_cpu_notifier_block);
	put_online_cpus();

	return 0;
}
device_initcall(amd_uncore_init);
