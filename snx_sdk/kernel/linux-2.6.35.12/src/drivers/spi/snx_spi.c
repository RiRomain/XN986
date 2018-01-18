/* 
 *linux/drivers/spi/snx_spi.c
*/

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/delay.h>

#include <linux/spi/spidev.h>
#include <mach/regs-spi.h>
#include "generated/snx_gpio_conf.h"

//#define LOOKBACK_MODE_TEST

//#define DEBUG_ME
#ifdef DEBUG_ME
#define DBG_ME(x...) 	printk(KERN_INFO x)
#else
#define DBG_ME(x...)   do { } while (0)
#endif

//=========================================================================
// SSP Frame Format
//=========================================================================
enum SSP_Frame_Format
{
	SSP_TISSP = 0,	// Texas Instrument Synchronous Serial Port (SSP)
	SSP_MSPI,	// Motorola Serial Peripheral Interface (SPI)
	SSP_NSM,	// National Semiconductor Mictowire
	SSP_PI2S,	// Philips I2S
	SSP_IAL		// Intel AC-Link
};

//=========================================================================
// SSP Operation Mode
//=========================================================================
enum SSP_Mode
{
	SSP_OPM_SLMO = 0, 	// Slave mono mode
	SSP_OPM_SLST,		// Slave stereo mode
	SSP_OPM_MSMO, 		// Master mono mode
	SSP_OPM_MSST		// Master stereo mode
};

//=========================================================================
// SSP Operation Mode
//=========================================================================
enum SSP_Clock_Phase
{
	SSP_SCLKPH_ONECLK = 0,	// serial clock start running after one SCLK cycle
	SSP_SCLKPH_HALFCLK	// serial clock start running after half SCLK cycle
};

struct snx_spi {
	/* bitbang has to be first */
	struct spi_bitbang	 bitbang;
	struct completion	 done;

	void __iomem		*regs;
	int			 irq;
	int			 len;
	int			 count;
	u8			bpw;// bits_per_word
	u8			fifo_depth;
	u8			sdl;

	/* data buffers */
	const unsigned char	*tx;
	unsigned char		*rx;

	struct clk		*clk;
	struct resource		*ioarea;
	struct spi_master	*master;
	struct spi_device	*curdev;
	struct device		*dev;
	u8	is_gpio;
	unsigned long hz;
};

//=========================================================================
// Set frame format
//=========================================================================
static void ssp_set_frame_format (struct snx_spi *hw, enum SSP_Frame_Format frame_format)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	data &= ~SSP_FFMT_MASK;
	data |= (frame_format << SSP_FFMT_BIT); 
	__raw_writel (data, hw->regs + SSP_CTRL0_OFFSET);
}

//=========================================================================
// Set operation mode
//=========================================================================
static void ssp_set_mode (struct snx_spi *hw, enum SSP_Mode mode)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	data &= ~SSP_OPM_MASK;
	data |= (mode << SSP_OPM_BIT); 
	__raw_writel (data, hw->regs + SSP_CTRL0_OFFSET);
}

#ifdef LOOKBACK_MODE_TEST
//=========================================================================
// Set lookback mode
//=========================================================================
static void ssp_set_lookback (struct snx_spi *hw, u8 enable)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	data &= ~SSP_LBM_MASK;
	data |= (enable << SSP_LBM_BIT); 
	__raw_writel (data, hw->regs + SSP_CTRL0_OFFSET);
	DBG_ME("%s:%d  lookback mode=%d done\n", __func__, __LINE__, enable);
}

static void display_fifo_depth (struct snx_spi *hw)
{
	u32 data,fifo,rd,td;
	data = __raw_readl (hw->regs + 0x44);
	fifo = data & 0xff;
	rd = (data & 0xff00) >> 8;
	td = (data & 0xff0000) >> 16;
	printk ("** the fifo depth = %d, rx depth = %d, tx depth = %d\n", fifo, rd, td);
}

#endif

//=========================================================================
// Set clock phase
//=========================================================================
static void ssp_set_clock_phase (struct snx_spi *hw, enum SSP_Clock_Phase phase)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	data &= ~SSP_SCLKPH_MASK;
	data |= (phase << SSP_SCLKPH_BIT); 
	__raw_writel (data, hw->regs + SSP_CTRL0_OFFSET);
}

//=========================================================================
// Set clock polarity
//=========================================================================
static void ssp_set_clock_polar (struct snx_spi *hw, u32 polar)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	data &= ~SSP_SCLKPO_MASK;
	data |= ((polar << SSP_SCLKPO_BIT) & SSP_SCLKPO_MASK); 
	__raw_writel (data, hw->regs + SSP_CTRL0_OFFSET);
}

