#!/usr/bin/perl -w
use strict;
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

#u-env
my $envs_tack = "\0";

#parameter from Makefile
my $hw_setting;
my $flash_layout;
my $dotconfig;
my $image;
my $bypass_tag;
my $enable_debug_mode = "n";
my $rtos_image_zip = "y";
my $platform;
my $sdk_version = "8888";
my $ddr_freq;
my $ddr_project = "";
my $ddr_df00_str = "";
my $ddr_df01_str = "";
my $ddr_df02_str = "";
my $image_path = "";
my $config_path;
my $hw_setting_update = "y";
my $flashlayout_update = "y";
my $user_update = "y";
my $image_dis = "null";
my $rescue_system = "y";
my $firmware_f_sp = "n";
my $rtos_start_address = 0x00008000;
#uexce related
my $header_file = "./src/header/header.bin"; 

my $HW_SETTING_PK_SIZE = 0xC00;
my $HEADER_FILE_PK_SIZE = 0x10000;
my $EXCUE_STAR_ADDR = 0x01000000;
my $LDIMG_STAR_ADDR = 0x00fffffc;
my $IMAGE_TABLE_SIZE = 0x200;
my $IMAGE_TABLE_EXC_SIZE = 0xDC;
my $SF_BLOCK_SIZE = 0x10000;
my $SF_SECTOR_SIZE = 0x1000;
my $nvram_bin = "nvram.bin";

my $hw_setting_str = 0;
my $hw_setting_end = 0;
my $flash_layout_str = 0;
my $flash_layout_end = 0;

my $bootsel_str = 0;
my $bootsel_end = 0;
my $bootsel_file = "null";
my $bootsel_file_size = 0;

my $user_str = 0;
my $user_end = 0;
my $user_file_size = 0;

my $rtos_str = 0;
my $rtos_end = 0;
my $rtos_file = "null";
my $rtos_file_size = 0;

my $nvram_file = "null";
my $factory_file = "null";
my $hw_setting_image = "hw_setting.image.d";
my $flash_layout_file = "flash_layout.bin.d";

my $rescue_str = 0;
my $rescue_end = 0;
my $rescue_file = "null";
my $rescue_file_size = 0;

my $factory_str = 0;
my $factory_end = 0;
my $factory_file_size = 0;

my $data_str = 0;
my $data_end = 0;
my $data_file = "null";
my $data_file_size = 0;



#flash_layout related
my $flash_layout_file_size = 512;
my @flash_spi =qw/hw_setting bootsel flash_layout nvram rtos factory data rescue add/;

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
	elsif (/^image_path=.*/){
		s/.*=//g;
		$image_path = $_;
	}
	elsif (/^image=.*/){
		s/.*=//g;
		$image = $_;
	}
	elsif(/^config_path=.*/){
		s/.*=//g;
		s/\/$//;
		$config_path = $_;
	}
	elsif(/^platform=.*/)	{
		s/.*=//g;
		$platform= $_;
	}
	elsif(/^sdk_version=.*/){
		s/.*=//g;
		$sdk_version= $_;
	}
	elsif(/^ddr_freq=.*/){
		s/.*=//g;
		$ddr_freq= $_;
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
	elsif(/^hw_setting_update=.*/){
		s/.*=//g;
		$hw_setting_update= $_;
	}
	elsif(/^flashlayout_update=.*/){
		s/.*=//g;
		$flashlayout_update= $_;
	}
	elsif(/^user_update=.*/){
		s/.*=//g;
		$user_update= $_;
	}
	elsif(/^image_dis=.*/){
		s/.*=//g;
		$image_dis= $_;

		open(oFILE,$image_dis) || die("Cannot Open File: $!");
		my @image_dis_raw = <oFILE>;
		close(oFILE);
		foreach (@image_dis_raw){
			if (/start address/) {
				my	@dis_raw_ln = split(/ /, $_);
				$dis_raw_ln[2] =~ s/\s+//g;
				$rtos_start_address = sprintf("%d", hex($dis_raw_ln[2]));
				print "rtos_start_address = $rtos_start_address\n";
			}
		}
	}
	elsif(/^rescue_system=.*/){
		s/.*=//g;
		$rescue_system= $_;
	}
	elsif(/^firmware_f_sp=.*/){
		s/.*=//g;
		$firmware_f_sp= $_;
	}
	elsif(/^bypass_tag=.*/)	{
		s/.*=//g;
		$bypass_tag= $_;
	}
	elsif(/^enable_debug_mode=.*/)	{
		s/.*=//g;
		$enable_debug_mode= $_;
	}
	elsif(/^rtos_image_zip=.*/)	{
		s/.*=//g;
		$rtos_image_zip= $_;
	}
	else{
		print $_ , "\n";
		print "the image parameter is wrong!\n";
	}

}
#####select flash_layout#######
$flash_layout = "$config_path/flash-layout/serial_flashlayout.conf";

unless (-e $flash_layout){
 print "\nError:	flash layout  file--->$flash_layout does not exist!\n\n";
 exit;
}

###VERSION###
my $FIRMWARE_VER = $sdk_version.$envs_tack ;
my $version_size = length $FIRMWARE_VER;

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

