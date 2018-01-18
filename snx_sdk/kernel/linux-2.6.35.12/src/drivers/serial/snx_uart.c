
/** \file snx_uart.c
 * Functions in this file are show :
 * \n 1.SONiX Uart Driver Implementation
 * \n 2.Use RX-Timeout-Interrupt Machanism
 * \n 
 * \Copyright:	(C) 2011 SONIX Technology, Inc.
 * \author Qingbin Li, Saxen Ko
 * \date   2011-09-08
 */


#if defined(CONFIG_SERIAL_SNX_CONSOLE) && defined(CONFIG_MAGIC_SYSRQ)
#define SUPPORT_SYSRQ
#endif

#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/circ_buf.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/serial_reg.h>
#include <linux/ctype.h>

#include <asm/io.h>
#include <asm/irq.h>

#include "generated/snx_sdk_conf.h" 

#include <mach/regs-serial.h>
//#define DBG_UART
#ifdef DBG_UART
#define DBG_UART(x...) 	printk(KERN_INFO x)
#else
#define DBG_UART(x...)   do { } while (0)
#endif
/** \struct snx_uart_port
 * \brief platform uart port data structure
 * \n
 * \n port:linux generic uart port abstraction struct
 * \n res:resource used by this port
 * \n clk:resource used by this port
 * \n name:port device name
 * \n baud:current device operation baud rate
 * \n rx_time_out:next timer time(in jiffies unit) 
 * \n rx_timer:rx-time-out timer
 * \n rx_timeout_enable:rx-time-out assert
 * \n rx_fifo_pre_cnt:store previous fifo counter value
 *
 */
struct snx_uart_port {
	struct uart_port	port;
	struct resource		*res;
	struct clk		*clk;
	char			*name;
	unsigned long		baud;
	unsigned int 		rx_time_out;
	struct timer_list 	rx_timer;
	int			rx_timeout_enable;
	unsigned int		rx_fifo_pre_cnt;
};

/** 
 * \def serial_in(sup, offset)
 * hardware register read macro
 */
#define serial_in(sup, offset)		readl(sup->port.membase + offset)

/** 
 * \def serial_out(sup, offset, value)
 * hardware register write macro
 */
#define serial_out(sup, offset, value)	writel(value, sup->port.membase + offset)

#define RX_TRIG_LEVEL	8	/*!< hardware RX interrupt trigger level default set value */
#define TX_TRIG_LEVEL	8	/*!< hardware TX interrupt trigger level default set value */


/**
 * \defgroup SERIAL_SNX_C_G1 FIFO Operation Helper Macros
 * Defined helper macro of the fifo control register's operations
 * \n
 * @{
 */

/** 
 * \def serial_out(sup, offset, value)
 * get data count in tx fifo
 */
#define TX_DAT_CNT(sup)\
({\
	unsigned int val;\
	val = serial_in(sup, UART_FIFO);\
	val = ((val>>TX_FIFO_CNT)&(FIFO_CNT_MASK));\
	val;\
})


/** 
 * \def serial_out(sup, offset, value)
 * get data count in rx fifo
 */
#define RX_DAT_CNT(sup)\
({\
	unsigned int val;\
	val = serial_in(sup, UART_FIFO);\
	val = ((val>>RX_FIFO_CNT)&(FIFO_CNT_MASK));\
	val;\
})

/** 
 * \def serial_out(sup, offset, value)
 * detect tx fifo full status
 */
#define TX_FIFO_Full(sup)\
({\
	unsigned int val;\
	val = serial_in(sup, UART_FIFO);\
	val = ((val>>TX_FIFO_CNT)&(FIFO_CNT_MASK));\
	val = (FIFO_SIZE-val)?(0):(1);\
	val;\
})

/** 
 * \def serial_out(sup, offset, value)
 * detect rx fifo full status
 */
#define RX_FIFO_Full(sup)\
({\
	unsigned int val;\
	val = serial_in(sup, UART_FIFO);\
	val = ((val>>RX_FIFO_CNT)&(FIFO_CNT_MASK));\
	val = (FIFO_SIZE-val)?(0):(1);\
	val;\
})


/** @} */


/**
 * \defgroup SERIAL_SNX_C_G2 Interrupt Enable/Disable functions
 * 
 * \n 
 * @{
 */

/** \fn void enable_tx_interrupt(struct snx_uart_port *s)
 * \brief enable tx interrupt
 * \param s :platform uart port data
 */
static inline void enable_tx_interrupt (struct snx_uart_port *s)
{
  
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_CONFIG);
	val |= BIT (RS232_TX_INT_EN);
  DBG_UART ("%s:%d: write UART_CONFIG val = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_CONFIG, val);
}

/** \fn void disable_tx_interrupt(struct snx_uart_port *s)
 * \brief disable tx interrupt
 */
