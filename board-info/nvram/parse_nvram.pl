#!/usr/bin/perl -w
use strict;
#use Spreadsheet::ParseExcel;
#use Digest::MD5;
#use Digest::CRC;
use Encode;
use Encode::Detect::Detector;


# get time
my	$NOW_TIME;	
my  @time_arr;
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
my	$CHAR_STR = "char";
my	$U8_STR = "uint8_t";
my	$U16_STR = "uint16_t";	
my	$U32_STR = "uint32_t";
my	$I32_STR = "int32_t";

#nvram related
my $nvram_bin_file = "nvram.bin";               
my $factory_bin_file = "factory.bin";
my $data_bin_file = "data.bin";

my $nvram_header_file;

my $all_tot_size = 0;
my $nvram_md5;
my $zero_num= 0;
my $inter_ff="";
my $envs_tack = "\0";
my $ori_file = "";
my $configuration_name = "configuration";
my $sdk_version = "";
my $fileziped = "y";
my $fileziped_evl = "";


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
################## abstract @ARGV ##############
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

######get parameters##########
foreach (@ARGV){
	if (/^orifile=.*/){
		s/.*=//g;
		$ori_file = $_;
	}
	elsif (/^fileziped=.*/){
		s/.*=//g;
		$fileziped = $_;
	}
	elsif (/^fileziped_evl=.*/){
		s/.*=//g;
		$fileziped_evl = "-".$_;
	}
	elsif (/^sdk_version=.*/){
		s/.*=//g;
		$sdk_version = $_;
	}
	elsif (/^nvram_bin=.*/){
		s/.*=//g;
		$nvram_bin_file = $_;
	}
	elsif (/^factory_bin=.*/){
		s/.*=//g;
		$factory_bin_file = $_;
	}
	elsif (/^data_bin=.*/){
		s/.*=//g;
		$data_bin_file = $_;
	}

}




######VERSION##########
my $nvram_ver = $sdk_version.$envs_tack ;
my $nvram_ver_size = length($nvram_ver);

################## main body  ##############
#&fun_nvram( $ori_file);
&fun_rtso_nvram($ori_file);

