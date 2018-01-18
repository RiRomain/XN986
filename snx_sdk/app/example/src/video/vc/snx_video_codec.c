/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode
 * File: video_codec.c
 *
 * NOTE:
 *       
 */
 
#include "snx_video_codec.h"

#include "generated/snx_sdk_conf.h"


char ascii_2_font[] =
{
	// ascii , shift
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x00 Can't display
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x08 Can't display
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x10 Can't display
	0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,	//0x18 Can't display
	0x24, 0x3F, 0x3C, 0x3D, 0x3A, 0x3E, 0x40, 0x3B,	//0x20
	0x2D, 0x2E, 0x41, 0x42, 0x26, 0x2A, 0x43, 0x28,	//0x28
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	//0x30
	0x08, 0x09, 0x27, 0x38, 0x31, 0x25, 0x32, 0x37,	//0x38

	0x36, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,	//0x40
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,	//0x48
	0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,	//0x50
	0x21, 0x22, 0x23, 0x33, 0x35, 0x34, 0x39, 0x2B,	//0x58
	0x3B, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,	//0x60
	0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,	//0x68
	0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,	//0x70
	0x21, 0x22, 0x23, 0x2F, 0x2C, 0x30, 0x29, 0x24,	//0x78

};


unsigned int  osd_font[] =
{
	//////////////////// 0 ////////////////////
	0X00000000, 0X007C0000, 0X00E600C6, 0X00DE00F6, 
	0X00C600CE, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// 1 ////////////////////
	0X00000000, 0X00300000, 0X003C0038, 0X00300030, 
	0X00300030, 0X00FC0030, 0X00000000, 0X00000000, 
	//////////////////// 2 ////////////////////
	0X00000000, 0X007C0000, 0X00C000C6, 0X00300060, 
	0X000C0018, 0X00FE00C6, 0X00000000, 0X00000000, 
	//////////////////// 3 ////////////////////
	0X00000000, 0X007C0000, 0X00C000C6, 0X007800C0, 
	0X00C000C0, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// 4 ////////////////////
	0X00000000, 0X00600000, 0X00780070, 0X0066006C, 
	0X006000FE, 0X00F00060, 0X00000000, 0X00000000, 
	//////////////////// 5 ////////////////////
	0X00000000, 0X00FE0000, 0X00060006, 0X007E0006, 
	0X00C000C0, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// 6 ////////////////////
	0X00000000, 0X00380000, 0X0006000C, 0X007E0006, 
	0X00C600C6, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// 7 ////////////////////
	0X00000000, 0X00FE0000, 0X00C000C6, 0X00300060, 
	0X00180018, 0X00180018, 0X00000000, 0X00000000, 
	//////////////////// 8 ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X007C00C6, 
	0X00C600C6, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// 9 ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X00FC00C6, 
	0X00C000C0, 0X003C0060, 0X00000000, 0X00000000, 
	//////////////////// A ////////////////////
	0X00000000, 0X00100000, 0X006C0038, 0X00C600C6, 
	0X00C600FE, 0X00C600C6, 0X00000000, 0X00000000, 
	//////////////////// B ////////////////////
	0X00000000, 0X007E0000, 0X00CC00CC, 0X007C00CC, 
	0X00CC00CC, 0X007E00CC, 0X00000000, 0X00000000, 
	//////////////////// C ////////////////////
	0X00000000, 0X00780000, 0X008600CC, 0X00060006, 
	0X00860006, 0X007800CC, 0X00000000, 0X00000000, 
	//////////////////// D ////////////////////
	0X00000000, 0X003E0000, 0X00CC006C, 0X00CC00CC, 
	0X00CC00CC, 0X003E006C, 0X00000000, 0X00000000, 
	//////////////////// E ////////////////////
	0X00000000, 0X00FE0000, 0X008C00CC, 0X003C002C, 
	0X008C002C, 0X00FE00CC, 0X00000000, 0X00000000, 
	//////////////////// F ////////////////////
	0X00000000, 0X00FE0000, 0X008C00CC, 0X003C002C, 
	0X000C002C, 0X001E000C, 0X00000000, 0X00000000, 
	//////////////////// G ////////////////////
	0X00000000, 0X00780000, 0X008600CC, 0X00060006, 
	0X00C600F6, 0X003E006C, 0X00000000, 0X00000000, 
	//////////////////// H ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00FE00C6, 
	0X00C600C6, 0X00C600C6, 0X00000000, 0X00000000, 
	//////////////////// I ////////////////////
	0X00000000, 0X003C0000, 0X00180018, 0X00180018, 
	0X00180018, 0X003C0018, 0X00000000, 0X00000000, 
	//////////////////// J ////////////////////
	0X00000000, 0X00F00000, 0X00600060, 0X00600060, 
	0X00660060, 0X003C0066, 0X00000000, 0X00000000, 
	//////////////////// K ////////////////////
	0X00000000, 0X00CE0000, 0X006C00CC, 0X003C006C, 
	0X006C006C, 0X00CE00CC, 0X00000000, 0X00000000, 
	//////////////////// L ////////////////////
	0X00000000, 0X001E0000, 0X000C000C, 0X000C000C, 
	0X008C000C, 0X00FE00CC, 0X00000000, 0X00000000, 
	//////////////////// M ////////////////////
	0X00000000, 0X00C60000, 0X00FE00EE, 0X00C600D6, 
	0X00C600C6, 0X00C600C6, 0X00000000, 0X00000000, 
	//////////////////// N ////////////////////
	0X00000000, 0X00C60000, 0X00DE00CE, 0X00F600FE, 
	0X00C600E6, 0X00C600C6, 0X00000000, 0X00000000, 
	//////////////////// O ////////////////////
	0X00000000, 0X00380000, 0X00C6006C, 0X00C600C6, 
	0X00C600C6, 0X0038006C, 0X00000000, 0X00000000, 
	//////////////////// P ////////////////////
	0X00000000, 0X007E0000, 0X00CC00CC, 0X007C00CC, 
	0X000C000C, 0X001E000C, 0X00000000, 0X00000000, 
	//////////////////// Q ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X00C600C6, 
	0X00F600D6, 0X0060007C, 0X000000E0, 0X00000000, 
	//////////////////// R ////////////////////
	0X00000000, 0X007E0000, 0X00CC00CC, 0X007C00CC, 
	0X00CC006C, 0X00CE00CC, 0X00000000, 0X00000000, 
	//////////////////// S ////////////////////
	0X00000000, 0X007C0000, 0X00C600C6, 0X0038000C, 
	0X00C60060, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// T ////////////////////
	0X00000000, 0X00FF0000, 0X00180099, 0X00180018, 
	0X00180018, 0X003C0018, 0X00000000, 0X00000000, 
	//////////////////// U ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00C600C6, 
	0X00C600C6, 0X007C00C6, 0X00000000, 0X00000000, 
	//////////////////// V ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00C600C6, 
	0X006C00C6, 0X00100038, 0X00000000, 0X00000000, 
	//////////////////// W ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X00C600C6, 
	0X00FE00D6, 0X00C600EE, 0X00000000, 0X00000000, 
	//////////////////// X ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X0038006C, 
	0X00C6006C, 0X00C600C6, 0X00000000, 0X00000000, 
	//////////////////// Y ////////////////////
	0X00000000, 0X00C60000, 0X00C600C6, 0X0038006C, 
	0X00380038, 0X007C0038, 0X00000000, 0X00000000, 
	//////////////////// Z ////////////////////
	0X00000000, 0X00FE0000, 0X006200C6, 0X00180030, 
	0X0086000C, 0X00FE00C6, 0X00000000, 0X00000000, 
	//////////////////// S ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000, 
	0X00000000, 0X00000000, 0X00000000, 0X00000000, 
	//////////////////// = ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X0000007E, 
	0X007E0000, 0X00000000, 0X00000000, 0X00000000, 
	//////////////////// , ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000, 
	0X00180000, 0X00180018, 0X0000000C, 0X00000000,
	//////////////////// : ////////////////////
	0X00000000, 0X00000000, 0X001C003C, 0X00000000,
	0X00000000, 0X001C003C, 0X00000000, 0X00000000,
	//////////////////// / ////////////////////
	0X00000000, 0X03800000, 0X01C00180, 0X00E000C0,
	0X00700060, 0X00380030, 0X001C0018, 0X00000000,
	//////////////////// ~ ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X31F800F0,
	0X0E001F0C, 0x00000000, 0x00000000, 0x00000000,
	//////////////////// - ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X000003FE,
	0X00000000, 0X00000000, 0X00000000, 0x00000000,
	//////////////////// _ ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0x7FFF0000, 0x00007FFF, 0x00000000,
	//////////////////// | ////////////////////
	0X00600060, 0X00600060, 0X00600060, 0X00600060,
	0X00600060, 0X00600060, 0X00600060, 0X00600060,
	//////////////////// ( ////////////////////
	0X00F000C0, 0X00180038, 0X000C001C, 0X000E000E,
	0X000E000E, 0X001C001C, 0X00380018, 0X00E00070,
	//////////////////// ) ////////////////////
	0X001C000E, 0X00700038, 0X00E000E0, 0X00E000E0,
	0X00E000E0, 0X006000E0, 0X00380070, 0X000E001C,
	//////////////////// { ////////////////////
	0X00C001C0, 0X00E000E0, 0X00E000E0, 0X006000E0,
	0X00E000F0, 0X00E000E0, 0X00E000E0, 0X038000C0,
	//////////////////// } ////////////////////
	0X01C000E0, 0X01C001C0, 0X01C001C0, 0X038001C0,
	0X01C003C0, 0X01C001C0, 0X01C001C0, 0X00E001C0,
	//////////////////// < ////////////////////
	0X00000000, 0X00E000C0, 0X001C0070, 0X000F000E,
	0X0038001C, 0X00E00070, 0X000000C0, 0X00000000,
	//////////////////// > ////////////////////
	0X00000000, 0X00070003, 0X0038001C, 0X00F00070,
	0X001C0038, 0X0007000E, 0X00000003, 0X00000000,
	//////////////////// [ ////////////////////
	0X0003001F, 0X00030003, 0X00030003, 0X00030003,
	0X00030003, 0X00030003, 0X00030003, 0X0000001F,
	//////////////////// ] ////////////////////
	0X0018001F, 0X00180018, 0X00180018, 0X00180018,
	0X00180018, 0X00180018, 0X00180018, 0X0000001F,
	//////////////////// \ ////////////////////
	0X00000000, 0X0038001C, 0X00700038, 0X00E00070,
	0X01C000E0, 0X03800180, 0X03000300, 0X00000000,
	//////////////////// @ ////////////////////
	0X00000000, 0X019C01F8, 0X03FE038C, 0X03FE03FE,
	0X01FE03FE, 0X01FE03FE, 0X01DC000C, 0X000000F8,
	//////////////////// ? ////////////////////
	0X00000000, 0X01C700FE, 0X01EE01CF, 0X007000E0,
	0X00180038, 0X00000000, 0X003C003C, 0X00000000,
	//////////////////// ; ////////////////////	
	0X00000000, 0X00000000, 0X00000000, 0X00380038,
	0X00000000, 0X00180000, 0X0030003C, 0X00180038,
	//////////////////// ^ ////////////////////
	0X00700000, 0X018C00D8, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// $ ////////////////////
	0X00300000, 0X00FC00F8, 0X003601B6, 0X0078003C,
	0X01B000F0, 0X01B601B0, 0X007C00FC, 0X00000030,
	//////////////////// ` ////////////////////
	0X000C0000, 0X00000018, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// " ////////////////////
	0X01B00000, 0X00D800D8, 0X000000D8, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// # ////////////////////
	0X06C00000, 0X06C006C0, 0X0FF006C0, 0X03600360,
	0X03600360, 0X01B007F8, 0X01B001B0, 0X000001B0,
	//////////////////// % ////////////////////
	0X00000000, 0X03F00660, 0X01F803D8, 0X00C001B0,
	0X036000C0, 0X06F007E0, 0X019803F0, 0X00000000,
	//////////////////// ! ////////////////////
	0X00000000, 0X00C000C0, 0X00C000C0, 0X00C000C0,
	0X00C000C0, 0X000000C0, 0X00C00000, 0X000000C0,
	//////////////////// & ////////////////////
	0X00000000, 0X00380000, 0X006C006C, 0X0018003C,
	0X00FC01FC, 0X00E600F6, 0X01E60066, 0X000000FC,
	//////////////////// * ////////////////////
	0X00000000, 0X00000000, 0X00300000, 0X007800FC,
	0X00780030, 0X003000FC, 0X00000000, 0X00000000,
	//////////////////// + ////////////////////
	0X00000000, 0X00000000, 0X00180018, 0X00180018,
	0X001800FF, 0X00180018, 0X00000018, 0X00000000,
	//////////////////// . ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X000C0000, 0X0000000C
};

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
unsigned int  OSD_Font_X4[] = 
{	
	//////////////////// S ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X03000000, 0X000000FF, 0X1F000000, 0X0000C0FF,
	0X7F000000, 0X0000C0FF, 0XFF000000, 0X0000C0FF,
	0XFF010000, 0X0000C0FF, 0XFC030000, 0X0000C003,
	0XF0030000, 0X00004000, 0XE0070000, 0X00000000,
	0XE0070000, 0X00000000, 0XE0070000, 0X00000000,
	0XE0070000, 0X00000000, 0XE0070000, 0X00000000,
	0XF0070000, 0X00000000, 0XF8030000, 0X00000000,
	0XFE030000, 0X00000000, 0XFF010000, 0X00000080,
	0XFF000000, 0X000000E0, 0X7F000000, 0X000000F8,
	0X3F000000, 0X000000FC, 0X0F000000, 0X000000FF,
	0X03000000, 0X000080FF, 0X00000000, 0X0000C0FF,
	0X00000000, 0X0000E03F, 0X00000000, 0X0000E00F,
	0X00000000, 0X0000E007, 0X00000000, 0X0000F003,
	0X00000000, 0X0000F003, 0X00000000, 0X0000F003,
	0X00000000, 0X0000F003, 0X00040000, 0X0000E007,
	0X00070000, 0X0000E007, 0XC0070000, 0X0000E01F,
	0XFF070000, 0X0000C0FF, 0XFF070000, 0X000080FF,
	0XFF070000, 0X000000FF, 0XFF030000, 0X000000FC,
	0X3F000000, 0X000000E0, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	
	//////////////////// O ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X1F000000, 0X000000F0, 0XFF000000, 0X000000FE,
	0XFF030000, 0X000080FF, 0XFF070000, 0X0000C0FF,
	0XFF0F0000, 0X0000F0FF, 0XF01F0000, 0X0000F81F,
	0XC03F0000, 0X0000F803, 0X007F0000, 0X0000FC01,
	0X007E0000, 0X0000FE00, 0X00FE0000, 0X00007E00,
	0X00FC0000, 0X00007E00, 0X00F80100, 0X00003F00,
	0X00F80100, 0X00003F00, 0X00F80100, 0X00003F00,
	0X00F00100, 0X00801F00, 0X00F00300, 0X00801F00,
	0X00F00300, 0X00801F00, 0X00F00300, 0X00801F00,
	0X00F00300, 0X00801F00, 0X00F00300, 0X00801F00,
	0X00F00300, 0X00801F00, 0X00F00300, 0X00801F00,
	0X00F00100, 0X00001F00, 0X00F80100, 0X00003F00,
	0X00F80100, 0X00003F00, 0X00F80100, 0X00003F00,
	0X00FC0000, 0X00007E00, 0X00FE0000, 0X00007E00,
	0X007E0000, 0X0000FC00, 0X007F0000, 0X0000FC01,
	0XC03F0000, 0X0000F803, 0XF01F0000, 0X0000F01F,
	0XFF0F0000, 0X0000E0FF, 0XFF070000, 0X0000C0FF,
	0XFF030000, 0X000080FF, 0XFF000000, 0X000000FE,
	0X1F000000, 0X000000F0, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,

	//////////////////// N ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X003F0000, 0X00001F00, 0X803F0000, 0X00001F00,
	0XC03F0000, 0X00001F00, 0XC03F0000, 0X00001F00,
	0XE03F0000, 0X00001F00, 0XF03F0000, 0X00001F00,
	0XF03F0000, 0X00001F00, 0XF83F0000, 0X00001F00,
	0XFC3E0000, 0X00001F00, 0XFC3E0000, 0X00001F00,
	0X7E3E0000, 0X00001F00, 0X3F3E0000, 0X00001F00,
	0X3F3E0000, 0X00001F00, 0X1F3E0000, 0X00001F80,
	0X0F3E0000, 0X00001F80, 0X0F3E0000, 0X00001FC0,
	0X073E0000, 0X00001FE0, 0X033E0000, 0X00001FE0,
	0X033E0000, 0X00001FF0, 0X013E0000, 0X00001FF8,
	0X003E0000, 0X00001FF8, 0X003E0000, 0X00001FFC,
	0X003E0000, 0X00001F7E, 0X003E0000, 0X00001F7E,
	0X003E0000, 0X00001F3F, 0X003E0000, 0X00001F1F,
	0X003E0000, 0X00009F1F, 0X003E0000, 0X0000DF0F,
	0X003E0000, 0X0000DF07, 0X003E0000, 0X0000FF07,
	0X003E0000, 0X0000FF03, 0X003E0000, 0X0000FF01,
	0X003E0000, 0X0000FF01, 0X003E0000, 0X0000FF00,
	0X003E0000, 0X00007F00, 0X003E0000, 0X00007F00,
	0X003E0000, 0X00003F00, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// I ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X3F000000, 0X000000FE,
	0X3F000000, 0X000000FE, 0X3F000000, 0X000000FE,
	0X3F000000, 0X000000FE, 0X3F000000, 0X000000FE,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X03000000, 0X000000E0,
	0X03000000, 0X000000E0, 0X3F000000, 0X000000FE,
	0X3F000000, 0X000000FE, 0X3F000000, 0X000000FE,
	0X3F000000, 0X000000FE, 0X3F000000, 0X000000FE,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	//////////////////// X ////////////////////
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00F80300, 0X0000F803, 0X00FC0100, 0X0000F003,
	0X00FC0000, 0X0000E007, 0X00FE0000, 0X0000C00F,
	0X007E0000, 0X0000C00F, 0X003F0000, 0X0000801F,
	0X003F0000, 0X0000003F, 0X801F0000, 0X0000003F,
	0XC01F0000, 0X0000007E, 0XC00F0000, 0X000000FC,
	0XE0070000, 0X000000FC, 0XE1070000, 0X000000F8,
	0XF3030000, 0X000000F0, 0XF3010000, 0X000000F0,
	0XFF010000, 0X000000E0, 0XFF000000, 0X000000C0,
	0XFF000000, 0X000000C0, 0X7F000000, 0X00000080,
	0X3F000000, 0X00000080, 0X7F000000, 0X00000080,
	0XFF000000, 0X000000C0, 0XFF000000, 0X000000E0,
	0XFB010000, 0X000000E0, 0XF3030000, 0X000000F0,
	0XF1030000, 0X000000F8, 0XE1070000, 0X000000F8,
	0XC00F0000, 0X000000FC, 0XC00F0000, 0X000000FC,
	0X801F0000, 0X0000007E, 0X803F0000, 0X0000003F,
	0X003F0000, 0X0000003F, 0X007E0000, 0X0000801F,
	0X00FE0000, 0X0000C01F, 0X00FC0000, 0X0000C00F,
	0X00FC0100, 0X0000E007, 0X00F80300, 0X0000F007,
	0X00F00700, 0X0000F003, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
	0X00000000, 0X00000000, 0X00000000, 0X00000000,
};
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA



#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
void snx_rc_mroi_flow(void *arg)
{
	stream_conf_t *stream = (stream_conf_t *)arg;
	struct snx_m2m *m2m = &stream->m2m;
	struct snx_rc *rc = &stream->rc;
	int num = 7;					//MROI Area 7 (Default using Area 7)
	int weight = 2;					//MROI Weight  setting (Default using 2) works when multi-area is using
	int mori_qp = -6;

	if (!m2m->ds_dev_name) {
		printf("NO ds_dev_name\n");
		return;
	}

	snx_codec_mroi_enable(m2m, 0);	//need to disable when set region atti

	if (rc) {
		rc->width = m2m->width;
		rc->height = m2m->height;
		rc->mroi_region[num].weight = weight;
		rc->mroi_region[num].qp = mori_qp;
		rc->mroi_region[num].pos_x = m2m->width/4;
		rc->mroi_region[num].pos_y = m2m->height/4;
		rc->mroi_region[num].dim_x = m2m->width/2;
		rc->mroi_region[num].dim_y = m2m->height/2;
		rc->mroi_region[num].ext_size = 1;				//Turn on extension size
	}

	snx_codec_mroi_set_region_attr(m2m, rc, num);

	snx_codec_mroi_enable(m2m, 1);
}

#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA

/*
	Before capture stream starts, make sure one m2m stream is started.
	The capture stream is related to the m2m stream, including 
	frame rate 
	scaling down (1 1/2 1/4)
	format (H264, MJPEG, RAW)
*/
void snx_cap_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	
	struct timeval tv;
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	int count_num = 0;
	int ret;

	unsigned char * pYUV420 = NULL;
	if(m2m->codec_fmt == V4L2_PIX_FMT_SNX420)
		pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);

	
	if (m2m->m2m) {
		printf("[Example] m2m = 1, should run m2m thread!\n");
		pthread_exit(NULL); 
	}

	if (stream->mutex) {
		/* Wait for the m2m stream has been start */
		pthread_mutex_lock(stream->mutex);
		(*stream->ref)++;
		pthread_cond_wait(stream->cond, stream->mutex);
		pthread_mutex_unlock(stream->mutex);
	}

	printf("============snx_cap_flow============\n");

	/* 1. Open Video Encode device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	

	/* 2. Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* 3. Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* QP setting */
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		
		//H264 QP
		snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP); 
		snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);

	} else {
		// JPEG 
		snx_codec_set_qp(m2m , 0);
	}
	
	/* 4. Start Video Encode */
	stream->streamon=1;
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;

	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	
	while(1) {
		/* 5. Read from Video Codec */
		ret = snx_codec_read(m2m);
		/* 6. Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;
			
			data_size += m2m->cap_bytesused;
			
			if (fps_debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;
					
					printf("snx_record %d x %d Real fps = %lld,(real_frames= %d)\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					,(((long long unsigned int)count_num * 1000000) / period)
					,count_num);
				}
			}
			
			/* Handle the encoded frame */
			if(stream->frame_num > 0) {
				if (fd) {
					/* Save the encoded frame to SD card */
					int has_written = 0;
					int leng;
					int size =0;

					unsigned char * target_ptr = NULL;

					if(m2m->codec_fmt != V4L2_PIX_FMT_SNX420) {
						size =m2m->cap_bytesused;
						target_ptr = m2m->cap_buffers[m2m->cap_index].start;
					}
					else {	//if(pYUV420)
						size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;

						ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
							, (m2m->width/m2m->scale)
							, (m2m->height/m2m->scale));
						target_ptr = pYUV420;
					}
				
					while(1){
						leng = write(fd, target_ptr + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
				

/*
						while(1){
							leng = write(fd, m2m->cap_buffers[m2m->cap_index].start + has_written, m2m->cap_bytesused - has_written);
							if(leng <= 0);
							else if(leng <  m2m->cap_bytesused - has_written){
								has_written += leng;
							}else  if(leng ==  m2m->cap_bytesused - has_written){
								break;
							}else{
								printf("Write error");
								break;
							}
						}
*/
				}

				if(!stream->live)
					stream->frame_num--;	//recording 
			}  
			
			
		} //if(m2m->cap_bytesused != 0)
		
		/* 8. Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
	    if(stream->frame_num == 0) {
			if (stream->mutex) {
				pthread_mutex_lock(stream->mutex);
				/* reference - 1 when this capture stream is done */
				(*stream->ref)--;
				pthread_mutex_unlock(stream->mutex);
			}
			break;
		}
	}

	/* To finish the m2m stream properly, call the apis in order */