//=========================================================================
// Set data length
//=========================================================================
void ssp_set_data_len (struct snx_spi *hw, u32 len)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL1_OFFSET);
	data &= ~SSP_SDL_MASK;
	data |= ((len << SSP_SDL_BIT) & SSP_SDL_MASK); 
	__raw_writel (data, hw->regs + SSP_CTRL1_OFFSET);
}

//=========================================================================
// Set clock divider
//=========================================================================
static void ssp_set_clock_div (struct snx_spi *hw, unsigned long hz)
{
	unsigned long clk;
	u32 data, div;
	
	clk = clk_get_rate(hw->clk);
	div = DIV_ROUND_UP(clk, hz * 2) - 1;

	//if (div == 0)
	//	div = 1;
	if (div > 0x8000)
		div = 0x8000;
	//printk ("##### div = %d, clk = 0x%x, hz=0x%x\n", div, clk, hz);
	data = __raw_readl (hw->regs + SSP_CTRL1_OFFSET);
	data &= ~SSP_SCLKDIV_MASK;
	data |= ((div << SSP_SCLKDIV_BIT) & SSP_SCLKDIV_MASK); 
	
	DBG_ME("%s:%d div = 0x%x done\n", __func__, __LINE__, div);
	__raw_writel (data, hw->regs + SSP_CTRL1_OFFSET);
}

//=========================================================================
// Set tx and rx fifo threshold
//=========================================================================
void ssp_set_fifo_threshold (struct snx_spi *hw, u32 tx_threshold, u32 rx_threshold)
{
	u32 data = __raw_readl (hw->regs + SSP_INTR_EN_OFFSET);
	data &= ~(SSP_RXF_TH_MASK | SSP_TXF_TH_MASK);
	data |= ((rx_threshold << SSP_RXF_TH_BIT) & SSP_RXF_TH_MASK)
		| ((tx_threshold << SSP_TXF_TH_BIT) & SSP_TXF_TH_MASK); 
	__raw_writel (data, hw->regs + SSP_INTR_EN_OFFSET);
}

//=========================================================================
// Claer rx fifo
//=========================================================================
void ssp_clear_rx_fifo (struct snx_spi *hw)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL2_OFFSET);
	data |= SSP_RXF_CLR_MASK;
	__raw_writel (data, hw->regs + SSP_CTRL2_OFFSET);
}

//=========================================================================
// Claer tx fifo
//=========================================================================
void ssp_clear_tx_fifo (struct snx_spi *hw)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL2_OFFSET);
	data |= SSP_TXF_CLR_MASK;
	__raw_writel (data, hw->regs + SSP_CTRL2_OFFSET);
}

//=========================================================================
// Enable SSP
//=========================================================================
void ssp_enable (struct snx_spi *hw, u8 enable)
{
	u32 data = __raw_readl (hw->regs + SSP_CTRL2_OFFSET);
	data &= ~SSP_EN_MASK;
	if(enable)
		data |= SSP_EN_MASK; 
	__raw_writel (data, hw->regs + SSP_CTRL2_OFFSET);
}

//=========================================================================
// Enable SSP rx fifo threshold interrupt
//=========================================================================
void ssp_enable_rxf_threshold_intr (struct snx_spi *hw, u8 enable)
{
	u32 data = __raw_readl (hw->regs + SSP_INTR_EN_OFFSET);
	data &= ~SSP_RXF_TH_INTR_EN_MASK;
	if(enable)
		data |= SSP_RXF_TH_INTR_EN_MASK; 
	__raw_writel (data, hw->regs + SSP_INTR_EN_OFFSET);
}

//=========================================================================
// Enable SSP tx fifo threshold interrupt
//=========================================================================
void ssp_enable_txf_threshold_intr (struct snx_spi *hw, u8 enable)
{
	u32 data = __raw_readl (hw->regs + SSP_INTR_EN_OFFSET);
	data &= ~SSP_TXF_TH_INTR_EN_MASK;
	if(enable)
		data |= SSP_TXF_TH_INTR_EN_MASK; 
	__raw_writel (data, hw->regs + SSP_INTR_EN_OFFSET);
}

//=========================================================================
// get received fifo count
//=========================================================================
u32 ssp_rx_rdy_cnt (struct snx_spi *hw)
{
	u32 status;
	status = __raw_readl (hw->regs + SSP_STATUS_OFFSET);

	return ((status & SSP_RXF_VE_MASK) >> SSP_RXF_VE_BIT);
	
}

//=========================================================================
// write data to tx fifo
//=========================================================================
void ssp_write_data (struct snx_spi *hw, u32 data)
{
	__raw_writel (data, hw->regs + SSP_DATA_OFFSET);
}