static inline void disable_tx_interrupt (struct snx_uart_port *s)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_CONFIG);
	val &= BIC (RS232_TX_INT_EN);
  DBG_UART ("%s:%d: write UART_CONFIG val = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_CONFIG, val);	
}

/** \fn void enable_rx_interrupt(struct snx_uart_port *s)
 * \brief enable rx interrupt
 */
static inline void enable_rx_interrupt (struct snx_uart_port *s)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_CONFIG);
	val |= BIT (RS232_RX_INT_EN);
  DBG_UART ("%s:%d: write UART_CONFIG val = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_CONFIG, val);	
}

/** \fn void disable_rx_interrupt(struct snx_uart_port *s)
 * \brief disable rx interrupt
 */
static inline void disable_rx_interrupt (struct snx_uart_port *s)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_CONFIG);
	val &= BIC (RS232_RX_INT_EN);
  DBG_UART ("%s:%d: write UART_CONFIG val = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_CONFIG, val);
}

/** \fn void clear_tx_interrupt_flag(struct snx_uart_port *s)
 * \brief clear tx interrupt flag
 */
static inline void clear_tx_interrupt_flag (struct snx_uart_port *s)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_CONFIG);
	val |= BIT (RS232_TX_INT_CLR);
  DBG_UART ("%s:%d: write UART_CONFIG val = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_CONFIG, val);
}

/** \fn void clear_rx_interrupt_flag(struct snx_uart_port *s)
 * \brief clear rx interrupt flag
 */
static inline void clear_rx_interrupt_flag (struct snx_uart_port *s)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_CONFIG);
	val |= BIT (RS232_RX_INT_CLR);
  DBG_UART ("%s:%d: write UART_CONFIG val = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_CONFIG, val);
}

/** \fn void set_tx_trigger_level(struct snx_uart_port *s, unsigned int v)
 * \brief set tx trigger level
 */
static inline void set_tx_trigger_level (struct snx_uart_port *s, unsigned int v)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_FIFO);
	val &= ~(FIFO_THD_MASK/*<<TX_FIFO_THD*/); //alek TX_FIFO_THD 0 so o need shift
	val |= (v/*<<TX_FIFO_THD*/); //alek TX_FIFO_THD 0 so o need shift
  DBG_UART ("%s:%d: write UART_FIFO = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_FIFO, val);
}

/** \fn void set_rx_trigger_level(struct snx_uart_port *s, unsigned int v)
 * \brief set rx trigger level
 */
static inline void set_rx_trigger_level (struct snx_uart_port *s, unsigned int v)
{
	unsigned int val;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	val = serial_in (s, UART_FIFO);
	val &= ~(FIFO_THD_MASK<<RX_FIFO_THD);
	val |= (v<<RX_FIFO_THD);
  DBG_UART ("%s:%d: write UART_FIFO = %x\n", __func__, __LINE__, val);
	serial_out (s, UART_FIFO, val);
}


/** @} */


/** \fn void receive_chars(struct snx_uart_port *sup)
 * \brief receive_chars - read datas from hardware fifo
 * \details 
 * \n 1)read datas from fifo
 * \n 2)acount rx statistic
 * \n 3)deal with system request char
 * \n 4)buffer received chars
 * \n 5)flip chars in buffer to read thread
 * \n
 * \param sup :platform uart port data
 * \return NON
 */

static  void receive_chars (struct snx_uart_port *sup)
{
	struct tty_struct *tty = sup->port.state->port.tty;
	unsigned int ch, flag;
	int max_count = 128;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	while (max_count-- > 0)
	{
		if (!RX_DAT_CNT(sup))
			break;
		
		ch = serial_in (sup, RS_DATA);
		flag = TTY_NORMAL;
		sup->port.icount.rx++;

		if (uart_handle_sysrq_char (&sup->port, ch))
			continue;

		uart_insert_char (&sup->port, 0, 0, ch, flag);
	} 
	tty_flip_buffer_push (tty);
}


/** \fn void transmit_chars(struct snx_uart_port *sup)
 * \brief transmit_chars - write datas to hardware fifo
 * \details 
 * \n 1)read datas from fifo
 * \n 2)acount rx statistic
 * \n 3)deal with system request char
 * \n 4)buffer received chars
 * \n 5)flip chars in buffer to read thread
 * \n
 * \param sup :platform uart port data
 * \return NON
 */
static void transmit_chars (struct snx_uart_port *sup)
{
	struct circ_buf *xmit = &sup->port.state->xmit;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);

	if (sup->port.x_char) {
		sup->port.icount.tx++;
		sup->port.x_char = 0;
		return;
	}

	if (uart_circ_empty (xmit))
		return;

	while (!TX_FIFO_Full (sup))
	{
		serial_out (sup, RS_DATA, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sup->port.icount.tx++;
		
		if (uart_circ_empty (xmit))
			break;
	} 

	if (uart_circ_chars_pending (xmit) < WAKEUP_CHARS)
		uart_write_wakeup (&sup->port);
}