################## Add rtos sub function ##############
sub fun_rtso_nvram
{
	my $ori_name = $_[0];
	my @Packheaderarea;
	my @Configheaderarea;
	my @Datafilearea;

	open(oFILE,$ori_name) || die("Cannot Open $ori_name: $!");
	my @nvram_data = <oFILE>;
	close(oFILE);

	#print @nvram_data;

	# Pack header area
	my $pack_header_size = 8*1024;
	my $pack_header_area_d = "pack_header_area.d";
	open oHEADER, ">" . $pack_header_area_d;

	my $PkgID_Cnt = 1;
	foreach(@nvram_data) {
		if (/^\/\//){
			next;
		}
		if (/^===/) {			
			my $str = $_;
#			$str =~ s/\s+//g;
			$str =~ s/\r\n//g; # remove dos-format EOL
			$str =~ s/\n//g; # remove unix-format EOL

			my @pack_member = split(/\,/, substr($str,3,length($str) - 3));
			my @member_raw_data = ();

			my $CRC16 = 0; #2
			my $cal_crc16_size = 0;
			my $PkgMode = 0xFD; #1
			my $PkgNameLength; #1
			my $PkgID; #4
			my $PkgName; #N


			if ($pack_member[1] eq "Y") { $PkgMode &= 0xF7 }
			if ($pack_member[2] eq "Y") { $PkgMode &= 0xFB }

			$PkgNameLength = length($pack_member[0]);
			$PkgID = $PkgID_Cnt;
			$PkgName = $pack_member[0];

			my $tmp_pack_member_d = "f_pack_member.d";

			open oFILE, ">" . $tmp_pack_member_d;
			print oFILE pack('H2',sprintf("%2x", $PkgMode));
			print oFILE pack('H2',sprintf("%2x", $PkgNameLength));
			print oFILE pack('L', $PkgID);
			print oFILE $PkgName;
			close oFILE;


			### Attach CRC16 [16-bytes] ###
			my $crc16;
			my @message;
			my @message_c;
			my $message_bytes = -s $tmp_pack_member_d;
			my $count=$message_bytes;
			my $index_message=0;
	
			open(GIF, "$tmp_pack_member_d") or die "can't open '$tmp_pack_member_d': $!";
			binmode(GIF);
			while ($count > 0){
				$count--;
				read(GIF, $message[$index_message], 1);
				$message_c[$index_message] = unpack("C",$message[$index_message]);
				$index_message++;
			}
			close GIF;
	
			&crcSlow(
				\$crc16,
				\@crc16_tab, 
				\@message_c,       
				\$message_bytes
 			);


 			print oHEADER pack('S',$crc16);
 			print oHEADER @message;

			$PkgID_Cnt++;
		}
	}

	close oHEADER;
	
	#  Padding to pack_header_size
	my $pack_header_area_d_size = -s $pack_header_area_d;
	open oHEADER, ">>" . $pack_header_area_d or die "Can't open '$pack_header_area_d': $!";
		my $pack_header_pad = $pack_header_size - $pack_header_area_d_size;
		for (my $i=0;
			$i < $pack_header_pad;
			$i++){
			print oHEADER pack('H2', 'ff');
		}
	close oHEADER;


	#  Config header area
	my $config_header_area_d = "config_header_area.d";
	open oCONFIG, ">" . $config_header_area_d;

	my $config_header_usrarea_d = "config_header_usrarea_d.d";
	open oUSRCONFIG, ">" . $config_header_usrarea_d;



	my $file_data_area_d = "file_data_area_d.d";
	open oDATA, ">" . $file_data_area_d;


	my $cPkgID_Cnt = 0;
	foreach(@nvram_data) {
		if (/^\/\//){
			next;
		}
		if (/^===/) {			
			$cPkgID_Cnt++;
		}
		elsif (/^-/) {
			my $cstr = $_;
#			$cstr =~ s/\s+//g;
			$cstr =~ s/\r\n//g; # remove dos-format EOL
			$cstr =~ s/\n//g; # remove unix-format EOL

			my @cpack_member = split(/\,/, substr($cstr,1,length($cstr)));
			my @cmember_raw_data = ();
			my $CfgMode = 0xFD;
			my $CfgNameLength = length($cpack_member[0]);
			my $cPkgID = $cPkgID_Cnt;
			my $CfgDataLength;
			my $CfgName = $cpack_member[0];
			my $CfgData;
			my @CfgData_f;

			if ($cpack_member[2] eq "0") { 	#file data
				my $gzipfile = $cpack_member[0].".gz".".d";

				if ($fileziped eq "y")	{
					system ("gzip -c $fileziped_evl $cpack_member[0] > $gzipfile");
					open GIF, $gzipfile or die "Couldn't open file: $!";
				}
				else {
					open GIF, $cpack_member[0] or die "Couldn't open file: $!";
				}

  				binmode GIF;
  	 		 	@CfgData_f = <GIF>;
  	 		 	close GIF;

				if ($fileziped eq "y")	{
  	 		 		$CfgDataLength = -s $gzipfile;
  	 		 	}
  	 		 	else {
  	 		 		$CfgDataLength = -s $cpack_member[0];
  	 		 	}
  	 		 	$CfgDataLength += 0x10000000;
			}
			else{ #not file data
				if ($cpack_member[0] eq "version") {
						$CfgDataLength = $cpack_member[2];
						$CfgDataLength += 0x0;

						$CfgDataLength = $nvram_ver_size;
						$CfgData = $nvram_ver;
					}
					else{
						$CfgDataLength = $cpack_member[2];
						
						if ($cpack_member[1] eq "char") {
							$CfgDataLength += 0x0;
							$CfgData = $cpack_member[3];
						}
						elsif ($cpack_member[1] eq "unsigned char") {
							$CfgDataLength += 0x50000000;
							$CfgData = pack('H*',$cpack_member[3]);
						}
						elsif ($cpack_member[1] eq "int") {
							$CfgDataLength += 0x20000000;
							$CfgData = pack('i', $cpack_member[3]);
						}
						elsif ($cpack_member[1] eq "unsigned int") {
							$CfgDataLength += 0x40000000;
							$CfgData = pack('I', $cpack_member[3]);
						}
						elsif ($cpack_member[1] eq "float") {
							$CfgDataLength += 0x30000000;
							$CfgData = pack('f', $cpack_member[3]);
						}
						else{ #unknow
							$CfgData = $cpack_member[3];
							print "XXXXX nvram unknow data type xxxxxxx\n";
						}
					}
			}


			my $tmp_config_member_d = "f_config_member.d";

			open oFILE, ">" . $tmp_config_member_d;
			print oFILE pack('H2',sprintf("%2x", $CfgMode));
			print oFILE pack('H2',sprintf("%2x", $CfgNameLength));
			print oFILE pack('L', $cPkgID);
			print oFILE pack('L', $CfgDataLength);
			print oFILE $CfgName;

			if ($cpack_member[2] eq "0") {
				print oFILE @CfgData_f;
			}
			else {
				print oFILE $CfgData;
			}
			
			close oFILE;


			### Attach CRC16 [16-bytes] ###
			my $ccrc16;
			my @cmessage;
			my @message_c;
			my $cmessage_bytes = -s $tmp_config_member_d;
			my $ccount=$cmessage_bytes;
			my $cindex_message=0;
	
			open(GIF, "$tmp_config_member_d") or die "can't open '$tmp_config_member_d': $!";
			binmode(GIF);
			while ($ccount > 0){
				$ccount--;
				read(GIF, $cmessage[$cindex_message], 1);
				$message_c[$cindex_message] = unpack("C",$cmessage[$cindex_message]);
				$cindex_message++;
			}
			close GIF;
	
			&crcSlow(
				\$ccrc16,
				\@crc16_tab, 
				\@message_c,       
				\$cmessage_bytes
 			);

 			if ($cpack_member[2] ne "0") {  # not file 
 				print oUSRCONFIG pack('S',$ccrc16);
 				print oUSRCONFIG pack('H2',sprintf("%2x", $CfgMode));
				print oUSRCONFIG pack('H2',sprintf("%2x", $CfgNameLength));
				print oUSRCONFIG pack('L', $cPkgID);
				print oUSRCONFIG pack('L', $CfgDataLength);
				print oUSRCONFIG $CfgName;
				print oUSRCONFIG $CfgData;

				print oCONFIG pack('S',$ccrc16);
 				print oCONFIG pack('H2',sprintf("%2x", $CfgMode));
				print oCONFIG pack('H2',sprintf("%2x", $CfgNameLength));
				print oCONFIG pack('L', $cPkgID);
				print oCONFIG pack('L', $CfgDataLength);
				print oCONFIG $CfgName;
				print oCONFIG $CfgData;	
 			}
 			else { # file
 				print oDATA pack('S',$ccrc16);
 				print oDATA pack('H2',sprintf("%2x", $CfgMode));
				print oDATA pack('H2',sprintf("%2x", $CfgNameLength));
				print oDATA pack('L', $cPkgID);
				print oDATA pack('L', $CfgDataLength);
				print oDATA $CfgName;
				print oDATA @CfgData_f;

				if ($cpack_member[4] eq "1") {
					print oCONFIG pack('S',$ccrc16);
 					print oCONFIG pack('H2',sprintf("%2x", $CfgMode));
					print oCONFIG pack('H2',sprintf("%2x", $CfgNameLength));
					print oCONFIG pack('L', $cPkgID);
					print oCONFIG pack('L', $CfgDataLength);
					print oCONFIG $CfgName;
					print oCONFIG @CfgData_f;		
				}
				else { #only prefix to factory
					print oCONFIG pack('S',$ccrc16);
 					print oCONFIG pack('H2',sprintf("%2x", $CfgMode));
					print oCONFIG pack('H2',sprintf("%2x", $CfgNameLength));
					print oCONFIG pack('L', $cPkgID);
					print oCONFIG pack('L', $CfgDataLength);
					print oCONFIG $CfgName;
				}
 			}
		}
	}

	close oCONFIG;
	close oUSRCONFIG;
	close oDATA;

	##### Dump ####
	my @packdata;
	my @configdata;
	my @userconfigdata;
	my @filedata;


	open GIF, $pack_header_area_d or die "Couldn't open file: $!";
  	binmode GIF;
  	@packdata = <GIF>;
  	close GIF;

	open GIF, $config_header_area_d or die "Couldn't open file: $!";
  	binmode GIF;
  	@configdata = <GIF>;
  	close GIF;

 	open GIF, $config_header_usrarea_d or die "Couldn't open file: $!";
  	binmode GIF;
  	@userconfigdata = <GIF>;
  	close GIF;
  	
  	open GIF, $file_data_area_d or die "Couldn't open file: $!";
  	binmode GIF;
  	@filedata = <GIF>;
  	close GIF; 	

	open ofFILE, ">" . $factory_bin_file or die "Can't open '$factory_bin_file': $!";
	print ofFILE @packdata;
	print ofFILE @configdata;
	close ofFILE;

	open ouFILE, ">" . $nvram_bin_file or die "Can't open '$nvram_bin_file': $!";
	print ouFILE @packdata;
	print ouFILE @userconfigdata;
	close ouFILE;

	open odFILE, ">" . $data_bin_file or die "Can't open '$data_bin_file': $!";
	print odFILE @filedata;
	close odFILE;
}