//=========================================================================
// read data from rx fifo
//=========================================================================
u32 ssp_read_data (struct snx_spi *hw)
{
	u32 data = 0;
	while(ssp_rx_rdy_cnt(hw)==0)
		ssp_write_data (hw, 0);
	data = __raw_readl (hw->regs + SSP_DATA_OFFSET);
	return data;
}

void ssp_spi_gpio_mode(struct snx_spi *hw, u32 mode)
{
	u32 v;
	v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	v &= ~SSP_GPIO_MASK;
	v |= mode << SSP_GPIO_BIT;
	__raw_writel (v, hw->regs + SSP_CTRL0_OFFSET);
}

static void ssp_gpio_set_input_mode(struct snx_spi *hw, u8 pin)
{
	u32 v = 0;
	
	switch (pin) {
		case SPI_GPIO_CLK_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			v &= ~SSP_CLK_GPIO_OE_MASK;
			__raw_writel (v, hw->regs + SSP_CTRL0_OFFSET);
			break;
		case SPI_GPIO_FS_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			v &= ~SSP_FS_GPIO_OE_MASK;
			__raw_writel (v, hw->regs + SSP_CTRL0_OFFSET);
			break;

		case SPI_GPIO_TX_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			v &= ~SSP_TX_GPIO_OE_MASK;
			__raw_writel (v, hw->regs + SSP_CTRL0_OFFSET);
			
			break;
		case SPI_GPIO_RX_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			v &= ~SSP_RX_GPIO_OE_MASK;
			__raw_writel (v, hw->regs + SSP_CTRL0_OFFSET);
			
			break;			
		default:
			printk ("%s:%d unknown pin\n",__func__, __LINE__);
	}
	DBG_ME("%s:%d done, pin = %d\n", __func__, __LINE__, pin);
}


void ssp_gpio_output_test(struct snx_spi *hw, u8 pin, u8 value)
{
	u32 v;
	v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
	switch (pin) {
		case SPI_GPIO_CLK_PIN:
			v |= SSP_CLK_GPIO_OE_MASK;
			v &= ~SSP_CLK_GPIO_O_MASK;
			v |= (value << SSP_CLK_GPIO_O_BIT);
			break;
		case SPI_GPIO_FS_PIN:
			v |= SSP_FS_GPIO_OE_MASK;
			v &= ~SSP_FS_GPIO_O_MASK;
			v |= (value << SSP_FS_GPIO_O_BIT);
			break;
		case SPI_GPIO_TX_PIN:
			v |= SSP_TX_GPIO_OE_MASK;
			v &= ~SSP_TX_GPIO_O_MASK;
			v |= (value << SSP_TX_GPIO_O_BIT);
			break;
		case SPI_GPIO_RX_PIN:
			v |= SSP_RX_GPIO_OE_MASK;
			v &= ~SSP_RX_GPIO_O_MASK;
			v |= (value << SSP_RX_GPIO_O_BIT);
			break;			
		default:
			printk ("%s:%d unknown pin\n",__func__, __LINE__);
	}
	__raw_writel (v, hw->regs + SSP_CTRL0_OFFSET);
}

void ssp_gpio_input_test(struct snx_spi *hw, u8 pin, u8 *value, u8 *mode)
{
	u32 v = 0;
	
	switch (pin) {
		case SPI_GPIO_CLK_PIN:	
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			*value = (v & SSP_CLK_GPIO_I_MASK) >> SSP_CLK_GPIO_I_BIT;
			*mode = (v & SSP_CLK_GPIO_OE_MASK) >> SSP_CLK_GPIO_OE_BIT;
			break;
		case SPI_GPIO_FS_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			*value = (v & SSP_FS_GPIO_I_MASK) >> SSP_FS_GPIO_I_BIT;
			*mode = (v & SSP_FS_GPIO_OE_MASK) >> SSP_FS_GPIO_OE_BIT;
			break;

		case SPI_GPIO_TX_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			*value = (v & SSP_TX_GPIO_I_MASK) >> SSP_TX_GPIO_I_BIT;
			*mode = (v & SSP_TX_GPIO_OE_MASK) >> SSP_TX_GPIO_OE_BIT;
			break;
		case SPI_GPIO_RX_PIN:
			v = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
			*value = (v & SSP_RX_GPIO_I_MASK) >> SSP_RX_GPIO_I_BIT;
			*mode = (v & SSP_RX_GPIO_OE_MASK) >> SSP_RX_GPIO_OE_BIT;
			break;			
		default:
			printk ("%s:%d unknown pin\n",__func__, __LINE__);
	}
	DBG_ME("%s:%d done, pin = %d, value=%d\n", __func__, __LINE__, pin, *value);
}