/** \fn void rx_timer_fn(unsigned long arg)
 * \brief rx_timer_fn - implement software rx-timeout machanism
 * \details 
 * \n hardware hasn't rx-timeout interrupt,if want use fifo while rx,
 * \n need to implement software rx-timeout machanism
 * \n
 * \param arg :which specify platform uart port data address
 * \return NON
 */
static void rx_timer_fn (unsigned long arg)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)(arg);
	unsigned long flags, cnt;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	cnt = RX_DAT_CNT (sup);

	if (!cnt){
		goto out;
	}
	
	local_irq_save (flags);
	if ((sup->rx_timeout_enable) && (sup->rx_fifo_pre_cnt == cnt)){
		receive_chars (sup);
		sup->rx_timeout_enable = 0;
		sup->rx_fifo_pre_cnt = 0;
		local_irq_restore (flags);
		goto out;
	}
	sup->rx_timeout_enable = 1;
	sup->rx_fifo_pre_cnt = cnt;
	local_irq_restore (flags);

out:	
	mod_timer (&sup->rx_timer, (jiffies + sup->rx_time_out));
	return;
}


/** \fn irqreturn_t snx_serial_irq(int irq, void *dev_id)
 * \brief snx_serial_irq - This handles the interrupt from one port.
 * \details 
 * \n 1)this handle demux rx/tx interrupt,
 * \n 2)the flow of deal with interrupt
 * \n	.disable interrupt
 * \n	.clear interrupt flag
 * \n	.process datas
 * \n	.enable interrupt
 * \n
 * \param irq :irq nomber
 * \param dev_id :platform uart port data
 * \return IRQ_HANDLED:sucess
 */
static  irqreturn_t snx_serial_irq (int irq, void *dev_id)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)dev_id;
	unsigned int lsr;
	DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	if (irq != sup->port.irq)
		return IRQ_NONE;

	lsr = serial_in (sup, UART_CONFIG);
	
	if ((lsr & RX_RDY_BIT) && (lsr & (BIT(RS232_RX_INT_EN))))
	{
		// 1)fist disable rx interrupt
		disable_rx_interrupt (sup);
		
		// 2)clear INTR
		clear_rx_interrupt_flag (sup);

		receive_chars(sup);
		sup->rx_timeout_enable = 0;
		sup->rx_fifo_pre_cnt = 0;
		
		// 3)enable timer
		mod_timer (&sup->rx_timer, (jiffies + sup->rx_time_out));

		// 4)enable rx interrupt
		enable_rx_interrupt (sup);
	}

	lsr = serial_in (sup, UART_CONFIG);
	if ((lsr & TX_RDY_BIT) && (lsr & (BIT (RS232_TX_INT_EN))))
	{		
		// 1)fist disable tx interrupt
		disable_tx_interrupt (sup);
		// 2)clear INTR
		clear_tx_interrupt_flag (sup);
		// 3)process tx
		transmit_chars (sup);	
		// 4)enable tx interrupt
		enable_tx_interrupt (sup);
	}

	return IRQ_HANDLED;
}




/**
 * \defgroup SERIAL_SNX_C_G3 Generic Linux Serial Driver Call functions
 * Implement callback functions which adapt to "struct uart_ops"
 * \n 
 * @{
 */

/** \fn unsigned int snx_uart_tx_empty(struct uart_port *port)
 * \brief snx_uart_tx_empty - Return TIOCSER_TEMT when transmitter is not busy.
 * \details 
 * \n read fifo tx counter to detect empty
 * \n
 * \param port :generic "struct uart_port" data address
 * \return 0:success, !0:fail
 */
static unsigned int snx_uart_tx_empty (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
	unsigned long flags;
	unsigned int ret;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	spin_lock_irqsave (&sup->port.lock, flags);
	ret = (TX_DAT_CNT(sup) == 0) ? TIOCSER_TEMT : 0;
	spin_unlock_irqrestore (&sup->port.lock, flags);
  DBG_UART ("%s:%d: return val = %x\n", __func__, __LINE__, ret);
	return ret;
}

/** \fn void snx_uart_set_mctrl(struct uart_port *port, unsigned int mctrl)
 * \brief snx_uart_set_mctrl - set modem ctrol data
 * \param port :generic "struct uart_port" data address
 * \param mctrl :modem ctrol data
 * \return NON
 */
static void snx_uart_set_mctrl (struct uart_port *port, unsigned int mctrl)
{
}


/** \fn unsigned int snx_uart_get_mctrl(struct uart_port *port)
 * \brief snx_uart_get_mctrl - get modem ctrol data
 * \param port :generic "struct uart_port" data address
 * \return 0:success, !0:fail
 */
static unsigned int snx_uart_get_mctrl (struct uart_port *port)
{
	return 0;
}


/** \fn void snx_uart_start_tx(struct uart_port *port)
 * \brief snx_uart_start_tx - start transmit
 * \param port :generic "struct uart_port" data address
 * \return NON
 */
