/*
 * Coldfire generic GPIO support.
 *
 * (C) Copyright 2009, Steven King <sfking@fdwdc.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef mcfgpio_h
#define mcfgpio_h

#ifdef CONFIG_GPIOLIB
#include <asm-generic/gpio.h>
#else

int __mcfgpio_get_value(unsigned gpio);
void __mcfgpio_set_value(unsigned gpio, int value);
int __mcfgpio_direction_input(unsigned gpio);
int __mcfgpio_direction_output(unsigned gpio, int value);
int __mcfgpio_request(unsigned gpio);
void __mcfgpio_free(unsigned gpio);

/*                                   */
static inline int __gpio_get_value(unsigned gpio)
{
	if (gpio < MCFGPIO_PIN_MAX)
		return __mcfgpio_get_value(gpio);
	else
		return -EINVAL;
}

static inline void __gpio_set_value(unsigned gpio, int value)
{
	if (gpio < MCFGPIO_PIN_MAX)
		__mcfgpio_set_value(gpio, value);
}

static inline int __gpio_cansleep(unsigned gpio)
{
	if (gpio < MCFGPIO_PIN_MAX)
		return 0;
	else
		return -EINVAL;
}

static inline int __gpio_to_irq(unsigned gpio)
{
	return -EINVAL;
}

static inline int gpio_direction_input(unsigned gpio)
{
	if (gpio < MCFGPIO_PIN_MAX)
		return __mcfgpio_direction_input(gpio);
	else
		return -EINVAL;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	if (gpio < MCFGPIO_PIN_MAX)
		return __mcfgpio_direction_output(gpio, value);
	else
		return -EINVAL;
}

static inline int gpio_request(unsigned gpio, const char *label)
{
	if (gpio < MCFGPIO_PIN_MAX)
		return __mcfgpio_request(gpio);
	else
		return -EINVAL;
}

static inline void gpio_free(unsigned gpio)
{
	if (gpio < MCFGPIO_PIN_MAX)
		__mcfgpio_free(gpio);
}

#endif /*                */


/*
                                                                            
                                                                              
                                                                           
                                                                              
                                                                             
                                                                          
                                                                        
                     
  
                                                                            
                                                                             
 */
#if defined(CONFIG_M5206) || defined(CONFIG_M5206e) || \
    defined(CONFIG_M520x) || defined(CONFIG_M523x) || \
    defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
    defined(CONFIG_M53xx) || defined(CONFIG_M54xx) || \
    defined(CONFIG_M5441x)

/*                                                */

#define MCFGPIO_PORTTYPE		u8
#define MCFGPIO_PORTSIZE		8
#define mcfgpio_read(port)		__raw_readb(port)
#define mcfgpio_write(data, port)	__raw_writeb(data, port)

#elif defined(CONFIG_M5307) || defined(CONFIG_M5407) || defined(CONFIG_M5272)

/*                                                 */

#define MCFGPIO_PORTTYPE		u16
#define MCFGPIO_PORTSIZE		16
#define mcfgpio_read(port)		__raw_readw(port)
#define mcfgpio_write(data, port)	__raw_writew(data, port)

#elif defined(CONFIG_M5249) || defined(CONFIG_M525x)

/*                                                 */

#define MCFGPIO_PORTTYPE		u32
#define MCFGPIO_PORTSIZE		32
#define mcfgpio_read(port)		__raw_readl(port)
#define mcfgpio_write(data, port)	__raw_writel(data, port)

#endif

#define mcfgpio_bit(gpio)		(1 << ((gpio) %  MCFGPIO_PORTSIZE))
#define mcfgpio_port(gpio)		((gpio) / MCFGPIO_PORTSIZE)

#if defined(CONFIG_M520x) || defined(CONFIG_M523x) || \
    defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
    defined(CONFIG_M53xx) || defined(CONFIG_M5441x)
/*
                                                                              
                                                                             
                                                                           
 */
#if defined(CONFIG_M528x)
/*
                                                                 
                                                                               
 */
#define MCFGPIO_SCR_START		40
#elif defined(CONFIGM5441x)
/*                                                              */
#define MCFGPIO_SCR_START		0
#else
#define MCFGPIO_SCR_START		8
#endif

#define MCFGPIO_SETR_PORT(gpio)		(MCFGPIO_SETR + \
					mcfgpio_port(gpio - MCFGPIO_SCR_START))

#define MCFGPIO_CLRR_PORT(gpio)		(MCFGPIO_CLRR + \
					mcfgpio_port(gpio - MCFGPIO_SCR_START))
#else

#define MCFGPIO_SCR_START		MCFGPIO_PIN_MAX
/*                                                                   */
#define MCFGPIO_SETR_PORT(gpio)		0
#define MCFGPIO_CLRR_PORT(gpio)		0