//=========================================================================
// Get status of SSP
//=========================================================================
u32 ssp_get_status (struct snx_spi *hw)
{
	u32 data;
	data = __raw_readl (hw->regs + SSP_STATUS_OFFSET);
	
	return data;
}

u32 ssp_tx_rdy_cnt (struct snx_spi *hw)
{
	return ((__raw_readl (hw->regs + SSP_STATUS_OFFSET) & SSP_TXF_VE_MASK)>>SSP_TXF_VE_BIT);
}

void ssp_change_len (struct snx_spi *hw, u32 data_len)
{
	u32 status;

	do {
		status = ssp_tx_rdy_cnt (hw);
	} while(status != 0);
	
	ssp_set_data_len (hw, data_len - 1);

	ssp_clear_rx_fifo (hw);
	ssp_clear_tx_fifo (hw);
}

static void snx_spi_initialsetup(struct snx_spi *hw, unsigned long hz)
{
	/* for the moment, permanently enable the clock */
	DBG_ME("%s:%d done\n", __func__, __LINE__);

	ssp_set_frame_format (hw, SSP_MSPI);

	ssp_set_mode (hw, SSP_OPM_MSST);
	ssp_set_clock_phase (hw, SSP_SCLKPH_ONECLK);
	ssp_set_clock_polar (hw, 0x0);
	
	ssp_set_data_len (hw, hw->bpw -1);
	
	ssp_set_clock_div (hw, hz);

	//if (hw->bpw < 16)
	//	ssp_set_fifo_threshold(hw, hw->bpw, 1);
	//else
	ssp_set_fifo_threshold(hw, 0, 1);
 
	// clear rx/tx fifo
	ssp_clear_rx_fifo (hw);
	ssp_enable (hw, 1);
	ssp_clear_tx_fifo (hw);	
	ssp_enable (hw, 0);
	hw->sdl = 8;
	
}

static inline struct snx_spi *to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static void snx_spi_chipsel(struct spi_device *spi, int value)
{
	DBG_ME("%s    %s:%d done\n", spi->modalias, __func__, __LINE__);
}

static int snx_spi_update_state(struct spi_device *spi,
				    struct spi_transfer *t)
{
	struct snx_spi *hw = to_hw(spi);
	unsigned int bpw;
	unsigned int hz;
	DBG_ME("%s    %s:%d doing\n", spi->modalias, __func__, __LINE__);
	bpw = t ? t->bits_per_word : hw->bpw;
	hz  = t ? t->speed_hz : hw->hz;

	if (!bpw)
		bpw = hw->bpw;

	if (!hz)
		hz = hw->hz;

	/* max fifo depth = 16 */
	if ((bpw % 8) || ((bpw > 128) && (bpw != 256) && (bpw != 384) && (bpw != 512))) {
		dev_err(&spi->dev, "wrong bpw param\n");
		return -ENOMEM;
	} else if (t) {
		if (t->len % (bpw/8)) {
			dev_err(&spi->dev, "wrong bpw param, len=0x%x, bpw=0x%x\n", t->len, bpw);
			return -ENOMEM;
		}
	}

	DBG_ME ("##state chage: bpw=0x%x, sdl=0x%x, fifo_depth=0x%x, bpw=0x%x\n", hw->bpw, hw->sdl, hw->fifo_depth, bpw);
	if (hw->bpw != bpw) {
		hw->bpw = bpw;
		if (bpw > 32) {
			if (bpw % 32) {
				if (bpw % 16)
					hw->sdl = 8;
				else
					hw->sdl = 16;
			} else
				hw->sdl = 32;
		} else
			hw->sdl = bpw;
		hw->fifo_depth = bpw / hw->sdl;

		ssp_change_len (hw, hw->sdl);
		//hw fifo limit 16 bytes
		//if (bpw < 16)
		//	ssp_set_fifo_threshold(hw, bpw, 1);
		//else
		ssp_set_fifo_threshold(hw, 0, hw->fifo_depth);
		
		printk("%s    %s:%d bpw change done\n", spi->modalias, __func__, __LINE__);
	}
	
	if (hw->hz != hz) {
		/* the apb clk must be greater than 6 times the spi clk */
		if (clk_get_rate(hw->clk) < 6*hz) {
			hz = clk_get_rate(hw->clk) / 6;
			dev_warn (&spi->dev, "Setting spi clk=%dMhz, it is the max spi clk\n", hz/1000000);
		}
		ssp_set_clock_div (hw, hz);
		
		hw->hz = hz;
		DBG_ME ("%s:%d  device hz change\n",
			__func__, __LINE__); 
	}
	
	return 0;
}

static int snx_spi_setupxfer(struct spi_device *spi,
				 struct spi_transfer *t)
{
	int ret;
	struct snx_spi *hw = to_hw(spi);
	