err_start:
	/* 9. Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* 10. un-initialize Video codec */
	snx_codec_uninit(m2m);
	/* 11. Close video encode device */
	close(m2m->codec_fd);
	if(pYUV420)
		free(pYUV420);	
	printf("============snx_cap_flow============End\n");
	pthread_exit(0);
}

/*
	The m2m flow starts to encode video and save to SD card
*/
void snx_m2m_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int ret;

	unsigned char * pYUV420 = NULL;
	if(m2m->codec_fmt == V4L2_PIX_FMT_SNX420)
		pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);

	if (!m2m->m2m) {
		printf("[Example] m2m = 0, should run cap thread!\n");
		pthread_exit(NULL); 
	}

	printf("============snx_m2m_flow============\n");

	if(m2m->m2m) {
		/* 1. Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* 2. Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* 3. Start ISP */
		snx_isp_start(m2m);
	}

	/* 4. Open Video Encode Device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	

	/* 5. Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* 6. Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* QP setting */
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		
		//H264 QP
		snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP); 
		snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);

	} else {
		// JPEG 
		snx_codec_set_qp(m2m , 0);
	}
	
	/* 7. Start Video Encode */
	stream->streamon=1;
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;
	
	/* Boardcast cond to let all capture streams start */
	if (stream->mutex) {
		pthread_mutex_lock(stream->mutex);
		pthread_cond_broadcast(stream->cond);
		pthread_mutex_unlock(stream->mutex);
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	
	while(1) {
		/* 8. Read from Video Codec */
		ret = snx_codec_read(m2m);
		/* 9. Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;
			
			data_size += m2m->cap_bytesused;
			
			if (fps_debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;
					
					printf("snx_record %d x %d Real fps = %lld,(real_frames= %d)\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					,(((long long unsigned int)count_num * 1000000) / period)
					,count_num);
				}
			}
			
			/* Handle the encoded frame */
			if(stream->frame_num > 0) {
				if (fd) {
					/* Save the encoded frame to SD card */
					/* Save the encoded frame to SD card */
					int has_written = 0;
					int leng;
					int size =0;

					unsigned char * target_ptr = NULL;

					if(m2m->codec_fmt != V4L2_PIX_FMT_SNX420) {
						size =m2m->cap_bytesused;
						target_ptr = m2m->cap_buffers[m2m->cap_index].start;
					}
					else {	//if(pYUV420)
						size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;

						ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
							, (m2m->width/m2m->scale)
							, (m2m->height/m2m->scale));
						target_ptr = pYUV420;
					}
				
					while(1){
						leng = write(fd, target_ptr + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
				}

				if(!stream->live)
					stream->frame_num--;	//recording 
			}  
	    	
		} //if(m2m->cap_bytesused != 0)
		
		/* 11. Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
	    if(stream->frame_num == 0) {
			if (stream->mutex) {
				pthread_mutex_lock(stream->mutex);
				if(*stream->ref == 0) {
					pthread_mutex_unlock(stream->mutex);
					break;
				}
				pthread_mutex_unlock(stream->mutex);
			} else {
				break;
			}
		}
		
	}
	
	/* To finish the m2m stream properly, call the apis in order */
err_start:
	/* 12. Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* 13. un-initialize Video codec */
	snx_codec_uninit(m2m);
	if(m2m->m2m) {
		/* 14. Stop ISP */
		snx_isp_stop(m2m);
		/* 15. Un-initialize ISP */
		snx_isp_uninit(m2m);
	}
	/* 16. Close video encode device */
	close(m2m->codec_fd);
	if(m2m->m2m) {
		/* 17. Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);
	printf("============snx_m2m_flow============End\n");	
	pthread_exit(0);
}

/*
	M2M + Cap : Integrate M2M and Capture stream flow 
*/
void snx_m2m_cap_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int ret;

	unsigned char * pYUV420 = NULL;
	if(m2m->codec_fmt == V4L2_PIX_FMT_SNX420)
		pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);

	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}

	printf("============snx_m2m_cap_flow============\n");

	if(m2m->m2m) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Open Video Encode Device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	

	/* Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* QP setting */
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		
		//H264 QP
		snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP); 
		snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);

	} else {
		// JPEG 
		snx_codec_set_qp(m2m , 0);
	}
	
	/* Start Video Encode */
	stream->streamon=1;
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;
	
	/* Boardcast cond to let all capture streams start */
	if (m2m->m2m) {
		if (stream->mutex) {
			pthread_mutex_lock(stream->mutex);
			pthread_cond_broadcast(stream->cond);
			pthread_mutex_unlock(stream->mutex);
		}
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	
	while(1) {
		/* Read from Video Codec */
		ret = snx_codec_read(m2m);
		/* Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;
			
			data_size += m2m->cap_bytesused;
			
			if (fps_debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;
					
					printf("snx_record %d x %d Real fps = %lld,(real_frames= %d)\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					,(((long long unsigned int)count_num * 1000000) / period)
					,count_num);
				}
			}
			
			/* Handle the encoded frame */
			if(stream->frame_num > 0) {
				if (fd) {
					/* Save the encoded frame to SD card */
					/* Save the encoded frame to SD card */
					int has_written = 0;
					int leng;
					int size =0;

					unsigned char * target_ptr = NULL;

					if(m2m->codec_fmt != V4L2_PIX_FMT_SNX420) {
						size =m2m->cap_bytesused;
						target_ptr = m2m->cap_buffers[m2m->cap_index].start;
					}
					else {	//if(pYUV420)
						size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;

						ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
							, (m2m->width/m2m->scale)
							, (m2m->height/m2m->scale));
						target_ptr = pYUV420;
					}
				
					while(1){
						leng = write(fd, target_ptr + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
				}

				if(!stream->live)
					stream->frame_num--;	//recording 
			}  
	    	
		} //if(m2m->cap_bytesused != 0)
		
		/* Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
	    if(stream->frame_num == 0) {
			if (stream->mutex) {
				if (m2m->m2m) {		//M2M stream
				
					pthread_mutex_lock(stream->mutex);
					if(*stream->ref == 0) {
						pthread_mutex_unlock(stream->mutex);
						break;
					}
					pthread_mutex_unlock(stream->mutex);
					
				} else {
					
					pthread_mutex_lock(stream->mutex);
					/* reference - 1 when this capture stream is done */
					(*stream->ref)--;
					pthread_mutex_unlock(stream->mutex);
					break;
				}
			} else {
				break;
			}
		}
		
	}
	
	/* To finish the m2m stream properly, call the apis in order */
