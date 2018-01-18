#!/usr/bin/perl -w
use strict;

# static value
my $target_file = "serial_flashlayout.conf";
my $hw_setting_size=0x1000;
my $storage_rootfs_size=0x1000;
my $storage_jffs2_size=0x1000;
my $uboot_size=0x7b000;
my $uenv_size=0x1000;
my $flash_layout_size=0x1000;

#dynamic
my $nvram_user_size=0;
my $nvram_factory_size=0;
my $etc_part_size=0;
my $kernel_part_size=0;
my $rootfsr_part_size=0;
my $jffs2_part_mount="null";
my $jffs2_part_size=0;
my $rescue_part_size=0;
my $custom1_part_size=0;
my $custom1_image_name="null";
my $custom1_part_mount="null";
my $custom1_mount_type="null";
my $custom2_part_size=0;
my $custom2_image_name="null";
my $custom2_part_mount="null";
my $custom2_mount_type="null";
my $addto_mount_partition = "no";
my $flash_total_size=0;
my $logo_part_size=0;
my $image_dir="";
my $rootfs_dir="";
my $rescuefs_dir="";
my $platform_name="no";
my $etc_mount_count=3;

foreach (@ARGV){
	if (/^nvram_user_size=.*/){
		s/.*=//g;
		$nvram_user_size = $_;
		$nvram_user_size = $nvram_user_size * 32 * 1024;
	}
	elsif (/^nvram_factory_size=.*/){
		s/.*=//g;
		$nvram_factory_size = $_;
		$nvram_factory_size = $nvram_factory_size * 32 * 1024;
	}
	elsif (/^etc_part_size=.*/){
		s/.*=//g;
		$etc_part_size = $_;
		$etc_part_size = $etc_part_size * 1024 * 32;
	}
	elsif (/^kernel_part_size=.*/){
		s/.*=//g;
		$kernel_part_size = $_;
		$kernel_part_size = $kernel_part_size * 32 * 1024;
	}
	elsif (/^rootfsr_part_size=.*/){
		s/.*=//g;
		$rootfsr_part_size = $_;
		$rootfsr_part_size = $rootfsr_part_size * 32 * 1024;
	}
	elsif (/^jffs2_part_size=.*/){
		s/.*=//g;
		$jffs2_part_size = $_;
		$jffs2_part_size = $jffs2_part_size * 32 * 1024;
	}
	elsif (/^jffs2_part_mount=.*/){
		s/.*=//g;
		$jffs2_part_mount = $_;
	}
	elsif (/^rescue_part_size=.*/){
		s/.*=//g;
		$rescue_part_size = $_;
		$rescue_part_size = $rescue_part_size * 32 * 1024;
	}
	elsif (/^custom1_part_size=.*/){
		s/.*=//g;
		$custom1_part_size = $_;
		$custom1_part_size = $custom1_part_size * 32 * 1024;
	}
	elsif (/^custom1_image_name=.*/){
		s/.*=//g;
		$custom1_image_name = $_;
	}
	elsif (/^custom1_part_mount=.*/){
		s/.*=//g;
		$custom1_part_mount = $_;
	}
	elsif (/^custom1_mount_type=.*/){
		s/.*=//g;
		$custom1_mount_type = $_;
	}
	elsif (/^custom2_part_size=.*/){
		s/.*=//g;
		$custom2_part_size = $_;
		$custom2_part_size = $custom2_part_size * 32 * 1024;
	}
	elsif (/^custom2_image_name=.*/){
		s/.*=//g;
		$custom2_image_name = $_;
	}
	elsif (/^custom2_part_mount=.*/){
		s/.*=//g;
		$custom2_part_mount = $_;
	}
	elsif (/^custom2_mount_type=.*/){
		s/.*=//g;
		$custom2_mount_type = $_;
	}
	elsif (/^addto_mount_partition=.*/){
		s/.*=//g;
		$addto_mount_partition = $_;
	}
	elsif (/^logo_part_size=.*/){
		s/.*=//g;
		$logo_part_size = $_;
		$logo_part_size = $logo_part_size * 32 * 1024;
	}
	elsif (/^flash_total_size=.*/){
		s/.*=//g;
		$flash_total_size = $_;
		$flash_total_size = $flash_total_size * 1024 * 1024;
	}
	elsif (/^image_dir=.*/){
		s/.*=//g;
		$image_dir = $_;
	}
	elsif (/^rootfs_dir=.*/){
		s/.*=//g;
		$rootfs_dir = $_;
	}
	elsif (/^rescuefs_dir=.*/){
		s/.*=//g;
		$rescuefs_dir = $_;
	}
	elsif (/^platform_name=.*/){
		s/.*=//g;
		$platform_name = $_;
	}
}