################## main body  ##############
if ($image =~ /^rots_bin$/) {
	&pack_rtos_image();
}
elsif ($image =~ /^rescue_bin$/) {
	&pack_rescue_image();
}
elsif ($image =~ /^bootsel_bin$/) {
	&pack_bootsel_image();
}
elsif ($image =~ /^firmware_factory$/) {
	&generate_selfburn_firmware($image);
}
elsif ($image =~ /^firmware$/) {
	&generate_selfburn_firmware($image);
}
else {print "the image parameter is wrong!\n"}

################## sub function ##############
sub pack_rtos_image
{
	#my $rtos_src_file = "$image_path/rtos.bin";
	my $rtos_src_file = "$image_path/rtos.bin.gz";
	my $rtos_dst_file = "$image_path/KERNEL.bin";
	my $tmp_rtos_bin = "rtos.image.d";
	system("cp -f $image_path/rtos.bin $image_path/rtos.bin.bk");

	system("gzip -9 -f $image_path/rtos.bin");	# better
 	system("mv -f $image_path/rtos.bin.bk $image_path/rtos.bin");
 
 	if ($rtos_image_zip ne "y") {
 		system("cp -f $image_path/rtos.bin $image_path/rtos.bin.gz");
	}

 #	gzip "$image_path/rtos.bin" => $rtos_src_file;
#	&padd32 ($rtos_src_file, $tmp_rtos_bin);
#	system("./src/code/image_tool -u $tmp_rtos_bin -v 0x12345678 -o ./ -e ./src/header/header.bin");	
	system("./src/code/image_tool -u $rtos_src_file -v 0x12345678 -o ./ -e ./src/header/header.bin");
	system("cp -rf  ./u_boot.image.d $rtos_dst_file");
}


sub pack_rescue_image
{
	#my $rtos_src_file = "$image_path/rescue.bin";
	my $rtos_src_file = "$image_path/rescue.bin.gz";
	my $rtos_dst_file = "$image_path/RESCUE.bin";
	my $tmp_rtos_bin = "rescue.image.d";

	system("cp -f $image_path/rescue.bin $image_path/rescue.bin.bk");
 	
 	system("gzip -9 -f $image_path/rescue.bin");	# better
 	system("mv -f $image_path/rescue.bin.bk $image_path/rescue.bin");


	system("./src/code/image_tool -u $rtos_src_file -v 0x12345678 -o ./ -e ./src/header/header.bin");	
	system("cp -rf  ./u_boot.image.d $rtos_dst_file");
}

sub pack_bootsel_image
{
	my $rtos_src_file = "$image_path/bootsel.bin";
	my $rtos_dst_file = "$image_path/BOOTSL.bin";
	my $tmp_rtos_bin = "bootsel.image.d";
	&padd32 ($rtos_src_file, $tmp_rtos_bin);
	system("./src/code/image_tool -u $tmp_rtos_bin -v 0x12345678 -o ./ -e ./src/header/header.bin");	
	system("cp -rf  ./u_boot.image.d $rtos_dst_file");
}

sub padd32
{
        my $ifile = $_[0];
        my $ofile = $_[1];
        my $if_sz = 0;
        my $pad;

        $if_sz = -s $ifile;
        $pad  = (($if_sz + 0x1F) & (~0x1F)) - $if_sz;
        system("cp -rf  $ifile $ofile");

	 	open oFILE, ">>" . $ofile or die "Can't open '$ofile': $!";
	 	for (my $i=0;
				$i < $pad;
				$i++){
				print oFILE pack('H2', 'ff');
		}
		close oFILE;
}	


# sub padd32
# {
#         my $file = $_[0];
#         my $pz_file = $_[1];

#         my $filesize = 0;
#         my @data_w;
#         my $buffer;
#         my $filebuff;
#         my @vtime;
#         my $i = 0;

#         $filesize = -s $file;
#         open(oFILE, $file) or die "Can't open '$file': $!";
#         read(oFILE, $filebuff, $filesize - 32, 0);
#         close oFILE;

#         open(oFILE, ">$pz_file") or die "Can't open '$pz_file': $!";
# 		print oFILE $filebuff;
#         close oFILE;

# 		open(oFILE, "<$file");
#         seek(oFILE,$filesize - 32,0);
#         read(oFILE, $buffer, 32, 0);
#         close(oFILE);

#         foreach (split(//, $buffer)) {
#                 $vtime[$i] = ord($_);
#                 $i++;
#         }
#         my $size = ($filesize + 4);
#         my $pad = 0;
#         $pad  = (($size + 0x1F) & (~0x1F)) - ($size);


#         if(($vtime[16]==50)&&
#         ($vtime[20]==47)&&
#         ($vtime[25]==47)) {

# 				open(oFILE, ">>$pz_file");
#                 for (my $i=0;
#                         $i < $pad;
#                         $i++){
#                         print oFILE pack('H2', 'FF');
#                         }

#                         foreach(@vtime){
#                                 my $hexval = sprintf("%x", $_);
#                                 print oFILE pack('H2',$hexval);
#                         }
#                 close oFILE;
#         }
#         else{
# 		 open(oFILE, ">>$pz_file");
#                 foreach(@vtime){
#                                 my $hexval = sprintf("%x", $_);
#                                 print oFILE pack('H2',$hexval);
#                         }