static void snx_uart_start_tx (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
	struct circ_buf *xmit = &sup->port.state->xmit;
  
	/*
	*be care: make sure to trigger the tx irq
	*	1)need to write datas to fifo firstly and 
	*	2)datas in tx fifo must reach the TX_TRIG_LEVEL.
	*/
	int i;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	for (i = 0; i <= (TX_TRIG_LEVEL + 1); i++)
	{
		if (TX_FIFO_Full (sup) )
			break;
		if (uart_circ_empty (xmit))
			break;
		serial_out (sup, RS_DATA, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		sup->port.icount.tx++;
	}
}


/** \fn void snx_uart_stop_tx(struct uart_port *port)
 * \brief snx_uart_stop_tx - stop transmit
 * \param port :generic "struct uart_port" data address
 * \return NON
 */
static void snx_uart_stop_tx (struct uart_port *port)
{
}


/** \fn void snx_uart_stop_rx(struct uart_port *port)
 * \brief snx_uart_stop_rx - stop revice
 * \param port :generic "struct uart_port" data address
 * \return NON
 */
static void snx_uart_stop_rx (struct uart_port *port)
{
  
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	sup->port.read_status_mask &= ~UART_LSR_DR;
}


/** \fn void snx_uart_enable_ms(struct uart_port *port)
 * \brief snx_uart_enable_ms - enable uart modem status
 * \n
 * \param port :generic "struct uart_port" data address
 * \return NON
 */
static void snx_uart_enable_ms (struct uart_port *port)
{
}


/** \fn void snx_uart_break_ctl(struct uart_port *port, int break_state)
 * \brief snx_uart_break_ctl - do actual break ctrol operation
 * \param port :generic "struct uart_port" data address
 * \param break_state :break state
 * \return NON
 */
static void snx_uart_break_ctl (struct uart_port *port, int break_state)
{
}

/** \fn int snx_uart_startup(struct uart_port *port)
 * \brief snx_uart_startup - startup serial port
 * \details 
 * \n 1)clear uart transmit buffer
 * \n 2)request irq
 * \n 3)config HW baud rate/fifo trigger level
 * \n 4)setup rx-timeout-timer
 * \n 5)enable interrupt transmit/revice
 * \n
 * \param port :generic "struct uart_port" data address
 * \return 0:success, !0:fail
 */
static int snx_uart_startup (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
	unsigned long config, rate, fsr;
	int retval;
	struct circ_buf *xmit = &sup->port.state->xmit;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	// Clear uart ring buffer
	uart_circ_clear (xmit);

	/*
	 * Allocate the IRQ
	 */
	retval = request_irq (sup->port.irq, snx_serial_irq, IRQF_SHARED,
				 to_platform_device (port->dev)->name, sup);
	if (retval)
		return retval;

	/*
	 * Clear the interrupt registers.
	 */
	config = serial_in (sup, UART_CONFIG);
	config |= BIT (RS232_RX_INT_CLR) | BIT (RS232_TX_INT_CLR);

	/*
	 * Now, initialize the UART
	 */
	config = BIT (TX_MODE_UART) | BIT (RX_MODE_UART);
	serial_out (sup, UART_CONFIG, config);

	rate = (115200/CONFIG_BAUDRATE) & RS_RATE_MASK;
	rate |= (((sup->port.uartclk + (115200 << 2))/(115200 << 3))<<TRX_BASE) & TRX_BASE_MASK;
	serial_out (sup, UART_CLOCK, rate);


	fsr = serial_in (sup, UART_FIFO);
	fsr &= ~(FIFO_THD_MASK<<TX_FIFO_THD | FIFO_THD_MASK<<RX_FIFO_THD);
	fsr = (TX_TRIG_LEVEL<<TX_FIFO_THD) | (RX_TRIG_LEVEL<<RX_FIFO_THD);//?
	serial_out (sup, UART_FIFO, fsr);

	sup->baud = CONFIG_BAUDRATE;
	sup->rx_time_out = 1;// 1jiffies ~= 5ms
	setup_timer (&sup->rx_timer, rx_timer_fn, (unsigned long)(sup));
	mod_timer (&sup->rx_timer, (jiffies + sup->rx_time_out));
	/*
	 * Finally, enable interrupts.	Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	config = serial_in (sup, UART_CONFIG);
	config |= BIT (RS232_RX_INT_EN) | BIT (RS232_TX_INT_EN);
	serial_out (sup, UART_CONFIG, config);

	/*
	 * And clear the interrupt registers again for luck.
	 */
	config = serial_in (sup, UART_CONFIG);
	config |= BIT (RS232_RX_INT_CLR) | BIT (RS232_TX_INT_CLR);
  DBG_UART ("%s:%d: write UART_CONFIG = %x\n", __func__, __LINE__, config);
	serial_out (sup, UART_CONFIG, config);

	return 0;
}

/** \fn void snx_uart_shutdown(struct uart_port *port)
 * \brief snx_uart_shutdown - shutdown serial
 * \n
 * \param port :generic "struct uart_port" data address
 * \return NON
 */
static void snx_uart_shutdown (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
	unsigned long config;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	free_irq (sup->port.irq, sup);

	/*
	 * Disable mode & interrupts from this port
	 */
	config = 0;
  DBG_UART ("%s:%d: write UART_CONFIG = %x\n", __func__, __LINE__, config);
	serial_out (sup, UART_CONFIG, config);
}

/** \fn snx_uart_set_termios(struct uart_port *port, struct ktermios *termios, struct ktermios *old)
 * \brief snx_uart_set_termios - config uart args
 * \details 
 * \n 1)caculate baud rate
 * \n 2)caculate transmit time-out
 * \n 3)write configration datas to HW register
 *
 * \param port :generic "struct uart_port" data address
 * \param termios :uart configuration data struct
 * \param old :old uart configuration data struct
 * \return NON
 */
static void snx_uart_set_termios (struct uart_port *port, struct ktermios *termios, struct ktermios *old)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
	unsigned char fcr = 0;
	unsigned long flags, rate;
	unsigned int baud, quot;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate (port, termios, old, 0, port->uartclk/8);
	quot = uart_get_divisor (port, baud);

	if ((sup->port.uartclk / quot) < (2400 * 8))
		fcr = UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_00;
	else
		fcr = UART_FCR_ENABLE_FIFO | UART_FCR_R_TRIG_11;

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	spin_lock_irqsave (&sup->port.lock, flags);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout (port, termios->c_cflag, baud);

	sup->port.read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		sup->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		sup->port.read_status_mask |= UART_LSR_BI;

	/*
	 * Characters to ignore
	 */
	sup->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		sup->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		sup->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			sup->port.ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		sup->port.ignore_status_mask |= UART_LSR_DR;

	
	rate = (115200/baud) & RS_RATE_MASK;
	rate |= (((sup->port.uartclk + (115200 << 2))/(8 * 115200))<<TRX_BASE) & TRX_BASE_MASK;
	DBG_UART ("%s:%d: write UART_CLOCK = %x\n", __func__, __LINE__, config);
  serial_out (sup, UART_CLOCK, rate);

	spin_unlock_irqrestore (&sup->port.lock, flags);
}