	DBG_ME("%s    %s:%d doing\n", spi->modalias, __func__, __LINE__);

	if (hw->is_gpio)
		return 0;
		
	ret = snx_spi_update_state(spi, t);
	return ret;
}

static int snx_spi_setup(struct spi_device *spi)
{
	struct snx_spi *hw = to_hw(spi);
	int ret;

	DBG_ME("%s    %s:%d doing, bpw=0x%x\n", spi->modalias, __func__, __LINE__, spi->bits_per_word);

	
	/*if (spi->bits_per_word == SPI_GPIO_BPW) {	
	if (hw
		hw->is_gpio = 1;
		DBG_ME("## gpio mode %s    %s:%d done\n", spi->modalias, __func__, __LINE__);
		ssp_spi_gpio_mode (hw, SSP_GPIO_MODE);*/
	if (hw->is_gpio == 1) 
		return 0;
	else { 
		/*if (hw->is_gpio == 1) {
			hw->is_gpio = 0;
			DBG_ME("##  spi mode %s    %s:%d done\n", spi->modalias, __func__, __LINE__);
			hw->hz = clk_get_rate(hw->clk);
			hw->bpw = 8;
			hw->hz = min(hw->hz, spi->max_speed_hz);
			snx_spi_initialsetup (hw, hw->hz);
		}*/
		/* initialise the state from the device */
		ret = snx_spi_update_state(spi, NULL);
		if (ret)
			return ret;
	}

	

	spin_lock(&hw->bitbang.lock);
	if (!hw->bitbang.busy) {
		hw->bitbang.chipselect(spi, BITBANG_CS_INACTIVE);
		/* need to ndelay for 0.5 clocktick ? */
	}
	spin_unlock(&hw->bitbang.lock);
	
	return 0;
}

static void snx_spi_cleanup(struct spi_device *spi)
{
	DBG_ME("%s    %s:%d done\n", spi->modalias, __func__, __LINE__);
}

static inline unsigned int hw_tx4byte(struct snx_spi *hw, int count)
{
	u32 data=0;
	
	if (hw->tx)
		data = ((hw->tx[count] << 24) & 0xff000000) | 
		((hw->tx[count+1] << 16) & 0xff0000) |
		((hw->tx[count+2] << 8) & 0xff00) |
		(hw->tx[count+3] & 0xff);
	else 
		data = 0;
	DBG_ME("%s:%d  send data = 0x%x done\n", __func__, __LINE__, data);
	return data;
}

static inline unsigned int hw_tx3byte(struct snx_spi *hw, int count)
{
	u32 data=0;
	if (hw->tx)
		data = ((hw->tx[count] << 16) & 0xff0000) |
		((hw->tx[count+1] << 8) & 0xff00) |
		(hw->tx[count+2] & 0xff);
	else 
		data = 0;
	DBG_ME("%s:%d  send data = 0x%x done\n", __func__, __LINE__, data);
	return data;
}

static inline unsigned int hw_tx2byte(struct snx_spi *hw, int count)
{
	u32 data=0;
	if (hw->tx)
		data = ((hw->tx[count] << 8) & 0xff00) | 
		(hw->tx[count+1] & 0xff);
	else 
		data = 0;
	DBG_ME("%s:%d  send data = 0x%x done\n", __func__, __LINE__, data);
	return data;
}

static inline unsigned int hw_txbyte(struct snx_spi *hw, int count)
{
	if (hw->tx)
		DBG_ME("%s:%d  send data = 0x%x done\n", __func__, __LINE__, hw->tx[count]);
	return hw->tx ? hw->tx[count] : 0;
}

