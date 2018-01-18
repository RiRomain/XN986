#!/usr/bin/perl -w
use strict;

my $outir = "";
my $phy_flashlayout = "";
my $src_dir = "";
my $physf_command ="./src/phy_sf";
my $sdk_version = "";

foreach (@ARGV){
	
	if (/^outir=.*/){
		s/.*=//g;
		$outir = $_;
	}
	elsif (/^phy_flashlayout=.*/){
		s/.*=//g;
		$phy_flashlayout = $_;
	}
	elsif (/^src_dir=.*/){
		s/.*=//g;
		$src_dir = $_;
	}
	elsif (/^sdk_version=.*/){
		s/.*=//g;
		$sdk_version = $_;
	}
	else{
		print $_ , "\n";
		print "the image parameter is wrong!\n";
	}	
}

system ("make -C ./src/");
print "execute tool:\n";
system("cd ./src/ && ./phy_sf -l $phy_flashlayout -o $outir -s $src_dir");



open oFILE, ">>" . "$outir/phy_sf_report.txt" or die "Can't open : $!";
print oFILE "\nVersion: \n";
print oFILE "$sdk_version";
close oFILE
