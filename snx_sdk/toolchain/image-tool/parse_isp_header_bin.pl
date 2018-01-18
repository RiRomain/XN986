#!/usr/bin/perl
use strict;
use Text::CSV;



use warnings;
#define config include file & struct
my $isp_header_file = "snx_isp_iq.h";
my $isp_value_bin_file = "snx_isp_iq.bin";
my $ips_cvs_file = "ispconfig.csv";
my $StructName = "StructName";


my $UINT32 = "unsigned int";
my $UINT32_ = "UINT32";
my $INT32 = "int";
my $INT32_ = "INT32";
my $CHAR = "char";
my $UINT8 = "UINT8";
my $UINT16 = "UINT16";
my $INT16 = "INT16";
my @struct_name;
my @member_name;
my $value = 0x0;
my $ifheader = ".h";
my $ifbin = ".bin";
my $TXT_FILE_NAME = "isp.txt";

my $csv = Text::CSV->new ({binary => 1,eol => "\r\n"})
	or die "Cannot use CSV: ".Text::CSV->error_diag ();
  
die "You must provide a filename to $0 to be parsed as an Excel file" unless @ARGV;  
if ($ARGV[0] =~ /$ifheader/){
  $isp_header_file = $ARGV[0];
  if($ARGV[1] =~ /$ifbin/){
    $isp_value_bin_file = $ARGV[1];
  }
  else{
    die "You must provide a filename to $0 to be parsed as an Excel file";
  }     
}
elsif ($ARGV[0] =~ /$ifbin/){
  $isp_value_bin_file = $ARGV[0];
  if($ARGV[1] =~ /$ifbin/){
    $isp_header_file = $ARGV[1];
  }
  else{
    die "You must provide a filename to $0 to be parsed as an Excel file";
  }     
}
else {
  die "You must provide a filename to $0 to be parsed as an Excel file";
} 
    
open(oFILE, $isp_header_file) || die("Cannot Open $isp_header_file");
my @config_raw = <oFILE>;
close(oFILE);

#if bin
my $filesize = -s $isp_value_bin_file;
open(oFILE,$isp_value_bin_file) || die("Cannot Open File");
binmode(oFILE);
my $text;
read (oFILE, $text, $filesize);
my @values_raw =  unpack("C*", $text);
close(oFILE);
#endif


#foreach (@config_raw){print "=";print;}
open my $fh, ">", $ips_cvs_file or die "$ips_cvs_file $!";
foreach (@config_raw) {
  if ($_ =~ /struct/){
		my @tml = split /struct/, $_;
    $tml[1] =~ s/\s+//g;
		push @struct_name, $tml[1];
	}
}
### open isp.txt ###
open oHEADER, ">" . $TXT_FILE_NAME;  

my $di = 0;
#foreach (@sc_name){print "=";print;}
foreach (@struct_name){
  printf oHEADER ("%-20s%-15s\r\n"),"#struct_name",$_;
  printf oHEADER ("%-20s%-15s%-15s%-15s\r\n"),"#member_vars","#member_type","#member_size","#member_val";
	my $this_struct = $_;
  my $this_struct_enb = 0;
  my @rdat;
  
  my $string;
  my $byte_number = 4;
  foreach (@config_raw) {
	if ($_ =~ $this_struct) {
    $this_struct_enb = 1;
    

    $csv->print($fh,[$StructName,$this_struct]);
#    $csv->print($fh,$this_struct); 
  }
  
  if($this_struct_enb == 1) {
    if(($_ =~ /$UINT32/)){
      @member_name = &scmember($_ , $UINT32); 
      $UINT32_ =~ s/\s+//g;
      # if two way array
      if ($_ =~ /\[(\d+)\]\[(\d+)\]/){ 
        my $two_array = "$member_name[1]x$member_name[2]";
        my $number = $member_name[1]*$member_name[2];
        print $number;
        @rdat = &get_val_string ($di ,$UINT32 ,$number,@values_raw);
        $string = join('',@rdat);
        #print $string;
        $csv->print($fh,[$member_name[0],$UINT32_,$two_array,$string]);
        #print oHEADER "$member_name[0]\t$UINT32_\t$two_array\t$string\r\n";
        printf oHEADER ("%-20s%-15s%-15s%-15s\r\n" ,$member_name[0],$UINT32_,$two_array,$string);
        $di = $di + $number*$byte_number;
      }
      else {
        @rdat = &get_val_string ($di ,$UINT32 ,$member_name[1],@values_raw);
        $string = join('',@rdat);       
        $csv->print($fh,[$member_name[0],$UINT32_,$member_name[1],$string]);
        #print oHEADER "$member_name[0]\t$UINT32_\t$member_name[1]\t$string\r\n";
        printf oHEADER ("%-20s%-15s%-15s%-15s\r\n" ,$member_name[0],$UINT32_,$member_name[1],$string);
        $di = $di + $member_name[1]*$byte_number;
      }
        

      #print "#"; foreach (@member_name){print;print",";}
      #print "\n";
    }
    elsif (($_ =~ /$INT32/)){
      @member_name = &scmember($_ , $INT32); 
      $INT32_ =~ s/\s+//g;
            # if two way array
      # if two way array
      if ($_ =~ /\[(\d+)\]\[(\d+)\]/){ 
        my $two_array = "$member_name[1]x$member_name[2]";
        my $number = $member_name[1]*$member_name[2];
        @rdat = &get_val_string ($di ,$INT32 ,$member_name[1],@values_raw);
        $string = join('',@rdat);
        $csv->print($fh,[$member_name[0],$INT32_,$two_array,$string]);
        #print oHEADER "$member_name[0]     $INT32_     $two_array     $string\r\n";
        printf oHEADER ("%-20s%-15s%-15s%-15s\r\n" ,$member_name[0],$INT32_,$two_array,$string);
        $di = $di + $number*$byte_number;
      }
      else {

          @rdat = &get_val_string ($di ,$INT32 ,$member_name[1],@values_raw);
          $string = join('',@rdat); 
        
        $csv->print($fh,[$member_name[0],$INT32_,$member_name[1],$string]);
       #print oHEADER "$member_name[0]     $INT32     $member_name[1]     $string\r\n";
        printf oHEADER ("%-20s%-15s%-15s%-15s\r\n" ,$member_name[0],$INT32_,$member_name[1],$string);
        $di = $di + $member_name[1]*$byte_number;
      } 
  }
  if ($_ =~ /};/) {$this_struct_enb = 0; } 
  }
  }
  print oHEADER "===\r\n";       
}
#alek add for if not use ispconfig.csv 
system ("rm $ips_cvs_file"); 
                            