static int snx_spi_txrx(struct spi_device *spi, struct spi_transfer *t)
{
	struct snx_spi *hw = to_hw(spi);
	u32 i;
	
	hw->tx = t->tx_buf;
	hw->rx = t->rx_buf;
	hw->len = t->len;
	hw->count = 0;

//just for Lookback mode test
#ifdef LOOKBACK_MODE_TEST
	if (hw->tx && hw->rx)
		ssp_set_lookback (hw, 1);
	else
		ssp_set_lookback (hw, 0);		

	//display fifo depth
	display_fifo_depth (hw);
#endif

	DBG_ME("%s    %s:%d  cs->is_gpio = %d, t-len=%d doing\n", spi->modalias, __func__, __LINE__, hw->is_gpio, t->len);
	
	if (hw->is_gpio) {
		if (t->len % 3 || t->len == 0) { // pin mode value
			dev_err(&spi->dev, "wrong tx len\n");
			return -ENOMEM;
		}
		
		if (hw->tx) {
			for (i = 0; i < t->len; i = i + 3) {
				if ((u8)(hw->tx[i+1] == 1)) {
					if ((u8)(hw->tx[i+2]))
						ssp_gpio_output_test (hw, (u8)(hw->tx[i]), 1);	
					else
						ssp_gpio_output_test (hw, (u8)(hw->tx[i]), 0);	
				} else {
					ssp_gpio_set_input_mode (hw, (u8)(hw->tx[i]));
				}
			}
		} else if (hw->rx) {
			for (i = 0; i < t->len; i = i + 3) {
				ssp_gpio_input_test (hw, (u8)(hw->rx[i]), (u8*)&(hw->rx[i+2]),(u8*)&(hw->rx[i+1]));
			}
		}
		return t->len;
	}

	ssp_clear_rx_fifo (hw);
	
	init_completion(&hw->done);
	ssp_enable (hw, 0);
	for (i=0; i<hw->fifo_depth; i++) {
		if (hw->sdl == 8)
			ssp_write_data (hw, hw_txbyte(hw, i));
		else if (hw->sdl == 16)
			ssp_write_data (hw, hw_tx2byte(hw, 2*i));
		else if (hw->sdl == 24)
			ssp_write_data (hw, hw_tx3byte(hw, 3*i));
		else	
			ssp_write_data (hw, hw_tx4byte(hw, 4*i));
	}
	ssp_enable_rxf_threshold_intr (hw, 1);
	ssp_enable (hw, 1);
	wait_for_completion(&hw->done);
	ssp_enable_rxf_threshold_intr (hw, 0);
	ssp_clear_tx_fifo (hw);	
		
	ssp_enable (hw, 0);	
	DBG_ME("%s    %s:%d done\n", spi->modalias, __func__, __LINE__);
	return hw->count;
}

static irqreturn_t snx_spi_irq(int irq, void *dev)
{
	struct snx_spi *hw = dev;
	unsigned int count = hw->count;
	u32 data;
	u32 i;

	if (hw->len == 0) {
		complete(&hw->done);
		goto irq_done;
	}
	
	DBG_ME("%s:%d spi irq happen bpw=%d,  len=%d hw->count=%d done\n", __func__, __LINE__, hw->bpw, hw->len, hw->count);
	if (hw->sdl == 8)
		hw->count += hw->fifo_depth;
	else if (hw->sdl == 16)
		hw->count += 2*hw->fifo_depth;
	else if (hw->sdl == 24)
		hw->count += 3*hw->fifo_depth;
	else
		hw->count += 4*hw->fifo_depth;

	if (hw->rx) {
		//hw->rx[count] = ssp_read_data (hw);
		for (i=0; i<hw->fifo_depth; i++) {
			if (hw->sdl == 8) {
				//hw->rx[count++] = ssp_read_data (hw);
				data = ssp_read_data (hw);
				hw->rx[count++] = data & 0xff;
			} else if (hw->sdl == 16) {
				data = ssp_read_data (hw);
				hw->rx[count++] = (data >> 8) & 0xFF;
				hw->rx[count++] = data & 0xFF;
			} else if (hw->sdl == 24) {
				data = ssp_read_data (hw);
				hw->rx[count++] = (data >> 16) & 0xFF;
				hw->rx[count++] = (data >> 8) & 0xFF;
				hw->rx[count++] = data & 0xFF;
			} else {
				data = ssp_read_data (hw);
				hw->rx[count++] = (data >> 24) & 0xFF;
				hw->rx[count++] = (data >> 16) & 0xFF;
				hw->rx[count++] = (data >> 8) & 0xFF;
				hw->rx[count++] = data & 0xFF;
			}
		}
	} else {
		if (hw->sdl == 8)
			count += hw->fifo_depth;
		else if (hw->sdl == 16)
			count += 2*hw->fifo_depth;
		else if (hw->sdl == 24)
			count += 3*hw->fifo_depth;
		else
			count += 4*hw->fifo_depth;
	}

	ssp_clear_rx_fifo (hw);
	//count++;

	if (count < hw->len) {
		ssp_enable (hw, 0);
		for (i=0; i<hw->fifo_depth; i++) {
			if (hw->sdl == 8) {
				ssp_write_data (hw, hw_txbyte(hw, count));
				count++;
			}
			else if (hw->sdl == 16) {
				ssp_write_data (hw, hw_tx2byte(hw, count));
				count += 2;
			}
			else if (hw->sdl == 24) {
				ssp_write_data (hw, hw_tx3byte(hw, count));
				count += 3;
			}
			else {	
				ssp_write_data (hw, hw_tx4byte(hw, count));
				count += 4;
			}
		}
		ssp_enable (hw, 1);
	} else
		complete(&hw->done);
	

 irq_done:
	return IRQ_HANDLED;
}