/** \fn void snx_uart_pm(struct uart_port *port, unsigned int state, unsigned int oldstate)
 * \brief snx_uart_pm - enable/disable power
 * \n
 * \param port :generic "struct uart_port" data address
 * \param state :power manegment flags
 * \param oldstate :
 * \return NON
 */
static void snx_uart_pm (struct uart_port *port, unsigned int state, unsigned int oldstate)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	switch (state) {
		case 0:
			/*
			 * Enable the peripheral clock for this serial port.
			 * This is called on uart_open() or a resume event.
			 */
			clk_enable (sup->clk);
			break;
		case 3:
			/*
			 * Disable the peripheral clock for this serial port.
			 * This is called on uart_close() or a suspend event.
			 */
			clk_disable (sup->clk);
			break;
		default:
			printk (KERN_ERR "snx_serial: unknown pm %d\n", state);
	}
}


/** \fn void snx_uart_release_port(struct uart_port *port)
 * \brief snx_uart_release_port - Release the memory region(s) being used by 'port'.
 * \param port :generic "struct uart_port" data address
 * \return NON
 */
static void snx_uart_release_port (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	release_mem_region (sup->port.mapbase, SZ_4K);
}

/** \fn int snx_uart_request_port(struct uart_port *port)
 * \brief snx_uart_request_port - Request the memory region(s) being used by 'port'.
 * \n
 * \param port :generic "struct uart_port" data address
 * \return 0:success, !0:fail
 */
static int snx_uart_request_port (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	return request_mem_region (sup->port.mapbase, SZ_4K, "snx_uart")
			!= NULL ? 0 : -EBUSY;
}

/** \fn void snx_uart_config_port(struct uart_port *port, int flags)
 * \brief snx_uart_config_port - Configure/autoconfigure the port.
 * \n
 * \param port :generic "struct uart_port" data address
 * \param flags :uart configuration flags
 * \return NON
 */
static void snx_uart_config_port (struct uart_port *port, int flags)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	if (flags & UART_CONFIG_TYPE &&
		snx_uart_request_port (&sup->port) == 0)
		sup->port.type = PORT_SNX;
}

/** \fn int snx_uart_verify_port(struct uart_port *port, struct serial_struct *ser)
 * \brief snx_uart_verify_port -  Verify the new serial_struct (for TIOCSSERIAL).
 * \details
 * The only change we allow are to the flags and type, and
 * even then only between PORT_SNX and PORT_UNKNOWN
 * \n
 * \param port :generic "struct uart_port" data address
 * \param ser :serial configruation data struct
 * \return 0:success, !0:fail
 */