#endif
/*
                                     
 */

/*                                              */
static inline u32 __mcfgpio_ppdr(unsigned gpio)
{
#if defined(CONFIG_M5206) || defined(CONFIG_M5206e) || \
    defined(CONFIG_M5307) || defined(CONFIG_M5407)
	return MCFSIM_PADAT;
#elif defined(CONFIG_M5272)
	if (gpio < 16)
		return MCFSIM_PADAT;
	else if (gpio < 32)
		return MCFSIM_PBDAT;
	else
		return MCFSIM_PCDAT;
#elif defined(CONFIG_M5249) || defined(CONFIG_M525x)
	if (gpio < 32)
		return MCFSIM2_GPIOREAD;
	else
		return MCFSIM2_GPIO1READ;
#elif defined(CONFIG_M520x) || defined(CONFIG_M523x) || \
      defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
      defined(CONFIG_M53xx) || defined(CONFIG_M5441x)
#if !defined(CONFIG_M5441x)
	if (gpio < 8)
		return MCFEPORT_EPPDR;
#if defined(CONFIG_M528x)
	else if (gpio < 16)
		return MCFGPTA_GPTPORT;
	else if (gpio < 24)
		return MCFGPTB_GPTPORT;
	else if (gpio < 32)
		return MCFQADC_PORTQA;
	else if (gpio < 40)
		return MCFQADC_PORTQB;
#endif /*                       */
	else
#endif /*                         */
		return MCFGPIO_PPDR + mcfgpio_port(gpio - MCFGPIO_SCR_START);
#else
	return 0;
#endif
}

/*                                                 */
static inline u32 __mcfgpio_podr(unsigned gpio)
{
#if defined(CONFIG_M5206) || defined(CONFIG_M5206e) || \
    defined(CONFIG_M5307) || defined(CONFIG_M5407)
	return MCFSIM_PADAT;
#elif defined(CONFIG_M5272)
	if (gpio < 16)
		return MCFSIM_PADAT;
	else if (gpio < 32)
		return MCFSIM_PBDAT;
	else
		return MCFSIM_PCDAT;
#elif defined(CONFIG_M5249) || defined(CONFIG_M525x)
	if (gpio < 32)
		return MCFSIM2_GPIOWRITE;
	else
		return MCFSIM2_GPIO1WRITE;
#elif defined(CONFIG_M520x) || defined(CONFIG_M523x) || \
      defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
      defined(CONFIG_M53xx) || defined(CONFIG_M5441x)
#if !defined(CONFIG_M5441x)
	if (gpio < 8)
		return MCFEPORT_EPDR;
#if defined(CONFIG_M528x)
	else if (gpio < 16)
		return MCFGPTA_GPTPORT;
	else if (gpio < 24)
		return MCFGPTB_GPTPORT;
	else if (gpio < 32)
		return MCFQADC_PORTQA;
	else if (gpio < 40)
		return MCFQADC_PORTQB;
#endif /*                       */
	else
#endif /*                         */
		return MCFGPIO_PODR + mcfgpio_port(gpio - MCFGPIO_SCR_START);
#else
	return 0;
#endif
}

/*                                                    */
static inline u32 __mcfgpio_pddr(unsigned gpio)
{
#if defined(CONFIG_M5206) || defined(CONFIG_M5206e) || \
    defined(CONFIG_M5307) || defined(CONFIG_M5407)
	return MCFSIM_PADDR;
#elif defined(CONFIG_M5272)
	if (gpio < 16)
		return MCFSIM_PADDR;
	else if (gpio < 32)
		return MCFSIM_PBDDR;
	else
		return MCFSIM_PCDDR;
#elif defined(CONFIG_M5249) || defined(CONFIG_M525x)
	if (gpio < 32)
		return MCFSIM2_GPIOENABLE;
	else
		return MCFSIM2_GPIO1ENABLE;
#elif defined(CONFIG_M520x) || defined(CONFIG_M523x) || \
      defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
      defined(CONFIG_M53xx) || defined(CONFIG_M5441x)
#if !defined(CONFIG_M5441x)
	if (gpio < 8)
		return MCFEPORT_EPDDR;
#if defined(CONFIG_M528x)
	else if (gpio < 16)
		return MCFGPTA_GPTDDR;
	else if (gpio < 24)
		return MCFGPTB_GPTDDR;
	else if (gpio < 32)
		return MCFQADC_DDRQA;
	else if (gpio < 40)
		return MCFQADC_DDRQB;
#endif /*                       */
	else
#endif /*                         */
		return MCFGPIO_PDDR + mcfgpio_port(gpio - MCFGPIO_SCR_START);
#else
	return 0;
#endif
}

#endif /*           */