################## sub function ##############
sub fun_nvram
{
	my $ori_name = $_[0];	
	
	my $nvram_id = "0x4dae7f6c";
	
	my $flag_struct = "===";
	my $flag_variable = "-";
	my $flag_interval = ",,,";
	
	open(oFILE,$ori_name) || die("Cannot Open $ori_name: $!");
	my @nvram_data = <oFILE>;
	close(oFILE);
	
	my @all_config_struct_name;
	my @all_config_struct_size;
	
  my @all_config_member_vars;
  my @all_config_member_type;
  my @all_config_member_size;
  my @all_config_member_val;
  
  my $all_total_struct;
  my $all_total_member;
	
	my $struct_num = 0;
	my $struct_name = "";
	my $struct_variable = "";
	my $first_enter = "yes";
	foreach(@nvram_data){
		if($_ =~ /^\s+/){
			next;
		}
		
		my $this_line = $_;
		
		if($this_line =~ /^===/){
			$struct_name = $this_line;
			$struct_name =~ s/^===//;
			$struct_name =~ s/\s+$//;
			push(@all_config_struct_name,$struct_name);
			push(@all_config_struct_name,$struct_num);
			if ($first_enter =~ /^no$/i){
				push(@all_config_struct_size,$struct_num);
			}
			$struct_num = 0;
			$first_enter = "no";
		}
		elsif($this_line =~ /^-/){
			$struct_variable = $this_line;
			$struct_variable =~ s/^-//;
			$struct_variable =~ s/\s+$//;
      $struct_num ++;
      my @array = split /,,,/,$struct_variable;
      push(@all_config_member_vars,$array[0]);
      push(@all_config_member_type,$array[1]);
      push(@all_config_member_size,$array[2]);
      push(@all_config_member_val,$array[3]);
		}
	}
	push(@all_config_struct_size,$struct_num);
	  
	  
	 #foreach(@all_config_struct_name){print $_."\n";}
	 #foreach(@all_config_struct_size){print $_."\n";}
	 #foreach(@all_config_member_type){print $_."\n";}
	 #foreach(@all_config_member_size){print $_."\n";}	  
	 #foreach(@all_config_member_val){print $_."\n";}	 
	 #foreach(@gpio_raw_data){print $_."\n";}	
	 #foreach(@all_config_struct_name_ori){print $_."\n";}
	 #foreach(@all_config_member_vars){print $_."\n";}
	 
		my @all_config_struct_name_ori = @all_config_struct_name;
	  foreach (@all_config_struct_name)	{
	  	if ($_ =~ /[a-zA-Z]/){
	  		s/\./__POINT__/g;
	  		s/\//__SKEWLINE__/g;
	  		s/\-/__SHORTLINE__/g;
	  	}
	  }
	  

	  my @gpio_raw_data = ();
		&parse_config_to_bin_file(
			\$nvram_bin_file, 
			\$nvram_id,
			\@all_config_struct_name, 
			\@all_config_struct_size,       
			\@all_config_member_type,
			\@all_config_member_size,
			\@all_config_member_val,
			\$all_tot_size,
			\@gpio_raw_data,
			\$configuration_name,
			\$nvram_ver,
			\$nvram_md5
		);	

		my $gpio_actual_num = 0;
		&parse_config_to_header_file (
		  \$zero_num,
			\$nvram_header_file,
			\$nvram_id,
			\@all_config_struct_name_ori, #@
			\$gpio_actual_num,
			\@all_config_struct_name,			#@
			\@all_config_struct_size,			#@  
			\@all_config_member_vars,			#@
			\@all_config_member_type,			#@
			\@all_config_member_size,			#@
			\@all_config_member_val,			#@				
		);
	 
	 
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

sub parse_config_to_bin_file
{				
	my (
			$file, 
			$file_id,
			$file_struct_name, 	#@
			$file_struct_size,	#@
			$file_member_type,	#@
			$file_member_size,	#@
			$file_member_val,	#@                       
			$file_tot_size,
			$gpio_raw_data,		#@
			$name,
			$version,
			$file_md5
		) = @_;	                                                  	
	
		### file.bin ###
		open oFILE, ">" . $$file;
		### --- nvram id -----------
		my	@p32u_id = split(/x/, $$file_id);
		my $ff32u_id = sprintf "%08d", hex($p32u_id[1]);
		print oFILE pack('L', $ff32u_id);
		
		my $str = pack("a64", $$version); 
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
		
		### --- body -----------
		my $mb_i = 0;
		my $sc_size = 0;
		my $ser_num = 0;  
		my $two_dimension;                           
			foreach (@$file_struct_name){               
				if ($_ =~ /[a-zA-Z]/){
					my $ssz = $$file_struct_size[$mb_i];        
					for (my $si=0;
					$si < $ssz;
					$si++){
						my $type = $$file_member_type[$ser_num];
						my $size = $$file_member_size[$ser_num];
						my $val	 = $$file_member_val[$ser_num];	
            $ser_num += 1;
            
            if($size =~ /[0-9]x[0-9]/i){
						  my @two_multi = split(/x/i, $size);
							$two_dimension = $two_multi[0] * $two_multi[1];
							$size = $two_dimension;
							}


#						if ($type =~ /$CHAR/i){
#							$val =~ s/^"//;
#							$val =~ s/"$//;
#							my $ctl = "a".$size;
#							my $str = pack $ctl, $val; 
#							foreach(unpack("(a1)*", $str)) {
#							print oFILE;
#							}
#						
#							$sc_size = $sc_size + 1*$size;
#						}

						if ($type =~ /$CHAR/i){
							$val =~ s/^"//;
							$val =~ s/"$//;
							#################################
              Encode::_utf8_off($val);
              my $kind = "ok";
              $kind = detect($val);
              $kind = $kind."sonix";

              ###kind 
              if ($kind =~ /^UTF\-8/){
              	my $this_size = length($val);
              	my $line = decode("UTF-8", $val);
              	my $this_utf8 = length($line);
              	my $diff_this = $this_size - $this_utf8;
              	$diff_this = $diff_this / 2;
              	my @chars=split//,$line;
              	my $size_what = 0;
								foreach my $char(@chars){
									my $just_len=length ($_);
									$size_what ++; 
									print oFILE encode("gbk",$char);
								} 
								my $add_size = $size - $this_size + $diff_this;
								while ($add_size > 0){
										$add_size --;
										my $add_str = sprintf("%s", "00");
										my $str_done = pack("a2", $add_str); 
										print oFILE pack('H*',$str_done);
								}
              }
              ### kind
              else{
	              my $ctl = "a".$size;
								my $str = pack $ctl, $val; 
	              foreach(unpack("(a1)*", $str)) {
									print oFILE;
								}
              }
              ##############################
							$sc_size = $sc_size + 1*$size;
						}
						
						
						elsif ($type =~ /$UINT8/i){
							$val =~ s/\s+//g;
							$val =~ s/"//g;
							my	@tmp8u = split(/,/, $val);
							my	$u8_size = scalar @tmp8u;
							foreach (@tmp8u){
								my	@p8u = split(/x/, $_);
								my	$ff8u = sprintf "%02x", hex($p8u[1]);
								print oFILE pack('H2', $ff8u);
							}
							
							#zeroing to size
							for (my $i=0;
								$i < ($size - $u8_size);
								$i++){
									print oFILE pack('H2', '00');
								}
								
							$sc_size = $sc_size + 1*$size;
						}
						elsif ($type =~ /$UINT16/i){
							$val =~ s/\s+//g;
							$val =~ s/"//g;
							my	@tmp16u = split(/,/, $val);
							my $u16_size = scalar @tmp16u;
				
							foreach (@tmp16u){
								my	@p16u = split(/x/, $_);
								my $ff16u = sprintf "%04d", hex($p16u[1]);
								print oFILE pack('S', $ff16u);
							}
							
							##zeroing to size
							for (my $i=0;
								$i < ($size - $u16_size);
								$i++){
									print oFILE pack('S',0);
								}
								
							$sc_size = $sc_size + 2*$size;	
						}
						elsif (($type =~ /$UINT32/i)|| ($type =~ /$INT32/i)){
							$val =~ s/\s+//g;
							$val =~ s/"//g;
							my	@tmp32u = split(/,/, $val);
							my $u32_size = scalar @tmp32u;
				
							foreach (@tmp32u){
								my	@p32u = split(/x/, $_);
								my $ff32u = sprintf "%08d", hex($p32u[1]);
								print oFILE pack('L', $ff32u);
							}
							
							##zeroing to size
							for (my $i=0;
								$i < ($size - $u32_size);
								$i++){
									print oFILE pack('L',0);
								}
							
							$sc_size = $sc_size + 4*$size;	
						}	
					}
					$mb_i = $mb_i + 1;
				}
			}	
			$$file_tot_size = $sc_size + 4 + 64;	#body + id + version

		close oFILE;

###calculate internel crc16
    ###step 16 times
	  my $what_large_inter = -s $$file;
	  until ($what_large_inter < 16){
		$what_large_inter -= 16;
    } 
    unless ($what_large_inter == 0){
	  my $how_add_inter = 16 - $what_large_inter;
	  $inter_ff=$how_add_inter;
	  $$file_tot_size += $how_add_inter;	
    open(oFILE, ">>$$file");	# Open for appending
		while ($how_add_inter > 0){
			$how_add_inter --;
			my $add_str = sprintf("%s", "FF");
			my $str_done = pack("a2", $add_str); 
			print oFILE pack('H*',$str_done);
		}
		close oFILE;  
  }

		###crc16
	  my $crc16_inter;
		my @message_inter;
		my $message_bytes_inter = -s $$file;
		my $count_inter=$message_bytes_inter;
		my $index_message_inter=0;
	
		open(GIFI, $$file) or die "can't open '$$file': $!";
		binmode(GIFI);
		while ($count_inter > 0){
			$count_inter--;
			read(GIFI, $message_inter[$index_message_inter], 1);
			$message_inter[$index_message_inter] = unpack("C",$message_inter[$index_message_inter]);
			$index_message_inter++;
		}
		close GIFI;
		&crcSlow(
			\$crc16_inter,
			\@crc16_tab, 
			\@message_inter,       
			\$message_bytes_inter
 		);
###calculate internel crc16 end


	  #make $file to 16times large
	  my $what_large = -s $$file;
	  $what_large = $what_large + 2;
	  until ($what_large < 16){
		$what_large -= 16;
    } 
    unless ($what_large == 0){
	  my $how_add = 16 - $what_large;
	  $zero_num=$how_add;
	  $$file_tot_size += $how_add;	
    open(oFILE, ">>$$file");	# Open for appending
		while ($how_add > 0){
			$how_add --;
			my $add_str = sprintf("%s", "FF");
			my $str_done = pack("a2", $add_str); 
			print oFILE pack('H*',$str_done);
		}
		close oFILE;  
  }
  ###########	
		
		### Attach crc32 in struct
#		my $filebuff;
	
#		my $filesize = -s $$file;
#		open(oFILE, $$file) or die "Can't open '$file': $!";
#		read(oFILE, $filebuff, $filesize , 0);
#		close oFILE;
		
#		my $crc = Digest::CRC->new( type => "crc32" );
#		$crc->add($filebuff);
#		my $crc32 = $crc->hexdigest();
#		print "nvram.bin crc32: $crc32\n";	

#		open(oFILE, ">>$$file");	# Open for appending CRC32
#		my $ff32u_crc32 = sprintf "%08d", hex($crc32);
#		print oFILE pack('L', $ff32u_crc32);
#		close oFILE;
		
 		open(GIFI, ">>$$file");	# Open for appending	
 		print GIFI pack('S',$crc16_inter);
		close GIFI;
		$$file_tot_size += 2;
				
		### Calculate Attach MD5 [16-bytes] ###
		#open(oFILE, $$file) or die "Can't open '$$file': $!";
		#binmode(oFILE);
		#$$file_md5 = Digest::MD5->new->addfile(*oFILE)->hexdigest;
		#print "$$file : $$file_md5\n";		
		#close oFILE;
		
	  ### Attach CRC16 [16-bytes] ###
	  my $crc16;
		my @message;
		my $message_bytes = -s $$file;
		my $count=$message_bytes;
		my $index_message=0;
	
		open(GIF, $$file) or die "can't open '$$file': $!";
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
		
		####### Attach Size &(body) & Name & Version & Time & MD5 #######
		open(oFILE,$$file) || die("Cannot Open File");
		my @file_raw = <oFILE>;
		close(oFILE);
		
		open oFILE, ">" . $$file;
		print oFILE pack('L', $$file_tot_size);
		print oFILE @file_raw;
		close (oFILE);
		
		open(oFILE, ">>$$file");	# Open for appending	
		$str = pack( "a16", $$name); 
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
		
		$str = pack("a64", $$version); 
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
			
		#my	$NOW_TIME = &gettime;	
		$str = pack("a16", $NOW_TIME); 
		foreach(unpack("(a1)*", $str)) {
			print oFILE;
		}
		#print oFILE pack('H*',$$file_md5);
		###############################
#		$crc16 = sprintf("%s", $crc16);
		#print "haha3:---$crc16---";
#	  $str = pack("a8", $crc16); 
	  #print "haha4:---$str---";
#	  print oFILE pack('H*',$str);
    print oFILE pack('L',$crc16);
	  
	  my $add_str = sprintf("%s", "FF");
		my $time_add = 12;
		while ($time_add){
			$time_add --;
			my $str_done = pack("a2", $add_str); 
			print oFILE pack('H*',$str_done);
		}
	  
		close oFILE;	

		#### END ####
		
}


### Add by leo_huang : 20130617 ###
sub parse_config_to_header_file
{	
	my (
	  $file_zero_num,
		$file_header_file,
		$file_header_id,
		$file_struct_name_ori,  #@
		$gpio_get_num,
		$file_struct_name,			#@
		$file_struct_size,			#@
		$file_member_vars,			#@
		$file_member_type,			#@
		$file_member_size,			#@
		$file_member_val,			#@				
	) = @_;		


### some fix strings ####
my	$confile_strs_00 = 
"/* The IOCTL commands for nvram lib */
#define SONIX_NVRAM_IOCTL_INIT          (0x00)
#define SONIX_NVRAM_IOCTL_COMMIT        (0x01)
#define SONIX_NVRAM_IOCTL_RESET         (0x03)



/* IOCTL parameter (nvram lib)*/
typedef struct nvram_ioctl_s {
        int         param1;
        int         param2;
} nvram_ioctl_t;


//NVRAM_CONFIG_STATE for nvram lib
#define CONF_STATE_VALID      (0)
#define CONF_STATE_MODIFIED   (1)
#define CONF_STATE_FAILED     (2)


typedef struct config_info_s {
        char            state;               //the status of sub_struct
        unsigned long   offset;
        unsigned int    size;
} config_info_t;


";

my	$confile_strs_01 = 
"typedef struct info_s {
        char            *state;
        unsigned long   offset;
#define CONFIG_TYPE         (1)
#define ELEMENT_TYPE        (0)
        unsigned int    misc;           //[31]Type, [30:0] Size
} info_t; 

//#ifndef LIB_NVRAM
extern nvram_info_t *nv_info;

/*  ------------------------------------------------------------------------ */
nvram_t             nvram_tmp;
info_t              info;

#define INFO_CONFIG_TYPE        (CONFIG_TYPE << 31)                 //Put type into the MSB
#define GET_INFO_SIZE(x)        (x & (~(0x1<<31)))                  //Get info size
#define GET_INFO_TYPE(x)        (x >> 31)                           //Get info type


";

my	$confile_strs_02 = 
"
typedef  info_t  INFO;

\#ifdef __KERNEL__

\#define SONIX_NVRAM_DEVNAME \"nvram\"
\#define SONIX_NVRAM_MTDNAME  \"userconfig\"
\#define SONIX_NVRAM_OFFSET   0x4

\#else
\#define NV_DEV      \"/dev/nvram\"            //NVRAM DEV

/* NVRAM_LIB APIs in user space */

int     nvram_init (int nvram_id);
void    nvram_get(INFO* info,void* data);
int     nvram_set(INFO* info, void* data);
int     nvram_commit(INFO* info);
int     nvram_reset(INFO* info);
int     nvram_close(void);
";

my	$confile_strs_03 =
"int     nvram_set_str(INFO *info_e,const char *data)\;
char    *nvram_get_str(INFO *info_e)\;
int     nvram_set_str_cache(INFO *info_e,const char *data)\;
int     nvram_from_cfgfile(INFO *info_e,const char *filename)\;
int     nvram_from_cfgfile_all(INFO *info_e,const char *filename)\;
int     nvram_from_cfgfile_simple(INFO *info_e,const char *filename)\;
int     nvram_from_cfgfile_cache(INFO *info_e,const char *filename)\;
int 		nvram_from_cfgfile_all_cache(INFO *info_e,const char *filename)\;
int 		nvram_from_cfgfile_simple_cache(INFO *info_e,const char *filename)\;
int 		nvram_to_cfgfile(INFO *info_e,const char *filename)\;
int 		nvram_reset_all()\;
int 		nvram_commit_all()\;
int 		fname_convert_infoname(const char *fname,char *info_name)\;
INFO *get_info(const char * info_name)\;

\#ifndef NULL
\#define NULL		(void*) 0
\#endif
\#ifndef __LIBNVRAM__
extern char *all_config;
\#else

";

	######## Start to parse ###########

		### file.h ###
		open oHEADER, ">" . $$file_header_file;
		my $mb_h = 0;
		my $ssz_st =0;
		my $ssz = 0;
		my $total_struct_number = 0;
		
		
		print oHEADER
"/*---------------------- created by Perl tool -------------------------------*/
/*
 * Perl tool version: XXXXX
 * Auther: XXXXXX
 */

 
"; 
		print oHEADER "/*  pre-define */\n"; 
		
		### pre-define : id = min,sec ###
		#my $now_string = localtime;  # e.g., "Thu Oct 13 04:54:34 1994"
		#my @now_time = split(/ /, $now_string);
		#my @minsec	= split(/:/, $now_time[3]);
		#print oHEADER "#define NVRAM_ID    ".$minsec[1].$minsec[2]."\n";
		
		print oHEADER "#define NVRAM_ID    ".$$file_header_id."\n";
		

		
		### pre-define : type ###
		print oHEADER "#define uint8_t unsigned char\n"; 
		print oHEADER "#define uint16_t unsigned short\n"; 
		print oHEADER "#define uint32_t unsigned long\n"; 
		print oHEADER "#define int32_t signed long\n";
		print oHEADER "#pragma pack (1)\n"; 
		print oHEADER "\n\n\n"; 
		
		### pre-define : total struct numbers ###
		$total_struct_number =  (scalar(@$file_struct_name))/2;
		$total_struct_number += 1;
		print oHEADER "#define IPCAM_CON_NUM    ";
		print oHEADER $total_struct_number;
		print oHEADER "\n\n\n";
			
		### each-con ###
			$mb_h = 0;
			foreach (@$file_struct_name){
				if ($_ =~ /[a-zA-Z]/){
				
					print oHEADER "/*  Config structure. */\n"; 
					print oHEADER "typedef "; 
					print oHEADER "struct ";
					print oHEADER $_;
					print oHEADER "_s {";
					print oHEADER "\n";
					
					$ssz = $ssz + $$file_struct_size[$mb_h];
					#print "#".$ssz."\n";
					for (my $si=$ssz_st;
					$si < $ssz;
					$si++){
						my $vars = $$file_member_vars[$si];
						my $type = $$file_member_type[$si];
						my $size = $$file_member_size[$si];
						my $val	 = $$file_member_val[$si];
						
						print oHEADER "		";
						if ($type =~ /$CHAR/i)			{print oHEADER $CHAR_STR;}
						elsif ($type =~ /$UINT8/i)		{print oHEADER $U8_STR;}
						elsif ($type =~ /$UINT16/i)	{print oHEADER $U16_STR;}
						elsif ($type =~ /$UINT32/i)	{print oHEADER $U32_STR;}
						elsif ($type =~ /$INT32/i) {print oHEADER $I32_STR;}
					
						print oHEADER " ";
						print oHEADER $vars;
						
						if($size =~ /[0-9]x[0-9]/i){
						   my @two_multi = split(/x/i, $size);
				    	 print oHEADER "[";
					  	 print oHEADER $two_multi[0];
					  	 print oHEADER "]";
					  	 print oHEADER "[";
					  	 print oHEADER $two_multi[1];
							 print oHEADER "];\n";
						   }
						else{
							  if($size == 1){
							 	 print oHEADER ";\n";
						     }else{
						     print oHEADER "[";
						 	   print oHEADER $size;
						 	   print oHEADER "];\n";
						     }
							}	
					}
					
					print oHEADER "} ";
					print oHEADER $_;
					print oHEADER "_t;\n\n\n";
					
					$mb_h = $mb_h + 1;
					$ssz_st = $ssz;
				}
			}


		### all-con : nvram_s
print oHEADER "
/* Configuration Structures useful for managing node. */
typedef struct nvram_s	{
		//save the NVRAM_ID at the first 4 bytes.
		unsigned int	nvr_id;
";

print oHEADER "		//version\n";

print oHEADER "		char nvram_version[64];
";

		#---- body ----
		print oHEADER "		//body\n";
		foreach (@$file_struct_name){
			if ($_ =~ /[a-zA-Z]/){
				
					print oHEADER "		";
					print oHEADER $_;
					print oHEADER "_t";
					print oHEADER "		";
					print oHEADER $_;
					print oHEADER "_node;\n";
			}
		}	
		
		#--- end ---

if ($inter_ff!=0){
				print oHEADER "
    char inter_ff\[$inter_ff\];
    ";
}
		
if ($$file_zero_num!=0){
				print oHEADER "
    char skip_zero\[$$file_zero_num\];
    ";
}			
		


print oHEADER "//crc\n";

print oHEADER "		unsigned short int nvram_crc;
";


print oHEADER"
} nvram_t;\n\n\n";

		
		### print fix string : 00
		print oHEADER	$confile_strs_00;
		
		### nvram : info
		#---- body ----
		print oHEADER "/* used in nvram library */\n";
		print oHEADER "typedef struct nvram_info_s {\n";
		foreach (@$file_struct_name){
			if ($_ =~ /[a-zA-Z]/){		
					print oHEADER "		";
					print oHEADER "config_info_t";
					print oHEADER "		";
					
					print oHEADER $_;
					print oHEADER "_info;\n";
			}
		}
		
		print oHEADER 
"		
        unsigned long           nvr_va;
        int                     nvr_id;
} nvram_info_t;