err_start:
	/* Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* un-initialize Video codec */
	snx_codec_uninit(m2m);
	if(m2m->m2m) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}
	/* Close video encode device */
	close(m2m->codec_fd);
	if(m2m->m2m) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);
	printf("============snx_m2m_cap_flow============End\n");	
	pthread_exit(0);
}

/*
	M2M + Cap + Rc : Integrate M2M and Capture stream flow with Rate Control
*/
void snx_m2m_cap_rc_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	struct snx_rc *rc = NULL;			//rate control use

	
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int real_fps = 0;					//real_fps frame count
	int ret;
	unsigned char * pYUV420 = NULL;
	if(m2m->codec_fmt == V4L2_PIX_FMT_SNX420)
		pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);

	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}

	printf("============snx_m2m_cap_rc_flow============\n");
	if(m2m->m2m) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Open Video Encode Device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	

	/* Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* Bitrate Rate Control is only support for H264 */
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		
		if (m2m->bit_rate) {
			rc = malloc(sizeof(struct snx_rc));
			stream->rc = rc;
			
			/* Initialize rate control arguments */
			rc->width = m2m->width/m2m->scale;	//Bit-rate control width
			rc->height = m2m->height/m2m->scale;	//Bit rate control height
			rc->codec_fd = m2m->codec_fd;		//point to the codec fd
			rc->Targetbitrate = m2m->bit_rate;	//rate control target bitrate
			rc->framerate = m2m->codec_fps;		//point to the codec frame rate
			rc->gop = m2m->gop;					//codec gop
			//rc->reinit = 1;
			/*Initialize rate control */
			m2m->qp = snx_codec_rc_init(rc, SNX_RC_INIT);
		} else {
			//H264 QP
			snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP); 
  			snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);
		}
	} else {
		// JPEG 
		snx_codec_set_qp(m2m , 0);
	}

	/* Start Video Encode */
	stream->streamon=1;
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;
	
	/* Data Stamp */
	if(strlen(stream->cds.dev_name))
		snx_vc_data_stamp(DS_SET_ALL, (void *)&stream->cds);

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	// MROI Set up flow ONLY for SN98660 with rate control
	snx_rc_mroi_flow(arg);