#                 for (my $i=0;
#                         $i < $pad;
#                         $i++){
#                         print oFILE pack('H2', 'FF');
#                         }
#                 close oFILE;
#         }

# }	


#gen selfburn_firmware
sub generate_selfburn_firmware
{
	my $target_name = $_[0];

	my $target_file = "FIRMWARE_SB.bin";
	my $image_file = "image.d";
	my $config_file = "config.d";
	my $hw_setting_file = $hw_setting_image;

	my @data_hw_setting = ();	
	my @data_config = ();	
	my @data_header_file = ();	
	my @data_u_boot = ();	
	my @data_u_env = ();	
	my @data_flash_layout = ();	

	### flash_layout.bin ###
	&fun_flash_layout_bin(
 		\$flash_layout_file, 
		\@flash_layout_value
	);

	&fun_hw_setting();

#	if ($target_name =~ /firmware_f/i) {
 		open(oFILE, $hw_setting_file) or die "Can't open '$hw_setting_file': $!";
	 	binmode(oFILE);
 		@data_hw_setting = <oFILE>;	
	 	close oFILE;
		my $hw_setting_size = -s $hw_setting_file;
	
		my $firmware_id = $platform;
			$firmware_id = "st58660";
		print "the firmware id = $firmware_id\n";
		$firmware_id =~ s/[^0-9]//g;

		my $fw_tag = sprintf "%08d", hex($firmware_id);
	
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

		  	# push @padd_config, pack('L', 0x00000077);	#overwrite load addr
  			# push @padd_config, pack('L', 0xffff601c);
		  	# push @padd_config, pack('L', 0x10fffffc);

  			# push @padd_config, pack('L', 0x00000077);	#overwrite jmp addr
	  		# push @padd_config, pack('L', 0xffff6090);
	  		# push @padd_config, pack('L', 0x01000000);


	  		push @padd_config, pack('L', 0x00000077);	#overwrite load addr
  			push @padd_config, pack('L', 0xffff601c);
		  	push @padd_config, pack('L', 0x107ffffC);

  			push @padd_config, pack('L', 0x00000077);	#overwrite jmp addr
	  		push @padd_config, pack('L', 0xffff6090);
	  		push @padd_config, pack('L', 0x00800000);


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

#flash_layout.bin.d
		my $flash_layout_size = -s $flash_layout_file;

	 	open(oFILE, $flash_layout_file) or die "Can't open '$flash_layout_file': $!";
 		binmode(oFILE);
	 	# read file into an array
 		@data_flash_layout = <oFILE>;
	 	close oFILE;

#	} #end firmware_f

	my $header_file_fir = "./src/header/header.bin";
	my $header_file_size = -s $header_file_fir;

 	open(oFILE, $header_file_fir) or die "Can't open '$header_file_fir': $!";
	binmode(oFILE);
	@data_header_file = <oFILE>;	
	close oFILE;

	my $hdr_fl_pad;
	if ($target_name =~ /firmware_f/i) {
		$hdr_fl_pad = $HEADER_FILE_PK_SIZE - $header_file_size -4;
	} else {
		$hdr_fl_pad = $HEADER_FILE_PK_SIZE - $header_file_size;
	}
		for (my $i=0;
			$i < $hdr_fl_pad;
			$i++){
			push @data_header_file, pack('H2', '00');			
		}
#	}

#nvram.bin 	
	my @data_nvram = ();	
	my $nvram_size = -s $nvram_file;

#rtos
 	open(oFILE, $rtos_file) or die "Can't open '$rtos_file': $!";
	binmode(oFILE);
 	my @data_rtos = <oFILE>;	
	close oFILE;

#rescue
my @data_rescue;
if ($rescue_system eq "y") {
 	open(oFILE, $rescue_file) or die "Can't open '$rescue_file': $!";
	binmode(oFILE);
 	@data_rescue = <oFILE>;	
	close oFILE;
}


#bootsel
 	open(oFILE, $bootsel_file) or die "Can't open '$bootsel_file': $!";
	binmode(oFILE);
 	my @data_bootsel = <oFILE>;	
	close oFILE;

#factory
 	open(oFILE, $factory_file) or die "Can't open '$bootsel_file': $!";
	binmode(oFILE);
 	my @data_factory = <oFILE>;	
	close oFILE;


#data null
 	open(oFILE, $data_file) or die "Can't open '$data_file': $!";
	binmode(oFILE);
 	my @data_data = <oFILE>;	
	close oFILE;

#IMAGE_TABLE
	my $flash_index = 0;
	foreach (@flash_spi)
	{
  		$_ =~s/-/_/g;

		if ($_ =~ /nvram/i) {
			if ($user_end > $user_str) {
				open(oFILE, $nvram_file) or die "Can't open '$nvram_file': $!";
				binmode(oFILE);
				@data_nvram = <oFILE>;
				close oFILE;
				if (($user_end - $user_str) < $nvram_size) {die "ERROR: please increase nvram partition size \n ";}		
			}
		}
		if ($_ =~ /rtos/i) {
			if ($rtos_end > $rtos_str) {
				if (($rtos_end - $rtos_str) < $rtos_file_size) {die "ERROR: please increase rtos partition size \n ";}		
			}
		}
	}

	my @data_image_table = ();
	my $resval = 0x12345678;
	my $image_star_addr = 0;

	$IMAGE_TABLE_EXC_SIZE = 0;

	if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) {
		$hw_setting_update = "y";
		$flashlayout_update = "y";
	}
	else {
		$hw_setting_update = "n";
		$flashlayout_update = "y";
		$bootsel_end = 0;
		$bootsel_str = 0;
	}


	if ($hw_setting_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}
	if ($bootsel_end > $bootsel_str) {$IMAGE_TABLE_EXC_SIZE += 0x14;}
	if ($flashlayout_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}	
	if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) { if ($user_end > $user_str) {if ($user_update eq "y") {$IMAGE_TABLE_EXC_SIZE += 0x14;}}}
	if ($rtos_end > $rtos_str) {$IMAGE_TABLE_EXC_SIZE += 0x14;}
	if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) { if ($rescue_system eq "y") { if ($rescue_end > $rescue_str) {$IMAGE_TABLE_EXC_SIZE += 0x14;} } } 
	if ($factory_end > $factory_str) {$IMAGE_TABLE_EXC_SIZE += 0x14;}
	if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) { if ($data_end > $data_str) {$IMAGE_TABLE_EXC_SIZE += 0x14;} }

		
			
		
	

	push @data_image_table, pack('L', $IMAGE_TABLE_EXC_SIZE);


				if ($hw_setting_update eq "y") {
					push @data_image_table, pack('L', $resval);
					push @data_image_table, pack('L', $image_star_addr);
					#push @data_image_table, pack('L', $hw_setting_size);
					push @data_image_table, pack('L', $HW_SETTING_PK_SIZE);
					push @data_image_table, pack('L', $hw_setting_str);
					push @data_image_table, pack('L', $hw_setting_end);

					printf "imstr = %x\n" , $image_star_addr;
					printf "HW_SETTING_PK_SIZE = %x\n" , $HW_SETTING_PK_SIZE;
					printf "hw_setting_str = %x\n" , $hw_setting_str;
					printf "hw_setting_end = %x\n" , $hw_setting_end;
					printf "---------------------------------\n";

				$image_star_addr = $image_star_addr + $HW_SETTING_PK_SIZE;
				}
		# if ($target_name =~ /firmware_f/i) {
		# 	if ($firmware_f_sp ne "y") {
		# 	if ($bootsel_end > $bootsel_str) {
		# 		push @data_image_table, pack('L', $resval);
		# 		push @data_image_table, pack('L', $image_star_addr);
		# 		push @data_image_table, pack('L', $bootsel_file_size);
		# 		push @data_image_table, pack('L', $bootsel_str);
		# 		push @data_image_table, pack('L', $bootsel_end);

		# 		printf "imstr = %x\n" , $image_star_addr;
		# 		printf "bootsel_size = %x\n" , $bootsel_file_size;
		# 		printf "bootsel_str = %x\n" , $bootsel_str;
		# 		printf "bootsel_end = %x\n" , $bootsel_end;
		# 		printf "---------------------------------\n";

		# 		$image_star_addr = $image_star_addr + $bootsel_file_size;
		# 	}
		# 	}
		# }

			if ($bootsel_end > $bootsel_str) {
				push @data_image_table, pack('L', $resval);
				push @data_image_table, pack('L', $image_star_addr);
				push @data_image_table, pack('L', $bootsel_file_size);
				push @data_image_table, pack('L', $bootsel_str);
				push @data_image_table, pack('L', $bootsel_end);

				printf "imstr = %x\n" , $image_star_addr;
				printf "bootsel_size = %x\n" , $bootsel_file_size;
				printf "bootsel_str = %x\n" , $bootsel_str;
				printf "bootsel_end = %x\n" , $bootsel_end;
				printf "---------------------------------\n";

				$image_star_addr = $image_star_addr + $bootsel_file_size;
			}

			if ($flashlayout_update eq "y") {
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

		if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) {
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
				}
		}
			if ($rtos_end > $rtos_str) {
				push @data_image_table, pack('L', $resval);
				push @data_image_table, pack('L', $image_star_addr);
				push @data_image_table, pack('L', $rtos_file_size);
				push @data_image_table, pack('L', $rtos_str);
				push @data_image_table, pack('L', $rtos_end);

				printf "imstr = %x\n" , $image_star_addr;
				printf "rtos_size = %x\n" , $rtos_file_size;
				printf "rtos_str = %x\n" , $rtos_str;
				printf "rtos_end = %x\n" , $rtos_end;
				printf "---------------------------------\n";

				$image_star_addr = $image_star_addr + $rtos_file_size;
			}
		if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) {
				if ($rescue_system eq "y") {	
					if ($rescue_end > $rescue_str) {
					push @data_image_table, pack('L', $resval);
					push @data_image_table, pack('L', $image_star_addr);
					push @data_image_table, pack('L', $rescue_file_size);
					push @data_image_table, pack('L', $rescue_str);
					push @data_image_table, pack('L', $rescue_end);

					printf "imstr = %x\n" , $image_star_addr;
					printf "rescue_size = %x\n" , $rescue_file_size;
					printf "rescue_str = %x\n" , $rescue_str;
					printf "rescue_end = %x\n" , $rescue_end;
					printf "---------------------------------\n";

					$image_star_addr = $image_star_addr + $rescue_file_size;
					}
				}
		}	

			if ($factory_end > $factory_str) {
				push @data_image_table, pack('L', $resval);
				push @data_image_table, pack('L', $image_star_addr);
				push @data_image_table, pack('L', $factory_file_size);
				push @data_image_table, pack('L', $factory_str);
				push @data_image_table, pack('L', $factory_end);

				printf "imstr = %x\n" , $image_star_addr;
				printf "factory_size = %x\n" , $factory_file_size;
				printf "factory_str = %x\n" , $factory_str;
				printf "factory_end = %x\n" , $factory_end;
				printf "---------------------------------\n";

				$image_star_addr = $image_star_addr + $factory_file_size;
			}
		if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) {
				if ($data_end > $data_str) {
				push @data_image_table, pack('L', $resval);
				push @data_image_table, pack('L', $image_star_addr);
				push @data_image_table, pack('L', $data_file_size);
				push @data_image_table, pack('L', $data_str);
				push @data_image_table, pack('L', $data_end);

				printf "imstr = %x\n" , $image_star_addr;
				printf "data_size = %x\n" , $data_file_size;
				printf "data_str = %x\n" , $data_str;
				printf "data_end = %x\n" , $data_end;
				printf "---------------------------------\n";

				$image_star_addr = $image_star_addr + $data_file_size;
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

	open oFILE, ">" . $image_file or die "Can't open '$image_file': $!";
		print oFILE @data_header_file;
		print oFILE @data_image_table;
		if ($hw_setting_update eq "y") {print oFILE @data_hw_setting;}
		if ($bootsel_end > $bootsel_str) {print oFILE @data_bootsel;}
		if ($flashlayout_update eq "y") {print oFILE @data_flash_layout;}
		if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) { if ($user_end > $user_str) {if ($user_update eq "y") {print oFILE @data_nvram;}} }
		if ($rtos_end > $rtos_str) {print oFILE @data_rtos;}
		if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) { if ($rescue_system eq "y") { if ($rescue_end > $rescue_str) {print oFILE @data_rescue;} } }
		if ($factory_end > $factory_str) {print oFILE @data_factory;}
		if (($target_name =~ /firmware_f/i)||($firmware_f_sp eq "y")) { if ($data_end > $data_str) {print oFILE @data_data;} }
	close oFILE;

	$image_size = -s $image_file;
	$image_pad_size  = (($image_size + 0x1F) & (~0x1F)) - $image_size;
	$image_size = $image_size + $image_pad_size;

	for (my $i=0;
		$i < $image_pad_size;
		$i++){
		push @image_pad, pack('H2', 'ff');		
	}

	open oFILE, ">>" . $image_file or die "Can't open '$image_file': $!";
		print oFILE @image_pad;
	close oFILE;

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
	if ($target_name =~ /firmware_factory/i) {
 		print oFILE @data_config;
	}
