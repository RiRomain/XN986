/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_REGS_SMC_H
#define __MACH_REGS_SMC_H

#include <mach/platform.h>

/*
 * SONiX SMC controller virtual addresses
 */
#define SMC_BNK0_CONFIG			IO_ADDRESS(SNX_SMC_BNK_CONFIG)
#define SMC_BNK0_PARAM			IO_ADDRESS(SNX_SMC_BNK_PARAM)

/*
 * SONiX SMC controller physical addresses
 */
#define SNX_SMC_BNK_CONFIG			(SNX_SMC_BASE + SMC_BNK0_CONFIG_OFFSET)
#define SNX_SMC_BNK_PARAM			(SNX_SMC_BASE + SMC_BNK0_PARAM_OFFSET)

/* SMC controller registers offset */
#define SMC_BNK0_CONFIG_OFFSET			0x00	/* SMC bank0 configuration register */
#define SMC_BNK0_PARAM_OFFSET			0x04	/* SMC bank0 timing parameter register */

/*
 * SMC bank configuration register bits
 */
#define BNK0_EN			(0x1<<28)     	/* Bank0 enable */
#define BNK0_PROT		(0x1<<11)     	/* Bank0 write protect */
#define BNK0_TYP1		(0x1<<10)     	/* Bank0 type1*/
#define BNK0_TYP2		(0x1<<9)     	/* Bank0 type2*/
#define BNK0_TYP3		(0x1<<8)     	/* Bank0 type3*/

#endif
