/*
 * Copyright (C) 2012 ARM Ltd.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __ASM_ARM_KVM_ARCH_TIMER_H
#define __ASM_ARM_KVM_ARCH_TIMER_H

#include <linux/clocksource.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>

struct arch_timer_kvm {
#ifdef CONFIG_KVM_ARM_TIMER
	/*                      */
	bool			enabled;

	/*                */
	cycle_t			cntvoff;
#endif
};

struct arch_timer_cpu {
#ifdef CONFIG_KVM_ARM_TIMER
	/*                                          */
	u32				cntv_ctl;	/*                */
	cycle_t				cntv_cval;	/*                */

	/*
                                                              
         
  */

	/*                                                     */
	struct hrtimer			timer;

	/*                                          */
	struct work_struct		expired;

	/*                         */
	bool				armed;

	/*           */
	const struct kvm_irq_level	*irq;
#endif
};

#ifdef CONFIG_KVM_ARM_TIMER
int kvm_timer_hyp_init(void);
int kvm_timer_init(struct kvm *kvm);
void kvm_timer_vcpu_init(struct kvm_vcpu *vcpu);
void kvm_timer_flush_hwstate(struct kvm_vcpu *vcpu);
void kvm_timer_sync_hwstate(struct kvm_vcpu *vcpu);
void kvm_timer_vcpu_terminate(struct kvm_vcpu *vcpu);
#else
static inline int kvm_timer_hyp_init(void)
{
	return 0;
};

static inline int kvm_timer_init(struct kvm *kvm)
{
	return 0;
}

static inline void kvm_timer_vcpu_init(struct kvm_vcpu *vcpu) {}
static inline void kvm_timer_flush_hwstate(struct kvm_vcpu *vcpu) {}
static inline void kvm_timer_sync_hwstate(struct kvm_vcpu *vcpu) {}
static inline void kvm_timer_vcpu_terminate(struct kvm_vcpu *vcpu) {}
#endif

#endif
