#!/usr/bin/perl -w
use strict;
#use Spreadsheet::ParseExcel;
use Digest::MD5;
use Digest::CRC;
use Encode;
use Encode::Detect::Detector;

# get time
my $NOW_TIME;	
my @time_arr;
my $half_left;
my $half_right;
my $m_d;
my $h_m;
my $reg_time;

# data type
my $CHAR = "char";
my $UINT8 = "UINT8";
my $UINT16 = "UINT16";
my $UINT32 = "UINT32";
my $INT32 = "INT32";
my $CHAR_STR = "char";
my $U8_STR = "uint8_t";
my $U16_STR = "uint16_t";	
my $U32_STR = "uint32_t";
my $I32_STR = "int32_t";

#confiuration related
my $configuration_file = "Configuration.bin";               
my $configuration_header_file = "nvram.h";      
my $all_tot_size;
my $configuration_md5;
my $configuration_name = "configuration";
my $all_con_idex = 0;
my @all_config_struct_sizesum;  
my $zero_num="";
my $inter_ff="";

#u-env
my $envs_tack = "\0";
my $mem_excel = 0;
my @envs_args;
my @envs_cmds;

#Package
my $UBOOT_NAME = "uboot";
my $KERNEL_NAME = "kernel";
my $ROOTFS_R_NAME = "rootfs-r";
my $ROOTFS_RW_NAME = "rootfs-rw";
my $ENGINE_NAME = "engine";
my $ULOGO_NAME = "ulogo";
my $RESCUE_NAME = "rescue";

my $UBOOT_PZNAME = "UBOOT.bin";              
my $KERNEL_PZNAME = "KERNEL.bin";            
my $RESCUE_PZNAME = "RESCUE.bin";            
my $ROOTFS_R_PZNAME = "ROOTFS-R.bin";        
my $ROOTFS_RW_PZNAME = "ROOTFS-RW.bin";      
my $ENGINE_PZNAME = "ENGINE.bin";  
my $ULOGO_PZNAME = "ulogo.bin.d";            

my $spi_file_d = "spi_flash_layout.d";       
my $spi_type_d = "SF";
my $nand_file_d = "nand_flash_layout.d";     
my $nand_type_d = "NAND";

my $NAND_BLK_SIZE = 0xFF4;
my $SPI_BLK_SIZE = 0xFF4;

#parameter from Makefile
my $flash_type;
my $uboot;
my $u_logo;
my $flash_info;
my $pll_setting;
my $hw_setting;
my $flash_layout;
my $kernel;
my $rescue;
my $rootfs_r;
my $rootfs_rw;
my $engine;
my $dotconfig;
my $image;
my $platform;
my $done_uboot_file;
my $done_kernel_file;
my $done_rootfsr_file;
my $done_nvram_file;
my $done_rescue_file;
my $rescue_enable;
my $isp_path;
my $sdk_version = "8888";
my $ddr_freq;
my $ddr_project = "";
my $ddr_df00_str = "";
my $ddr_df01_str = "";
my $ddr_df02_str = "";
my $config_path;
my $hw_version="";
my $st58660_type = "n";
my $sn98660_type = "n";
my $uboot_update = "y";
my $kernel_update = "y";
my $rootfs_update = "y";
my $jffs2_update = "y";
my $rescue_update = "y";
my $etc_update = "y";
my $user_update = "y";
my $factory_update = "y";
my $custom1_update = "y";
my $custom2_update = "y";

#uexce related
my $uexce_command ="./src/code/image_tool";
my $header_file = "./src/header/header.bin"; 
my $uboot_file = "./uexce.bin.d";
my $u_boot_env_file = "u-boot-env.bin.d";
my $flash_layout_header = "flash_layout.h";
my $flash_layout_file = "flash_layout.bin.d";

my $hw_setting_image = "hw_setting.image.d";
my $header_file_pk = "header.bin.d";
my $HW_SETTING_PK_SIZE = 0xC00;
my $HEADER_FILE_PK_SIZE = 0x10000;
my $EXCUE_STAR_ADDR = 0x01000000;
my $LDIMG_STAR_ADDR = 0x00fffffc;
my $IMAGE_TABLE_SIZE = 0x200;
my $IMAGE_TABLE_EXC_SIZE = 0xDC;
my $SF_BLOCK_SIZE = 0x10000;
my $SF_SECTOR_SIZE = 0x1000;

my $u_boot_image_d = "u_boot.image.d";
my $u_boot_env_bin_d = "u-boot-env.bin.d";

my $nvram_bin = "nvram.bin";

my $hw_setting_str = 0;
my $hw_setting_end = 0;
my $rootfs_info_str = 0;
my $rootfs_info_end = 0;
my $jffs2_info_str = 0;
my $jffs2_info_end = 0;
my $u_boot_str = 0;
my $u_boot_end = 0;
my $u_env_str = 0;
my $u_env_end = 0;
my $flash_layout_str = 0;
my $flash_layout_end = 0;
my $factory_str = 0;
my $factory_end = 0;
my $kernel_str = 0;
my $kernel_end = 0;
my $rootfs_r_str = 0;
my $rootfs_r_end = 0;
my $rootfs_jffs2_str = 0;
my $rootfs_jffs2_end = 0;
my $rescue_str = 0;
my $rescue_end = 0;
my $rootfs_rw_str = 0;
my $rootfs_rw_end = 0;
my $user_str = 0;
my $user_end = 0;
my $custom1_str = 0;
my $custom1_end = 0;
my $custom2_str = 0;
my $custom2_end = 0;
my $rootfs_jffs2_path = "null";
my $custom1_path = "null";
my $custom2_path = "null";

#FIRMWARE.bin
my $layoutfile_fir = "flash_layout_firmware.bin";
my $layoutfile_fir_size = 1024;
my $firmware_file = "FIRMWARE.bin";
my $firmware_factory_file = "FIRMWARE_F.bin";
my $FIRMWARE_NAME = "firmware";
my $FIRMWARE_FACTORY_NAME = "firmware_f";

#U-ENV
my $U_BOOT_ENV_NAME = "u-boot-env";
my $env_file = "u-boot-";

#flash_layout related
my $flash_layout_file_size = 512;
my @flash_nand = qw/flash-info-1-10 hw_setting-1 u-boot-1 reserve-1 u-env-1 flash-layout-1 
	                   hw_setting-2 u-boot-2 reserve-2 u-env-2 flash-layout-2 factory-1 kernel 
	                   rootfs-r rootfs-rw user-1 reserved-for-user u-logo rescue user-2 factory-2 
	                   user-3 factory-3 hw_setting-3 u-boot-3 reserve-3 u-env-3 flash-layout-3 bbt add/;
my @flash_spi =qw/hw_setting rootfs_info jffs2_info u_boot u_env flash_layout factory kernel rootfs_r fs_jffs2 rescue rootfs_rw
				user u_logo custom1 custom2
	                add/;

#calculate crc16
my @crc16_tab=(
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
);

