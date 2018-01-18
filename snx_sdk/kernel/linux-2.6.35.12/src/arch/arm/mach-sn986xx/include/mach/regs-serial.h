/** \file regs-serial.h
 * Functions in this file are show :
 * \n 1.ST58200 Uart Control registers offset address
 * \n 2.Control registers' bit operation
 * \n 
 * \author Qingbin Li
 * \date   2011-11-21
 */


#ifndef __MACH_REGS_SERIAL_H
#define __MACH_REGS_SERIAL_H

#include <mach/platform.h>


#ifndef BIT
#define BIT(n)				(1<<n) /*!< helper macro of bit set */
#endif
#define BIC(n)				(~BIT(n)) /*!< helper macro of bit clear */


#define CONFIG_BAUDRATE 	115200 /*!< uart's default baud rate */


/**
 * \defgroup REGS_SERIAL_H_G1 ST58200 Uart Config Register
 * Defined helper macro of the config register's offset address 
 * \n and register's bits operations
 * @{
 */
#define UART_CONFIG 		0x0 /*!< offset address of fifo control register (from uart's base address) */
#define TX_MODE_UART		0 /*!< RS232 TX Mode bit field (1: RS232 TX Mode, 0: TX GPIO Mode.) */
#define TX_STOP_BIT_2		1 /*!< RS232 Stop bit number selection filed (0-> 1bits , 1-> 2bits) */
#define TX_RDY				2 /*!< RS232 TX RDY bit filed (1: Ready for CPU Write to RS232 TX.) */
#define TX_RDY_BIT			(1<<TX_RDY) /*!< Ready for CPU Write to RS232 TX */
#define RX_MODE_UART		4 /*!< RS232 RX Mode bit field (1: RS232 RX Mode, 0: RX GPIO Mode.) */
#define RX_RDY				5 /*!< RS232 RX RDY bit filed (1: Ready for CPU read RS232 RX.) */
#define RX_RDY_BIT			(1<<RX_RDY) /*!< Ready for CPU Write to RS232 RX */
#define RS485_EN			7 /*!< RS485 enable bit filed */
#define RS232_TX_INT_EN 	16 /*!< enable TX interrupt bit filed */
#define RS232_RX_INT_EN 	17 /*!< enable RX interrupt bit filed */
#define RS232_TX_INT_CLR	18 /*!< clear TX interrupt flag bit filed */
#define RS232_RX_INT_CLR	19 /*!< clear RX interrupt flag bit filed */
/** @} */

/**
 * \defgroup REGS_SERIAL_H_G2 ST58200 Uart Clock Register
 * Defined helper macro of the clock register's offset address 
 * \n and register's bits operations
 * @{
 */
#define UART_CLOCK			0xc /*!< offset address of fifo control register (from uart's base address) */
#define RS_RATE 				0 /*!< RS232 TX/RX rate bits filed */
#define RS_RATE_MASK		0x7f /*!< TX/RX rate bits mask */
#define TRX_BASE			7 /*!< TRX rate generate base val bits filed (Used to generate the 115200 Hz clock) */
#define TRX_BASE_MASK		0xf80 /*!< TRX rate base val bits mask */
/** @} */ 

/**
 * \defgroup REGS_SERIAL_H_G3 ST58200 Uart Data Register
 * Defined helper macro of the data register's offset address 
 * \n and register's bits operations
 * @{
 */
#define RS_DATA 				0x10 /*!< offset address of data register(from uart's base address) */
#define DATA_MASK			0xff /*!< 8 bits data mask */
/** @} */

/**
 * \defgroup REGS_SERIAL_H_G4 ST58200 Uart FIFO Register
 * Defined helper macro of the fifo control register's offset address 
 * \n and register's bits operations
 * @{
 */