static int snx_uart_verify_port (struct uart_port *port, struct serial_struct *ser)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
	int ret = 0;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	if (ser->type != PORT_UNKNOWN && ser->type != PORT_SNX)
		ret = -EINVAL;
	if (ser->irq != sup->port.irq)
		ret = -EINVAL;
	if (ser->io_type != UPIO_MEM)
		ret = -EINVAL;
	if (ser->baud_base != sup->port.uartclk / 16)
		ret = -EINVAL;
	if (ser->iomem_base != (void *)sup->port.mapbase)
		ret = -EINVAL;

	return ret;
}

/** \fn char *snx_uart_type(struct uart_port *port)
 * \brief snx_uart_type - return a pointer to serial type
 * \n
 * \param port :generic "struct uart_port" data address
 * \return pointer to string
 */
static const char *snx_uart_type (struct uart_port *port)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	return sup->port.type == PORT_SNX ? "SONiX" : NULL;
}

/** @}*/


#if 0
static void snx_serial_dma_init (struct pxa_uart *sup)
{
	sup->rxdma =
		pxa_request_dma (sup->name, DMA_PRIO_LOW, pxa_receive_dma, sup);
	if (sup->rxdma < 0)
		goto out;
	sup->txdma =
		pxa_request_dma (sup->name, DMA_PRIO_LOW, pxa_transmit_dma, sup);
	if (sup->txdma < 0)
		goto err_txdma;
	sup->dmadesc = kmalloc (4 * sizeof(pxa_dma_desc), GFP_KERNEL);
	if (!sup->dmadesc)
		goto err_alloc;

	/* ... */
err_alloc:
	pxa_free_dma (sup->txdma);
err_rxdma:
	pxa_free_dma (sup->rxdma);
out:
	return;
}
#endif


#ifdef CONFIG_SERIAL_SNX_CONSOLE

static struct snx_uart_port snx_uart_ports[UART_NR];
static struct snx_uart_port *snx_ports[UART_NR];
static struct uart_driver snx_uart_driver;

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)


/** \fn void wait_for_xmitr(struct snx_uart_port *sup)
 * \brief wait_for_xmitr - wait uart finish transmit,check to see TX_RDY_BIT whether ready
 * \n
 * \param sup :platform uart port data
 * \return NON
 */
static inline void wait_for_xmitr (struct snx_uart_port *sup)
{
	unsigned int status, tmout = 10000;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	/* Wait sup to 10ms for the character(s) to be sent. */
	do {
		status = serial_in (sup, UART_CONFIG);

		if (--tmout == 0)
			break;
		udelay (1);
	} while (!(status & TX_RDY_BIT));
}


/** \fn void snx_serial_console_putchar(struct uart_port *port, int ch)
 * \brief snx_serial_console_putchar - put char to console
 * \details 
 * \n 1)wait uart transmit idle
 * \n 2)write char to uart
 * \param port :generic "struct uart_port" data address
 * \param ch :which uart to operate
 * \return NON
 */
static void snx_serial_console_putchar (struct uart_port *port, int ch)
{
	struct snx_uart_port *sup = (struct snx_uart_port *)port;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	wait_for_xmitr (sup);
  DBG_UART ("%s:%d: write RS_DATA = %x\n", __func__, __LINE__, ch);
	serial_out (sup, RS_DATA, ch);
}


/** \fn void snx_serial_console_write(struct console *co, const char *s, unsigned int count)
 * \brief snx_serial_console_write - Print a string to the serial port trying not to disturb
 * \n any possible real use of the port...
 * \details 
 * \n 1)write datas in buf to uart
 * \n 2)wait for finish
 * \param co :console data struct
 * \param s :datas write to uart
 * \param count :the number of datas will be write
 * \return NON
 */
static void snx_serial_console_write (struct console *co, const char *s, unsigned int count)
{
	struct snx_uart_port *sup = &snx_uart_ports[co->index];
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	uart_console_write (&sup->port, s, count, snx_serial_console_putchar);

	/*
	 *	Finally, wait for transmitter to become empty
	 */
	wait_for_xmitr (sup);
}


/** \fn int snx_serial_console_setup(struct console *co, char *options)
 * \brief snx_serial_console_setup - 
 * \details 
 * \n 1)parse uart option from commandline
 * \n 2)setup uart option
 * \n
 * \param co :console data struct
 * \param options :commandline setup args
 * \return 0:success, !0:fail
 */
static int __init snx_serial_console_setup (struct console *co, char *options)
{
	struct snx_uart_port *sup;
	int baud = CONFIG_BAUDRATE;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	if (co->index == -1 || co->index >= snx_uart_driver.nr)
		co->index = 0;

	sup = &snx_uart_ports[co->index];

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	sup->port.uartclk = 10 * 1000000;
#else
	sup->port.uartclk = APB_CLK;
#endif
	if (options)
		uart_parse_options (options, &baud, &parity, &bits, &flow);

	return uart_set_options (&sup->port, co, baud, parity, bits, flow);
}