#endif	

	/* Boardcast cond to let all capture streams start */
	if (m2m->m2m) {
		if (stream->mutex) {
			pthread_mutex_lock(stream->mutex);
			pthread_cond_broadcast(stream->cond);
			pthread_mutex_unlock(stream->mutex);
		}
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	
	while(1) {
		if(stream->streamon!=m2m->codec_streamon){
			if(stream->streamon==1)
				snx_codec_start(m2m);
			else	
				snx_codec_stop(m2m);
		}

		/* Read from Video Codec */
		ret = snx_codec_read(m2m);


		/* Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			real_fps ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;
#if 0	// Show bit rate base on I frame
			if(m2m->flags & V4L2_BUF_FLAG_KEYFRAME) {
				printf("%dX%d %d fps %lld Kbps  QP == %d\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					, (data_size>>7) 
					, m2m->qp);
					real_fps = 0;
					data_size = 0;								
			}
			data_size += m2m->cap_bytesused;
#else // Show bit rate base on 1 sec
			data_size += m2m->cap_bytesused;

			if (fps_debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;

					printf("(%s) %d x %d fps=%d (frames= %d), bps=%lld Kbps(QP=%d) \n"
					, (m2m->codec_fmt == V4L2_PIX_FMT_H264)?"H264":"JPEG"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					,count_num
					, (data_size>>7) 
					, m2m->qp);

					real_fps = 0;
					data_size = 0;
				}
			}
#endif
			
			/* 
				Bit Rate Control Flow 
				Update the QP of the next frame to keep the bitrate. (CBR).
			*/

			if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {
				m2m->qp = snx_codec_rc_update(m2m, rc);
			}
			
			/* Handle the encoded frame */
			if(stream->frame_num > 0) {
				if (fd) {
					/* Save the encoded frame to SD card */
					/* Save the encoded frame to SD card */
					int has_written = 0;
					int leng;
					int size =0;

					unsigned char * target_ptr = NULL;

					if(m2m->codec_fmt != V4L2_PIX_FMT_SNX420) {
						size =m2m->cap_bytesused;
						target_ptr = m2m->cap_buffers[m2m->cap_index].start;
					}
					else {	//if(pYUV420)
						size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;

						ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
							, (m2m->width/m2m->scale)
							, (m2m->height/m2m->scale));
						target_ptr = pYUV420;
					}
	
					while(1){
						leng = write(fd, target_ptr + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
				}

				if(!stream->live)
					stream->frame_num--;	//recording 
			}  
	    	
		} //if(m2m->cap_bytesused != 0)

		/* Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);

//		if(m2m->cap_bytesused == 0) {
//			usleep(10000);
//			sleep(1);
//		}

		
		/* Save the frames until the frame number has been reached */
	    if(stream->frame_num == 0) {
			if (stream->mutex) {
				if (m2m->m2m) {		//M2M stream
				
					pthread_mutex_lock(stream->mutex);
					if(*stream->ref == 0) {
						pthread_mutex_unlock(stream->mutex);
						break;
					}
					pthread_mutex_unlock(stream->mutex);
					
				} else {			//Capture Stream
					
					pthread_mutex_lock(stream->mutex);
					/* reference - 1 when this capture stream is done */
					(*stream->ref)--;
					pthread_mutex_unlock(stream->mutex);
					break;
				}
			} else {
				break;
			}
		}
		
	}

	/* To finish the m2m stream properly, call the apis in order */
err_start:
	if(rc)
		free(rc);

	/* Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* un-initialize Video codec */

	snx_codec_uninit(m2m);
	if(m2m->m2m) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}
	/* Close video encode device */
	close(m2m->codec_fd);
	if(m2m->m2m) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);	
	printf("============snx_m2m_cap_rc_flow============End\n");	
	pthread_exit(0);
}


/*
	M2M + input file + Rc : Integrate M2M and Capture stream flow with Rate Control
*/
void snx_m2m_infile_rc_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	int in_fd = stream->in_fd;
	
	struct snx_rc *rc = NULL;			//rate control use
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int real_fps = 0;					//real_fps frame count
	int ret;
	unsigned char * target_ptr = NULL;

	unsigned char * pYUV420 = NULL;
	unsigned char * i_pYUV420 = NULL;
	
	if(m2m->codec_fmt == V4L2_PIX_FMT_SNX420)
		pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);

	if(m2m->out_mem == V4L2_MEMORY_MMAP)
		i_pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);

	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}

	printf("============snx_m2m_infile_rc_flow============\n");

	if((m2m->m2m) && (m2m->out_mem != V4L2_MEMORY_MMAP)) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Open Video Encode Device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	

	/* Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* Bitrate Rate Control is only support for H264 */
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		
		if (m2m->bit_rate) {
			rc = malloc(sizeof(struct snx_rc));
			stream->rc = rc;
			
			/* Initialize rate control arguments */
			rc->width = m2m->width/m2m->scale;	//Bit-rate control width
			rc->height = m2m->height/m2m->scale;	//Bit rate control height
			rc->codec_fd = m2m->codec_fd;		//point to the codec fd
			rc->Targetbitrate = m2m->bit_rate;	//rate control target bitrate
			rc->framerate = m2m->codec_fps;		//point to the codec frame rate
			rc->gop = m2m->gop;					//codec gop
			//rc->reinit = 1;
			/*Initialize rate control */
			m2m->qp = snx_codec_rc_init(rc, SNX_RC_INIT);
		} else {
			//H264 QP
			snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP); 
  			snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);
		}
	} else {
		// JPEG 
		snx_codec_set_qp(m2m , 0);
	}

	/* Start Video Encode */
	stream->streamon=1;
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;
	
	/* Data Stamp */
	if(strlen(stream->cds.dev_name))
		snx_vc_data_stamp(DS_SET_ALL, (void *)&stream->cds);

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	// MROI Set up flow ONLY for SN98660 with rate control
	snx_rc_mroi_flow(arg);