";		
		#--- end ---
		
		### nvram : con define
		$mb_h = 0;
		$ssz_st =0;
		$ssz = 0;
			foreach (@$file_struct_name){
				if ($_ =~ /[a-zA-Z]/){
				
					#--- print : struct define ---
					print oHEADER "/* -----------------------	";
					print oHEADER $_;
					print oHEADER " Definition	-----------------------------*/\n";
					
					#--- print : struct  ---
					my $str_con = uc($_);
					my $str_con_ori = $_;

					print oHEADER
"#define ".$str_con." { \\
                            info.state = &(nv_info->".$_."_info.state); \\
                            info.offset = (unsigned long)&(nvram_tmp.".$_."_node) - (unsigned long)&(nvram_tmp); \\
                            info.misc   =  sizeof(nvram_tmp.".$_."_node); \\
                            return &info; \\
                            }
";
					#--- print : config member  ---
					$ssz = $ssz + $$file_struct_size[$mb_h];
					for (my $si=$ssz_st;
					$si < $ssz;
					$si++){
						my $str_fig = uc($$file_member_vars[$si]);
						my $str_fig_ori = $$file_member_vars[$si];
						print oHEADER
"#define ".$str_con."_".$str_fig." { \\
                            info.state = &(nv_info->".$str_con_ori."_info.state); \\
                            info.offset = (unsigned long)&(nvram_tmp.".$str_con_ori."_node.".$str_fig_ori.") - (unsigned long)&(nvram_tmp); \\
                            info.misc   = sizeof(nvram_tmp.".$str_con_ori."_node.".$str_fig_ori."); \\
                            return &info; \\
                            }
";						
					}
				
					$mb_h = $mb_h + 1;
					$ssz_st = $ssz;
				}
			}
			
		### nvram : gpio
		print oHEADER
