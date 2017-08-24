/*
                        
 */
#include <linux/platform_device.h>
#include <linux/mc146818rtc.h>
#include <linux/acpi.h>
#include <linux/bcd.h>
#include <linux/export.h>
#include <linux/pnp.h>
#include <linux/of.h>

#include <asm/vsyscall.h>
#include <asm/x86_init.h>
#include <asm/time.h>
#include <asm/mrst.h>
#include <asm/rtc.h>

#ifdef CONFIG_X86_32
/*
                                                                      
                                                                      
                                                                       
 */
volatile unsigned long cmos_lock;
EXPORT_SYMBOL(cmos_lock);
#endif /*               */

/*                                                      */
#define CMOS_YEARS_OFFS 2000

DEFINE_SPINLOCK(rtc_lock);
EXPORT_SYMBOL(rtc_lock);

/*
                                                                   
                                                                   
                                                                   
                                                                     
                                                      
 */
int mach_set_rtc_mmss(unsigned long nowtime)
{
	struct rtc_time tm;
	int retval = 0;

	rtc_time_to_tm(nowtime, &tm);
	if (!rtc_valid_tm(&tm)) {
		retval = set_rtc_time(&tm);
		if (retval)
			printk(KERN_ERR "%s: RTC write failed with error %d\n",
			       __FUNCTION__, retval);
	} else {
		printk(KERN_ERR
		       "%s: Invalid RTC value: write of %lx to RTC failed\n",
			__FUNCTION__, nowtime);
		retval = -EINVAL;
	}
	return retval;
}

unsigned long mach_get_cmos_time(void)
{
	unsigned int status, year, mon, day, hour, min, sec, century = 0;
	unsigned long flags;

	spin_lock_irqsave(&rtc_lock, flags);

	/*
                                                            
                                                             
                                                              
                                              
  */
	while ((CMOS_READ(RTC_FREQ_SELECT) & RTC_UIP))
		cpu_relax();

	sec = CMOS_READ(RTC_SECONDS);
	min = CMOS_READ(RTC_MINUTES);
	hour = CMOS_READ(RTC_HOURS);
	day = CMOS_READ(RTC_DAY_OF_MONTH);
	mon = CMOS_READ(RTC_MONTH);
	year = CMOS_READ(RTC_YEAR);

#ifdef CONFIG_ACPI
	if (acpi_gbl_FADT.header.revision >= FADT2_REVISION_ID &&
	    acpi_gbl_FADT.century)
		century = CMOS_READ(acpi_gbl_FADT.century);
#endif

	status = CMOS_READ(RTC_CONTROL);
	WARN_ON_ONCE(RTC_ALWAYS_BCD && (status & RTC_DM_BINARY));

	spin_unlock_irqrestore(&rtc_lock, flags);

	if (RTC_ALWAYS_BCD || !(status & RTC_DM_BINARY)) {
		sec = bcd2bin(sec);
		min = bcd2bin(min);
		hour = bcd2bin(hour);
		day = bcd2bin(day);
		mon = bcd2bin(mon);
		year = bcd2bin(year);
	}

	if (century) {
		century = bcd2bin(century);
		year += century * 100;
	} else
		year += CMOS_YEARS_OFFS;

	return mktime(year, mon, day, hour, min, sec);
}

/*                                          */
unsigned char rtc_cmos_read(unsigned char addr)
{
	unsigned char val;

	lock_cmos_prefix(addr);
	outb(addr, RTC_PORT(0));
	val = inb(RTC_PORT(1));
	lock_cmos_suffix(addr);

	return val;
}
EXPORT_SYMBOL(rtc_cmos_read);

void rtc_cmos_write(unsigned char val, unsigned char addr)
{
	lock_cmos_prefix(addr);
	outb(addr, RTC_PORT(0));
	outb(val, RTC_PORT(1));
	lock_cmos_suffix(addr);
}
EXPORT_SYMBOL(rtc_cmos_write);

int update_persistent_clock(struct timespec now)
{
	return x86_platform.set_wallclock(now.tv_sec);
}

/*                           */
void read_persistent_clock(struct timespec *ts)
{
	unsigned long retval;

	retval = x86_platform.get_wallclock();

	ts->tv_sec = retval;
	ts->tv_nsec = 0;
}


static struct resource rtc_resources[] = {
	[0] = {
		.start	= RTC_PORT(0),
		.end	= RTC_PORT(1),
		.flags	= IORESOURCE_IO,
	},
	[1] = {
		.start	= RTC_IRQ,
		.end	= RTC_IRQ,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device rtc_device = {
	.name		= "rtc_cmos",
	.id		= -1,
	.resource	= rtc_resources,
	.num_resources	= ARRAY_SIZE(rtc_resources),
};

static __init int add_rtc_cmos(void)
{
#ifdef CONFIG_PNP
	static const char * const  const ids[] __initconst =
	    { "PNP0b00", "PNP0b01", "PNP0b02", };
	struct pnp_dev *dev;
	struct pnp_id *id;
	int i;

	pnp_for_each_dev(dev) {
		for (id = dev->id; id; id = id->next) {
			for (i = 0; i < ARRAY_SIZE(ids); i++) {
				if (compare_pnp_id(id, ids[i]) != 0)
					return 0;
			}
		}
	}
#endif
	if (of_have_populated_dt())
		return 0;

	/*                                           */
	if (mrst_identify_cpu())
		return -ENODEV;

	platform_device_register(&rtc_device);
	dev_info(&rtc_device.dev,
		 "registered platform RTC device (no PNP device found)\n");

	return 0;
}
device_initcall(add_rtc_cmos);