#define UART_FIFO			0x18 /*!< offset address of fifo control register(from uart's base address) */
#define FIFO_THD_MASK		0x1F /*!< fifo's trigger level bits mask */
#define FIFO_CNT_MASK		0x3F /*!< fifo's counter bits mask */
#define TX_FIFO_THD 		0 /*!< fifo's transmit trigger level bit start */
#define RX_FIFO_THD 		8 /*!< fifo's recvie trigger level bit start */
#define TX_FIFO_CNT 		16 /*!< fifo's transmit counter bit start */
#define RX_FIFO_CNT 		24 /*!< fifo's recvie counter bit start */
#define FIFO_SIZE			32 /*!< uart fifio size */
/** @} */

#define UART_NR 				2 /*!< number of uart controllers */

/*
 * SONiX UART controller virtual addresses
 */
#if 0
#define UART_THR		IO_ADDRESS(SNX_UART_THR)
#define UART_RBR		IO_ADDRESS(SNX_UART_RBR)
#define UART_IER		IO_ADDRESS(SNX_UART_IER)
#define UART_FCR		IO_ADDRESS(SNX_UART_FCR)
#define UART_LCR		IO_ADDRESS(SNX_UART_LCR)
#define UART_MCR		IO_ADDRESS(SNX_UART_MCR)
#define UART_LSR		IO_ADDRESS(SNX_UART_LSR)
#define UART_MSR		IO_ADDRESS(SNX_UART_MSR)
#define UART_SPR		IO_ADDRESS(SNX_UART_SPR)
#define UART_MDR		IO_ADDRESS(SNX_UART_MDR)

/*
 * SONiX UART controller physical addresses
 */
#define SNX_UART_THR			(SNX_UART0_BASE + UART_THR_OFFSET)
#define SNX_UART_RBR			(SNX_UART0_BASE + UART_RBR_OFFSET)
#define SNX_UART_IER			(SNX_UART0_BASE + UART_IER_OFFSET)
#define SNX_UART_FCR			(SNX_UART0_BASE + UART_FCR_OFFSET)
#define SNX_UART_LCR			(SNX_UART0_BASE + UART_LCR_OFFSET)
#define SNX_UART_MCR			(SNX_UART0_BASE + UART_MCR_OFFSET)
#define SNX_UART_LSR			(SNX_UART0_BASE + UART_LSR_OFFSET)
#define SNX_UART_MSR			(SNX_UART0_BASE + UART_MSR_OFFSET)
#define SNX_UART_SPR			(SNX_UART0_BASE + UART_SPR_OFFSET)
#define SNX_UART_MDR			(SNX_UART0_BASE + UART_MDR_OFFSET)

#else

#define SNX_UART_LSR			(SNX_UART0_BASE + UART_CONFIG)
#define SNX_UART_THR			(SNX_UART0_BASE + RS_DATA)
#endif

/* UART controller registers offset */
#define UART_THR_OFFSET		0x00	/* Transmitter Holding */ 
#define UART_RBR_OFFSET		0x00	/* Receive Buffer */ 
#define UART_IER_OFFSET		0x04	/* Interrupt Enable */
#define UART_FCR_OFFSET		0x08	/* FIFO control */
#define UART_LCR_OFFSET		0x0c	/* Line Control */
#define UART_MCR_OFFSET		0x10	/* Modem Control */
#define UART_LSR_OFFSET		0x14	/* Line status */
#define UART_MSR_OFFSET		0x18	/* Modem Status */
#define UART_SPR_OFFSET		0x1c	/* Scratch Pad */
#define UART_MDR_OFFSET		0x20	/* Mode Definition */

/*
 * LSR register bits
 */
#define LSR_DE			(0x1<<7)     	/* FIFO Data Error */
#define LSR_TE			(0x1<<6)     	/* Transmitter Empty */
#define LSR_TR			(0x1<<5)     	/* THR Empty */
#define LSR_BI			(0x1<<4)     	/* Break Interrupt */
#define LSR_FE			(0x1<<3)      	/* Framing Error */
#define LSR_PE			(0x1<<2)      	/* Parity Error */
#define LSR_OE			(0x1<<1)      	/* Overrun Error */
#define LSR_DR			(0x1)      	/* Data Ready */

#endif