###deal with time###
$NOW_TIME = &gettime;
@time_arr = split(/\//, $NOW_TIME);

$half_left=substr($time_arr[1],0,2);
$half_right=substr($time_arr[1],2,2);
$m_d=$half_left."-".$half_right;

$half_left=substr($time_arr[2],0,2);
$half_right=substr($time_arr[2],2,2);
$h_m=$half_left.":".$half_right;

$reg_time=$time_arr[0]."-".$m_d." ".$h_m;
$NOW_TIME=$reg_time;

###abstract @ARGV###
foreach (@ARGV){
	
	if (/^dotconfig=.*/){
		s/.*=//g;
		$dotconfig = $_;
	}
	elsif (/^image=.*/){
		s/.*=//g;
		$image = $_;
	}
	elsif(/^flash_type=.*/){
		s/.*=//g;
		$flash_type = $_;
	}
	elsif(/^st58660_type=.*/){
		s/.*=//g;
		$st58660_type = $_;
	}
	elsif(/^sn98660_type=.*/){
		s/.*=//g;
		$sn98660_type = $_;
	}
	elsif(/^uboot=.*/){
		s/.*=//g;
		$uboot = $_;
	}
	elsif(/^u_logo=.*/){
		s/.*=//g;
		$u_logo = $_;
	}
	elsif(/^config_path=.*/){
		s/.*=//g;
		s/\/$//;
		$config_path = $_;
	}
	elsif(/^rescue=.*/){
		s/.*=//g;
		$rescue= $_;
	}	
	elsif(/^kernel=.*/){
		s/.*=//g;
		$kernel= $_;
	}	
	elsif(/^rootfs_r=.*/){
		s/.*=//g;
		$rootfs_r= $_;
	}
	elsif(/^rootfs_rw=.*/){
		s/.*=//g;
		$rootfs_rw= $_;
	}	
	elsif(/^engine=.*/){
		s/.*=//g;
		$engine= $_;
	}
	elsif(/^platform=.*/)	{
		s/.*=//g;
		$platform= $_;
	}
	elsif(/^hw_version=.*/)	{
		s/.*=//g;
		$hw_version= $_;
	}
	elsif (/^done_uboot_file=.*/){
	  s/.*=//g;
		$done_uboot_file= $_;
	}
	elsif(/^done_kernel_file=.*/){
		s/.*=//g;
		$done_kernel_file= $_;
	}
	elsif(/^done_rootfsr_file=.*/){
		s/.*=//g;
		$done_rootfsr_file= $_;
	}
	elsif(/^done_rescue_file=.*/){
		s/.*=//g;
		$done_rescue_file= $_;
	}
	elsif(/^done_nvram_file=.*/){
		s/.*=//g;
		$done_nvram_file= $_;
	}
	elsif(/^isp_path=.*/){
		s/.*=//g;
		s/\/$//;
		$isp_path= $_;
	}
	elsif(/^sdk_version=.*/){
		s/.*=//g;
		$sdk_version= $_;
	}
	elsif(/^ddr_freq=.*/){
		s/.*=//g;
		$ddr_freq= $_;
	}
	elsif(/^rescue_system=.*/){
		s/.*=//g;
		$rescue_enable= $_;
	}
	elsif(/^ddr_project=.*/){
		s/.*=//g;
		$ddr_project= $_;
	}
	elsif(/^ddr0_ddr_str=.*/){
		s/.*=//g;
		$ddr_df00_str= $_;
	}
	elsif(/^ddr1_ddr_str=.*/){
		s/.*=//g;
		$ddr_df01_str= $_;
	}
	elsif(/^ddr2_ddr_str=.*/){
		s/.*=//g;
		$ddr_df02_str= $_;
	}
	elsif(/^uboot_update=.*/){
		s/.*=//g;
		$uboot_update= $_;
	}
	elsif(/^kernel_update=.*/){
		s/.*=//g;
		$kernel_update= $_;
	}
	elsif(/^rootfs_update=.*/){
		s/.*=//g;
		$rootfs_update= $_;
	}
	elsif(/^jffs2_update=.*/){
		s/.*=//g;
		$jffs2_update= $_;
	}
	elsif(/^rescue_update=.*/){
		s/.*=//g;
		$rescue_update= $_;
	}
	elsif(/^etc_update=.*/){
		s/.*=//g;
		$etc_update= $_;
	}
	elsif(/^user_update=.*/){
		s/.*=//g;
		$user_update= $_;
	}
	elsif(/^factory_update=.*/){
		s/.*=//g;
		$factory_update= $_;
	}
	elsif(/^custom1_update=.*/){
		s/.*=//g;
		$custom1_update= $_;
	}
	elsif(/^custom2_update=.*/){
		s/.*=//g;
		$custom2_update= $_;
	}
	else{
		print $_ , "\n";
		print "the image parameter is wrong!\n";
	}	
}
#####select flash_layout#######
if ($flash_type =~ /NAND/i){
 $flash_layout = "$config_path/flash-layout/nand_flashlayout_1Gb.conf";
}
elsif ($flash_type =~ /SF/i){
 $flash_layout = "$config_path/flash-layout/serial_flashlayout.conf";
}

if ($sn98660_type =~ /y/i) {
	$st58660_type="y";
}

unless (-e $flash_layout){
 print "\nError:	flash layout  file--->$flash_layout does not exist!\n\n";
 exit;
}

###VERSION###
my $UBOOT_VER = $sdk_version.$envs_tack ;
my $KERNEL_VER = $sdk_version.$envs_tack ;
my $ROOTFS_R_VER = $sdk_version.$envs_tack ;
my $ROOTFS_RW_VER = $sdk_version.$envs_tack ;
my $ENGINE_VER = $sdk_version.$envs_tack ;
my $ULOGO_VER = $sdk_version.$envs_tack ;
my $RESCUE_VER = $sdk_version.$envs_tack ;
my $FIRMWARE_VER = $sdk_version.$envs_tack ;
my $U_BOOT_ENV_VER = $sdk_version.$envs_tack ;
my $configuration_ver = $sdk_version.$envs_tack ;

################## abstract flash_layout data ##############
open(oFILE,$flash_layout) || die("Cannot Open File: $!");
my @flash_layout_raw = <oFILE>;
close(oFILE);
my @flash_layout_value;
foreach (@flash_layout_raw){
 unless((/^Flash-Type.*/)||(/^Content.*/)||(/^#.*/)){
  if(/(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/){
   push @flash_layout_value,$2;
   push @flash_layout_value,$3;
   push @flash_layout_value,$4;
  }
 }
}

my $version_size = length $FIRMWARE_VER;
if ($image =~ /firmware$/) {
	if ($version_size > 48) {
        	die ("version size is longer than 48bytes, now is $version_size\n");
	}
}

################## main body  ##############
if ($image =~ /uboot/) {
	&fun_uboot($flash_type,$uboot,$u_logo,$rescue,$flash_info,$pll_setting,$hw_setting,$flash_layout,$dotconfig,$hw_setting_image)
	}
elsif ($image =~ /rescue/) {
	&packize($rescue,$RESCUE_PZNAME,$RESCUE_NAME,$RESCUE_VER,$NOW_TIME,'S')
	}
elsif ($image =~ /kernel/) {
	&packize($kernel,$KERNEL_PZNAME,$KERNEL_NAME,$KERNEL_VER,$NOW_TIME,'S')
	}
elsif ($image =~ /^rootfs-r$/) {
	&packize($rootfs_r,$ROOTFS_R_PZNAME,$ROOTFS_R_NAME,$ROOTFS_R_VER,$NOW_TIME,'N')
	}
elsif ($image =~ /^rootfs-rw$/) {
	&packize($rootfs_rw,$ROOTFS_RW_PZNAME,$ROOTFS_RW_NAME,$ROOTFS_RW_VER,$NOW_TIME,'N')
	}
elsif ($image =~ /engine/) {
	# spi_flash_layout.d - SPI
	&fun_flash_layout_bin(\$spi_file_d, \$spi_type_d, \@flash_layout_value);
	# nand_flash_layout.d - NAND
	&fun_flash_layout_bin(\$nand_file_d, \$nand_type_d, \@flash_layout_value);
	# ./Image/u-boot.bin - ENGINE.bin
	&packize($engine,$ENGINE_PZNAME,$ENGINE_NAME,$ENGINE_VER,$NOW_TIME,'F',$spi_file_d,$nand_file_d);
	#system("mv ./$ENGINE_PZNAME ../Engine");	
	}
elsif ($image =~ /^firmware_factory$/) {
	generate_selfburn_firmware($hw_setting_image,$header_file,$u_boot_image_d,$u_boot_env_bin_d,$flash_layout_file,$done_nvram_file,$done_kernel_file,$done_rootfsr_file,$done_rescue_file,$FIRMWARE_FACTORY_NAME);
}
elsif ($image =~ /^firmware$/) {
	generate_selfburn_firmware($hw_setting_image,$header_file,$u_boot_image_d,$u_boot_env_bin_d,$flash_layout_file,$done_nvram_file,$done_kernel_file,$done_rootfsr_file,$done_rescue_file,$FIRMWARE_NAME);
}
else {print "the image parameter is wrong!\n"}

################## sub function ##############
#gen selfburn_firmware
sub generate_selfburn_firmware
{
	my $hw_setting_fir = $_[0];
	my $hw_setting_size = 0;
	my $header_file_fir = $_[1];
	my $header_file_size = 0;
	my $u_boot_fir = $_[2];
	my $u_boot_size = 0;
	my $u_env_fir = $_[3];
	my $u_env_size = 0;
	my $flash_layout_fir = $_[4];
	my $flash_layout_size = 0;
	my $nvram_fir = $_[5];
	my $nvram_size = 0;
	my $kernel_fir = $_[6];
	my $kernel_size = 0;
	my $rootfsr_fir = $_[7];
	my $rootfsr_size = 0;
	my $rootfs_jffs2_size = 0;
	my $rescue_fir = $_[8];
	my $rescue_size = 0;
	my $custom1_size = 0;
	my $custom2_size = 0;
	my $target_name = $_[9];

	my $target_file = "FIRMWARE_SB.bin";
	my $image_file = "image.d";
	my $config_file = "config.d";
	my $hw_setting_file = $hw_setting_fir;

	my @data_hw_setting = ();	
	my @data_config = ();	
	my @data_header_file = ();	
	my @data_u_boot = ();	
	my @data_u_env = ();	
	my @data_flash_layout = ();	
	my @data_rescue = ();	
	if ($target_name =~ /firmware_f/i) {
 		open(oFILE, $hw_setting_file) or die "Can't open '$hw_setting_file': $!";
	 	binmode(oFILE);
 		@data_hw_setting = <oFILE>;	
	 	close oFILE;
		$hw_setting_size = -s $hw_setting_file;
	
		my $firmware_id = $platform;
		if ($st58660_type eq "y") {
			$firmware_id = "st58660";
		}
		print "the firmware id = $firmware_id\n";
		$firmware_id =~ s/[^0-9]//g;

		my $fw_tag = sprintf "%08d", hex($firmware_id);
	

		if ($st58660_type eq "y") {
			my $muti_image = "muti.image.d";
			my @config_pad;
			my $config_size;
			my $config_pad_size;

			$config_size = -s $muti_image;
			$config_size = $config_size + 4 + 24; #crc + overwrite 2 command
			$config_pad_size  = (($config_size + 0x1F) & (~0x1F)) - $config_size;
			$config_size = $config_size + $config_pad_size;

			my @padd_config = ();	
		  	open(oFILE, $muti_image) or die "Can't open '$muti_image': $!";
  			binmode(oFILE);
	  		@padd_config = <oFILE>;	
	  		close oFILE;

		  	push @padd_config, pack('L', 0x00000077);	#overwrite load addr
  			push @padd_config, pack('L', 0xffff601c);
		  	push @padd_config, pack('L', 0x10fffffc);

  			push @padd_config, pack('L', 0x00000077);	#overwrite jmp addr
	  		push @padd_config, pack('L', 0xffff6090);
	  		push @padd_config, pack('L', 0x01000000);

			for (my $i=0;
				$i < $config_pad_size;
				$i++){
				push @padd_config, pack('H2', 'ff');		
			}

		  	unshift @padd_config, pack('L', $config_size);

 			unshift @padd_config, pack('L',$fw_tag);

  			open oFILE, ">" . $config_file or die "Can't open '$config_file': $!";
	 			print oFILE @padd_config;
			close oFILE;

			my $config_crc16 = 0;
			my @config_message;
			my $config_count=$config_size;
			my $config_index_message=0;


			open(GIF, $config_file) or die "can't open '$config_file': $!";
			binmode(GIF);
			while ($config_count > 0){
				$config_count--;
				read(GIF, $config_message[$config_index_message], 1);
				$config_message[$config_index_message] = unpack("C",$config_message[$config_index_message]);
				$config_index_message++;
			}
			close GIF;

			&crcSlow(
				\$config_crc16,
				\@crc16_tab, 
				\@config_message,       
				\$config_size
		 	);

			open oFILE, ">>" . $config_file or die "Can't open '$config_file': $!";
				print oFILE pack('L', $config_crc16);
			close oFILE;

			$config_size = -s $config_file;

			my $config_file_pad = $HW_SETTING_PK_SIZE - $config_size;

		 	open(oFILE, $config_file) or die "Can't open '$config_file': $!";
 			binmode(oFILE);
		 	@data_config = <oFILE>;	
 			close oFILE;

	 		open oFILE, ">>" . $config_file or die "Can't open '$config_file': $!";
	 		for (my $i=0;
				$i < $config_file_pad;
				$i++){
				push @data_config, pack('H2', 'ff');
				print oFILE pack('H2', 'ff');
			}
			close oFILE;
		} 
		else {
			# just same as firmware_f format
			push @data_config, pack('L',$fw_tag);
	 		for (my $i=0;
				$i < ($HW_SETTING_PK_SIZE - 4);
				$i++){
				push @data_config, pack('H2', 'ff');
			}
		}

#u_boot.image.d
		$u_boot_size = -s $u_boot_fir;

	 	open(oFILE, $u_boot_fir) or die "Can't open '$u_boot_fir': $!";
 		binmode(oFILE);
	 	# read file into an array
 		@data_u_boot = <oFILE>;
	 	close oFILE;

#u-boot-env.bin.d
		$u_env_size = -s $u_env_fir;

 		open(oFILE, $u_env_fir) or die "Can't open '$u_env_fir': $!";
	 	binmode(oFILE);
 		# read file into an array
	 	@data_u_env = <oFILE>;
 		close oFILE;

#flash_layout.bin.d
		$flash_layout_size = -s $flash_layout_fir;

	 	open(oFILE, $flash_layout_fir) or die "Can't open '$flash_layout_fir': $!";
 		binmode(oFILE);
	 	# read file into an array
 		@data_flash_layout = <oFILE>;
	 	close oFILE;

#RESCUE.bin
		if ($rescue_enable =~ /^y$/) {
			$rescue_size = -s $rescue_fir;
 			open(oFILE, $rescue_fir) or die "Can't open '$rescue_fir': $!";
		 	binmode(oFILE);
 			# read file into an array
	 		@data_rescue = <oFILE>;
	 		close oFILE;	
		}
	} #end firmware_f

#header.bin
	system("make -C ./src/header/ clean;make -C ./src/header/ FW_BURN_FLOW=yes");
	$header_file_size = -s $header_file_fir;

 	open(oFILE, $header_file_fir) or die "Can't open '$header_file_fir': $!";
	binmode(oFILE);
 	@data_header_file = <oFILE>;	
	close oFILE;

	if ($target_name =~ /firmware_f/i) {
		my $hdr_fl_pad = $HEADER_FILE_PK_SIZE - $header_file_size -4;
		for (my $i=0;
			$i < $hdr_fl_pad;
			$i++){
			push @data_header_file, pack('H2', '00');			
		}
	}

#nvram.bin 	
	my @data_nvram = ();	

#KERNEL.bin
	$kernel_size = -s $kernel_fir;

	my @data_kernel = ();	
 	open(oFILE, $kernel_fir) or die "Can't open '$kernel_fir': $!";
 	binmode(oFILE);
 	# read file into an array
 	@data_kernel = <oFILE>;
 	close oFILE;

#ROOTFS-R.bin
	$rootfsr_size = -s $rootfsr_fir;

	my @data_rootfsr = ();	
 	open(oFILE, $rootfsr_fir) or die "Can't open '$rootfsr_fir': $!";
 	binmode(oFILE);
 	# read file into an array
 	@data_rootfsr = <oFILE>;
 	close oFILE;

 	my @data_rootfsr_size_sector = ();
 	my $rootfsr_sector_pad = $SF_SECTOR_SIZE - 4;	
	if ($st58660_type eq "y") {
		push @data_rootfsr_size_sector, pack('L', $rootfsr_size);
	}
 	for (my $i=0;
		$i < $rootfsr_sector_pad;
		$i++){
		push @data_rootfsr_size_sector, pack('H2', 'ff');			
	}
	if ($st58660_type eq "y") {
	} else {
		push @data_rootfsr_size_sector, pack('L', $rootfsr_size);
	}

# rootfs jffs2
	my @data_rootfs_jffs2 = ();
 	my @data_rootfs_jffs2_size_sector = ();
	my $rootfs_jffs2_sector_pad = $SF_SECTOR_SIZE - 4;	

	my @data_custom1 = ();
	my @data_custom2 = ();
#IMAGE_TABLE
	my $flash_index = 0;
	foreach (@flash_spi)
	{

  		$_ =~s/-/_/g;

  		my $fl_str;
  		my @p32u_fl_str;
  		my $flash_value = 0;

		$fl_str = $flash_layout_value[$flash_index];
		@p32u_fl_str = split(/x/, $fl_str);
		$flash_value = sprintf "%08d", hex($p32u_fl_str[1]);

		$flash_index ++;

		if ($target_name =~ /firmware_f/i) {
			if ($_ =~ /hw_setting/i) {$hw_setting_str = $flash_value;}
			if ($_ =~ /u_boot/i) {$u_boot_str = $flash_value;}
			if ($_ =~ /u_env/i) {$u_env_str = $flash_value;}
			if ($_ =~ /flash_layout/i) {$flash_layout_str = $flash_value;}
			if ($_ =~ /user/i) {$user_str = $flash_value;}
			if ($rescue_enable =~ /^y$/) {
				if ($_ =~ /rescue/i) {$rescue_str = $flash_value;}
			}
			if ($_ =~ /rootfs_rw/i) {$rootfs_rw_str = $flash_value;}
		}
		if ($_ =~ /rootfs_info/i) {$rootfs_info_str = $flash_value;}
		if ($_ =~ /jffs2_info/i) {$jffs2_info_str = $flash_value;}
		if ($_ =~ /factory/i) {$factory_str = $flash_value;}
		if ($_ =~ /kernel/i) {$kernel_str = $flash_value;}
		#if ($_ =~ /rootfs_r/i) {$rootfs_r_str = $flash_value;}
		if ($_ eq "rootfs_r") {$rootfs_r_str = $flash_value;}
		if ($_ eq "fs_jffs2") {$rootfs_jffs2_str = $flash_value;}
		if ($_ eq "custom1") {$custom1_str = $flash_value;}
		if ($_ eq "custom2") {$custom2_str = $flash_value;}

		$fl_str = $flash_layout_value[$flash_index];
		@p32u_fl_str = split(/x/, $fl_str);
		$flash_value = sprintf "%08d", hex($p32u_fl_str[1]);

		$flash_index ++;

		my $fl_image_path = $flash_layout_value[$flash_index];
		$flash_index ++;

		if ($target_name =~ /firmware_f/i) {
			if ($_ =~ /hw_setting/i) {$hw_setting_end = $flash_value;}
			if ($_ =~ /u_boot/i) {$u_boot_end = $flash_value;}
			if ($_ =~ /u_env/i) {$u_env_end = $flash_value;}
			if ($_ =~ /flash_layout/i) {$flash_layout_end = $flash_value;}
			if ($rescue_enable =~ /^y$/) {
				if ($_ =~ /rescue/i) {
					$rescue_end = $flash_value;
					if (($rescue_end - $rescue_str) < $rescue_size) {die "ERROR please increase rescue partiton size, st=$rescue_str,ed=$rescue_end,size=$rescue_size\n";}
				}
			}
			if ($_ =~ /rootfs_rw/i) {$rootfs_rw_end = $flash_value;}
			if ($_ =~ /user/i) {
				$user_end = $flash_value;
				if ($user_end > $user_str) {
					if ($nvram_size > 0) {
						if (($user_end - $user_str) < $nvram_size) {die "ERROR please increase factory partiton size\n";}
					}
					else {
						$nvram_size = -s $fl_image_path;
						if (($user_end - $user_str) < $nvram_size) {die "ERROR please increase user partiton size\n";}
	 					open(oFILE, $fl_image_path) or die "Can't open '$fl_image_path': $!";
					 	binmode(oFILE);
				 		# read file into an array
					 	@data_nvram = <oFILE>;
					 	close oFILE;	
					}
				}
			}
		}
		if ($_ =~ /rootfs_info/i) {$rootfs_info_end = $flash_value;}
		if ($_ =~ /jffs2_info/i) {$jffs2_info_end = $flash_value;}
		if ($_ =~ /factory/i) {
			$factory_end = $flash_value;
			if ($factory_end > $factory_str) {
				if ($nvram_size > 0) {
					if (($factory_end - $factory_str) < $nvram_size) {die "ERROR please increase factory partiton size,st=$factory_str,ed=$factory_end,size=$nvram_size\n";}
				}
				else {
					$nvram_size = -s $fl_image_path;
					if (($factory_end - $factory_str) < $nvram_size) {die "ERROR please increase factory partiton size,1st=$factory_str,ed=$factory_end,size=$nvram_size\n";}
 					open(oFILE, $fl_image_path) or die "Can't open '$fl_image_path': $!";
				 	binmode(oFILE);
			 		# read file into an array
				 	@data_nvram = <oFILE>;
				 	close oFILE;	
				}
			}
		}
		if ($_ =~ /kernel/i) {
			$kernel_end = $flash_value;
			if (($kernel_end - $kernel_str) < $kernel_size) {die "ERROR please increase kernel partiton size\n";}	
		}
		#if ($_ =~ /rootfs_r/i) {$rootfs_r_end = $flash_value;}
		if ($_ eq "rootfs_r") {
			$rootfs_r_end = $flash_value;
			if (($rootfs_r_end - $rootfs_r_str) < $rootfsr_size) {die "ERROR please increase rootfsr partiton size\n";}
		}
		
		if ($_ eq "fs_jffs2") {
			$rootfs_jffs2_end = $flash_value;
			if ($rootfs_jffs2_end > $rootfs_jffs2_str) {
				$rootfs_jffs2_size = -s $fl_image_path;
				if (($rootfs_jffs2_end - $rootfs_jffs2_str) < $rootfs_jffs2_size) {die "ERROR please increase rootfs_jffs2  partiton size\n";}
				push @data_rootfs_jffs2_size_sector, pack('L', $rootfs_jffs2_size);

	 			open(oFILE, $fl_image_path) or die "Can't open '$fl_image_path': $!";
				binmode(oFILE);
				# read file into an array
				@data_rootfs_jffs2 = <oFILE>;
				close oFILE;	
			} 
			else {
				push @data_rootfs_jffs2_size_sector, pack('L', 0xffffffff);
			}
			for (my $i=0;
				$i < $rootfs_jffs2_sector_pad;
				$i++){
				push @data_rootfs_jffs2_size_sector, pack('H2', 'ff');			
			}
		}

		if ($_ eq "custom1") {
			$custom1_end = $flash_value;	
			if ($fl_image_path eq "null") {
				$custom1_size = 0;
			}
			elsif ($custom1_end > $custom1_str) {
				$custom1_size = -s $fl_image_path;
				if (($custom1_end - $custom1_str) < $custom1_size) {die "ERROR please increase custom1 partiton size\n";}
 				open(oFILE, $fl_image_path) or die "Can't open '$fl_image_path': $!";
			 	binmode(oFILE);
			 	# read file into an array
			 	@data_custom1 = <oFILE>;
			 	close oFILE;	
			}
		}
		if ($_ eq "custom2") {
			$custom2_end = $flash_value;	
			if ($fl_image_path eq "null") {
				$custom2_size = 0;
			}
			elsif ($custom2_end > $custom2_str) {
				$custom2_size = -s $fl_image_path;
				if (($custom2_end - $custom2_str) < $custom2_size) {die "ERROR please increase custom2 partiton size\n";}
 				open(oFILE, $fl_image_path) or die "Can't open '$fl_image_path': $!";
			 	binmode(oFILE);
			 	# read file into an array
			 	@data_custom2 = <oFILE>;
			 	close oFILE;	
			}
		}
	}

	my @data_image_table = ();
	my $resval = 0x12345678;
	my $image_star_addr = 0;

	$IMAGE_TABLE_EXC_SIZE = 0;
	if ($target_name =~ /firmware_f/i) {
		if ($uboot_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x50;}
		if ($kernel_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		if ($rootfs_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x28;}
		if ($etc_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		if ($rescue_enable =~ /^y$/) {
			if ($rescue_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
		if ($factory_end > $factory_str) {
			if ($factory_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
		if ($user_end > $user_str)  {
			if ($user_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
		if ($rootfs_jffs2_end > $rootfs_jffs2_str) {
			if ($st58660_type eq "y") {
				if ($jffs2_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x28;}
			} else {
				if ($jffs2_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
			}
		}
		if ($custom1_size > 0) {
			if ($custom1_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
		if ($custom2_size > 0) {
			if ($custom2_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
	} else {
		$image_star_addr = $header_file_size;
		if ($kernel_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		if ($rootfs_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x28;}
		if ($factory_end > $factory_str) {
			if ($factory_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
		if ($rootfs_jffs2_end > $rootfs_jffs2_str) {
			if ($st58660_type eq "y") {
				if ($jffs2_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x28;}
			} else {
				if ($jffs2_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
			}
		}
		if ($custom1_size > 0) {
			if ($custom1_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
		if ($custom2_size > 0) {
			if ($custom2_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
		}
	}

	push @data_image_table, pack('L', $IMAGE_TABLE_EXC_SIZE);

	if ($target_name =~ /firmware_f/i) {
		if ($uboot_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			#push @data_image_table, pack('L', $hw_setting_size);
			if ($st58660_type eq "y") {
				push @data_image_table, pack('L', $HW_SETTING_PK_SIZE);
			} else {
				push @data_image_table, pack('L', $hw_setting_size);
			}
			push @data_image_table, pack('L', $hw_setting_str);
			push @data_image_table, pack('L', $hw_setting_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "HW_SETTING_PK_SIZE = %x\n" , $HW_SETTING_PK_SIZE;
			printf "hw_setting_str = %x\n" , $hw_setting_str;
			printf "hw_setting_end = %x\n" , $hw_setting_end;
			printf "---------------------------------\n";

			#$image_star_addr = $image_star_addr + $hw_setting_size + 4;
			if ($st58660_type eq "y") {
				$image_star_addr = $image_star_addr + $HW_SETTING_PK_SIZE;
			} else {
				$image_star_addr = $image_star_addr + $hw_setting_size;
			}
		}
	}
	
	if ($st58660_type eq "y") {	
		if ($rootfs_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $SF_SECTOR_SIZE);
			push @data_image_table, pack('L', $rootfs_info_str);
			push @data_image_table, pack('L', $rootfs_info_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "rootfs_info_size = %x\n" , $SF_SECTOR_SIZE;
			printf "rootfs_info_str = %x\n" , $rootfs_info_str;
			printf "rootfs_info_end = %x\n" , $rootfs_info_end;
			printf "---------------------------------\n";
	
			$image_star_addr = $image_star_addr + $SF_SECTOR_SIZE;
		}

		if ($jffs2_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $SF_SECTOR_SIZE);
			push @data_image_table, pack('L', $jffs2_info_str);
			push @data_image_table, pack('L', $jffs2_info_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "jffs2_info_size = %x\n" , $SF_SECTOR_SIZE;
			printf "jffs2_info_str = %x\n" , $jffs2_info_str;
			printf "jffs2_info_end = %x\n" , $jffs2_info_end;
			printf "---------------------------------\n";
	
			$image_star_addr = $image_star_addr + $SF_SECTOR_SIZE;
		}
	}

	if ($target_name =~ /firmware_f/i) {
		if ($uboot_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $u_boot_size);
			push @data_image_table, pack('L', $u_boot_str);
			push @data_image_table, pack('L', $u_boot_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "u_boot_size = %x\n" , $u_boot_size;
			printf "u_boot_str = %x\n" , $u_boot_str;
			printf "u_boot_end = %x\n" , $u_boot_end;
			printf "---------------------------------\n";

			$image_star_addr = $image_star_addr + $u_boot_size;	
	
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $u_env_size);
			push @data_image_table, pack('L', $u_env_str);
			push @data_image_table, pack('L', $u_env_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "u_env_size = %x\n" , $u_env_size;
			printf "u_env_str = %x\n" , $u_env_str;
			printf "u_env_end = %x\n" , $u_env_end;
			printf "---------------------------------\n";

			$image_star_addr = $image_star_addr + $u_env_size;

			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $flash_layout_size);
			push @data_image_table, pack('L', $flash_layout_str);
			push @data_image_table, pack('L', $flash_layout_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "flash_layout_size = %x\n" , $flash_layout_size;
			printf "flash_layout_str = %x\n" , $flash_layout_str;
			printf "flash_layout_end = %x\n" , $flash_layout_end;
			printf "---------------------------------\n";

			$image_star_addr = $image_star_addr + $flash_layout_size;
		}
	}
	
	if ($factory_end > $factory_str) {
		if ($factory_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $nvram_size);
			push @data_image_table, pack('L', $factory_str);
			push @data_image_table, pack('L', $factory_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "nvram_size = %x\n" , $nvram_size;
			printf "factory_str = %x\n" , $factory_str;
			printf "factory_end = %x\n" , $factory_end;
			printf "---------------------------------\n";
		}
	}

	if ($target_name =~ /firmware_f/i) {
		if ($user_end > $user_str) {
			if ($user_update eq "y") {
				push @data_image_table, pack('L', $resval);
				push @data_image_table, pack('L', $image_star_addr);
				push @data_image_table, pack('L', $nvram_size);
				push @data_image_table, pack('L', $user_str);
				push @data_image_table, pack('L', $user_end);

				printf "imstr = %x\n" , $image_star_addr;
				printf "nvram_size = %x\n" , $nvram_size;
				printf "user_str = %x\n" , $user_str;
				printf "user_end = %x\n" , $user_end;
				printf "---------------------------------\n";
				$image_star_addr = $image_star_addr + $nvram_size;
			}
			elsif ($factory_end > $factory_str) {
				if ($factory_update eq "y") {
					$image_star_addr = $image_star_addr + $nvram_size;
				}
			}
		}
		elsif ($factory_end > $factory_str) {
			if ($factory_update eq "y") {
				$image_star_addr = $image_star_addr + $nvram_size;
			}
		}
	} 
	elsif ($factory_end > $factory_str) {
		if ($factory_update eq "y") {
			$image_star_addr = $image_star_addr + $nvram_size;
		}
	}

	if ($kernel_update eq "y") {
		push @data_image_table, pack('L', $resval);
		push @data_image_table, pack('L', $image_star_addr);
		push @data_image_table, pack('L', $kernel_size);
		push @data_image_table, pack('L', $kernel_str);
		push @data_image_table, pack('L', $kernel_end);

		printf "imstr = %x\n" , $image_star_addr;
		printf "kernel_size = %x\n" , $kernel_size;
		printf "kernel_str = %x\n" , $kernel_str;
		printf "kernel_end = %x\n" , $kernel_end;
		printf "---------------------------------\n";

		$image_star_addr = $image_star_addr + $kernel_size;
	}

	if ($rootfs_update eq "y") {
		push @data_image_table, pack('L', $resval);
		push @data_image_table, pack('L', $image_star_addr);
		push @data_image_table, pack('L', $rootfsr_size);
		push @data_image_table, pack('L', $rootfs_r_str);
		if ($st58660_type eq "y") {
			push @data_image_table, pack('L', $rootfs_r_end);
		} else {
			push @data_image_table, pack('L', $rootfs_r_end - $SF_SECTOR_SIZE);
		}

		printf "imstr = %x\n" , $image_star_addr;
		printf "rootfsr_size = %x\n" , $rootfsr_size;
		printf "rootfs_r_str = %x\n" , $rootfs_r_str;
		printf "rootfs_r_end = %x\n" , $rootfs_r_end;
		printf "---------------------------------\n";

		$image_star_addr = $image_star_addr + $rootfsr_size;

		if ($st58660_type eq "y") {
		} else {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $SF_SECTOR_SIZE);
			push @data_image_table, pack('L', $rootfs_r_end - $SF_SECTOR_SIZE + 1);
			push @data_image_table, pack('L', $rootfs_r_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "rootfs_info_size = %x\n" , $SF_SECTOR_SIZE;
			printf "rootfs_info_str = %x\n" , $rootfs_info_str;
			printf "rootfs_info_end = %x\n" , $rootfs_info_end;
			printf "---------------------------------\n";
	
			$image_star_addr = $image_star_addr + $SF_SECTOR_SIZE;
		}
	}

	if ($target_name =~ /firmware_f/i) {
		my $etc_size = 0x00000000;
		my $etc_image_addr = 0x00000000;

		if ($etc_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $etc_image_addr);
			push @data_image_table, pack('L', $etc_size);
			push @data_image_table, pack('L', $rootfs_rw_str);
			push @data_image_table, pack('L', $rootfs_rw_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "etc_size = %x\n" , $etc_size;
			printf "rootfs_rw_str = %x\n" , $rootfs_rw_str;
			printf "rootfs_rw_end = %x\n" , $rootfs_rw_end;
			printf "---------------------------------\n";
		}
	}

	if ($rootfs_jffs2_end > $rootfs_jffs2_str) {
		if ($jffs2_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $rootfs_jffs2_size);
			push @data_image_table, pack('L', $rootfs_jffs2_str);
			push @data_image_table, pack('L', $rootfs_jffs2_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "rootfs_jffs2_size = %x\n" , $rootfs_jffs2_size;
			printf "rootfs_jffs2_str = %x\n" , $rootfs_jffs2_str;
			printf "rootfs_jffs2_end = %x\n" , $rootfs_jffs2_end;
			printf "---------------------------------\n";

			$image_star_addr = $image_star_addr + $rootfs_jffs2_size;
		}
	}

	if ($target_name =~ /firmware_f/i) {
		if ($rescue_enable =~ /^y$/) {
			if ($rescue_update eq "y") {
				push @data_image_table, pack('L', $resval);
				push @data_image_table, pack('L', $image_star_addr);
				push @data_image_table, pack('L', $rescue_size);
				push @data_image_table, pack('L', $rescue_str);
				push @data_image_table, pack('L', $rescue_end);

				printf "imstr = %x\n" , $image_star_addr;
				printf "rescue_size = %x\n" , $rescue_size;
				printf "rescue_str = %x\n" , $rescue_str;
				printf "rescue_end = %x\n" , $rescue_end;
				printf "---------------------------------\n";

				$image_star_addr = $image_star_addr + $rescue_size;
			}
		}
	}

	if ($custom1_size > 0) {
		if ($custom1_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $custom1_size);
			push @data_image_table, pack('L', $custom1_str);
			push @data_image_table, pack('L', $custom1_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "custom1_size = %x\n" , $custom1_size;
			printf "custom1_str = %x\n" , $custom1_str;
			printf "custom1_end = %x\n" , $custom1_end;
			printf "---------------------------------\n";

			$image_star_addr = $image_star_addr + $custom1_size;
		}
	}

	if ($custom2_size > 0) {
		if ($custom2_update eq "y") {
			push @data_image_table, pack('L', $resval);
			push @data_image_table, pack('L', $image_star_addr);
			push @data_image_table, pack('L', $custom2_size);
			push @data_image_table, pack('L', $custom2_str);
			push @data_image_table, pack('L', $custom2_end);

			printf "imstr = %x\n" , $image_star_addr;
			printf "custom2_size = %x\n" , $custom2_size;
			printf "custom2_str = %x\n" , $custom2_str;
			printf "custom2_end = %x\n" , $custom2_end;
			printf "---------------------------------\n";

			$image_star_addr = $image_star_addr + $custom2_size;
		}
	}

	for (my $i=0;
		$i < ($IMAGE_TABLE_SIZE - $IMAGE_TABLE_EXC_SIZE -4);
		$i++){
		push @data_image_table, pack('H2', '00');			
	}


#############	CRC16 IMAGE ##########
	my @image_pad;
	my $image_size;
	my $image_pad_size;
	my $firmware_f_md5_file = "firmware_md5_file.bin";

	open oFILE, ">" . $image_file or die "Can't open '$image_file': $!";
	if ($target_name =~ /firmware_f/i) {
		print oFILE @data_header_file;
	}
	close oFILE;

	open oFILE, ">" . $firmware_f_md5_file or die "Can't open '$firmware_f_md5_file': $!";
		print oFILE @data_image_table;
	if ($target_name =~ /firmware_f/i) {
		if ($uboot_update eq "y") {print oFILE @data_hw_setting;}
	} else {
		print oFILE @data_header_file;
	}

	if ($st58660_type eq "y") {
		if ($rootfs_update eq "y") {print oFILE @data_rootfsr_size_sector;}
		if ($jffs2_update eq "y") {print oFILE @data_rootfs_jffs2_size_sector;}	
	}
	if ($target_name =~ /firmware_f/i) {
		if ($uboot_update eq "y") {
			print oFILE @data_u_boot;
			print oFILE @data_u_env;
			print oFILE @data_flash_layout;
		}
	}

	my $fill_nvram_data="n";
	if ($factory_end > $factory_str) {
		if ($factory_update eq "y") {print oFILE @data_nvram; $fill_nvram_data="y";}
		
	}
	if ($target_name =~ /firmware_f/i) {
		if ($user_end > $user_str) {
			if ($user_update eq "y") {
				if ($fill_nvram_data eq "n") {print oFILE @data_nvram;}
			}
		}
	}
		if ($kernel_update eq "y") {print oFILE @data_kernel;}
		if ($rootfs_update eq "y") {print oFILE @data_rootfsr;}
	if ($st58660_type eq "y") {
	} else {
		if ($rootfs_update eq "y") {print oFILE @data_rootfsr_size_sector;}
	}
	if ($rootfs_jffs2_end > $rootfs_jffs2_str) {
		if ($jffs2_update eq "y") {print oFILE @data_rootfs_jffs2;}
	}
	if ($target_name =~ /firmware_f/i) {
		if ($rescue_enable =~ /^y$/) {
			if ($rescue_update eq "y") {print oFILE @data_rescue;}
		}
	}
	if ($custom1_end > $custom1_str) {
		if ($custom1_update eq "y") {print oFILE @data_custom1;}
	}
	if ($custom2_end > $custom2_str) {
		if ($custom2_update eq "y") {print oFILE @data_custom2;}
	}
	
	close oFILE;

	$image_size = -s $firmware_f_md5_file;
	$image_size = $image_size - 4;
	$image_pad_size  = (($image_size + 0x1F) & (~0x1F)) - $image_size;
	$image_size = $image_size + $image_pad_size;

	for (my $i=0;
		$i < $image_pad_size;
		$i++){
		push @image_pad, pack('H2', 'ff');		
	}

	open oFILE, ">>" . $firmware_f_md5_file or die "Can't open '$firmware_f_md5_file': $!";
		print oFILE @image_pad;
	close oFILE;
	
		open(oFILE, $firmware_f_md5_file) or die "Can't open '$firmware_f_md5_file': $!"; 
		my $file_md5 = Digest::MD5->new->addfile(*oFILE)->hexdigest;	
		print "generate md5  $firmware_f_md5_file : $file_md5\n";         
		close oFILE; 

		my $md5_encrypt_file = "md5_encrypt.bin";
		system("make -C ./src/md5_encrypt/ clean"); 
		system("make -C ./src/md5_encrypt/"); 
		system("./src/md5_encrypt/md5_encrypt $file_md5 $md5_encrypt_file"); 
		my $size_md5_file = -s $md5_encrypt_file; 
		print "generate md5  file size :$size_md5_file\n";         

		open(oFILE, $md5_encrypt_file) or die "Can't open '$md5_encrypt_file': $!"; 
		binmode(oFILE); 
		my @data_md5_encrypt = <oFILE>; 
		close oFILE; 

		open(oFILE, $firmware_f_md5_file) or die "Can't open '$firmware_f_md5_file': $!"; 
		binmode(oFILE); 
		my @image_data = <oFILE>; 
		close oFILE; 
	
		open oFILE, ">>" . $image_file or die "Can't open '$image_file': $!";
			print oFILE @image_data;
		close oFILE;

		system ("rm -rf $firmware_f_md5_file");
	
	$image_size = -s $image_file;
	my $image_crc16 = 0;
	if ($target_name =~ /firmware_f/i) {
		my @message;
		my $count=$image_size;
		my $index_message=0;

		open(GIF, $image_file) or die "can't open '$image_file': $!";
		binmode(GIF);
		while ($count > 0){
			$count--;
			read(GIF, $message[$index_message], 1);
			$message[$index_message] = unpack("C",$message[$index_message]);
			$index_message++;
		}
		close GIF;

		&crcSlow(
			\$image_crc16,
			\@crc16_tab, 
			\@message,       
			\$image_size
	 	);

		open oFILE, ">>" . $image_file or die "Can't open '$image_file': $!";
			print oFILE pack('L', $image_crc16);
		close oFILE;

		$image_size = -s $image_file;
		$image_size = $image_size;


		my @data_img_tmp = ();	
	  	open(oFILE, $image_file) or die "Can't open '$image_file': $!";
  		binmode(oFILE);
	  	# read file into an array
  		@data_img_tmp = <oFILE>;	
	  	close oFILE;

  		unshift @data_img_tmp, pack('L', $image_size);

	  	open oFILE, ">" . $image_file or die "Can't open '$image_file': $!";
  		print oFILE @data_img_tmp;
	  	close oFILE;

		open oFILE, ">>" . $image_file or die "Can't open '$image_file': $!";
			print oFILE @data_md5_encrypt;
		close oFILE;
	}

	my @data_image = ();	
 	open(oFILE, $image_file) or die "Can't open '$image_file': $!";
 	binmode(oFILE);
 	# read file into an array
 	@data_image = <oFILE>;
 	close oFILE;	

	print "#####img size=";
	print $image_size;
	print "\n";

	print "#####img pad size=";
	print $image_pad_size;
	print "\n";

	print "#####img crc16=";
	print $image_crc16;
	print "\n";

###############################################################################
	open oFILE, ">" . $target_file or die "Can't open '$target_file': $!";

#CONFIG
	if ($target_name =~ /firmware_f/i) {
 		print oFILE @data_config;
	}
#IMAGE
	print oFILE @data_image;

	close oFILE;
	
	if ($target_name eq "firmware") {
		system("make -C ./src/fw_update/ clean;cp $target_file ./src/fw_update/FIRMWARE.bin;make -C ./src/fw_update/");
		system("cp -f ./src/fw_update/firmware_update $target_file");

		my $target_file_size = -s $target_file;
		my $target_pad_size  = (($target_file_size + 0xf) & (~0xf)) - $target_file_size;

		open oFILE, ">>" . $target_file or die "Can't open '$target_file': $!";
		for (my $i=0;
			$i < $target_pad_size;
			$i++){
			print oFILE pack('H2', 'ff');		
		}
		close oFILE; 

	#read firmware data
		open(oFILE, $target_file) or die "Can't open '$target_file': $!";
		binmode(oFILE);
		my @data_firmware = <oFILE>;
		close oFILE;

	#
		open oFILE, ">" . $target_file or die "Can't open '$target_file': $!";
		print oFILE @data_firmware;
	#name
		my $str = pack( "a16", $target_name);
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
	#version
		$str = pack("a64", $FIRMWARE_VER);
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
	#time
		$str = pack("a16", $NOW_TIME);
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
	#crc16
		my $add_str = sprintf("%s", "FF");
		my $time_add = 16;
		while ($time_add){
			$time_add --;
			my $str_done = pack("a2", $add_str);
			print oFILE pack('H*',$str_done);
		}
		close oFILE;
	
	# generate md5 [16bytes]  
		open(oFILE, $target_file) or die "Can't open '$target_file': $!"; 
		my $file_md5 = Digest::MD5->new->addfile(*oFILE)->hexdigest;	
		print "generate md5  $target_file : $file_md5\n";         
		close oFILE; 

		my $md5_encrypt_file = "md5_encrypt.bin";
		system("make -C ./src/md5_encrypt/ clean"); 
		system("make -C ./src/md5_encrypt/"); 
		system("./src/md5_encrypt/md5_encrypt $file_md5 $md5_encrypt_file"); 
		my $size_md5_file = -s $md5_encrypt_file; 
		print "generate md5  file size :$size_md5_file\n";         

		open(oFILE, $md5_encrypt_file) or die "Can't open '$md5_encrypt_file': $!"; 
		binmode(oFILE); 
		my @data_md5_encrypt = <oFILE>; 
		close oFILE; 

	#open oFILE, ">>" . $target_file or die "Can't open '$target_file': $!";
	#print oFILE @data_md5_encrypt;
	#close oFILE; 

	
		open oFILE, ">" . $target_file or die "Can't open '$target_file': $!";
		print oFILE @data_firmware;
	#name
		$str = pack( "a16", $target_name);
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
	#version
		$str = pack("a48", $FIRMWARE_VER);
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}

		print oFILE @data_md5_encrypt;
	#time
		$str = pack("a16", $NOW_TIME);
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
	#crc16
		$add_str = sprintf("%s", "FF");
		$time_add = 16;
		while ($time_add){
			$time_add --;
			my $str_done = pack("a2", $add_str);
			print oFILE pack('H*',$str_done);
		}
		close oFILE;
	}
	print "\ngenerate self-burn firmware(_f).bin end\n";
}

sub fun_uboot
{		
	my $flash_type_para = $_[0];
	my $uboot_para = $_[1];
	my $u_logo_para = $_[2];
	my $flash_layout_para = $_[7];
	my $pz_ufile = "./uboot.d";
	my $config = $_[8];

############################# Config processing  ################################
	my $hw_setting_af_pll = "$config_path/template/hw_setting_af_pll.txt";
	my $hw_setting_af_ddr = "$config_path/template/hw_setting_af_ddr_asic.txt";
	if ($st58660_type eq "y") {
		$hw_setting_af_pll = "$config_path/template/hw_setting_af_pll_98660asic.txt";
		$hw_setting_af_ddr = "$config_path/template/hw_setting_af_ddr_98660asic.txt";
	}
	my $flashinfo_file = "$config_path/template/flash_info_nl.txt";
	my $ddr_config_file = "";
	my $ddr_config_58660_main = "";
	my $ddr_config_58660_df00 = "";
	my $ddr_config_58660_df01 = "";
	my $ddr_config_58660_df02 = "";
	my $ddr_config_58660_end = "";
	my $hw_file;
	if ($st58660_type eq "y") {
		$hw_file = "sn9866x";
	} else {
		$hw_file = "sn986xx";
	}
	$ddr_config_file = "$config_path/hw-setting/".$hw_file."/";
	my $hw_template_file = "$config_path/template/hw_setting_af_pll_asic.txt";
	if ($st58660_type eq "y") {
		$hw_template_file = "$config_path/template/hw_setting_af_pll_98660asic.txt";
	}
	my $flash_layout_header_file = "$config_path/flash-layout/";

	my $flashtype = "";
	my $rescue_mode = "none";

	my $ramboot_file;
	my $burn_file;

	my $project = "";
	my $ptye = "full";
	
	my $ddr_project_para = "";
	my $bits = "";
	my $odt = "_2.5_BL4_no_ODT";
	
	my $mem_kconfig = "";
	
	if ($ddr_project =~ /([a-zA-Z0-9]+)_([0-9a-zA-Z]+)/){
		$ddr_project_para = $1;
		my $mem_bit = $2;
		if ($mem_bit =~ /([^x]+)x([^x]+)/){
			$mem_kconfig = $1;
			$bits = $2;
		}
	}

	#################platform#################
	if(($platform =~ /sn98600/) || ($platform =~ /sn98601/)){
  		my $hw_which_file = "sn9860x";
  		unless ($hw_version =~ /none/){
  			$hw_which_file = $hw_which_file."_".$hw_version;
  		}
			$ddr_config_file = $ddr_config_file.$hw_which_file."/";
			$project = $ddr_project_para;
			$ptye = "half";
	}
#	elsif (($platform =~ /sn93560/) || ($platform =~ /sn93561/)){
#  		my $hw_which_file = "sn9356x";
#  		unless ($hw_version =~ /none/){
#  			$hw_which_file = $hw_which_file."_".$hw_version;
#  		}
#			$ddr_config_file = $ddr_config_file.$hw_which_file."/";
#			$project = $ddr_project_para;
#			$ptye = "half";
#	}
	elsif (($platform =~ /sn98605/)){
  		my $hw_which_file = "sn98605";
  		unless ($hw_version =~ /none/){
  			$hw_which_file = $hw_which_file."_".$hw_version;
  		}
			$ddr_config_file = $ddr_config_file.$hw_which_file."/";
			$project = $ddr_project_para;
			$ptye = "half";
	}
	elsif (($platform =~ /sn98610/)){
  		my $hw_which_file = "sn98610";
  		unless ($hw_version =~ /none/){
  			$hw_which_file = $hw_which_file."_".$hw_version;
  		}
			$ddr_config_file = $ddr_config_file.$hw_which_file."/";
			$project = $ddr_project_para;
			$ptye = "half";
	}
	
	if($platform =~ /sn98660/){
  		my $hw_which_file = "sn98660";
  		unless ($hw_version =~ /none/){
  			$hw_which_file = $hw_which_file."_".$hw_version;
  		}
			$ddr_config_file = $ddr_config_file.$hw_which_file."/";
			$project = $ddr_project_para;
			$ptye = "half";
	}
	
	my $mhz_str = $ddr_freq."mhz";
	my $tmp_df_file;
#	&touch_tmp_hw_file ($ptye, $mhz_str, $bits, $project, $odt, $ddr_config_file);
	$ddr_config_58660_main = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$project.$odt;

	if ($ddr_df00_str eq "") {
		$ddr_config_58660_df00 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df00_str.$odt;
		system ("touch $ddr_config_58660_df00");
	} 
	else {
		$tmp_df_file = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df00_str.$odt;
		$ddr_config_58660_df00 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df00_str.$odt.".d";
		system("diff -fibB $ddr_config_58660_main $tmp_df_file | grep \"WM32\" > $ddr_config_58660_df00");
	}
#        system ("touch $ddr_config_58660_df00");

	if ($ddr_df01_str eq "") {
		$ddr_config_58660_df01 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df01_str.$odt;
		system ("touch $ddr_config_58660_df01");
	} 
	else {
		$tmp_df_file = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df01_str.$odt;
		$ddr_config_58660_df01 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df01_str.$odt.".d";
		system("diff -fibB $ddr_config_58660_main $tmp_df_file | grep \"WM32\" > $ddr_config_58660_df01");
	}
#        system ("touch $ddr_config_58660_df01");

	if ($ddr_df02_str eq "") {
		$ddr_config_58660_df02 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df02_str.$odt;
		system ("touch $ddr_config_58660_df02");
	} 
	else {
		$tmp_df_file = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df02_str.$odt;
		$ddr_config_58660_df02 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df02_str.$odt.".d";
		system("diff -fibB $ddr_config_58660_main $tmp_df_file | grep \"WM32\" > $ddr_config_58660_df02");
	}
#        system ("touch $ddr_config_58660_df02");

	$ddr_config_58660_end = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$project.$odt."_END";
 #       system ("touch $ddr_config_58660_end");

	###################ddr######################################
	if ($ddr_freq =~ /25/){
		if ($st58660_type =~ /^y$/) {
			$hw_template_file = "$config_path/template/hw_setting_af_pll_58660fpga.txt";
			$hw_setting_af_ddr = "$config_path/template/hw_setting_af_ddr_58660fpga.txt";
			$project = $ddr_project_para;
			$ddr_config_file = "$config_path/hw-setting/sn9866x/st58660/";
			$ddr_config_58660_main = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$project."_2.5_BL4";
			$ddr_config_58660_df00 = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$ddr_df00_str."_2.5_BL4";
			$ddr_config_58660_df01 = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$ddr_df01_str."_2.5_BL4";
			$ddr_config_58660_df02 = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$ddr_df02_str."_2.5_BL4";
			$ddr_config_58660_end = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$project."_2.5_BL4_END";
		} else {
			$hw_template_file = "$config_path/template/hw_setting_af_pll_fpga.txt";
			$hw_setting_af_ddr = "$config_path/template/hw_setting_af_ddr_fpga.txt";
			$project = $ddr_project_para;
			$ddr_config_file = "$config_path/hw-setting/sn986xx/st58600_fpga/";
			$ddr_config_file = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$project."_2.5_BL4";
		}
	}
	elsif ($ddr_freq =~ /150/){
			$ddr_config_file = $ddr_config_file ."ddr2_".$ptye."_150mhz_".$bits."_".$project.$odt;
	}
	elsif ($ddr_freq =~ /198/) {
			$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_198mhz_".$bits."_".$project.$odt;
	}
	elsif ($ddr_freq =~ /201/) {
			$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_201mhz_".$bits."_".$project.$odt;
	}
	elsif ($ddr_freq =~ /312/) {
			$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_312mhz_".$bits."_".$project.$odt;
	}
	elsif ($ddr_freq =~ /324/) {
			$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_324mhz_".$bits."_".$project.$odt;
	}
	elsif ($ddr_freq =~ /336/) {
			$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_336mhz_".$bits."_".$project.$odt;
	}
	else{
		if ($platform =~ /sn98610/){
			if ($bits =~ /32bit/){
				$odt = "_2.5_BL4_ODT_150ohm";
			}
		}
		if ($ddr_freq =~ /348/){
				$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_348mhz_".$bits."_".$project.$odt;
		}
		elsif ($ddr_freq =~ /360/) {
				$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_360mhz_".$bits."_".$project.$odt;
		}
		elsif ($ddr_freq =~ /372/){
				$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_372mhz_".$bits."_".$project.$odt;	
		}
		elsif ($ddr_freq =~ /384/){
				$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_384mhz_".$bits."_".$project.$odt;	
		}
		elsif ($ddr_freq =~ /396/){
				$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_396mhz_".$bits."_".$project.$odt;	
		}
		elsif ($ddr_freq =~ /402/){
				$ddr_config_file = $ddr_config_file . "ddr2_".$ptye."_402mhz_".$bits."_".$project.$odt;	
		}
	}
	
	if ($st58660_type =~ /^y$/) {
		my $witch_platform = "AFTER_986xx";

		system ("make -C $config_path/hw-setting/ sdk-f WHICH_VALUE=$ddr_config_58660_main PLATFORM=$witch_platform END_FILE=yes");
		$ddr_config_58660_main =~ s/\/sn9866x\//\/sdk\/sn9866x\//;
		unless (-e $ddr_config_58660_main){
			print "\nError:	ddr target file--->$ddr_config_58660_main does not exist!\n\n";
			exit;
		}
		system ("make -C $config_path/hw-setting/ sdk-f WHICH_VALUE=$ddr_config_58660_df00 PLATFORM=$witch_platform END_FILE=no");
		$ddr_config_58660_df00 =~ s/\/sn9866x\//\/sdk\/sn9866x\//;
		unless (-e $ddr_config_58660_df00){
			print "\nError:	ddr target file--->$ddr_config_58660_df00 does not exist!\n\n";
			exit;
		}
		system ("make -C $config_path/hw-setting/ sdk-f WHICH_VALUE=$ddr_config_58660_df01 PLATFORM=$witch_platform END_FILE=no");
		$ddr_config_58660_df01 =~ s/\/sn9866x\//\/sdk\/sn9866x\//;
		unless (-e $ddr_config_58660_df01){
			print "\nError:	ddr target file--->$ddr_config_58660_df01 does not exist!\n\n";
			exit;
		}
		system ("make -C $config_path/hw-setting/ sdk-f WHICH_VALUE=$ddr_config_58660_df02 PLATFORM=$witch_platform END_FILE=no");
		$ddr_config_58660_df02 =~ s/\/sn9866x\//\/sdk\/sn9866x\//;
		unless (-e $ddr_config_58660_df02){
			print "\nError:	ddr target file--->$ddr_config_58660_df02 does not exist!\n\n";
			exit;
		}
#		system ("make -C $config_path/hw-setting/ sdk-f WHICH_VALUE=$ddr_config_58660_end PLATFORM=$witch_platform");
		$ddr_config_58660_end =~ s/\/sn9866x\//\/sdk\/sn9866x\//;
		unless (-e $ddr_config_58660_end){
			print "\nError:	ddr target file--->$ddr_config_58660_end does not exist!\n\n";
			exit;
		}
	} else {
		my $witch_platform = "986xx";
		###generate ddr config
		unless (-e $ddr_config_file){
			print "\nError:	ddr origin file--->$ddr_config_file does not exist!\n\n";
			exit;
		}
		system ("make -C $config_path/hw-setting/ sdk-f WHICH_VALUE=$ddr_config_file PLATFORM=$witch_platform END_FILE=no");
		$ddr_config_file =~ s/\/sn986xx\//\/sdk\/sn986xx\//;
		unless (-e $ddr_config_file){
			print "\nError:	ddr target file--->$ddr_config_file does not exist!\n\n";
			exit;
		}
	}
  
	### DEBUG ###
	print "flinfo = $flashinfo_file\n";
	print "hw_t = $hw_template_file\n";
	if ($st58660_type =~ /^y$/) {
		print "ddr_main = $ddr_config_58660_main\n";
		print "ddr_df00 = $ddr_config_58660_df00\n";
		print "ddr_df01 = $ddr_config_58660_df01\n";
		print "ddr_df02 = $ddr_config_58660_df02\n";
		print "ddr_end = $ddr_config_58660_end\n";
	} else {
		print "ddr = $ddr_config_file\n";
	}
	unless (-e $flashinfo_file){
		print "\nError:	flashinfo file--->$flashinfo_file does not exist!\n\n";
		exit;
	}
	unless (-e $hw_template_file){
		print "\nError:	hw template file--->$hw_template_file does not exist!\n\n";
		exit;
	}
	unless (-e $uboot_para){
		print "\nError:	uboot file--->$uboot_para does not exist!\n\n";
		exit;
	}	
	unless (-e $hw_setting_af_ddr){
		print "\nError:	af_ddr file--->$hw_setting_af_ddr does not exist!\n\n";
		exit;
	}	
	############### ADD ##########################
	$uexce_command = $uexce_command." -e ./src/header/header.bin";
	if($flash_type_para =~ /NAND/i ){
		$uexce_command = $uexce_command." -f $flashinfo_file";
	}

	my @data_ddr;	
	my @data_ddr_main;	
	my @data_ddr_df00;	
	my @data_ddr_df01;	
	my @data_ddr_df02;	
	my @data_ddr_end;	
	if ($st58660_type =~ /^y$/) {
		open(oDDR, $ddr_config_58660_main) or die "Can't open '$ddr_config_58660_main': $!";
		@data_ddr_main = <oDDR>;	
		close oDDR;
		open(oDDR, $ddr_config_58660_df00) or die "Can't open '$ddr_config_58660_df00': $!";
		@data_ddr_df00 = <oDDR>;	
		close oDDR;
		open(oDDR, $ddr_config_58660_df01) or die "Can't open '$ddr_config_58660_df01': $!";
		@data_ddr_df01 = <oDDR>;	
		close oDDR;
		open(oDDR, $ddr_config_58660_df02) or die "Can't open '$ddr_config_58660_df02': $!";
		@data_ddr_df02 = <oDDR>;	
		close oDDR;
		open(oDDR, $ddr_config_58660_end) or die "Can't open '$ddr_config_58660_end': $!";
		@data_ddr_end = <oDDR>;	
		close oDDR;
	} else {
		open(oDDR, $ddr_config_file) or die "Can't open '$ddr_config_file': $!";
		@data_ddr = <oDDR>;	
		close oDDR;
	}

	open(oHW, $hw_template_file) or die "Can't open '$hw_template_file': $!";
	#binmode(oHW);
	# read file into an array
	my @data_hw = <oHW>;	
	close oHW;
	open(oAFDDR, $hw_setting_af_ddr) or die "Can't open '$hw_setting_af_ddr': $!";
	# read file into an array
	my @data_afddr = <oAFDDR>;	
	close oAFDDR;

	system("make -C ./src/code/ clean;make -C ./src/code/");
	my $muti_main_fir = "hw_setting.image.main.d";
	my $muti_f00_fir = "hw_setting.image.f00.d";
	my $muti_f01_fir = "hw_setting.image.f01.d";
	my $muti_f02_fir = "hw_setting.image.f02.d";
	my $muti_end_fir = "hw_setting.image.end.d";
	my $hw_setting_file = $_[9];

	if ($st58660_type =~ /^y$/) {
		# main
		open oHW_SETTING, ">" . "./hw_setting.main.txt.d";
		foreach (@data_hw){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		foreach (@data_ddr_main){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		close oHW_SETTING;
		system("touch ./src/header/header.bin");
		system("./src/code/image_tool -h ./hw_setting.main.txt.d -o ./ -e ./src/header/header.bin");
		my $file_len = -s $hw_setting_file;
		system("mv $hw_setting_file $muti_main_fir");

		#end main
		#df00 
		open oHW_SETTING, ">" . "./hw_setting.f00.txt.d";
		foreach (@data_ddr_df00){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		close oHW_SETTING;

		# -e just for compile
		system("./src/code/image_tool -h ./hw_setting.f00.txt.d -o ./ -e ./src/header/header.bin");
		system("mv $hw_setting_file $muti_f00_fir");
		#end df00
		#df01 
		open oHW_SETTING, ">" . "./hw_setting.f01.txt.d";
		foreach (@data_ddr_df01){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		close oHW_SETTING;
		system("./src/code/image_tool -h ./hw_setting.f01.txt.d -o ./ -e ./src/header/header.bin");
		system("mv $hw_setting_file $muti_f01_fir");
		#end df00
		#df02
		open oHW_SETTING, ">" . "./hw_setting.f02.txt.d";
		foreach (@data_ddr_df02){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		close oHW_SETTING;
		system("./src/code/image_tool -h ./hw_setting.f02.txt.d -o ./ -e ./src/header/header.bin");
		system("mv $hw_setting_file $muti_f02_fir");
		#end df00
		#end ddr
		open oHW_SETTING, ">" . "./hw_setting.end.txt.d";
		foreach (@data_ddr_end){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		foreach (@data_afddr){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		close oHW_SETTING;
		system("./src/code/image_tool -h ./hw_setting.end.txt.d -o ./ -e ./src/header/header.bin");
		system("mv $hw_setting_file $muti_end_fir");
	} else {	
		my $last_line = "";
		my $fill = "empty";
		my $find_030 = "empty";
		open oHW_SETTING, ">" . "./hw_setting.txt.d";
		foreach (@data_hw){
			if(($_ =~ /^\#/)||($_ =~ /^b/)){
				print oHW_SETTING;
			}
		}
		foreach (@data_ddr){
			my $th_li = $_;
			if (($th_li =~ /^w 0x9030/)&&($find_030 =~ /^empty$/)){
				$find_030 = "full";
			}
			if($find_030 =~ /^full$/){
				unless($th_li =~ /^w 0x9030/){
					foreach(@data_afddr){
						unless($_ =~ /^\#/){
							print oHW_SETTING;
							$last_line = $_;
						}
					}
					unless($last_line =~ /^\s*$/){
						print oHW_SETTING "\n";
					}
				}
			}
			print oHW_SETTING $th_li;
			if (($_ =~ /^n/)&&($fill =~ /^empty$/)){
				$fill = "full";
				foreach (@data_hw){
					unless(($_ =~ /^\#/)||($_ =~ /^b/)){
						print oHW_SETTING;
						$last_line = $_;
					}
				}
				unless($last_line =~ /^\s*$/){
					print oHW_SETTING "\n";
				}
			}
		}
		close oHW_SETTING;

		#system("cat $hw_template_file $ddr_config_file > ./hw_setting.txt.d");
		$uexce_command = $uexce_command." -h ./hw_setting.txt.d";
	}
	
	if ($st58660_type =~ /^y$/) {
		# st58660 start
		my $muti_image = "muti.image.d";

		my $muti_main_size = -s $muti_main_fir;
		my $muti_f00_size = -s 	$muti_f00_fir;
		my $muti_f01_size = -s  $muti_f01_fir;
		my $muti_f02_size = -s 	$muti_f02_fir;
		my $muti_end_size = -s 	$muti_end_fir;

		my @muti_padd_main_fir = ();
		my @muti_padd_f00_fir = ();
		my @muti_padd_f01_fir = ();
		my @muti_padd_f02_fir = ();
		my @muti_padd_end_fir = ();

		my $muti_bypass_mode_data=0;
		my $muti_bypass_addr_data=0;
		my $muti_bypass_mode_val=0;
		my $muti_bypass_addr_val=0;

		my $muti_f00_str_addr;
		my $muti_f01_str_addr;
		my $muti_f02_str_addr;
		my $muti_end_str_addr;


  		open(oFILE, $muti_main_fir) or die "Can't open '$muti_main_fir': $!";
  		binmode(oFILE);
	  	read(oFILE, $muti_bypass_mode_data, 4);
  		read(oFILE, $muti_bypass_addr_data, 4);
	 	@muti_padd_main_fir = <oFILE>;
  		close oFILE;

	  	$muti_bypass_mode_val = unpack("V*",$muti_bypass_mode_data);
  		$muti_bypass_addr_val = unpack("V*",$muti_bypass_addr_data);




	  	printf "muti_bypass_mode_val = %x\n" ,$muti_bypass_mode_val;
  		printf "muti_bypass_addr_val = %x\n" ,$muti_bypass_addr_val;

	  	open(oFILE, $muti_f00_fir) or die "Can't open '$muti_f00_fir': $!";
  		binmode(oFILE);
	  	@muti_padd_f00_fir = <oFILE>;	
  		close oFILE;

	  	open(oFILE, $muti_f01_fir) or die "Can't open '$muti_f01_fir': $!";
  		binmode(oFILE);
	  	@muti_padd_f01_fir = <oFILE>;	
  		close oFILE;

	  	open(oFILE, $muti_f02_fir) or die "Can't open '$muti_f02_fir': $!";
  		binmode(oFILE);
	  	@muti_padd_f02_fir = <oFILE>;	
  		close oFILE;

	  	open(oFILE, $muti_end_fir) or die "Can't open '$muti_end_fir': $!";
  		binmode(oFILE);
	  	@muti_padd_end_fir = <oFILE>;	
  		close oFILE;

	  	$muti_f00_str_addr = 28 + ($muti_main_size -8);
		$muti_f01_str_addr = $muti_f00_str_addr + $muti_f00_size;
		$muti_f02_str_addr = $muti_f01_str_addr + $muti_f01_size;
		$muti_end_str_addr = $muti_f02_str_addr + $muti_f02_size;

		printf "muti_f00_str_addr = %x\n" , $muti_f00_str_addr;
		printf "muti_f01_str_addr = %x\n" , $muti_f01_str_addr;
		printf "muti_f02_str_addr = %x\n" , $muti_f02_str_addr;
		printf "muti_end_str_addr = %x\n" , $muti_end_str_addr;

		unshift @muti_padd_main_fir, pack('L', $muti_end_str_addr);
		unshift @muti_padd_main_fir, pack('L', $muti_f02_str_addr);
		unshift @muti_padd_main_fir, pack('L', $muti_f01_str_addr);
		unshift @muti_padd_main_fir, pack('L', $muti_f00_str_addr);
		unshift @muti_padd_main_fir, pack('L', $muti_bypass_addr_val);
		unshift @muti_padd_main_fir, pack('L', $muti_bypass_mode_val);

		open oFILE, ">" . $muti_image or die "Can't open '$muti_image': $!";
		 	print oFILE @muti_padd_main_fir;
	 		print oFILE @muti_padd_f00_fir;
		 	print oFILE @muti_padd_f01_fir;
		 	print oFILE @muti_padd_f02_fir;
	 		print oFILE @muti_padd_end_fir;
		close oFILE;

		#muti-hw_setting.image
		#=begin REM
		my @hw_setting_pad;
		my $hw_setting_size;
		my $hw_setting_pad_size;

		$hw_setting_size = -s $muti_image;
		$hw_setting_size = $hw_setting_size + 4;
		$hw_setting_pad_size  = (($hw_setting_size + 0x1F) & (~0x1F)) - $hw_setting_size;
		$hw_setting_size = $hw_setting_size + $hw_setting_pad_size;

		my @padd_hw_setting = ();	
  		open(oFILE, $muti_image) or die "Can't open '$muti_image': $!";
	  	binmode(oFILE);
  		@padd_hw_setting = <oFILE>;	
	  	close oFILE;

		for (my $i=0;
			$i < $hw_setting_pad_size;
			$i++){
			push @padd_hw_setting, pack('H2', 'ff');		
		}

  		unshift @padd_hw_setting, pack('L', $hw_setting_size);

	  	open oFILE, ">" . $hw_setting_file or die "Can't open '$hw_setting_file': $!";
		 	print oFILE @padd_hw_setting;
		close oFILE;

		my $hw_setting_crc16 = 0;
		my @hw_setting_message;
		my $hw_setting_count=$hw_setting_size;
		my $hw_setting_index_message=0;


		open(GIF, $hw_setting_file) or die "can't open '$hw_setting_file': $!";
		binmode(GIF);
		while ($hw_setting_count > 0){
			$hw_setting_count--;
			read(GIF, $hw_setting_message[$hw_setting_index_message], 1);
			$hw_setting_message[$hw_setting_index_message] = unpack("C",$hw_setting_message[$hw_setting_index_message]);
			$hw_setting_index_message++;
		}
		close GIF;

		&crcSlow(
			\$hw_setting_crc16,
			\@crc16_tab, 
			\@hw_setting_message,       
			\$hw_setting_size
	 	);

		open oFILE, ">>" . $hw_setting_file or die "Can't open '$hw_setting_file': $!";
			print oFILE pack('L', $hw_setting_crc16);
		close oFILE;

		$hw_setting_size = -s $hw_setting_file;

		my $hw_setting_file_pad = $HW_SETTING_PK_SIZE - $hw_setting_size;

 		open oFILE, ">>" . $hw_setting_file or die "Can't open '$hw_setting_file': $!";
	 	for (my $i=0;
			$i < $hw_setting_file_pad;
			$i++){
			print oFILE pack('H2', 'ff');
		}
		close oFILE;
		# st58660 end

		$uexce_command = $uexce_command." -m nothing";
	}

	&padd32($uboot_para,$pz_ufile);
	$uexce_command = $uexce_command." -u ".$pz_ufile;


	### flash_layout.h ###
	&fun_flash_layout_header(
		\$flash_layout_header,
		\$flash_type_para,
		\@flash_layout_value
                      );
                       
	### flash_layout.bin ###
	&fun_flash_layout_bin(
 		\$flash_layout_file, 
    \$flash_type_para, 
    \@flash_layout_value
                   );

	$env_file = $config_path."/u-boot-env/sn986xx/".$env_file;
	my $env_type = lc($flash_type_para);
	$env_file = $env_file."$platform-".$env_type.".txt";
	unless (-e $env_file){
		print "\nError:	env file--->$env_file does not exist!\n\n";
		exit;
	}
	&read_env;
	$uexce_command = $uexce_command." -n ".$u_boot_env_file;
	#mem_excel check
	my $bit_value = "";
	if ($bits =~ /([0-9]+)[a-zA-Z]+/){
	 	 $bit_value = $1;
	}
	my $mem_size = $mem_kconfig * $bit_value / 8;
	if ($mem_size != $mem_excel){
	 	 print "WARNING: mem size between 'excel' and 'kconfig' are different!\n";
	 	 if ($mem_excel > $mem_size){
	 	 	 print "ERROR: mem size in 'excel' is larger than in 'kconfig'!\n";
	 	 	 exit;
	 	 }
	}

	system("mv ./$flash_layout_header ./src/header/");
	system("make -C ./src/header/ clean;make -C ./src/header/ FW_BURN_FLOW=no");
	$uexce_command = $uexce_command." -l ".$flash_layout_file;

	$uexce_command = $uexce_command." -v 0x12345678";
	$uexce_command = $uexce_command." -o ./";

	system($uexce_command);
	print $uexce_command;
	print "\n";	
	
	# ./uexce.bin.d - UBOOT.bin -
	&packize($uboot_file,$UBOOT_PZNAME,$UBOOT_NAME,$UBOOT_VER,$NOW_TIME,'N');
}

sub read_env
{
	# read input file 
	open(oFILE, $env_file) or die "Can't open '$env_file': $!";
	binmode(oFILE);
	# read file into an array
	my @env_data = <oFILE>;	
	close oFILE;
	
	my $record_start = "no";
	foreach(@env_data){
		if($_ =~ /^[\s]*$/){
			next;
		}
		if($record_start =~ /^yes$/i){
			my $this_line = $_;
			my @this_lines = split /=/,$this_line;
			my $env_arg = "";
			my $env_cmd = "";
			###get args content
			$env_arg = 	$this_lines[0];
			$env_arg = $env_arg."=";
			###get cmds content
			$this_line =~ s/^$this_lines[0]=//;
			if ($env_arg =~ /^bootargs=$/i) {
				my $tmp_value = 0;
				my @cmd_lines = split /:/,$this_line;
				my $cmd_pre = $cmd_lines[0];
				
				# uboot
				if ($factory_end > $factory_str) {
					$tmp_value = 512 + ($factory_end - $factory_str + 1) / 1024;
				} else {
					$tmp_value = 512;
				}
				$env_cmd = $cmd_pre.":".$tmp_value."k(uboot)";
				#kernel
				$tmp_value = ($kernel_end - $kernel_str + 1) / 1024;
				$env_cmd = $env_cmd.",".$tmp_value."k(kernel)";
				#rootfs
				$tmp_value = ($rootfs_r_end - $rootfs_r_str + 1) / 1024;
				$env_cmd = $env_cmd.",".$tmp_value."k(rootfs)";
				#jffs2
				if ($rootfs_jffs2_end > $rootfs_jffs2_str) {
					$tmp_value = ($rootfs_jffs2_end - $rootfs_jffs2_str + 1) / 1024;
					$env_cmd = $env_cmd.",".$tmp_value."k(appimage)";
				}
				#rescue
				if ($rescue_end > $rescue_str) {
					$tmp_value = ($rescue_end - $rescue_str + 1) / 1024;
					$env_cmd = $env_cmd.",".$tmp_value."k(rescue)";
				}
				#etc
				$tmp_value = ($rootfs_rw_end - $rootfs_rw_str + 1) /1024;
				$env_cmd = $env_cmd.",".$tmp_value."k(etc)";
				#user
				if ($user_end > $user_str) {
					$tmp_value = ($user_end - $user_str + 1) / 1024;
					$env_cmd = $env_cmd.",".$tmp_value."k(userconfig)";
				}
				#custom1
				if ($custom1_end > $custom1_str) {
					$tmp_value = ($custom1_end - $custom1_str + 1) / 1024;
					$env_cmd = $env_cmd.",".$tmp_value."k(custom1)";
				}
				#custom2
				if ($custom2_end > $custom2_str) {
					$tmp_value = ($custom2_end - $custom2_str + 1) / 1024;
					$env_cmd = $env_cmd.",".$tmp_value."k(custom2)";
				}
				print "env_cmd:$env_cmd\n";
			}
			else {
				$env_cmd = $this_line;
			}	
			$env_cmd =~ s/\s*$//;
			###push into it
			push(@envs_args,$env_arg);
			push(@envs_cmds,$env_cmd);
		}
		if($_ =~ /^---/){
			$record_start = "yes";
		}
	}

	#find out $mem_excel
	foreach (@envs_cmds){
		if (/mem=([0-9]+)/){
			 $mem_excel = $1;
			 last;
		}
	}
	
	### u-boot-env.bin ###
		my $uenv_i = 0;
		my $uenv_str;
		open oFILE, ">" . $u_boot_env_file;
			foreach (@envs_args){
				if ($envs_cmds[$uenv_i] !~ /NULL/){
					$uenv_str = $envs_args[$uenv_i].$envs_cmds[$uenv_i].$envs_tack;
					foreach(unpack("(a1)*", $uenv_str)) {
					print oFILE;
					}
				}
				$uenv_i = $uenv_i + 1;
			}
		close oFILE;
		&padd_env($u_boot_env_file, $flash_type);	
}

sub padd32                             
{
	my $file = $_[0];
	my $pz_file = $_[1];
	
	my $filesize = 0;
	my @data_w;
	my $buffer;
	my $filebuff;
	my @vtime;
	my $i = 0;
	
	$filesize = -s $file;          
	open(oFILE, $file) or die "Can't open '$file': $!";     
	read(oFILE, $filebuff, $filesize - 32, 0);              
	close oFILE;
	
	open(oFILE, ">$pz_file") or die "Can't open '$pz_file': $!";
	#binmode(oFILE);
	#print oFILE @data_w;
	print oFILE $filebuff;                                       
	close oFILE;
	
	#print "============================\n";	
	open(oFILE, "<$file");
	seek(oFILE,$filesize - 32,0);                          
	read(oFILE, $buffer, 32, 0);                             
	close(oFILE);

	foreach (split(//, $buffer)) {
		$vtime[$i] = ord($_);                              
		$i++;
	}
	my $size = ($filesize + 4);
	my $pad = 0;
	$pad  = (($size + 0x1F) & (~0x1F)) - ($size);    


	if(($vtime[16]==50)&&        
	($vtime[20]==47)&&
	($vtime[25]==47)) {
	#uboot format
		open(oFILE, ">>$pz_file");        
		for (my $i=0;           
			$i < $pad;
			$i++){
			print oFILE pack('H2', 'FF');      
			}	
			
			foreach(@vtime){
				my $hexval = sprintf("%x", $_);	
				print oFILE pack('H2',$hexval);   
			}
		close oFILE;
	}
	else{                                                
	#others
		open(oFILE, ">>$pz_file");
		foreach(@vtime){
				my $hexval = sprintf("%x", $_);	
				print oFILE pack('H2',$hexval);
			}
		
		for (my $i=0;
			$i < $pad;
			$i++){
			print oFILE pack('H2', 'FF');
			}
		close oFILE;
	}
}

sub padd_env
{
	my $file = $_[0];
	my $type = $_[1];
	my $filesize;
	my $pad = 0;
	my $crc32 = 0;


	$filesize = -s $file;
	
	if ($type =~ /NAND/i){
		$pad = $NAND_BLK_SIZE - $filesize - 116;
	}
	elsif ($type =~ /SF/i){
		$pad = $SPI_BLK_SIZE - $filesize - 116 ;
	}
	else{
		print "flash type ERROR!!\n"
	}

	open(oFILE, ">>$file");	# Open for appending
	for (my $i=0;
			$i < $pad;
			$i++){
			print oFILE pack('H2', 'FF');
	}
	close oFILE;

	#make $file to 16times large
	my $what_large = -s $file;
	until ($what_large < 16){
		$what_large -= 16;
  } 
  unless ($what_large == 0){
  	print "\n\n\n--------------------\n\n\n";
	  my $how_add = 16 - $what_large;
    open(oFILE, ">>$file");	# Open for appending
		while ($how_add > 0){
			$how_add --;
			my $add_str = sprintf("%s", "FF");
			my $str_done = pack("a2", $add_str); 
			print oFILE pack('H*',$str_done);
		}
		close oFILE;  
  }

	### Attach CRC16 [16-bytes] ###
	my $crc16;
	my @message;
	my $message_bytes = -s $file;
	my $count=$message_bytes;
	my $index_message=0;
	
	open(GIF, $file) or die "can't open '$file': $!";
	binmode(GIF);
	while ($count > 0){
		$count--;
		read(GIF, $message[$index_message], 1);
		$message[$index_message] = unpack("C",$message[$index_message]);
		$index_message++;
	}
	close GIF;
	
	&crcSlow(
		\$crc16,
		\@crc16_tab, 
		\@message,       
		\$message_bytes
 	);


	my $filebuff;
	
	$filesize = -s $file;
	open(oFILE, $file) or die "Can't open '$file': $!";
	read(oFILE, $filebuff, $filesize , 0);
	close oFILE;
	
	open(oFILE, ">$file");	# Open for appending
	print oFILE pack('L',$filesize);
	print oFILE $filebuff;
	close oFILE;
	
#name
	open(oFILE, ">>$file");	# Open for appending
	my $str = pack( "a16", $U_BOOT_ENV_NAME); 
	foreach(unpack("(a1)*", $str)) {
		print oFILE;
	}
#version
	$str = pack("a64", $U_BOOT_ENV_VER); 
	foreach(unpack("(a1)*", $str)) {
		print oFILE;
	}
#time	
	$str = pack("a16", $NOW_TIME); 
	foreach(unpack("(a1)*", $str)) {
		print oFILE;
	}
  print oFILE pack('L',$crc16);
	
	my $add_str = sprintf("%s", "FF");
	my $time_add = 12;
	while ($time_add){
		$time_add --;
		my $str_done = pack("a2", $add_str); 
		print oFILE pack('H*',$str_done);
	}	

	close (oFILE);

	
	open(oFILE, ">./env.d");	# Open for appending
	print oFILE $filebuff;
	close oFILE;
}

sub packize
{
	my $file = $_[0];
	my $pz_file = $_[1];
	my $name = $_[2];
	my $version = $_[3];
	my $time = $_[4];
	my $append_size = $_[5]; # 'S'
	my $spi_file_d = $_[6];
	my $nand_file_d = $_[7];
	my @data_w;
	my @data_spi;
	my @data_nand;
	my $filesize;
	my $str;
	
	#######
	#make $file to 16times large
	my $what_large = -s $file;
	until ($what_large < 16){
		$what_large -= 16;
	} 
	unless ($what_large == 0){
		my $how_add = 16 - $what_large;
		open(oFILE, ">>$file");	# Open for appending
		while ($how_add > 0){
			$how_add --;
			my $add_str = sprintf("%s", "FF");
			my $str_done = pack("a2", $add_str); 
			print oFILE pack('H*',$str_done);
		}
		close oFILE;  
	}

	### Attach CRC16 [16-bytes] ###
	my $crc16;
	my @message;
	my $message_bytes = -s $file;
	my $count=$message_bytes;
	my $index_message=0;
	
	open(GIF, $file) or die "can't open '$file': $!";
	binmode(GIF);
	while ($count > 0){
		$count--;
		read(GIF, $message[$index_message], 1);
		$message[$index_message] = unpack("C",$message[$index_message]);
		$index_message++;
	}
	close GIF;
	
	&crcSlow(
		\$crc16,
		\@crc16_tab, 
		\@message,       
		\$message_bytes
 	);
	
	open(oFILE, $file) or die "Can't open '$file': $!";
	binmode(oFILE);
	# read file into an array
	@data_w = <oFILE>;	
	close oFILE;
	
	#packize : file size
	if(($append_size eq "S")|| 
		($append_size eq "I")||
		($append_size eq "F"))
	{
		#open(oFILE, $pz_file) or die "Can't open '$pz_file': $!";
		open(my $out, '>:raw', $pz_file) or die "Unable to open: $!";
		$filesize = -s $file;
		print $out pack('L',$filesize);
		close $out;
		
		open(oFILE, ">>$pz_file");	# Open for appending
		print oFILE @data_w;
		close oFILE;
	}
	else{
		open(oFILE, ">$pz_file") or die "Can't open '$pz_file': $!";
		binmode(oFILE);
		print oFILE @data_w;
		close oFILE;
	}
	
	if($append_size eq "F"){
		open(oFILE, $spi_file_d) or die "Can't open '$file': $!";
		binmode(oFILE);
		@data_spi = <oFILE>;	
		close oFILE;
	
		open(oFILE, $nand_file_d) or die "Can't open '$file': $!";
		binmode(oFILE);
		@data_nand = <oFILE>;	
		close oFILE;
		
		open(oFILE, ">>$pz_file");	# Open for appending
		print oFILE @data_spi;
		close oFILE;
		
		open(oFILE, ">>$pz_file");	# Open for appending
		print oFILE @data_nand;
		close oFILE;
	}
	
	open(oFILE, ">>$pz_file");	# Open for appending
	$str = pack( "a16", $name); 
	
	if($append_size ne "I") {
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
	}
	
	$str = pack("a64", $version); 
	foreach(unpack("(a1)*", $str)) {
		print oFILE;
	}
	
	$str = pack("a16", $time); 
	foreach(unpack("(a1)*", $str)) {
		print oFILE;
	}

	print oFILE pack('L',$crc16);
	
	my $add_str = sprintf("%s", "FF");
	my $time_add = 12;
	while ($time_add){
		$time_add --;
		my $str_done = pack("a2", $add_str); 
		print oFILE pack('H*',$str_done);
	}
	
	close oFILE;
}

sub fun_flash_layout_header
{
	my (
			$filelayout_file, 
			$filelayout_type, 
			$flash_layout_value_para	#@                      
	) = @_;	 
	open oHEADER, ">" . $$filelayout_file or die "Can't open '$$filelayout_file': $!";
	print oHEADER "/*\n";
	print oHEADER "*	NAND\n";
	print oHEADER "*/\n";
	my $flash_index = 0;
	my $flash_value ="0x00000000";
	foreach (@flash_nand){
  	$_ =~s/-/_/g;
  	if($$filelayout_type =~ /NAND/i){
  		$flash_value = $$flash_layout_value_para[$flash_index];
  	}
		print oHEADER "#define    ";
		print oHEADER "NAND_".uc($_)."_STR";
		print oHEADER "\t".$flash_value;
		print oHEADER "\n";
		$flash_index ++;
			
		if($$filelayout_type =~ /NAND/i){
  		$flash_value = $$flash_layout_value_para[$flash_index];
  	}	
		print oHEADER "#define    ";
		print oHEADER "NAND_".uc($_)."_END";
		print oHEADER "\t".$flash_value;
		print oHEADER "\n";
		$flash_index ++;
	}
	print oHEADER "\n\n";
	print oHEADER "/*\n";
	print oHEADER "*	SPI\n";
	print oHEADER "*/\n";
	$flash_index = 0;
	$flash_value ="0x00000000";
	
	foreach (@flash_spi){
  		$_ =~s/-/_/g;
  		if($$filelayout_type =~ /SF/i){
  			$flash_value = $$flash_layout_value_para[$flash_index];
  		}

		print oHEADER "#define    ";
		print oHEADER "SPI_".uc($_)."_STR";
		print oHEADER "\t".$flash_value;
		print oHEADER "\n";
		$flash_index ++;

		if($$filelayout_type =~ /SF/i){
  			$flash_value = $$flash_layout_value_para[$flash_index];
  		}	
		
		print oHEADER "#define    ";
		print oHEADER "SPI_".uc($_)."_END";
		print oHEADER "\t".$flash_value;
		print oHEADER "\n";
		$flash_index ++;
		# skip path vlaue
		$flash_index ++;
	}
  close(oHEADER);

}

sub fun_flash_layout_bin
{
	my (
			$filelayout_file, 
			$filelayout_type, 
			$flash_layout_value_para	#@                      
	) = @_;	 

	my $flash_index = 0;
		
	open oFILE, ">" . $$filelayout_file;
	if ($$filelayout_type =~ /NAND/i){
	my $i = 0;
	foreach (@flash_nand){
		my $str	= $$flash_layout_value_para[$flash_index];
		$flash_index ++;
		my $end = $$flash_layout_value_para[$flash_index];
		$flash_index ++;
				
		$str =~ s/\s+//g;
		$end =~ s/\s+//g;
					
		my	@p32u_str = split(/x/, $str);
		my $ff32u_str = sprintf "%08d", hex($p32u_str[1]);
		print oFILE pack('L', $ff32u_str);
					
		my	@p32u_end = split(/x/, $end);
		my $ff32u_end = sprintf "%08d", hex($p32u_end[1]);
		print oFILE pack('L', $ff32u_end);
		}
	}
	elsif ($$filelayout_type =~ /SF/i){
		my $i = 0;
		$flash_index = 0;
		foreach (@flash_spi){
			my $str	= $$flash_layout_value_para[$flash_index];
			$flash_index ++;
			my $end = $$flash_layout_value_para[$flash_index];
			$flash_index ++;
			$flash_index ++;
					
			$str =~ s/\s+//g;
			$end =~ s/\s+//g;
					
			my	@p32u_str = split(/x/, $str);
			my $ff32u_str = sprintf "%08d", hex($p32u_str[1]);
			print oFILE pack('L', $ff32u_str);
					
			my	@p32u_end = split(/x/, $end);
			my $ff32u_end = sprintf "%08d", hex($p32u_end[1]);
			print oFILE pack('L', $ff32u_end);
			if ($_ eq "factory") {
				$factory_str = $ff32u_str;
				$factory_end = $ff32u_end;
			}
			elsif ($_ eq "user") {
				$user_end = $ff32u_end;
				$user_str = $ff32u_str;
			} 
			elsif ($_ eq "fs_jffs2") {
				$rootfs_jffs2_end = $ff32u_end;
				$rootfs_jffs2_str = $ff32u_str;
			} 
			elsif ($_ eq "rescue") {
				$rescue_str = $ff32u_str;
				$rescue_end = $ff32u_end;
			}
			elsif ($_ eq "kernel") {
				$kernel_str = $ff32u_str;
				$kernel_end = $ff32u_end;
			}
			elsif ($_ eq "rootfs_r") {
				$rootfs_r_str = $ff32u_str;
				$rootfs_r_end = $ff32u_end;
			} 
			elsif ($_ eq "rootfs_rw") {
				$rootfs_rw_str = $ff32u_str;
				$rootfs_rw_end = $ff32u_end;
			}
			elsif ($_ eq "custom1") {
				$custom1_str = $ff32u_str;
				$custom1_end = $ff32u_end;
			}
			elsif ($_ eq "custom2") {
				$custom2_str = $ff32u_str;
				$custom2_end = $ff32u_end;
			}
		}
	}
	else{
		print "Error !! Flash Type Mistake....\n"
	}
		
	close oFILE;
	
	#make $file to 16times large
	my $what_large = -s $$filelayout_file;
	until ($what_large < 16){
		$what_large -= 16;
	} 
	unless ($what_large == 0){
	  my $how_add = 16 - $what_large;
		open(oFILE, ">>$$filelayout_file");	# Open for appending
		while ($how_add > 0){
			$how_add --;
			my $add_str = sprintf("%s", "FF");
			my $str_done = pack("a2", $add_str); 
			print oFILE pack('H*',$str_done);
		}
		close oFILE;  
	}
	
	### Attach CRC16 [16-bytes] ###
	my $crc16;
	my @message;
	my $message_bytes = -s $$filelayout_file;
	my $count=$message_bytes;
	my $index_message=0;
	
	open(GIF, $$filelayout_file) or die "can't open '$$filelayout_file': $!";
	binmode(GIF);
	while ($count > 0){
		$count--;
		read(GIF, $message[$index_message], 1);
		$message[$index_message] = unpack("C",$message[$index_message]);
		$index_message++;
	}
	close GIF;
	
	&crcSlow(
		\$crc16,
		\@crc16_tab, 
		\@message,       
		\$message_bytes
 	);
		
	my $flash_layout_size = -s $$filelayout_file;
	
	open(oFILE, ">>$$filelayout_file");	# Open for appending
	print oFILE pack('L',$crc16);
	
	my $add_str = sprintf("%s", "FF");
	my $time_add = 12;
	while ($time_add){
		$time_add --;
		my $str_done = pack("a2", $add_str); 
		print oFILE pack('H*',$str_done);
	}
	
	#zeroing to size
	for (my $i=0;
		$i < ($flash_layout_file_size - $flash_layout_size -16);
		$i++){
			print oFILE pack('H2', 'FF');
		}	
	close oFILE;
}

sub gettime
{
	my $now_string = localtime;  # e.g., "Thu Oct 13 04:54:34 1994"
	my @now_time = split(/ +/, $now_string);
	$now_time[3] =~ s/://g; #hrs:min:sec
	
	my $month = $now_time[1];
	my $mth;
	
	if ($month	=~	/Jan/){		$mth = "01";}
	elsif ($month	=~	/Feb/){ $mth = "02";}
	elsif ($month	=~	/Mar/){ $mth = "03";}
	elsif ($month	=~	/Apr/){ $mth = "04";}
	elsif ($month	=~	/May/){ $mth = "05";}
	elsif ($month	=~	/Jun/){ $mth = "06";}
	elsif ($month	=~	/Jul/){ $mth = "07";}
	elsif ($month	=~	/Aug/){ $mth = "08";}
	elsif ($month	=~	/Sep/){ $mth = "09";}
	elsif ($month	=~	/Oct/){ $mth = "10";}
	elsif ($month	=~	/Nov/){ $mth = "11";}
	elsif ($month	=~	/Dec/){	$mth = "12";} 

	my $what_size = length($now_time[2]);
	if($what_size == 1){
		$now_time[2] = "0".$now_time[2];
	}
	my $time_string = $now_time[4]."/".$mth.$now_time[2]."/".$now_time[3];
	
	$time_string;
}

#calculate crc16
sub crcSlow
{
	my (
	     $crc_16_value, 
	     $crc_16_tab,  #@
	     $crc_message, #@
	     $crc_bytes
	   ) = @_;
	   
	my $crc_count=$$crc_bytes;
	my $crc_temp;
	my $crc_sum=0xffff;
	my $message_index="0";
	
	while ($crc_count > "0"){
		$crc_count--;
		$crc_temp = $$crc_message[$message_index] ^ $crc_sum;
		my $temp_x = sprintf ("%04x",$crc_temp);
		$temp_x =~ s/^[0-9a-zA-Z][0-9a-zA-Z]//i;
		my $temp_d = hex($temp_x);
		$crc_sum >>= 8;
		$crc_sum ^= $$crc_16_tab[$temp_d];
		$message_index++;	
	}
	$$crc_16_value = $crc_sum;
}