static struct console snx_serial_console = {
	.name		= "ttyS",
	.write		= snx_serial_console_write,
	.device 	= uart_console_device,
	.setup		= snx_serial_console_setup,
	.flags		= CON_PRINTBUFFER,
	.index		= -1,
	.data		= &snx_uart_driver,
};


/** \fn int snx_serial_console_init(void)
 * \brief snx_serial_console_init - register a serial console to kernel
 * \details 
 * \n 1)this function call by the kernel init code
 * \n
 * \return 0:success, !0:fail
 */
static int __init snx_serial_console_init (void)
{
	register_console (&snx_serial_console);
	return 0;
}

console_initcall (snx_serial_console_init);

#define SNX_CONSOLE	&snx_serial_console
#else
#define SNX_CONSOLE	NULL
#endif

struct uart_ops snx_uart_ops =	{
	.tx_empty	= snx_uart_tx_empty,
	.set_mctrl	= snx_uart_set_mctrl,
	.get_mctrl	= snx_uart_get_mctrl,
	.stop_tx	= snx_uart_stop_tx,
	.start_tx	= snx_uart_start_tx,
	.stop_rx	= snx_uart_stop_rx,
	.enable_ms	= snx_uart_enable_ms,
	.break_ctl	= snx_uart_break_ctl,
	.startup	= snx_uart_startup,
	.shutdown	= snx_uart_shutdown,
	.set_termios	= snx_uart_set_termios,
	.pm 		= snx_uart_pm,
	.type		= snx_uart_type,
	.release_port	= snx_uart_release_port,
	.request_port	= snx_uart_request_port,
	.config_port	= snx_uart_config_port,
	.verify_port	= snx_uart_verify_port,
};

static struct snx_uart_port snx_uart_ports[] = {
	 {	/* UART0 */
	.port	= {
		/* the four parameters below are necessary for console initialization */
		.membase	= (void *) io_p2v (SNX_UART0_BASE),
		.mapbase	= SNX_UART0_BASE,
//		.uartclk	= APB_CLK,
		.ops		= &snx_uart_ops,
	},
#if UART_NR >= 2
  }, {	/* UART1 */
	.port	= {
		/* the four parameters below are necessary for console initialization */
		.membase	= (void *) io_p2v (SNX_UART1_BASE),
		.mapbase	= SNX_UART1_BASE,
//		.uartclk	= APB_CLK,
		.ops		= &snx_uart_ops,
	},
#elif UART_NR >= 3
  }, {	/* UART2 */
	.port	= {
		/* the four parameters below are necessary for console initialization */
		.membase	= (void *) io_p2v (SNX_UART2_BASE),
		.mapbase	= SNX_UART2_BASE,
//		.uartclk	= APB_CLK,
		.ops		= &snx_uart_ops,
	},
#endif
  }
};

static struct uart_driver snx_uart_driver = {
	.owner			= THIS_MODULE,
	.driver_name	= "snx_uart",
	.dev_name		= "ttyS",
	.major			= TTY_MAJOR,
	.minor			= 64,
	.nr 			= ARRAY_SIZE (snx_uart_ports),
	.cons			= SNX_CONSOLE,
};

#ifdef CONFIG_PM

/** \fn int snx_uart_suspend(struct platform_device *pdev)
 * \brief snx_uart_suspend - suspend the uart port
 * \details 
 * \n 1)
 * \param pdev :
 * \param state
 * \n
 * \param pdev :platform device data
 * \return 0:success, !0:fail
 */
static int snx_uart_suspend (struct platform_device *pdev, pm_message_t state)
{
		struct snx_uart_port *sport = platform_get_drvdata(pdev);
    DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
		if (sport)
				uart_suspend_port (&snx_uart_driver, &sport->port);

		return 0;
}

/** \fn int snx_uart_resume(struct platform_device *pdev)
 * \brief snx_uart_resume - resume the uart port
 * \details 
 * \n 1)
 * \n 2)
 * \n 3)
 * \n 4)
 * \n 5)
 * \n
 * \param pdev :platform device data
 * \return 0:success, !0:fail
 */
static int snx_uart_resume (struct platform_device *pdev)
{
		struct snx_uart_port *sport = platform_get_drvdata(pdev);
    DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
		if (sport)
				uart_resume_port (&snx_uart_driver, &sport->port);

		return 0;
}
#else
#define snx_uart_suspend NULL
#define snx_uart_resume NULL
#endif


/** \fn int snx_uart_remove(struct platform_device *pdev)
 * \brief snx_uart_remove - release used resource and datas
 * \details 
 * \n 1)remove registed port
 * \n 2)disable clock
 * \n 3)free arch device resource
 * \n
 * \param pdev :platform device data
 * \return 0:success, !0:fail
 */