static int __init snx_spi_probe(struct platform_device *pdev)
{
	struct snx_spi *hw;
	struct spi_master *master;
	struct resource *res;
	int err = 0, value = 0;

	master = spi_alloc_master(&pdev->dev, sizeof(struct snx_spi));
	if (master == NULL) {
		dev_err(&pdev->dev, "No memory for spi_master\n");
		err = -ENOMEM;
		goto err_nomem;
	}

	hw = spi_master_get_devdata(master);
	memset(hw, 0, sizeof(struct snx_spi));

	hw->master = spi_master_get(master);
	hw->dev = &pdev->dev;

	platform_set_drvdata(pdev, hw);
	init_completion(&hw->done);

	/* setup the master state. */

	/* the spi->mode bits understood by this driver: */
	master->mode_bits = SPI_MODE_0 | SPI_CS_HIGH;

	master->num_chipselect = 1;
	if (pdev->id == 0)
		master->bus_num = 0;
	else
		master->bus_num = 1;

	/* setup the state for the bitbang driver */

	hw->bitbang.master         = hw->master;
	hw->bitbang.setup_transfer = snx_spi_setupxfer;
	hw->bitbang.chipselect     = snx_spi_chipsel;
	hw->bitbang.txrx_bufs      = snx_spi_txrx;

	hw->master->setup  = snx_spi_setup;
	hw->master->cleanup = snx_spi_cleanup;

	DBG_ME("bitbang at %p\n", &hw->bitbang);

	/* find and map our resources */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		err = -ENOENT;
		goto err_no_iores;
	}

	hw->ioarea = request_mem_region(res->start, resource_size(res),
					pdev->name);

	if (hw->ioarea == NULL) {
		dev_err(&pdev->dev, "Cannot reserve region\n");
		err = -ENXIO;
		goto err_no_iores;
	}

	hw->regs = ioremap(res->start, resource_size(res));
	if (hw->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		err = -ENXIO;
		goto err_no_iomap;
	}

	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq < 0) {
		dev_err(&pdev->dev, "No IRQ specified\n");
		err = -ENOENT;
		goto err_no_irq;
	}

	err = request_irq(hw->irq, snx_spi_irq, 0, pdev->name, hw);
	if (err) {
		dev_err(&pdev->dev, "Cannot claim IRQ\n");
		goto err_no_irq;
	}

	if (pdev->id == 0)
		hw->clk = clk_get(&pdev->dev, "ssp1_clk");
	if (pdev->id == 1)
		hw->clk = clk_get(&pdev->dev, "ssp2_clk");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_no_clk;
	}

	clk_enable(hw->clk);

	if ((pdev->id == 0 && CONFIG_SPI_GPIO_00_ENABLE ==0 && CONFIG_SPI_GPIO_01_ENABLE == 0 
		&& CONFIG_SPI_GPIO_02_ENABLE == 0 && CONFIG_SPI_GPIO_03_ENABLE == 0) 
#ifdef CONFIG_MACH_SN98610
		|| ( pdev->id == 1 && CONFIG_SPI_GPIO_04_ENABLE ==0 && CONFIG_SPI_GPIO_05_ENABLE == 0 
		&& CONFIG_SPI_GPIO_06_ENABLE == 0 && CONFIG_SPI_GPIO_07_ENABLE == 0) 
#endif
	) {

			hw->is_gpio = 0;
			DBG_ME ("##  spi mode   %s:%d done\n", __func__, __LINE__);
			/* the apb clk must be greater than 6 times the spi clk */
			hw->hz = clk_get_rate(hw->clk) / 6;
			hw->bpw = 8;
			hw->fifo_depth = 1;
			snx_spi_initialsetup (hw, hw->hz);
	} else {
		hw->is_gpio = 1;
		DBG_ME ("##  gpio mode   %s:%d done\n", __func__, __LINE__);
		ssp_spi_gpio_mode (hw, SSP_GPIO_MODE);
		if ((pdev->id == 0 && CONFIG_SPI_GPIO_00_ENABLE == 1)
#ifdef CONFIG_MACH_SN98610
				|| (pdev->id == 1 && CONFIG_SPI_GPIO_04_ENABLE == 1)
#endif
				) {
			if ((pdev->id == 0 && CONFIG_SPI_GPIO_00_MODE == 1)
#ifdef CONFIG_MACH_SN98610
				|| (pdev->id == 1 && CONFIG_SPI_GPIO_04_MODE == 1)
#endif
				) {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value |= SSP_CLK_GPIO_OE_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			} else {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value &= ~SSP_CLK_GPIO_OE_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			}
		} else if ((pdev->id == 0 && CONFIG_SPI_GPIO_01_ENABLE == 1)
#ifdef CONFIG_MACH_SN98610
			|| (pdev->id == 1 && CONFIG_SPI_GPIO_05_ENABLE == 1)
#endif
			) {
			if ((pdev->id == 0 && CONFIG_SPI_GPIO_01_MODE == 1)
#ifdef CONFIG_MACH_SN98610
				|| (pdev->id == 1 && CONFIG_SPI_GPIO_05_MODE == 1)
#endif
				) {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value |= SSP_FS_GPIO_OE_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			} else {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value &= ~SSP_FS_GPIO_OE_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			}
		} else if ((pdev->id == 0 && CONFIG_SPI_GPIO_02_ENABLE == 1)
#ifdef CONFIG_MACH_SN98610
			|| (pdev->id == 1 && CONFIG_SPI_GPIO_06_ENABLE == 1)
#endif
			) {
			if ((pdev->id == 0 && CONFIG_SPI_GPIO_02_MODE == 1)
#ifdef CONFIG_MACH_SN98610
				|| (pdev->id == 1 && CONFIG_SPI_GPIO_06_MODE == 1)
#endif
				) {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value |= SSP_TX_GPIO_OE_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			} else {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value &= ~SSP_TX_GPIO_OE_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			}
		} else if ((pdev->id == 0 && CONFIG_SPI_GPIO_03_ENABLE == 1)
#ifdef CONFIG_MACH_SN98610
			|| (pdev->id == 1 && CONFIG_SPI_GPIO_07_ENABLE == 1)
#endif
			) {
			if ((pdev->id == 0 && CONFIG_SPI_GPIO_03_MODE == 1)
#ifdef CONFIG_MACH_SN98610
				|| (pdev->id == 1 && CONFIG_SPI_GPIO_07_MODE == 1)
#endif
				) {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value |= SSP_RX_GPIO_O_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			} else {
				value = __raw_readl (hw->regs + SSP_CTRL0_OFFSET);
				value &= ~SSP_RX_GPIO_O_MASK;
				__raw_writel (value, hw->regs + SSP_CTRL0_OFFSET);
			}
		} 	
	}

	/* register our spi controller */

	err = spi_bitbang_start(&hw->bitbang);
	if (err) {
		dev_err(&pdev->dev, "Failed to register SPI master\n");
		goto err_register;
	}

	return 0;

 err_register:
	clk_disable(hw->clk);
	clk_put(hw->clk);
 err_no_clk:
	free_irq(hw->irq, hw);

 err_no_irq:
	iounmap(hw->regs);

 err_no_iomap:
	release_resource(hw->ioarea);
	kfree(hw->ioarea);

 err_no_iores:
	spi_master_put(hw->master);

 err_nomem:
	return err;
}

