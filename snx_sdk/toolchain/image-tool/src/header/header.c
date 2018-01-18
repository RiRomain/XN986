#include "common.h"
#include "ms_nf.h"
#include "generated/snx_sdk_conf.h"

u32 FLASH_INFO_START;	
u32 HW_SETTING_START;   
u32 U_BOOT_START;       
u32 U_ENV_START;        
u32 FLASH_LAYOUT_START; 
u32 FACTORY_START;      
u32 U_LOGO_START;       
u32 RESCUE_START;
u32 AHB2_CLOCK;

u32 IMAGE_TABLE_START;
u32 IMAGE_TABLE_SIZE;


#define ITCM_ZI 0xFFFF6000
#define SB_FWIMG_LOAD_ADDR 	(ITCM_ZI + 0x1c)
#define SB_FWIMG_SIZE 	0x00010000

#define SNX_SYS_BASE			0x98000000

#define WD_BASE 	0x98700000

#define SNX_PWM1_BASE           0x99000000
#define SNX_PWM2_BASE           0x99100000

static int get_clock ()
{
	u32 pll800=0, ahb2_rate=0;

	pll800 = ((* (volatile unsigned int *) SNX_SYS_BASE) & 0x3f8000) >> 15;
	ahb2_rate = ((* (volatile unsigned int *) (SNX_SYS_BASE + 4)) & 0xf8000) >> 15;
		
	AHB2_CLOCK = (pll800 * 12 * 1000000) / ahb2_rate;
	
	return (0);
}


static int wd_reset ()
{
	u32 pll800=0, ahb2_rate=0;

	(* (volatile unsigned int *) (WD_BASE + 0x4)) = 0x1000;
	(* (volatile unsigned int *) (WD_BASE + 0xc)) = 0x3;
		
	
	return (0);
}

#define FIRMWARE_IMAGE_START	0x10114a0

static int set_green_led_on(void)
{
	(* (volatile unsigned int *) (SNX_PWM1_BASE+0x0)) = 0x10;
	
	(* (volatile unsigned int *) (SNX_PWM2_BASE+0x4)) = 0;
	(* (volatile unsigned int *) (SNX_PWM2_BASE+0x8)) = 0x7d334;
	(* (volatile unsigned int *) (SNX_PWM2_BASE+0x0)) = 0x19;

	return (0);
}

int main(void)
{
/* check id */
	u32 platform_id;

	platform_id = (* (volatile unsigned int *) (SNX_SYS_BASE + 0x10)) & 0xfffff;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	if (platform_id && (platform_id != 0x58660)) {
		serial_printf ("ERROR: id is not match, hardware platform id = %x, but image id is 58660\n", platform_id);
		return -1;
	}
#else
	if (platform_id && (platform_id != 0x58600)) {
		serial_printf ("ERROR: id is not match, hardware platform id = %x, but image id is 58600\n", platform_id);
		return -1;
	}
#endif

#ifndef FW_BURN_FLOW 
	FLASH_INFO_START		=	_FLASH_INFO_START		+ _TEXT_BASE;
	HW_SETTING_START        =	_HW_SETTING_START       + _TEXT_BASE;
	U_BOOT_START            =	_U_BOOT_START           + _TEXT_BASE;
	U_ENV_START             =	_U_ENV_START            + _TEXT_BASE;
	FLASH_LAYOUT_START      =	_FLASH_LAYOUT_START     + _TEXT_BASE;
	FACTORY_START           =	_FACTORY_START          + _TEXT_BASE;
	U_LOGO_START            =	_U_LOGO_START           + _TEXT_BASE;
	RESCUE_START            =	_RESCUE_START           + _TEXT_BASE;
	
	serial_puts("----UPDATE FIRMWARE----\n");
	
	serial_printf("FLASH_INFO_START = 0x%x\n", FLASH_INFO_START);
	serial_printf("HW_SETTING_START = 0x%x\n", HW_SETTING_START);
	serial_printf("U_BOOT_START = 0x%x\n", U_BOOT_START);
	serial_printf("U_ENV_START = 0x%x\n", U_ENV_START);
	serial_printf("FLASH_LAYOUT_START = 0x%x\n", FLASH_LAYOUT_START);
	serial_printf("FACTORY_START = 0x%x\n", FACTORY_START);
	serial_printf("U_LOGO_START = 0x%x\n", U_LOGO_START);
	serial_printf("RESCUE_START = 0x%x\n", RESCUE_START);
	
	serial_printf("_TEXT_BASE = 0x%x\n", _TEXT_BASE);
	serial_printf("_bss_start = 0x%x\n", _bss_start);
	serial_printf("_bss_end = 0x%x\n", _bss_end);


	get_clock();
	
	serial_init();


#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)

#else
	ms1_set_mode(GPIO_MODE);
	ms1_gpio_mode(0x0, 0xD);

	if(!ms1_gpio_read(0xD)) {
#endif
		if(ms_serial_flash_init())
			return -1;
		if(ms_serial_flash_update()) {
			serial_puts("serial flash update failed\n");
			return -1;
		}
		serial_puts("serial flash update success\n");

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)

#else
	}
	else {
		if(ms_nand_flash_init())
			return -1;
		if(ms_nand_flash_update()) {
			serial_puts("nand flash update failed\n");
			return -1;
		}
		serial_puts("nand flash update success\n");
	}