#IMAGE
	print oFILE @data_image;

#BYPASS TAG
	if ($bypass_tag ne "y") {
		$bypass_tag = "UPGRADE_MODE";
	}else{
		$bypass_tag = "BYPASS_MODE";
	}
	print oFILE $bypass_tag;
	print oFILE pack('L',length($bypass_tag));	
#PLATFORM
	print oFILE $platform;
	print oFILE pack('L',length($platform));

#VERSION
	print oFILE $FIRMWARE_VER;
	print oFILE pack('L',$version_size);   

	close oFILE;
	
	# generate md5 [16bytes]  
	open(oFILE, $target_file) or die "Can't open '$target_file': $!"; 
	my $file_md5 = Digest::MD5->new->addfile(*oFILE)->hexdigest;	
	print "generate md5  $target_file : $file_md5\n";         
	close oFILE; 

	my $md5_encrypt_file = "md5_encrypt.bin";
	system("./src/md5_encrypt/md5_encrypt $file_md5 $md5_encrypt_file"); 
	my $size_md5_file = -s $md5_encrypt_file; 
	print "generate md5  file size :$size_md5_file\n";         

	open(oFILE, $md5_encrypt_file) or die "Can't open '$md5_encrypt_file': $!"; 
	binmode(oFILE); 
	my @data_md5_encrypt = <oFILE>; 
	close oFILE; 

	open oFILE, ">>" . $target_file or die "Can't open '$target_file': $!";
	print oFILE @data_md5_encrypt;
	close oFILE;

	print "\ngenerate self-burn firmware(_f).bin end\n";
}