static int __exit snx_spi_remove(struct platform_device *dev)
{
	struct snx_spi *hw = platform_get_drvdata(dev);

	platform_set_drvdata(dev, NULL);

	spi_unregister_master(hw->master);

	clk_disable(hw->clk);
	clk_put(hw->clk);

	free_irq(hw->irq, hw);
	iounmap(hw->regs);

	release_resource(hw->ioarea);
	kfree(hw->ioarea);

	spi_master_put(hw->master);
	return 0;
}

#ifdef CONFIG_PM

static int snx_spi_suspend(struct device *dev)
{
	struct snx_spi *hw = platform_get_drvdata(to_platform_device(dev));
	clk_disable (hw->clk);
	return 0;
}

static int snx_spi_resume(struct device *dev)
{
	struct snx_spi *hw = platform_get_drvdata(to_platform_device(dev));

	snx_spi_initialsetup (hw);
	return 0;
}

static const struct dev_pm_ops snx_spi_pmops = {
	.suspend	= s3c24xx_spi_suspend,
	.resume		= s3c24xx_spi_resume,
};

#define SNX_SPI_PMOPS &snx_spi_pmops
#else
#define SNX_SPI_PMOPS NULL
#endif /* CONFIG_PM */

//MODULE_ALIAS("platform:snx-spi");
static struct platform_driver snx_spi_driver = {
	.remove		= __exit_p(snx_spi_remove),
	.driver		= {
		.name	= "snx-spi",
		.owner	= THIS_MODULE,
		.pm	= SNX_SPI_PMOPS,
	},
};

static int __init snx_spi_init(void)
{	
	printk ("snx_spi_init register\n");
        	return platform_driver_probe(&snx_spi_driver, snx_spi_probe);
}
module_init(snx_spi_init);

//static void __exit snx_spi_exit(void)
static void snx_spi_exit(void)
{
       	platform_driver_unregister(&snx_spi_driver);
}
module_exit(snx_spi_exit);

MODULE_AUTHOR("chao zhang");
MODULE_DESCRIPTION("SONIX SPI master Driver");
MODULE_LICENSE("Dual BSD/GPL");
