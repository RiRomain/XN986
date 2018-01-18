#ifndef __SNX_MACH_GPIO_H__
#define __SNX_MACH_GPIO_H__

#define gpio_get_value	__gpio_get_value
#define gpio_set_value	__gpio_set_value
#define gpio_cansleep	__gpio_cansleep
#define gpio_to_irq	__gpio_to_irq

#define ARCH_NR_GPIOS	(4)

#ifdef	CONFIG_MACH_SN98610
#define ARCH_NR_GPIOS	(6)
#endif

#ifdef CONFIG_SYSTEM_PLATFORM_SN98660
#define ARCH_NR_GPIOS (7)
#endif

#include <asm-generic/gpio.h>



#endif /*__SNX_MACH_GPIO_H__*/