my $total_part_size=0;
$total_part_size = $etc_part_size + $storage_rootfs_size + $storage_jffs2_size + $kernel_part_size + $rootfsr_part_size + $hw_setting_size + $uboot_size + $uenv_size + $flash_layout_size + $nvram_user_size + $nvram_factory_size + $jffs2_part_size + $rescue_part_size + $logo_part_size + $custom1_part_size + $custom2_part_size;

if ($total_part_size > $flash_total_size) {die "\n ERROR: please reduce partition size, used size($total_part_size), flash size:($flash_total_size)\n";}
elsif ($total_part_size < $flash_total_size) {print "\n WARRNING: some flash space is free, used size:($total_part_size), flash size:($flash_total_size) \n";}

my $end_addr=0;
my $start_addr=0;
open oFILE, ">" . $target_file or die "Can't open '$target_file': $!";
#header
print oFILE "#flash-layout\n";
print oFILE "#type		start		end		location\n";

#hw-setting
$end_addr=$hw_setting_size - 1;
my $start_addr_str = sprintf("0x%x", $start_addr);
my $end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "hw-setting	$start_addr_str		$end_addr_str		$image_dir/../toolchain/image-tool/hw_setting.image.d\n";

#rootfs storage info
if ($platform_name eq "sn9866x") {
	$start_addr+=$hw_setting_size;
	$end_addr+=$storage_rootfs_size;
	$start_addr_str = sprintf("0x%x", $start_addr);
} 
else {
	$start_addr_str = sprintf("0x%x", $end_addr);
	$storage_rootfs_size = 0;
}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "rootfs-info	$start_addr_str		$end_addr_str		null\n";

#jffs2 storage info
if ($platform_name eq "sn9866x") {
	$start_addr+=$storage_rootfs_size;
	$end_addr+=$storage_jffs2_size;
	$start_addr_str = sprintf("0x%x", $start_addr);
}
else {
	$start_addr_str = sprintf("0x%x", $end_addr);
	$storage_jffs2_size = 0;
}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "jffs2-info	$start_addr_str		$end_addr_str		null\n";

#u-boot
if ($platform_name eq "sn9866x") {
	$start_addr+=$storage_jffs2_size;
	$end_addr+=$uboot_size;
}
else {
	$start_addr+=$hw_setting_size;
	$uboot_size = 0x7d000;
	$end_addr+=$uboot_size;
}
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "u-boot		$start_addr_str		$end_addr_str		$image_dir/../toolchain/image-tool/u_boot.image.d\n";

#u-env
$start_addr+=$uboot_size;
$end_addr+=$uenv_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "u-env		$start_addr_str		$end_addr_str		$image_dir/../toolchain/image-tool/u-boot-env.bin.d\n";

#flash layout
$start_addr+=$uenv_size;
$end_addr+=$flash_layout_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "flash-layout	$start_addr_str		$end_addr_str		$image_dir/../toolchain/image-tool/flash_layout.bin.d\n";

#factory
$start_addr+=$flash_layout_size;
$end_addr+=$nvram_factory_size;
if ($nvram_factory_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr);}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "factory		$start_addr_str		$end_addr_str		$image_dir/nvram.bin\n";

#kernel
$start_addr+=$nvram_factory_size;
$end_addr+=$kernel_part_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "kernel		$start_addr_str		$end_addr_str	$image_dir/KERNEL.bin\n";

#rootfs-r
$start_addr+=$kernel_part_size;
$end_addr+=$rootfsr_part_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "rootfs-r	$start_addr_str	$end_addr_str	$image_dir/ROOTFS-R.bin\n";