#endif
	
	/* Boardcast cond to let all capture streams start */
	if (m2m->m2m) {
		if (stream->mutex) {
			pthread_mutex_lock(stream->mutex);
			pthread_cond_broadcast(stream->cond);
			pthread_mutex_unlock(stream->mutex);
		}
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;


	while(1) {
		// from input file
		if((m2m->out_mem == V4L2_MEMORY_MMAP)) {
			read(in_fd, i_pYUV420,  (m2m->width*m2m->height*3)>>1);
			target_ptr = m2m->out_buffers[m2m->cap_index].start;
			snx_420_to_420line((char *)i_pYUV420, (char *)target_ptr, m2m->width, m2m->height);
		}
		
		/* Read from Video Codec */
		ret = snx_codec_read(m2m);
		/* Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			real_fps ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;
#if 0	// Show bit rate base on I frame
			if(m2m->flags & V4L2_BUF_FLAG_KEYFRAME) {
				printf("%dX%d %d fps %lld Kbps  QP == %d\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					, (data_size>>7) 
					, m2m->qp);
					real_fps = 0;
					data_size = 0;								
			}
			data_size += m2m->cap_bytesused;
#else // Show bit rate base on 1 sec
			data_size += m2m->cap_bytesused;

			if (fps_debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;
					
					printf("%d x %d fps=%d (frames= %d), bps=%lld Kbps(QP=%d) \n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					,count_num
					, (data_size>>7) 
					, m2m->qp);
					
					real_fps = 0;
					data_size = 0;
				}
			}
#endif
			
			/* 
				Bit Rate Control Flow 
				Update the QP of the next frame to keep the bitrate. (CBR).
			*/
			if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {
				m2m->qp = snx_codec_rc_update(m2m, rc);
			        snx_md_drop_fps(rc, &m2m->force_i_frame);
			}
			
			/* Handle the encoded frame */
			if(stream->frame_num > 0) {
				if (fd) {
					/* Save the encoded frame to SD card */
					/* Save the encoded frame to SD card */
					int has_written = 0;
					int leng;
					int size =0;


					if(m2m->codec_fmt != V4L2_PIX_FMT_SNX420) {
						size =m2m->cap_bytesused;
						target_ptr = m2m->cap_buffers[m2m->cap_index].start;
					}
					else {	//if(pYUV420)
						size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;

						ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
							, (m2m->width/m2m->scale)
							, (m2m->height/m2m->scale));
						target_ptr = pYUV420;
					}
	
					while(1){
						leng = write(fd, target_ptr + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
				}

				if(!stream->live)
					stream->frame_num--;	//recording 
			}  
	    	
		} //if(m2m->cap_bytesused != 0)

		/* Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
	    if(stream->frame_num == 0) {
			if (stream->mutex) {
				if (m2m->m2m) {		//M2M stream
				
					pthread_mutex_lock(stream->mutex);
					if(*stream->ref == 0) {
						pthread_mutex_unlock(stream->mutex);
						break;
					}
					pthread_mutex_unlock(stream->mutex);
					
				} else {			//Capture Stream
					
					pthread_mutex_lock(stream->mutex);
					/* reference - 1 when this capture stream is done */
					(*stream->ref)--;
					pthread_mutex_unlock(stream->mutex);
					break;
				}
			} else {
				break;
			}
		}
		
	}

	/* To finish the m2m stream properly, call the apis in order */
err_start:
	if(rc)
		free(rc);

	/* Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* un-initialize Video codec */

	snx_codec_uninit(m2m);
	if((m2m->m2m) && (m2m->out_mem != V4L2_MEMORY_MMAP)) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}
	/* Close video encode device */
	close(m2m->codec_fd);
	if((m2m->m2m) && (m2m->out_mem != V4L2_MEMORY_MMAP)) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);
	if(i_pYUV420)
		free(i_pYUV420);
		
	printf("============snx_m2m_infile_rc_flow============End\n");	
	pthread_exit(0);
}

void snx_m2m_yuv_flow(void *arg)
{
	long long unsigned int data_size = 0;					//Count the total datasize
	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	struct snx_rc *rc = NULL;			//rate control use
	
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int real_fps = 0;					//real_fps frame count
	int ret;

	int yuv_rate = 0;
	int yuv_count = 0;
	unsigned char * pYUV420 = NULL;
	pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);
	yuv_rate = (m2m->isp_fps/stream->yuv_frame) -1;
	
	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}

	printf("============snx_m2m_yuv_flow============\n");

	if(m2m->m2m) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Open Video Encode Device */
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	

	/* Initialize Video Encode */
	ret = snx_codec_init(m2m);
	if(ret != 0) goto err_init;

	/* Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* Bitrate Rate Control is only support for H264 */
	if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {
		
		rc = malloc(sizeof(struct snx_rc));
		
		/* Initialize rate control arguments */
		rc->width = m2m->width;				//Bit-rate control width
		rc->height = m2m->height;			//Bit rate control height
		rc->codec_fd = m2m->codec_fd;		//point to the codec fd
		rc->Targetbitrate = m2m->bit_rate;  //rate control target bitrate
		rc->framerate = m2m->codec_fps;		//point to the codec frame rate
		rc->gop = m2m->gop;					//codec gop
		//rc->reinit = 1;
		/*Initialize rate control */
		m2m->qp = snx_codec_rc_init(rc, SNX_RC_INIT);
	}
	
	/* Start Video Encode */
	stream->streamon=1;
	ret = snx_codec_start(m2m);
	if(ret != 0) goto err_start;
	
	/* Data Stamp */
	if(strlen(stream->cds.dev_name))
		snx_vc_data_stamp(DS_SET_ALL, (void *)&stream->cds);
	
	/* Boardcast cond to let all capture streams start */
	if (m2m->m2m) {
		if (stream->mutex) {
			pthread_mutex_lock(stream->mutex);
			pthread_cond_broadcast(stream->cond);
			pthread_mutex_unlock(stream->mutex);
		}
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	while(1) {
		/* Read from Video Codec */
		ret = snx_codec_read(m2m);
		/* Check if any frame encodec */
		if(m2m->cap_bytesused != 0) {
			count_num ++;
			real_fps ++;
			gettimeofday(&tv,NULL);
			end_t = tv.tv_sec * 1000000 + tv.tv_usec;
			period = end_t - start_t;
			
			data_size += m2m->cap_bytesused;
			
			if (fps_debug != 0) {
				if (period - pre_period >= 1000000) {
					pre_period = period;
					
					printf("%d x %d fps=%d (frames= %d), bps=%lld Kbps(QP=%d) \n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					,count_num
					, (data_size>>7) 
					, m2m->qp);
					real_fps = 0;
					data_size = 0;
				}
			}
			
			/* 
				Bit Rate Control Flow 
				Update the QP of the next frame to keep the bitrate. (CBR).
			*/
			if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {
				m2m->qp = snx_codec_rc_update(m2m, rc);
			        snx_md_drop_fps(rc, &m2m->force_i_frame);

				/*
				if (fps_debug) {
					printf("[RC] frames %d frames_gop = %d QP== %d\n"	
						, rc->frames
						, rc->frames_gop
						,m2m->qp);
				}
				*/
			}
			

			/* Handle the encoded frame */
			if(stream->frame_num > 0) {
//				if (fd) {
				if ((yuv_rate == yuv_count) && fd) {
					/* Save the encoded frame to SD card */
					//fwrite(m2m->cap_buffers[m2m->cap_index].start,  m2m->cap_bytesused, 1, fd);
					int has_written = 0;
					int leng;
					int size =0;
					int disable_uv=0;
					
					size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;
					if(pYUV420){
						ret = snx_420line_to_420(m2m->cap_buffers[m2m->cap_index].start, (char *)pYUV420
							, (m2m->width/m2m->scale)+disable_uv
							, (m2m->height/m2m->scale));
					}
					while(1){
						leng = write(fd, pYUV420 + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
					yuv_count =0;
					if(!stream->live)
						stream->frame_num--;	//recording 
				}
				else
					yuv_count++;    	

			} 
		} //if(m2m->cap_bytesused != 0)

		/* Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
	    if(stream->frame_num == 0) {
			if (stream->mutex) {
				if (m2m->m2m) {		//M2M stream
				
					pthread_mutex_lock(stream->mutex);
					if(*stream->ref == 0) {
						pthread_mutex_unlock(stream->mutex);
						break;
					}
					pthread_mutex_unlock(stream->mutex);
					
				} else {			//Capture Stream
					
					pthread_mutex_lock(stream->mutex);
					/* reference - 1 when this capture stream is done */
					(*stream->ref)--;
					pthread_mutex_unlock(stream->mutex);
					break;
				}
			} else {
				break;
			}
		}
		
	}
	
	/* To finish the m2m stream properly, call the apis in order */
err_start:
	if(rc)
		free(rc);
	/* Stop Video codec */
	snx_codec_stop(m2m);
err_init:
	/* un-initialize Video codec */
	snx_codec_uninit(m2m);
	if(m2m->m2m) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}
	/* Close video encode device */
	close(m2m->codec_fd);
	if(m2m->m2m) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);	
	printf("============snx_m2m_yuv_flow============End\n");	
	pthread_exit(0);
}