close $fh;


# sub function
sub scmember
{
	my $string = $_[0];
	my $type = $_[1];

	my @tname_1 = split($type , $string);
  # for [][];
  if ($tname_1[1] =~ /\[(\d+)\]\[(\d+)\]/) {
    $tname_1[1] =~ s/\s+//g;
	  $tname_1[1] =~ s/;//g;
	  $tname_1[1] =~ s/]//g;
    #print $tname_1[1]; 
	  my @tname_2 = split(/\[*\[/ , $tname_1[1]);
    #$tname_2[1] = "$tname_2[1]x$tname_2[2]";
    @tname_2;  
   #print "##"; foreach (@tname_2){print;print",";}
   #print "\n";
  }
  else {
    if($tname_1[1] =~ /\[/) {
  	 $tname_1[1] =~ s/\s+//g;
  	 $tname_1[1] =~ s/;//g;
  	 $tname_1[1] =~ s/]//g;
  	 my @tname_2 = split(/\[/ , $tname_1[1]);
  #   print "##"; foreach (@tname_2){print;print",";}
  #   print "\n";
  	 @tname_2;
     
    }
    else {    #if no array
      $tname_1[1] =~ s/\s+//g;
      my @tname_2 = split(/\;/ , $tname_1[1]);    
      $tname_2[1] = 1;
  #    print "#"; foreach (@tname_2){print;print",";}
  #    print "\n";
      @tname_2;
    }
  }
}


sub get_val_string
{
	my (
		$indx,
		$type,
		$size,
		@hda
		) = @_;
	my $every_value_end;	
  if($size == 1){
    $every_value_end = '';
  }
  else{
    $every_value_end = ',';  
  }
     
	my @value;
  my $byte_number = 4;
  if (($type eq $CHAR) || ($type eq $UINT8)){
    $byte_number = 1;  
  }
  elsif (($type eq $UINT16) || ($type eq $INT16)){
    $byte_number = 2; 
  }
  elsif (($type eq $UINT32) || ($type eq $INT32)){
     $byte_number = 4;
  }      
	my $indx_end = $indx + $size*$byte_number;
	
	
	if ($type eq $CHAR){
		for (my $i = $indx; $i < $indx_end ; $i++){
			push @value ,(sprintf("%c",$hda[$i]));
		}
	}
	elsif ($type eq $UINT8){
		for (my $i = $indx; $i < $indx_end ; $i++){
			push @value ,(sprintf("0x%02x$every_value_end",$hda[$i]));
		}
	}
	elsif (($type eq $UINT16) || ($type eq $INT16)) {
		for (my $i = $indx; $i < $indx_end ; $i = $i + 2){
			push @value ,(sprintf("0x%02x%02x$every_value_end",$hda[$i+1] ,$hda[$i]));
		}
	}
	elsif (($type eq $UINT32) || ($type eq $INT32)){
		for (my $i = $indx; $i < $indx_end ; $i = $i + 4){
			push @value ,(sprintf("0x%02x%02x%02x%02x$every_value_end",$hda[$i+3] ,$hda[$i+2],$hda[$i+1],$hda[$i]));
		}
	}
	else{
		print "ERROR !!\n"
	}
#	foreach (@value){print;print"\n";}
	@value;
	
}