sub fun_hw_setting
{		
	my $hw_setting_af_pll = "$config_path/hw-setting/template/hw_setting_af_pll_98660asic.txt";
	my $hw_setting_af_ddr = "$config_path/hw-setting/template/hw_setting_af_ddr_98660asic.txt";
	my $ddr_config_58660_main = "";
	my $ddr_config_58660_df00 = "";
	my $ddr_config_58660_df01 = "";
	my $ddr_config_58660_df02 = "";
	my $ddr_config_58660_end = "";
	my $hw_file = "sn9866x";
	my $ddr_config_file = "$config_path/hw-setting/".$hw_file."/";
	my $hw_template_file = "$config_path/hw-setting/template/hw_setting_af_pll_98660asic.txt";
	my $flash_layout_header_file = "$config_path/flash-layout/";

	my $project = "";
	my $ptye = "full";
	
	my $ddr_project_para = "";
	my $bits = "";
	my $odt = "_2.5_BL4_no_ODT";
	
	
	if ($ddr_project =~ /([a-zA-Z0-9]+)_([0-9a-zA-Z]+)/){
		$ddr_project_para = $1;
		my $mem_bit = $2;
		if ($mem_bit =~ /([^x]+)x([^x]+)/){
			$bits = $2;
		}
	}

	#################platform#################
	if($platform =~ /sn98660/){
  		my $hw_which_file = "sn98660";
		$ddr_config_file = $ddr_config_file.$hw_which_file."/";
		$project = $ddr_project_para;
		$ptye = "half";
	}
	
	if($platform =~ /sn98293/){
  		my $hw_which_file = "sn98660";
		$ddr_config_file = $ddr_config_file.$hw_which_file."/";
		$project = $ddr_project_para;
		$ptye = "half";
	}

	if($platform =~ /sn98670/){
  		my $hw_which_file = "sn98670";
		$ddr_config_file = $ddr_config_file.$hw_which_file."/";
		$project = $ddr_project_para;
		$ptye = "half";
	}

	if($platform =~ /sn98671/){
  		my $hw_which_file = "sn98671";
		$ddr_config_file = $ddr_config_file.$hw_which_file."/";
		$project = $ddr_project_para;
		$ptye = "half";
	}
	
	if($platform =~ /sn98672/){
  		my $hw_which_file = "sn98671";
		$ddr_config_file = $ddr_config_file.$hw_which_file."/";
		$project = $ddr_project_para;
		$ptye = "half";
	}
	
	my $mhz_str = $ddr_freq."mhz";
	my $tmp_df_file;
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

	if ($ddr_df01_str eq "") {
		$ddr_config_58660_df01 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df01_str.$odt;
		system ("touch $ddr_config_58660_df01");
	} 
	else {
		$tmp_df_file = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df01_str.$odt;
		$ddr_config_58660_df01 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df01_str.$odt.".d";
		system("diff -fibB $ddr_config_58660_main $tmp_df_file | grep \"WM32\" > $ddr_config_58660_df01");
	}

	if ($ddr_df02_str eq "") {
		$ddr_config_58660_df02 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df02_str.$odt;
		system ("touch $ddr_config_58660_df02");
	} 
	else {
		$tmp_df_file = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df02_str.$odt;
		$ddr_config_58660_df02 = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$ddr_df02_str.$odt.".d";
		system("diff -fibB $ddr_config_58660_main $tmp_df_file | grep \"WM32\" > $ddr_config_58660_df02");
	}

	$ddr_config_58660_end = $ddr_config_file."ddr2_".$ptye."_".$mhz_str."_".$bits."_".$project.$odt."_END";

	###################ddr######################################
	if ($ddr_freq =~ /25/){
		$hw_template_file = "$config_path/hw-setting/template/hw_setting_af_pll_58660fpga.txt";
		$hw_setting_af_ddr = "$config_path/hw-setting/template/hw_setting_af_ddr_58660fpga.txt";
		$project = $ddr_project_para;
		$ddr_config_file = "$config_path/hw-setting/sn9866x/st58660/";
		$ddr_config_58660_main = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$project."_2.5_BL4";
		$ddr_config_58660_df00 = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$ddr_df00_str."_2.5_BL4";
		$ddr_config_58660_df01 = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$ddr_df01_str."_2.5_BL4";
		$ddr_config_58660_df02 = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$ddr_df02_str."_2.5_BL4";
		$ddr_config_58660_end = $ddr_config_file ."ddr2"."_25mhz_". $bits ."_".$project."_2.5_BL4_END";
	}
	
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
  
	### DEBUG ###
		print "ddr_main = $ddr_config_58660_main\n";
		print "ddr_df00 = $ddr_config_58660_df00\n";
		print "ddr_df01 = $ddr_config_58660_df01\n";
		print "ddr_df02 = $ddr_config_58660_df02\n";
		print "ddr_end = $ddr_config_58660_end\n";
	unless (-e $hw_template_file){
		print "\nError:	hw template file--->$hw_template_file does not exist!\n\n";
		exit;
	}
	unless (-e $hw_setting_af_ddr){
		print "\nError:	af_ddr file--->$hw_setting_af_ddr does not exist!\n\n";
		exit;
	}	
	############### ADD ##########################
	my @data_ddr;	
	my @data_ddr_main;	
	my @data_ddr_df00;	
	my @data_ddr_df01;	
	my @data_ddr_df02;	
	my @data_ddr_end;	
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

	open(oHW, $hw_template_file) or die "Can't open '$hw_template_file': $!";
	#binmode(oHW);
	# read file into an array
	my @data_hw = <oHW>;	
	close oHW;

	if ($enable_debug_mode eq "y") {
		if ($data_hw[1] =~ /b 0x00001111/) {
			$data_hw[1] ="b 0x0000ffff\n";	
		}
	}

	open(oAFDDR, $hw_setting_af_ddr) or die "Can't open '$hw_setting_af_ddr': $!";
	# read file into an array
	my @data_afddr = <oAFDDR>;	
	close oAFDDR;

	my $muti_main_fir = "hw_setting.image.main.d";
	my $muti_f00_fir = "hw_setting.image.f00.d";
	my $muti_f01_fir = "hw_setting.image.f01.d";
	my $muti_f02_fir = "hw_setting.image.f02.d";
	my $muti_end_fir = "hw_setting.image.end.d";
	my $hw_setting_file = $hw_setting_image;

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


# .equ  LOAD_IMAGE_ADDR_NL,			(ITCM_ZI + 0x128)
# .equ  SF_UIMAGE_ADDR_NL,			(ITCM_ZI + 0x12c)
# .equ  LOAD_IMAGE_SIZE_NL,			(ITCM_ZI + 0x130)

# .equ  LOAD_IMAGE_ADDR_RS,			(ITCM_ZI + 0x134)
# .equ  SF_UIMAGE_ADDR_RS,			(ITCM_ZI + 0x138)
# .equ  LOAD_IMAGE_SIZE_RS,			(ITCM_ZI + 0x13c)

# .equ  JUMP_RTOS_ADDR,				(ITCM_ZI + 0x140)
# .equ  NOW_RTOS_STATUS,				(ITCM_ZI + 0x144)

# .equ  LOAD_ZIMAGE_ADDR_NL,			(ITCM_ZI + 0x148)
# .equ  LOAD_DZIMAGE_SIZE_NL,			(ITCM_ZI + 0x14c)
# .equ  LOAD_DZIMAGE_SIZE_RS,			(ITCM_ZI + 0x150)

# 	LDR	r0, [r0]			@ r0 = dst addr on ddr
# 	LDR	r1, [r1]			@ r1 = src addr on sf
# 	LDR	r2, [r2]			@ r2 = size in byte

# @	LDR	r0, = 0x00007ffc
# @	LDR	r1, = 0x00043000
# @	LDR	r2, = 0x000e0268


		printf oHW_SETTING "w 0xffff60f8,0x%x\n", $bootsel_str;
		# printf oHW_SETTING "w 0xffff6020,0x%x\n", $bootsel_file_size;
		printf oHW_SETTING "w 0xffff6020,0x00020000\n";

		# printf oHW_SETTING "w 0xffff60f8,0x%x\n", $rtos_str;
		# printf oHW_SETTING "w 0xffff6020,0x%x\n", $rtos_file_size;
		# printf oHW_SETTING "w 0xffff60f8,0x%x\n", $rescue_str;
		# printf oHW_SETTING "w 0xffff6020,0x%x\n", $rescue_file_size;

		 printf oHW_SETTING "w 0xffff6128,0x%x\n", ($rtos_start_address -4);
		 printf oHW_SETTING "w 0xffff612c,0x%x\n", $rtos_str;
		 printf oHW_SETTING "w 0xffff6130,0x%x\n", $rtos_file_size;

if ($rescue_system eq "y") {
		 printf oHW_SETTING "w 0xffff6134,0x%x\n", ($rtos_start_address -4);
		 printf oHW_SETTING "w 0xffff6138,0x%x\n", $rescue_str;
		 printf oHW_SETTING "w 0xffff613c,0x%x\n", $rescue_file_size;
}else {
		 printf oHW_SETTING "w 0xffff6134,0x%x\n", ($rtos_start_address -4);
		 printf oHW_SETTING "w 0xffff6138,0x%x\n", $rtos_str;
		 printf oHW_SETTING "w 0xffff613c,0x%x\n", $rtos_file_size;		
}

		 printf oHW_SETTING "w 0xffff6140,0x%x\n", $rtos_start_address;

		 # d-gzip 	 
		 printf oHW_SETTING "w 0xffff6148,0x00900000\n";
#		 printf oHW_SETTING "w 0xffff6148,0x01500000\n"; 	#LOAD_ZIMAGE_ADDR_NL
		 printf oHW_SETTING "w 0xffff614c,0x00200000\n";	#LOAD_DZIMAGE_SIZE_NL
		 printf oHW_SETTING "w 0xffff6150,0x00200000\n";	#LOAD_DZIMAGE_SIZE_RS

		 printf oHW_SETTING "w 0xffff6154,0x%x\n", $flash_layout_str;		#FLASH_LAYOUT_STR

 		if ($rtos_image_zip ne "y") {
 			printf oHW_SETTING "w 0xffff6144,0x70697a6e\n";
		}

		# Add sdk_version for rescue mode
		my $envs_tack = "\0";
		my $nvram_ver = $sdk_version.$envs_tack;
		my $nvram_ver_size = length($nvram_ver);

		printf oHW_SETTING "w 0xffff6160,0x%08x\n", $nvram_ver_size;
		my @nvarr = split //, $nvram_ver;
		@nvarr = unpack("C*", $nvram_ver);
		my $nvarr_size = @nvarr;


		my $nvarr_dummy_size = 4 - ($nvarr_size%4);

		for (my $i=0;
			$i < $nvarr_dummy_size;
			$i++
			) {
			push(@nvarr, (0));
		}	

		for (my $i=0;
			$i < ($nvarr_size + $nvarr_dummy_size);
			$i = $i +4
			){
			my $hwaddr = 0xffff6164 + $i;
			my $hwval = ($nvarr[$i+0] << 0)+($nvarr[$i+1] << 8)+($nvarr[$i+2] << 16)+($nvarr[$i+3] << 24);

			printf oHW_SETTING "w 0x%08x,0x%08x\n", $hwaddr,$hwval;
		}
		# addend

		foreach (@data_afddr){
			unless ($_ =~ /^\#/){
				print oHW_SETTING;
			}
		}
		close oHW_SETTING;
		system("./src/code/image_tool -h ./hw_setting.end.txt.d -o ./ -e ./src/header/header.bin");
		system("mv $hw_setting_file $muti_end_fir");
	
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

}