void snx_isp_yuv_flow(void *arg)
{
	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
	
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int real_fps = 0;					//real_fps frame count
	int ret;

	int yuv_rate = 0;
	int yuv_count = 0;

	unsigned char * pYUV420 = NULL;
	pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);
	yuv_rate = (m2m->isp_fps/stream->yuv_frame) -1;
	
	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}

	printf("============snx_m2m_isp_yuv_flow============\n");

	if(m2m->m2m) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Boardcast cond to let all capture streams start */
	if (m2m->m2m) {
		if (stream->mutex) {
			pthread_mutex_lock(stream->mutex);
			pthread_cond_broadcast(stream->cond);
			pthread_mutex_unlock(stream->mutex);
		}
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	while(1) {
		/* Read from ISP */
		ret = snx_isp_read(m2m);

		count_num ++;
		real_fps ++;
		gettimeofday(&tv,NULL);
		end_t = tv.tv_sec * 1000000 + tv.tv_usec;
		period = end_t - start_t;
		
		if (fps_debug != 0) {
			if (period - pre_period >= 1000000) {
				pre_period = period;
					
				printf("%d x %d Real fps = %d,(real_frames= %d)\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					,count_num);
			}
		}
			

		/* Handle the yuv frame */
		if(stream->frame_num > 0) {
			if ((yuv_rate == yuv_count) && fd) {
				/* Save the encoded frame to SD card */
				//fwrite(m2m->cap_buffers[m2m->cap_index].start,  m2m->cap_bytesused, 1, fd);
				int has_written = 0;
				int leng;
				int size =0;
				size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale)*3)>>1;
				if(pYUV420){
					ret = snx_420line_to_420(m2m->isp_buffers[m2m->cap_index].start, (char *)pYUV420
						, (m2m->width/m2m->scale)
						, (m2m->height/m2m->scale));
					while(1){
						leng = write(fd, pYUV420 + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}
				}
				yuv_count =0;
				if(!stream->live)
					stream->frame_num--;	//recording 
			}
			else
				yuv_count++;
			} 
		/* Reset ISP for the next frame */
		ret = snx_isp_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
		if(stream->frame_num == 0) {
			if (stream->mutex) {
				if (m2m->m2m) {		//M2M stream
				
					pthread_mutex_lock(stream->mutex);
					if(*stream->ref == 0) {
						pthread_mutex_unlock(stream->mutex);
						break;
					}
					pthread_mutex_unlock(stream->mutex);
					
				} else {			//Capture Stream
					
					pthread_mutex_lock(stream->mutex);
					/* reference - 1 when this capture stream is done */
					(*stream->ref)--;
					pthread_mutex_unlock(stream->mutex);
					break;
				}
			} else {
				break;
			}
		}
		
	}

	/* To finish the m2m stream properly, call the apis in order */

err_init:
	if(m2m->m2m) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}

	if(m2m->m2m) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);
	printf("============snx_m2m_isp_yuv_flow============End\n");	
	pthread_exit(0);
}





void snx_isp_bmp_flow(void *arg)
{
	BITMAPFILEHEADER bmp_file_header;
	BITMAPINFOHEADER bmp_map_head;

	stream_conf_t *stream = (stream_conf_t *)arg;		
	struct snx_m2m *m2m = &stream->m2m;
	int fd = stream->fd;
//	struct snx_rc *rc = NULL;			//rate control use
	
	struct timeval tv;					//for time calculation
	unsigned long long start_t =0, end_t =0, period=0, pre_period=0;
	
	int count_num = 0;					//Encoded frame count
	int real_fps = 0;					//real_fps frame count
	int ret;

	int yuv_rate = 0;
	int yuv_count = 0;
	unsigned char * pYUV420 = NULL;
	pYUV420 = (unsigned char*)malloc(m2m->width*m2m->height*2);
	yuv_rate = (m2m->isp_fps/stream->yuv_frame) -1;
	
	if (m2m->m2m == 0) {
		if (stream->mutex) {
			/* Wait for the m2m stream has been start */
			pthread_mutex_lock(stream->mutex);
			(*stream->ref)++;
			pthread_cond_wait(stream->cond, stream->mutex);
			pthread_mutex_unlock(stream->mutex);
		}
	}

	printf("============snx_isp_bmp_flow============\n");

	if(m2m->m2m) {
		/* Open ISP device */
		m2m->isp_fd = snx_open_device(m2m->isp_dev);
		
		/* Initialize ISP */
		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;

		/* Start ISP */
		snx_isp_start(m2m);
	}

	/* Boardcast cond to let all capture streams start */
	if (m2m->m2m) {
		if (stream->mutex) {
			pthread_mutex_lock(stream->mutex);
			pthread_cond_broadcast(stream->cond);
			pthread_mutex_unlock(stream->mutex);
		}
	}
	
	/* Start M2M Video Fetech and Record */
	gettimeofday(&tv,NULL);
	start_t = tv.tv_sec * 1000000 + tv.tv_usec;
	while(1) {
		/* Read from ISP */
		ret = snx_isp_read(m2m);

		count_num ++;
		real_fps ++;
		gettimeofday(&tv,NULL);
		end_t = tv.tv_sec * 1000000 + tv.tv_usec;
		period = end_t - start_t;
		
		if (fps_debug != 0) {
			if (period - pre_period >= 1000000) {
				pre_period = period;
					
				printf("%d x %d Real fps = %d,(real_frames= %d)\n"
					,(m2m->width/m2m->scale)
					,(m2m->height/m2m->scale) 
					, real_fps
					,count_num);
			}
		}

		/* Handle the yuv frame */
		if(stream->frame_num > 0) {
			if ((yuv_rate == yuv_count) && fd) {
				/* Save the encoded frame to SD card */
				//fwrite(m2m->cap_buffers[m2m->cap_index].start,  m2m->cap_bytesused, 1, fd);
				int has_written = 0;
				int leng;
				int size =0;

				size =((m2m->width/m2m->scale)*(m2m->height/m2m->scale));
				if(pYUV420){
					ret = snx_420line_to_420(m2m->isp_buffers[m2m->cap_index].start, (char *)pYUV420
						, (m2m->width/m2m->scale)
						, (m2m->height/m2m->scale));

					bmp_file_header.bftype = 0x4D42; // bftypeBM				
					bmp_file_header.bfSize = sizeof(BITMAPFILEHEADER) 
								+ sizeof(BITMAPINFOHEADER) 
								+ (m2m->width/m2m->scale)*(m2m->height/m2m->scale)*2;
					bmp_file_header.bfReserved1 = 0;
					bmp_file_header.bfReserved2 = 0;
					bmp_file_header.bfOffBits = sizeof(BITMAPFILEHEADER);

					bmp_map_head.biSize = sizeof(BITMAPINFOHEADER);
					bmp_map_head.biWidth = (m2m->width/m2m->scale);
					bmp_map_head.biHeight = (m2m->height/m2m->scale);
					bmp_map_head.biPlanes = 1;
					bmp_map_head.biBitCount = 8;
					bmp_map_head.biCompression = 0;
					bmp_map_head.biSizeImage = (m2m->width/m2m->scale)*(m2m->height/m2m->scale)*2;
					bmp_map_head.biXPelsPerMeter = (m2m->width/m2m->scale);
					bmp_map_head.biYPelsPerMeter = (m2m->height/m2m->scale);
					bmp_map_head.biClrUsed = 0;
					bmp_map_head.biClrImportant = 0;

					printf("write bmp file!\n");
					leng = write(fd, &bmp_file_header, sizeof(BITMAPFILEHEADER));
					leng = write(fd, &bmp_map_head, sizeof(BITMAPINFOHEADER));

					while(1){
						leng = write(fd, pYUV420 + has_written, size - has_written);
						if(leng <= 0);
						else if(leng <  size - has_written){
							has_written += leng;
						}else  if(leng ==  size - has_written){
							break;
						}else{
							printf("Write error");
							break;
						}
					}

				}
				yuv_count =0;
				if(!stream->live)
					stream->frame_num--;	//recording 
			}
			else
				yuv_count++;
			} 
		/* Reset ISP for the next frame */
		ret = snx_isp_reset(m2m);
		
		/* Save the frames until the frame number has been reached */
		if(stream->frame_num == 0) {
			if (stream->mutex) {
				if (m2m->m2m) {		//M2M stream
				
					pthread_mutex_lock(stream->mutex);
					if(*stream->ref == 0) {
						pthread_mutex_unlock(stream->mutex);
						break;
					}
					pthread_mutex_unlock(stream->mutex);
					
				} else {			//Capture Stream
					
					pthread_mutex_lock(stream->mutex);
					/* reference - 1 when this capture stream is done */
					(*stream->ref)--;
					pthread_mutex_unlock(stream->mutex);
					break;
				}
			} else {
				break;
			}
		}
		
	}
	/* To finish the m2m stream properly, call the apis in order */

err_init:
	if(m2m->m2m) {
		/* Stop ISP */
		snx_isp_stop(m2m);
		/* Un-initialize ISP */
		snx_isp_uninit(m2m);
	}

	if(m2m->m2m) {
		/* Close ISP device */
		close(m2m->isp_fd);
	}
	if(pYUV420)
		free(pYUV420);

	printf("============snx_isp_bmp_flow============End\n");		
	pthread_exit(0);

}