#endif

#else
//	IMAGE_TABLE_START = readl(SB_FWIMG_LOAD_ADDR) + SB_FWIMG_SIZE;
	IMAGE_TABLE_START = readl(SB_FWIMG_LOAD_ADDR);
	IMAGE_TABLE_START &= 0x0fffffff;
	IMAGE_TABLE_START += SB_FWIMG_SIZE;


	IMAGE_TABLE_SIZE = readl(IMAGE_TABLE_START);

	serial_puts("----UPDATE FIRMWARE----\n");
	serial_printf("IMAGE_TABLE_START = 0x%x\n", IMAGE_TABLE_START);
	serial_printf("IMAGE_TABLE_SIZE = 0x%x\n", IMAGE_TABLE_SIZE);

	set_green_led_on();

	get_clock();
	serial_init();

	extern void md5 (unsigned char *input, int len, unsigned char output[16]);
	extern int snx_encrypt_decrypt(const int encrypt, const unsigned char input[16], unsigned char output[16]);

	unsigned int image_addr = readl(SB_FWIMG_LOAD_ADDR);
	unsigned int image_size = *(unsigned int*)image_addr;
	unsigned char md5_encrypt[16];
	unsigned char md5_firmware[16];
	unsigned char md5_sum[16];
	unsigned char *pmd5 = (unsigned char*)(image_addr + image_size + 4);

	if (IMAGE_TABLE_START <= 0x1010000 || IMAGE_TABLE_START >= 0x1012000) {	// 0x1010000 ~ 0x1020000: firmware image table addr, firmware can't check md5
	//if (IMAGE_TABLE_START != 0x00114a0) {
		/* md5 check */
		memcpy (md5_encrypt, pmd5, 16);
		snx_encrypt_decrypt (0, md5_encrypt, md5_sum);
		md5 ((unsigned char*)IMAGE_TABLE_START, image_size - SB_FWIMG_SIZE, md5_firmware);

		serial_printf ("image_addr = 0x%x, size=0x%x\n", IMAGE_TABLE_START, image_size - SB_FWIMG_SIZE);
		if(strncmp((char*)md5_sum, (char*)md5_firmware, 16) != 0) {
			serial_printf ("decrypt check failed\n");
			int i=0;
			for (i = 0; i < 16; i++) {
				serial_printf ("index=%d, encrypt=0x%x, firmware=0x%x, sum=0x%x\n", i, md5_encrypt[i], md5_firmware[i],md5_sum[i]);
			}
			return -1;
		}	
	}

	if(ms_serial_flash_init())
		return -1;
		
	if(ms_serial_flash_update()) {
		//serial_puts("serial flash update failed\n");
		return -1;
	}

	//serial_puts("serial flash update success\n");

//	wd_reset();

	// memcpy ((uchar *)(0x00000000),(uchar *)(0x80000000),0xA000);
	// 	__asm__ __volatile__("ldr r0, = 0x00000000\n"
	// 		     "mov pc, r0\n");


#endif
	return 0;
}