"                           
/* ----------------------- NVRAM definition ----------------------------------*/

#define NVRAM       {\\
                            info.state = NULL; \\
                            info.offset = 0; \\
                            info.misc   = sizeof(nvram_t); \\
                            return &info; \\
                            }
					
							
";
	
		### print fix string : 01
		print oHEADER	$confile_strs_01;
		
		### print fix string : 02
		print oHEADER	$confile_strs_02;
		
		### print fix string : 03
		print oHEADER	$confile_strs_03;
		
		my $index_struct_size = 0;
		my $index_member_vars =0;
		my $cfig_current ='';
		foreach (@$file_struct_name){               
		 if ($_ =~ /[a-zA-Z]/){
		 my $struct_current = $_;
		 my $back_num = $_;
		 $back_num =~ s/.*_//;
     print oHEADER "inline INFO* ".uc($_)."_FUN()\n";
     print oHEADER uc($_)."\n\n";
     my $num_current_struct = $$file_struct_size[$index_struct_size];
     $index_struct_size ++;
     #print "\n$num_current_struct\n";
     foreach (1..$num_current_struct){
     	$cfig_current = $$file_member_vars[$index_member_vars];
     	$index_member_vars ++;
      print oHEADER "inline INFO* ".uc($struct_current)."_".uc($cfig_current)."_FUN()\n";
      print oHEADER uc($struct_current)."_".uc($cfig_current)."\n\n";
      }
      print oHEADER "/*======================================*/\n";
     }
	  }	