#rootfs-jffs2
$start_addr+=$rootfsr_part_size;
$end_addr+=$jffs2_part_size;
if ($jffs2_part_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr); $etc_mount_count ++;}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "jffs2-image	$start_addr_str	$end_addr_str	$image_dir/fs.jffs2\n";

#rescue
$start_addr+=$jffs2_part_size;
$end_addr+=$rescue_part_size;
if ($rescue_part_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr); $etc_mount_count ++;}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "rescue		$start_addr_str	$end_addr_str	$image_dir/RESCUE.bin\n";

#rootfs-rw
$start_addr+=$rescue_part_size;
$end_addr+=$etc_part_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "rootfs-rw	$start_addr_str	$end_addr_str	null\n";

#user
$start_addr+=$etc_part_size;
$end_addr+=$nvram_user_size;
if ($nvram_user_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr);}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "user		$start_addr_str	$end_addr_str	$image_dir/nvram.bin\n";

#logo
$start_addr+=$nvram_user_size;
$end_addr+=$logo_part_size;
if ($logo_part_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr);}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "ulogo		$start_addr_str	$end_addr_str	null\n";

#custom1
$start_addr+=$logo_part_size;
$end_addr+=$custom1_part_size;
if ($custom1_part_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr);}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
if ($custom1_image_name eq "null") {
	print oFILE "custom1		$start_addr_str	$end_addr_str	null\n";
}
else {
	print 	oFILE "custom1		$start_addr_str	$end_addr_str	$image_dir/$custom1_image_name\n";
}

#custom2
$start_addr+=$custom1_part_size;
$end_addr+=$custom2_part_size;
if ($custom2_part_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr);}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
if ($custom2_image_name eq "null") {
	print oFILE "custom2		$start_addr_str	$end_addr_str	null\n";
}
else {
	print 	oFILE "custom2		$start_addr_str	$end_addr_str	$image_dir/$custom2_image_name\n";
}

#end
print oFILE "add		0x00000000	0x00000000	null\n";
close oFILE;

# add mount line
my $mount_file="$rootfs_dir/root/etc_default/fstab";
if ($addto_mount_partition eq "yes") {
	if ( -e $mount_file) {
		open oFILE, ">>" . $mount_file or die "Can't open '$mount_file': $!";
	
		my $mount_mtd = 4;
		# jffs2
		if ($jffs2_part_mount eq "null") {}
		else {
			print oFILE "/dev/mtdblock$mount_mtd	$jffs2_part_mount		jffs2	defaults          0      0\n";
			$mount_mtd ++;
		}
		#rescue
		if ($rescue_part_size > 0) {$mount_mtd ++;}
		#user
		if ($nvram_user_size > 0) {$mount_mtd ++;}
		#custom1
		if ($custom1_part_mount eq "null") {
			if ($custom1_part_size > 0) {$mount_mtd ++;}
		}
		else {
			print oFILE "/dev/mtdblock$mount_mtd	$custom1_part_mount	$custom1_mount_type	defaults          0      0\n";
			$mount_mtd ++;
		}
		#custom2
		if ($custom2_part_mount eq "null") {}
		else {
			print oFILE "/dev/mtdblock$mount_mtd	$custom2_part_mount	$custom2_mount_type	defaults          0      0\n";
		}

		close oFILE;
	}
	else {
		die "$mount_file is not exist!"
	}
}

my $rootfs_linuxrc="$rootfs_dir/linuxrc";
if ( -e $rootfs_linuxrc) {
	system ("sed -i 's/mtd[1-5]/mtd$etc_mount_count/g' $rootfs_linuxrc");
	system ("sed -i 's/mtdblock[1-5]/mtdblock$etc_mount_count/g' $rootfs_linuxrc");
}
else {
	die "$rootfs_linuxrc is not exist!"
}

if ($rescue_part_size > 0) {
	my $rescue_init="$rescuefs_dir/init";
	if ( -e $rescue_init) {
		system ("sed -i 's/mtd[1-5]/mtd$etc_mount_count/g' $rescue_init");
		system ("sed -i 's/mtdblock[1-5]/mtdblock$etc_mount_count/g' $rescue_init");
	}
	else {
		die "$rescue_init is not exist!"
	}
}