void snx_vc_data_stamp(int op, void *arg)
{

	struct snx_cds *cds = (struct snx_cds*)arg;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	unsigned int *font_table;
#endif
	if(strlen(cds->dev_name)==0) {
		printf("NO Devname for Data stamp\n");
		return;
	}
	
	switch (op) 
	{
		case DS_SET_ALL:
			snx_cds_set_all(cds->dev_name, cds);
			break;
		case DS_GET_ALL:
			snx_cds_get_enable(cds->dev_name, (int *)&cds->enable);
			snx_cds_get_color(cds->dev_name, &cds->t_color, &cds->b_color);\
			snx_cds_get_color_attr(cds->dev_name, &cds->attr);
			snx_cds_get_position(cds->dev_name, &cds->pos, &cds->dim);
			//snx_cds_get_datastamp(cds->dev_name, cds->string, 0);
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			break;
		case DS_SET_EN:
			snx_cds_set_enable(cds->dev_name, cds->enable);
			break;
		case DS_GET_EN:
			snx_cds_get_enable(cds->dev_name, (int *)&cds->enable);
			break;
		case DS_SET_COLOR:
			snx_cds_set_color(cds->dev_name, &cds->t_color, &cds->b_color);
			break;
		case DS_GET_COLOR:
			snx_cds_get_color(cds->dev_name, &cds->t_color, &cds->b_color);
			break;
		case DS_SET_COLOR_ATTR:
			snx_cds_set_color_attr(cds->dev_name, &cds->attr);
			break;
		case DS_GET_COLOR_ATTR:
			snx_cds_get_color_attr(cds->dev_name, &cds->attr);
			break;
		case DS_SET_POS:
			snx_cds_set_position(cds->dev_name, &cds->pos, &cds->dim);
			break;
		case DS_GET_POS:
			snx_cds_get_position(cds->dev_name, &cds->pos, &cds->dim);
			break;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
#else
		case DS_SET_STRING:
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			snx_cds_set_string(cds->dev_name, cds->string, cds->scale);
			break;
#endif
		case DS_SET_FONT_STRING:			
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
			if(cds->enable){				
				//font_table = malloc(((cds->font_width <= 32?32:64)>>3)*(cds->font_height)*strlen(cds->string));
				font_table = snx_cds_get_font_table(cds);
				if(font_table)
			    	snx_cds_set_string(cds, NULL, font_table, 0);
				//free(font_table);
		    }
		    else
		        snx_cds_set_enable(cds->dev_name, 0);
#else
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			snx_cds_set_font(cds, &ascii_2_font, &osd_font, sizeof(osd_font));
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA
			break;
		case DS_SET_DATA:
			snx_cds_set_datastamp(cds->dev_name, cds->string, strlen(cds->string));
			break;
		case DS_GET_DATA:
			snx_cds_get_datastamp(cds->dev_name, cds->string, 0);
			break;
		case DS_SET_BMP:
			snx_cds_get_scale(cds->dev_name, &cds->scale);
//			snx_cds_set_bmp(cds->dev_name, cds->string, cds->scale);
			snx_cds_set_bmp(cds);
			break;
		case DS_SET_SCALE:
			snx_cds_set_scale(cds->dev_name, cds->scale);
			break;
		case DS_GET_SCALE:
			snx_cds_get_scale(cds->dev_name, &cds->scale);
			break;
	}
}



/*
	The entrance of this file.
	One M2M streams would be created.
	The configuration can set on the setting definitions above. 
	After the stream conf are done, the thread of the stream would be created.
	The bitstreams of the stream will be record on the SD card.
*/
void snx_dynamic_modify(stream_conf_t *stream){
	char	str[20];
	int		target_fps=0;
	float	float_fps=0;
	char	target_fps_proc[64];
	char	*type;
	struct	snx_m2m *m2m = &stream->m2m;
	char	sys_cmd[128];
	int		ret;

	int		fd_stdin;
	fd_set	readfds;
	int		num_readable;
	struct	timeval tv;
	int		num_bytes;

	// wating codec stable
	usleep(500000);

	memset(target_fps_proc, 0x0, 64);
	sprintf(target_fps_proc, "/proc/codec/%s/target_fps", m2m->ds_dev_name);
	
	ret= snx_get_file_value(target_fps_proc, &target_fps, 10);
	if(ret == -1)	return;

	// Show message
	printf("bps==================================\n");
	printf("(%s) dynamic modify setup example\n", m2m->ds_dev_name);
	printf("INPUT >>>s:%d (on/off 1:start, 2:pause) \n", stream->streamon);
	printf("INPUT >>>f:%d (frame rate)\n", target_fps);
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264)
		printf("INPUT >>>b:%d (bit rate)\n", stream->rc->Targetbitrate);
	printf("bps==================================\n");

	float_fps = target_fps;
	memset(str, 0x0, 20);
//	gets(str);
	fd_stdin = fileno(stdin);

	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fileno(stdin), &readfds);
		tv.tv_sec =	10;	
		tv.tv_usec = 0;	
		fflush(stdout);	
		num_readable = select(fd_stdin + 1,	&readfds, NULL,	NULL, &tv);	
		if (num_readable ==	-1)	{
			fprintf(stderr,	"\nError in	select \n");
			exit(1);
		}
		if (num_readable ==	0) {
			printf("\get string timeout (10 seconds)\n");
			break;	/* since I don't want to test forever */
		} else {
			num_bytes =	read(fd_stdin, str,	20);
			if (num_bytes <	0) {
				fprintf(stderr,	"\nError on	read\n");	
				exit(1);
			}
			/* process command,	maybe by sscanf	*/
			break; /* to terminate loop, since I don't process anything	*/
		}
	}
		
	type = strstr(str,"s:");
	if(type != NULL) {
		stream->streamon = strtoul((type+2), NULL, 10);
		if(stream->streamon!=1)
			stream->streamon = 2;
	}

	type = strstr(str,"f:");
	if(type != NULL) {
		target_fps = strtoul((type+2), NULL, 10);
		float_fps = atof((type+2));
		snx_set_file_value(target_fps_proc, &target_fps, 10);
		sprintf(sys_cmd,"%f", float_fps);
		snx_set_file_string(target_fps_proc, sys_cmd);
	}

	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		type = strstr(str,"b:");
		if(type != NULL)
			stream->rc->Targetbitrate = strtoul((type+2), NULL, 10);
	}		

	// Show new message
	printf("bps==================================\n");
	printf("(New %s) \nstream %s\n"
			, m2m->ds_dev_name
			, (stream->streamon==1)?"start":"pause");
	printf("fps=%.2f \n", float_fps);
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264)
		printf("bps=%d\n", stream->rc->Targetbitrate);
	printf("bps==================================\n");

}