sub fun_flash_layout_bin
{
	my (
			$filelayout_file, 
			$flash_layout_value_para	#@                      
	) = @_;	 

	if ($rescue_system eq "y") {
		 @flash_spi =qw/hw_setting bootsel flash_layout rtos factory nvram data rescue add/;
	}
	else {
			@flash_spi =qw/hw_setting bootsel flash_layout rtos factory nvram data add/;
	}


	my $flash_index = 0;
		
	open oFILE, ">" . $$filelayout_file;
		my $i = 0;
		$flash_index = 0;
		foreach (@flash_spi){
			my $str	= $$flash_layout_value_para[$flash_index];
			$flash_index ++;
			my $end = $$flash_layout_value_para[$flash_index];
			$flash_index ++;
			my $image_file = $$flash_layout_value_para[$flash_index];
			$flash_index ++;
					
			$str =~ s/\s+//g;
			$end =~ s/\s+//g;
					
			my	@p32u_str = split(/x/, $str);
			my $ff32u_str = sprintf "%08d", hex($p32u_str[1]);
			print oFILE pack('L', $ff32u_str);
					
			my	@p32u_end = split(/x/, $end);
			my $ff32u_end = sprintf "%08d", hex($p32u_end[1]);
			print oFILE pack('L', $ff32u_end);
			if ($_ eq "nvram") {
				$user_end = $ff32u_end;
				$user_str = $ff32u_str;
				$nvram_file = $image_file;	
				if ($user_end > $user_str) {
					unless (-e $nvram_file){
						print "\nError: $nvram_file file does not exist!\n\n";
						exit;
					}
					$user_file_size = -s $nvram_file;
				}
			}
			elsif ($_ eq "factory") {
				$factory_end = $ff32u_end;
				$factory_str = $ff32u_str;
				$factory_file = $image_file;	
				if ($factory_end > $factory_str) {
					unless (-e $factory_file){
						print "\nError: $factory_file file does not exist!\n\n";
						exit;
					}

					$factory_file_size = -s $factory_file;
				}
			}
			elsif ($_ eq "hw_setting") {
				$hw_setting_str = $ff32u_str;
				$hw_setting_end = $ff32u_end;
				$hw_setting_image = $image_file;
			}
			elsif ($_ eq "bootsel") {
				$bootsel_str = $ff32u_str;
				$bootsel_end = $ff32u_end;
				$bootsel_file = $image_file;	
				if ($bootsel_end > $bootsel_str) {
					unless (-e $bootsel_file){
						print "\nError: $rtos_file file does not exist!\n\n";
						exit;
					}
					$bootsel_file_size = -s $bootsel_file;
				}
			}
			elsif ($_ eq "rtos") {
				$rtos_str = $ff32u_str;
				$rtos_end = $ff32u_end;
				$rtos_file = $image_file;	
				if ($rtos_end > $rtos_str) {
					unless (-e $rtos_file){
						print "\nError: $rtos_file file does not exist!\n\n";
						exit;
					}
					$rtos_file_size = -s $rtos_file;
				}
			}
			elsif ($_ eq "flash_layout") {
				$flash_layout_str = $ff32u_str;
				$flash_layout_end = $ff32u_end;
			} 
			elsif ($_ eq "rescue") {
				$rescue_str = $ff32u_str;
				$rescue_end = $ff32u_end;
				$rescue_file = $image_file;	
				if ($rescue_end > $rescue_str) {
					unless (-e $rescue_file){
						print "\nError: $rescue_file file does not exist!\n\n";
						exit;
					}
					$rescue_file_size = -s $rescue_file;
				}
			}
			elsif ($_ eq "data") {
				$data_str = $ff32u_str;
				$data_end = $ff32u_end;
				$data_file = $image_file;	
				if ($data_end > $data_str) {
					unless (-e $data_file){
						print "\nError: $rescue_file file does not exist!\n\n";
						exit;
					}
					$data_file_size = -s $data_file;
				}
			}
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