print oHEADER "
inline INFO* NVRAM_FUN()
NVRAM

/*======================================*/
";

	   $index_struct_size =0;
	   $index_member_vars =0;
	   print oHEADER "INFO *get_info(const char *infoname)\n{\n";
	   print oHEADER "char info_name[256];\nfname_convert_infoname(infoname,info_name);\n";
	   foreach (@$file_struct_name){               
		 if ($_ =~ /[a-zA-Z]/){
		 my $struct_current = $_;
		 my $back_num = $_;
		 $back_num =~ s/.*_//;
     print oHEADER "	if(!strcasecmp(info_name,\"".uc($_)."\"))\n";
     print oHEADER "		return ".uc($_)."_FUN();\n";
     my $num_current_struct = $$file_struct_size[$index_struct_size];
     $index_struct_size ++;
     #print "\n$num_current_struct\n";
     foreach (1..$num_current_struct){
     	$cfig_current = $$file_member_vars[$index_member_vars];
     	$index_member_vars ++;
      print oHEADER "	if(!strcasecmp(info_name,\"".uc($struct_current)."_".uc($cfig_current)."\"))\n";
      print oHEADER "		return ".uc($struct_current)."_".uc($cfig_current)."_FUN();\n";
      }
     } 
	  }	
	  
	  print oHEADER "  if(!strcasecmp(info_name,\"NVRAM\"))"."\n"."		return  NVRAM_FUN();\n"; 
	  print oHEADER "\n  return NULL;\n}\n";
	  
     $index_struct_size =0;
	   $index_member_vars =0;
	   print oHEADER "char *all_config=\n";
	   foreach (@$file_struct_name_ori){               
		 if ($_ =~ /[a-zA-Z]/){
		 my $struct_current = $_;
		 my $back_num = $_;
		 $back_num =~ s/.*_//;
     print oHEADER "		"."\"".($struct_current)."\\n"."\""."\n";
     my $num_current_struct = $$file_struct_size[$index_struct_size];
     $index_struct_size ++;
     #print "\n$num_current_struct\n";
     foreach (1..$num_current_struct){
     	$cfig_current = $$file_member_vars[$index_member_vars];
     	$index_member_vars ++;
      print oHEADER "		"."\"".($struct_current)."_".($cfig_current)."\\n"."\"";
      if($index_member_vars == @$file_member_vars){
      	 print oHEADER ";\n";
      }else{
     		 print oHEADER "\n";
      }
      }
     } 
	  }	
	  
	  print oHEADER "\n#endif\n#endif\n\n";
		close oHEADER;	
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
	#print "haha1:---$crc_sum---";
#  #my $data16=sprintf("%x", $crc_sum);
  #print "1:---$data16---";
#  my $pre_section=substr($data16,0,2);
#  my $bac_section=substr($data16,2,2);
#  $data16=$bac_section.$pre_section;
  #print "3:---$data16---";
#	$$crc_16_value = $data16;
	$$crc_16_value = $crc_sum;
}