static int __devexit snx_uart_remove (struct platform_device *pdev)
{
	struct snx_uart_port *sport = platform_get_drvdata(pdev);
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	if (sport != NULL) {
		snx_ports[pdev->id] = NULL;
		platform_set_drvdata (pdev, NULL);
		uart_remove_one_port (&snx_uart_driver, &sport->port);

		if (sport->clk != NULL) {
			clk_disable (sport->clk);
			clk_put (sport->clk);
		}
#if 0
		if (sport->port.membase != NULL)
			iounmap (sport->port.membase);

		if (sport->res == NULL) {
			release_resource (sport->res);
			kfree (sport->res);
		}
#endif
		kfree (sport);
	}

printk (KERN_INFO "Remvoe SONIX UART driver\n");
	return 0;
}


/** \fn int snx_uart_probe(struct platform_device *pdev)
 * \brief snx_uart_probe - initialize some platform data struct
 * \details 
 * \n 1)get resource from platform device, and allocate platform uart data
 * \n 2)get irq and clk
 * \n 3)initialize platform uart data and register uart port to serial
 * \n generic architecture
 * \n
 * \param pdev :platform device data
 * \return 0:success, !0:fail
 */
static int __devinit snx_uart_probe (struct platform_device *pdev)
{
	struct snx_uart_port *sport;
	struct resource *res;
	int ret;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	res = platform_get_resource (pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		ret = -EINVAL;
		goto out;
	}

	sport = kzalloc (sizeof (struct snx_uart_port), GFP_KERNEL);
	if (sport == NULL) {
		ret = -ENOMEM;
		goto err;
	}
#if 0
	/* already done in the platform_device_register */
	sport->res = request_mem_region(res->start, PAGE_SIZE, pdev->dev.bus_id);
	if (sport->res == NULL) {
		ret = -EBUSY;
		goto err;
	}
#endif
	sport->port.mapbase = res->start;
	sport->port.membase = (void *) io_p2v(sport->port.mapbase);
#if 0
	sport->port.membase = ioremap(sport->port.mapbase, PAGE_SIZE);
	if (sport->port.membase == NULL) {
		ret = -ENOMEM;
		goto err;
	}
#endif
	sport->port.irq 	= platform_get_irq (pdev, 0);
	if (pdev->id == 0)
		sport->clk	= clk_get (&pdev->dev, "uart0_clk");
#if UART_NR >= 2
	if (pdev->id == 1)
		sport->clk	= clk_get (&pdev->dev, "uart1_clk");
#elif UART_NR >= 3
	if (pdev->id == 2)
		sport->clk	= clk_get (&pdev->dev, "uart2_clk");
#endif
	if (sport->clk == NULL) {
		ret = PTR_ERR (sport->clk);
		goto err;
	}

	clk_enable (sport->clk);

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	sport->port.uartclk = 10 * 1000000;
#else
	sport->port.uartclk = clk_get_rate (sport->clk);
#endif
	sport->port.fifosize	= 32; // only 32 bytes
	sport->port.iotype	= UPIO_MEM;
	sport->port.type	= PORT_SNX;
	sport->port.ops 	= &snx_uart_ops;
	sport->port.line	= pdev->id;
	sport->port.dev 	= &pdev->dev;

//	strlcpy(sport->name, pdev->name, 64);

	ret=uart_add_one_port (&snx_uart_driver, &sport->port);
	if (!ret) {
		platform_set_drvdata (pdev, sport);
		snx_ports[pdev->id] = sport;
		goto out;
	}

err:
	snx_uart_remove (pdev);
out:
	return ret;
}

static struct platform_driver snx_serial_driver = {
	.probe		= snx_uart_probe,
	.remove 	= __devexit_p (snx_uart_remove),
	.suspend	= snx_uart_suspend,
	.resume 	= snx_uart_resume,
	.driver 	= {
	.name		= "snx_uart",
	.owner		= THIS_MODULE,
	},
};


/** \fn void snx_uart_init(void)
 * \brief snx_uart_init - calls while module install to kernel
 * \return 0:success, !0:fail
 */
int __init snx_uart_init (void)
{
	int ret;
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	printk (KERN_INFO "SONIX UART driver, (c) 2013 Sonix\n");
	ret = uart_register_driver (&snx_uart_driver);
	if (ret != 0)
		return ret;

	ret = platform_driver_register (&snx_serial_driver);
	if (ret != 0)
		uart_unregister_driver (&snx_uart_driver);

	return ret;
}


/** \fn void snx_uart_exit(void)
 * \brief snx_uart_exit - calls while module remove from kernel
 */
void __exit snx_uart_exit (void)
{
  DBG_UART ("%s:%d: -------->\n", __func__, __LINE__);
	platform_driver_unregister (&snx_serial_driver);
	uart_unregister_driver (&snx_uart_driver);
}

module_init (snx_uart_init);
module_exit (snx_uart_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Saxen Ko");
MODULE_AUTHOR ("Qingbin Li");
MODULE_DESCRIPTION ("SONIX serial port driver");

