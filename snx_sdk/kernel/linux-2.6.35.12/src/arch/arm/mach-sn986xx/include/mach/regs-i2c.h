/* file regs-i2c.h
 * Functions list:
 * 1. Define the I2C controller registers offset
 * 2. Define the I2C control register bits
 * 3. Define the I2C status register bits
 *
 * author Timing Gong
 * date   2012/01/18
 */

#ifndef __MACH_REGS_I2C_H
#define __MACH_REGS_I2C_H

#include <mach/platform.h>

/* 
 * defgroup  REGS_I2C_G1 I2C controller registers offset 
 */
#define I2C_ICR_OFFSET			0x00	/* The offset of I2C Control register */
#define I2C_ISR_OFFSET			0x04	/* The offset of I2C Status register */
#define I2C_ICDR_OFFSET			0x08	/* The offset of I2C Clock Division register */
#define I2C_IDBR_OFFSET			0x0c    /* The offset of I2C Data register */
#define I2C_ISAR_OFFSET			0x10	/* The offset of I2C Slave Address register */
#define I2C_TGSR_OFFSET			0x14	/* The offset of I2C Setup/Hold Time/Glitch Suppression Setting register */
#define I2C_IBMR_OFFSET			0x18	/* The offset of I2C Bus Monitor register */

/*
 * defgroup REGS_I2C_G2 I2C control register bits
 */
#define ICR_STARTIE     (0x1<<14)     	/* Enable detecting start condition interrupt */
#define ICR_ALDIE       (0x1<<13)     	/* Enable detecting arbitration lose interrupt */
#define ICR_SAMIE       (0x1<<12)     	/* Enable detecting slave address match interrupt */
#define ICR_STOPIE      (0x1<<11)     	/* Enable detecting stop condition interrupt */
#define ICR_BEIE        (0x1<<10)     	/* Enable detecting non-ACK responses interrupt */
#define ICR_DRIE        (0x1<<9)     	/* Enable receiveing one data byte interrupt */
#define ICR_DTIE        (0x1<<8)     	/* Enable transmitting one data byte interrupt */
#define ICR_TB          (0x1<<7)     	/* Enable transfer byte */
#define ICR_ACKNAK      (0x1<<6)     	/* Send NACK in master receive mode */
#define ICR_STOP        (0x1<<5)     	/* Initiate stop condition */
#define ICR_START       (0x1<<4)     	/* Initiate start condition */
#define ICR_GCE         (0x1<<3)     	/* Enable responding general call message */
#define ICR_SCLE        (0x1<<2)     	/* Enable I2C clock output */
#define ICR_I2CE        (0x1<<1)     	/* Enable I2C controller */
#define ICR_RST         (0x1<<0)     	/* Reset I2C controller */

/*
 * defgroup REGS_I2C_G3 I2C status register bits
 */
#define ISR_START       (0x1<<11)     	/* Detect start condition */
#define ISR_ALD         (0x1<<10)     	/* Detect arbitration lose */
#define ISR_GCD         (0x1<<9)     	/* Detect general call message */
#define ISR_SAM         (0x1<<8)     	/* Detect slave address match */
#define ISR_STOP        (0x1<<7)     	/* Detect stop condition */
#define ISR_BED         (0x1<<6)     	/* Detect non-ACK responses */
#define ISR_DR          (0x1<<5)     	/* Receive one data byte */
#define ISR_DT          (0x1<<4)     	/* Transmit one data byte */
#define ISR_BB          (0x1<<3)     	/* I2C bus busy */
#define ISR_CB          (0x1<<2)     	/* I2C controller busy */
#define ISR_ACKNAK      (0x1<<1)     	/* Receive/send ACK/NACK */
#define ISR_RW          (0x1<<0)     	/* Receive or transmit mode */

#endif
