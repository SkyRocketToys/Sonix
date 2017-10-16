#!/usr/bin/perl -w
use strict;

sub usage
{
	print "usage:	
	hw_setting_convert.pl muti_hwsetting(yes/no) ver(none/e1) ./sn986xx sdk
	hw_setting_convert.pl muti_hwsetting(yes/no) ver(none/e1) ./sn986xx sdk -f filename
	hw_setting_convert.pl muti_hwsetting(yes/no) ver(none/e1) ./sn986xx keil
	hw_setting_convert.pl muti_hwsetting(yes/no) ver(none/e1) ./sn986xx jlink
	hw_setting_convert.pl muti_hwsetting(yes/no) ver(none/e1) ./sn986xx quake -h 372\n";
}

###check parameter###
my $muti_hwsetting = "no";
my $serial_name = undef;
my $which_type = undef;
my $which_flag = "none";
my $which_value = "none";
my $platform = "none";
my $version = "";
my $end_file="";
if ((@ARGV != 4)&&(@ARGV != 8) ){
	print "the param num:".@ARGV."\n";
  &usage;
  exit(1);
}

if (@ARGV == 4){
	($muti_hwsetting,$version,$serial_name,$which_type) = @ARGV;
}else{
	($muti_hwsetting,$version,$serial_name,$which_type,$which_flag,$which_value,$platform,$end_file) = @ARGV;
}
unless (-d $serial_name){
	print "dir $serial_name doesn't exist!\n";
	&usage;
	exit(1);
}
unless (($which_type =~ /^sdk$/i)||($which_type =~ /^quake$/i)||($which_type =~ /^keil$/i)||($which_type =~ /^jlink$/i)){
	print "type $which_type wrong!\n";
	&usage;
	exit(1);
}
if (@ARGV == 6){
	unless (($which_type =~ /^sdk$/i)||($which_type =~ /^quake$/i)){
		print "the second parameter must be sdk/quake!\n";
		&usage;
		exit(1);
	}else{
		if($which_type =~ /^sdk$/i){
			unless($which_flag =~ /^-f$/){
				print "the third parameter must be '-f'!\n";
				&usage;
				exit(1);
			}
		}else{ 
			unless($which_flag =~ /^-h$/){
				print "the third parameter must be '-h'!\n";
				&usage;
				exit(1);				
			}
		}
	}
}else{
	if ($which_type =~ /^quake$/i){
		print "'quake' need parameter '-h'\n";
		&usage;
		exit(1);
	}
}

###suport absolute path###
#$serial_name = "sn986xx";
my $dir_absolute = "";
foreach (`cd $serial_name&&pwd`) {
	$dir_absolute = $_;
}
$dir_absolute =~ s/\s+//g;
$dir_absolute =~ s/\/$//g;
#print "$dir_absolute\n";
#if ($serial_name =~ /(.+)\/(.+)/){
#	$serial_name = $2;
#	$serial_name =~ s/\///g;
#}


#####get sn986xx/sn9356x sn986xx/sn98610 sn986xx/snsn9860x sn986xx/st58600_fpga/... #####
opendir ( DIR, $serial_name) || die "error in opening dir $serial_name\n";
my @folder_names_one;
if ($serial_name =~ /\/$/)
{
	$serial_name =~ s/\/$//;
}
while(my $file = readdir(DIR))
{
  if ($file =~ /^[^\.]/){
  	#with _e1
  	unless ($version =~ /none/){
  		if($file =~ /_$version$/){
  			my $this_file = $serial_name."/".$file;
	  		#print "[$this_file]\n";
				push (@folder_names_one,$this_file);
  		}
  	}
  	#without _e1
  	else{
  		unless($file =~ /_e[0-9]+$/){
	  		my $this_file = $serial_name."/".$file;
	  		#print "[$this_file]\n";
				push (@folder_names_one,$this_file);
  		}
  	}
  }
}
closedir(DIR);



#my $this_dir = "./".$which_type;
#$this_dir = lc($this_dir);
#if (-d $this_dir){
#	system("rm -rf $this_dir");
#}
&process_one_target($dir_absolute);



sub process_one_target
{				
	my $this_dir = $_[0];

  ###sdk -f
	if(($which_type =~ /^sdk$/i)&&($which_flag =~ /-f/)){
			my $which_file = $which_value;
			$which_file =~ s/\/$//;
			#print "[$which_file]\n";
			my $create_dir = "";
			my $near_dir = "";
			my $last_dir = "";
			if ($this_dir =~ /(.+)\/(.+)/){
				$create_dir = $1;
				$near_dir = $2;
				#print $near_dir."\n";
				$create_dir = $create_dir."/".$which_type."/".$near_dir;
			}
			my $target_file = "";
			my @paths = split (/$near_dir/,$which_file);
			$create_dir = $create_dir.$paths[-1];
			$target_file = $create_dir;
			if ($create_dir =~ /(.+)\/(.+)/){
				$create_dir = $1;
			}
		  print "create: ".$create_dir."\n";
		  unless (-d $create_dir){
		  	system("mkdir -p $create_dir");
		  }
		  #print $target_file."\n";
			if ($platform eq "AFTER_986xx"){
				print "##################### AFTER_986xx ################## \n";
				&parse_to_asm_file_after_986xx($which_file,$target_file,$muti_hwsetting);
			}
			else {
				&parse_to_asm_file($which_file,$target_file,$muti_hwsetting);
			}
	}else{
	  my $create_dir = "";
		#####get files in @folder_names_one #####
		foreach(@folder_names_one){
			my $which_file = $_;
			#print "[$which_file]\n";
			my $near_dir = "";
			my $last_dir = "";
			if ($this_dir =~ /(.+)\/(.+)/){
				$create_dir = $1;
				$near_dir = $2;
				#print $near_dir."\n";
				$create_dir = $create_dir."/".$which_type."/".$near_dir;
				#print $create_dir."\n";
			}
			my @paths = split (/$near_dir/,$which_file);
			$create_dir = $create_dir.$paths[-1];
		  print "create: ".$create_dir."\n";
		  unless (-d $create_dir){
		  	system("mkdir -p $create_dir");
		  }
		 
			opendir (DIR, $which_file) || die "error in opening dir $which_file\n";
			###jlink
			if($which_type =~ /^jlink$/i){
				while(my $file = readdir(DIR))
				{
					if ($file =~ /^[^\.]/){
						my $this_file = $which_file."/".$file;
						#print "[$this_file]\n";
						my $target_file = $create_dir."/".$file.".jlink";
						#print "[$target_file]\n";
						&parse_to_jlink_file($this_file,$target_file);					
					}
				}
			}
			###keil
			elsif($which_type =~ /^keil$/i){
				while(my $file = readdir(DIR))
				{
					if ($file =~ /^[^\.]/){
						my $this_file = $which_file."/".$file;
						#print "[$this_file]\n";
						my $target_file = $create_dir."/".$file.".keil";
						#print "[$target_file]\n";
						&parse_to_keil_file($this_file,$target_file);
					}
				}			
			}
			###sdk
			elsif(($which_type =~ /^sdk$/i)&&($which_flag =~ /^none$/)){
				while(my $file = readdir(DIR))
				{
					if ($file =~ /^[^\.]/){
						my $this_file = $which_file."/".$file;
						#print "[$this_file]\n";
						my $target_file = $create_dir."/".$file;
						#print "[$target_file]\n";
						&parse_to_asm_file($this_file,$target_file,$muti_hwsetting);
					}
				}			
			}
			###quake -h
			elsif($which_type =~ /^quake$/i){
				while(my $file = readdir(DIR))
				{
					if ($file =~ /^[^\.]/){
						my $this_file = $which_file."/".$file;
						#print "[$this_file]\n";
						my $this_hz = "_".$which_value."mhz_";
						#print "$this_hz\n";
						unless($this_file =~ /.*$this_hz.*/){
							next;
						}
						my $target_file = $create_dir."/".$file;
						#print "[$target_file]\n";
						&parse_to_quake_file($this_file,$target_file);
					}
				}			
			}	
			closedir(DIR);
		}	
	}
}

sub parse_to_keil_file
{				
	my $source_file = $_[0];
	my $keil_file = $_[1]; 

	my $WM32 = "WM32";
	my $WCP15 = "WCP15";
	my $delay = "DELAY";
	my $addr_1 = "0x98A00000";
	my $addr_2 = "0x98A0000C";
	my $addr_3 = "0x90200000";
	my $addr_4 = "0x90200004";
	my $addr_5 = "0x90200040";
	my $addr_6 = "0x90200080";
	my $addr_7 = "0x90200084";
	
	my $data_begin = "[INIT]";
#	my $source_file;
#	my $jlink_file;
#	my $keil_file;
	
	#if (@ARGV != 3) {
	#	print "the param num:".@ARGV."\n";
	#	die "you should be use as: script.plx src=src_file jlink=jlink_file keil=keil_file\n";
	#}
	
	#foreach (@ARGV) {
	#	if (/^src=.*/) {
	#		s/.*=//g;
	#		$source_file = $_;		
	#	} elsif (/^jlink=.*/) {
	#		s/.*=//g;
	#		$jlink_file = $_;
	#	} elsif (/^keil=.*/) {
	#		s/.*=//g;
	#		$keil_file = $_;
	#	} else {
	#		die "you should be use as: script.plx src=src_file jlink=jlink_file keil=keil_file\n";
	#	}		
	#}
	
	#get src data
	open (oSRC, $source_file) or die "Can't open '$source_file': $! ";
	binmode (oSRC);
	my @src_data = <oSRC>;
	close oSRC;
	
	
#	open oJLINK, ">" . $jlink_file or die "Can't open '$jlink_file': $! ";
	open oKEIL, ">" . $keil_file or die "Can't open '$keil_file': $! ";
	
	print oKEIL "/******************************************************************************/
	/* Dbg_RAM.ini: Initialization Script for Debugging in RAM                    */
	/******************************************************************************/
	/* This file is part of the uVision/ARM development tools.                    */
	/* Copyright (c) 2010 Keil Software. All rights reserved.                     */
	/* This software may only be used under the terms of a valid, current,        */
	/* end user licence from KEIL for a compatible version of KEIL software       */
	/* development tools. Nothing else gives you the right to use this software.  */
	/******************************************************************************/
	
	// Setup memory mapping
	map   0x00000000, 0x00400000 read write exec
	map   0x03FF0000, 0x03FFFFFF read write
	map   0x07F00000, 0x08000000 read write
	map   0x90000000, 0x92000000 read write
	map   0x90000000, 0x92000000 read write
	map   0x98000000, 0x99200000 read write
	
	// Setup Program Counter
	
	";

	
	my $data="";
	my $type="";
	my $addr="";
#	my $value_jlink="";
	my $value_keil="";
	my $note="";
	my $state=0;
	foreach (@src_data) {
		$note = $_;
		$note =~ s/^.*;//;
		$note =~ s/\r//;# delete ^M
		#print "note: ".$note."\n";
		s/;.*//;
		$data = $_;
		#print "data: ".$data."\n";
	
		if (($state == 1) && (/^(\S+)\s+(\S+)\s+(\S+)\s+/i)) {
			$type = $1;
			$addr = $2;
			$value_keil = $3;
#			$value_jlink = $3;
			#print "the value :".$value."\n";
			# $1:type  $2:addr   $3:value 
			
			if (($type eq $WCP15) || ($addr eq $addr_1) || ($addr eq $addr_2)) {
				next; 		
			}
	
#			$value_jlink =~ s/^0x0*/0x/i;
#			$value_jlink =~ tr/[A-Z]/[a-z/;			
#			if ($value_jlink eq "0x") {
#				$value_jlink = "0x0"
#			}
		
			if (($addr eq $addr_3) || ($addr eq $addr_4) || ($addr eq $addr_5) || ($addr eq $addr_6) || ($addr eq $addr_7)||($addr =~/0x9800/)){
				$addr =~ tr/[A-Z]/[a-z]/;
#				print oJLINK "Write32(".$addr.", ".$value_jlink.");\n";
			} else {
				#print "the second 3 value is :".$addr."\n";
				print oKEIL "_WDWORD(".$addr.", ".$value_keil.");\t\t// ".$note;	
				if (($addr eq "0x98200024") || ($addr eq "0x98000004") || (($addr eq "0x90300000") && ($value_keil eq "0x00000401"))) {
					print oKEIL "\n";
				}
				if ($addr eq "0x98000004") {
					print oKEIL "// DDR initial\n";
				}		
				$addr =~ tr/[A-Z]/[a-z]/;
#				print oJLINK "Write32(".$addr.", ".$value_jlink.");\n";
			}
		} 
#		elsif (($state == 1) && (/^(\S+)\s+(\S+)\s+/i)) {
#			$value_jlink = $2;
#			if ($1 eq $delay) {
#				print oJLINK "Delay(".$value_jlink.");\n";
#			}
#		} 
		elsif (/^\[INIT\]/) {
			$state = 1;	
		}
	}
	
	
	print oKEIL "
	PC = 0x100000;
	LOAD %L INCREMENTAL		// load FW";
	
#	print oJLINK "SetJTAGSpeed(1000);";
	
#	close oJLINK;
	close oKEIL;
}


sub parse_to_jlink_file
{				
	my $source_file = $_[0];
	my $jlink_file = $_[1]; 

	my $WM32 = "WM32";
	my $WCP15 = "WCP15";
	my $delay = "DELAY";
	my $addr_1 = "0x98A00000";
	my $addr_2 = "0x98A0000C";
	my $addr_3 = "0x90200000";
	my $addr_4 = "0x90200004";
	my $addr_5 = "0x90200040";
	my $addr_6 = "0x90200080";
	my $addr_7 = "0x90200084";
	
	my $data_begin = "[INIT]";
#	my $source_file;
#	my $jlink_file;
#	my $keil_file;
	
	#if (@ARGV != 3) {
	#	print "the param num:".@ARGV."\n";
	#	die "you should be use as: script.plx src=src_file jlink=jlink_file keil=keil_file\n";
	#}
	
	#foreach (@ARGV) {
	#	if (/^src=.*/) {
	#		s/.*=//g;
	#		$source_file = $_;		
	#	} elsif (/^jlink=.*/) {
	#		s/.*=//g;
	#		$jlink_file = $_;
	#	} elsif (/^keil=.*/) {
	#		s/.*=//g;
	#		$keil_file = $_;
	#	} else {
	#		die "you should be use as: script.plx src=src_file jlink=jlink_file keil=keil_file\n";
	#	}		
	#}
	
	#get src data
	open (oSRC, $source_file) or die "Can't open '$source_file': $! ";
	binmode (oSRC);
	my @src_data = <oSRC>;
	close oSRC;
	
	
	open oJLINK, ">" . $jlink_file or die "Can't open '$jlink_file': $! ";
#	open oKEIL, ">" . $keil_file or die "Can't open '$keil_file': $! ";
	
	
	print oJLINK "/*********************************************************************
	*
	* JLink setup file - Generated by J-Flash ARM V2.16b
	*
	* Syntax:
	*   SetJTAGSpeed(x);           // Sets the JTAG speed, x = speed in kHz (0 = Auto)
	*   Delay(x);                  // Waits a given time,  x = delay in milliseconds
	*   DisableMMU();              // Disables the MMU
	*   Go();                      // Starts the ARM core
	*   Halt();                    // Halts the ARM core
	*   Reset(x);                  // Resets the target,   x = delay in milliseconds
	*   ResetBP0(x);               // Resets the target using breakpoint at address 0
	*   ResetADI(x);               // Resets the target using ADI software reset
	*   Read8(Addr);               // Reads a 8/16/32 bit value,
	*   Read16(Addr);              //   Addr = address to read (as hex value)
	*   Read32(Addr);
	*   Verify8(Addr, Data);       // Verifies a 8/16/32 bit value,
	*   Verify16(Addr, Data);      //   Addr = address to verify (as hex value)
	*   Verify32(Addr, Data);      //   Data = data to verify (as hex value)
	*   Write8(Addr, Data);        // Writes a 8/16/32 bit value,
	*   Write16(Addr, Data);       //   Addr = address to write (as hex value)
	*   Write32(Addr, Data);       //   Data = data to write (as hex value)
	*   WriteVerify8(Addr, Data);  // Writes and verifies a 8/16/32 bit value,
	*   WriteVerify16(Addr, Data); //   Addr = address to write (as hex value)
	*   WriteVerify32(Addr, Data); //   Data = data to write (as hex value)
	*   WriteRegister(Reg, Data);  // Writes a register
	*   WriteJTAG_IR(Cmd);         // Writes the JTAG instruction register
	*   WriteJTAG_DR(nBits, Data); // Writes the JTAG data register
	*
	**********************************************************************
	*/
	
	SetJTAGSpeed(30);
	Reset(20);
	//DisableMMU();
	";
	
	my $data="";
	my $type="";
	my $addr="";
	my $value_jlink="";
#	my $value_keil="";
	my $note="";
	my $state=0;
	foreach (@src_data) {
		$note = $_;
		$note =~ s/^.*;//;
		$note =~ s/\r//;# delete ^M
		#print "note: ".$note."\n";
		s/;.*//;
		$data = $_;
		#print "data: ".$data."\n";
	
		if (($state == 1) && (/^(\S+)\s+(\S+)\s+(\S+)\s+/i)) {
			$type = $1;
			$addr = $2;
#			$value_keil = $3;
			$value_jlink = $3;
			#print "the value :".$value."\n";
			# $1:type  $2:addr   $3:value 
			
			if (($type eq $WCP15) || ($addr eq $addr_1) || ($addr eq $addr_2)) {
				next; 		
			}
	
			$value_jlink =~ s/^0x0*/0x/i;
			$value_jlink =~ tr/[A-Z]/[a-z/;			
			if ($value_jlink eq "0x") {
				$value_jlink = "0x0"
			}
		
			if ((($addr eq $addr_3) || ($addr eq $addr_4) || ($addr eq $addr_5) || ($addr eq $addr_6) || ($addr eq $addr_7))||($addr =~/0x9800/)){
				$addr =~ tr/[A-Z]/[a-z]/;
				print oJLINK "Write32(".$addr.", ".$value_jlink.");\n";
			} else {
				#print "the second 3 value is :".$addr."\n";
#				print oKEIL "_WDWORD(".$addr.", ".$value_keil.");\t\t// ".$note;	
#				if (($addr eq "0x98200024") || ($addr eq "0x98000004") || (($addr eq "0x90300000") && ($value_keil eq "0x00000401"))) {
#					print oKEIL "\n";
#				}
#				if ($addr eq "0x98000004") {
#					print oKEIL "// DDR initial\n";
#				}		
				$addr =~ tr/[A-Z]/[a-z]/;
				print oJLINK "Write32(".$addr.", ".$value_jlink.");\n";
			}
		} elsif (($state == 1) && (/^(\S+)\s+(\S+)\s+/i)) {
			$value_jlink = $2;
			if ($1 eq $delay) {
				print oJLINK "Delay(".$value_jlink.");\n";
			}
		} elsif (/^\[INIT\]/) {
			$state = 1;	
		}
	}
	
	
#	print oKEIL "
#	PC = 0x100000;
#	LOAD %L INCREMENTAL		// load FW";
	
	print oJLINK "SetJTAGSpeed(1000);";
	
	close oJLINK;
#	close oKEIL;
}

#=begin REM
sub parse_to_asm_file
{ 
	
	 my $in = $_[0];
	 my $out = $_[1];
	 my $end_delay = $_[2];

	 my $cmdCount;

#   ($in, $out) = @ARGV;
   die "Missing input file name.\n" unless $in;
#   die "Missing output file name.\n" unless $out;
   $cmdCount = 0;
   

   my $out_file = $out;

   
   #print "$out_file\n";
   
   open(IN, "< $in") or die "can't open $in:$!\n";
   my @script=<IN>;
 
   open(OUT, "> $out_file");
   
   foreach(@script){	
		if (/WM32/){
			my @line = split(' ', $_);
				
			$line[1] =~ s/^\s+//;
			$line[1] =~ s/\s+$//;
			
			$line[2] =~ s/^\s+//;
			$line[2] =~ s/\s+$//;
			
			my $reg = lc($line[1]);
			my $dat = lc($line[2]);
			
			# if (($line[1] =~/0x903/) ||($line[1] =~/0x98A/) ||($line[1] =~/0x98000000/) ||($line[1] =~/0x98000004/)||($line[1] =~/0x98000020/)||($line[1] =~/0x98000024/)){
			# 	print OUT "w $reg,$dat\n";
			# }

			if (($line[1] =~/0x903/) ||($line[1] =~/0x98A/) ||($line[1] =~/0x9800/)){
				print OUT "w $reg,$dat\n";
			}
		}
		elsif (/DELAY/){
		#	print OUT "n 0x00100000\n";
		}
		$cmdCount++;
   }
	if ($end_delay =~ /^no$/) {print OUT "n 0x00001000\n";}
  
   close(IN);
   close(OUT);
#   print "Number of bytes converted = $cmdCount\n";
}
#=end REM
#=cut
sub parse_to_asm_file_after_986xx
{ 
	
	 my $in = $_[0];
	 my $out = $_[1];
	 my $end_delay = $_[2];

	 my $cmdCount;

#   ($in, $out) = @ARGV;
   die "Missing input file name.\n" unless $in;
#   die "Missing output file name.\n" unless $out;
   $cmdCount = 0;
   

   my $out_file = $out;

   
   #print "$out_file\n";
   
   open(IN, "< $in") or die "can't open $in:$!\n";
   my @script=<IN>;
 
   open(OUT, "> $out_file");
	my $out_end_file=$out_file."_END";
	my $write_end_file=0;
	if ($end_file eq "yes") {
		open(OUT_END, "> $out_end_file");
	}

   my $zro_addr_cnt = 0;

   
   foreach(@script){
		if (/WM32/){
			my @line = split(' ', $_);
				
			$line[1] =~ s/^\s+//;
			$line[1] =~ s/\s+$//;
			
			$line[2] =~ s/^\s+//;
			$line[2] =~ s/\s+$//;
			
			$line[3] =~ s/^\s+//;
			$line[3] =~ s/\s+$//;

			if ($end_file eq "yes") {
				if ($line[3] eq ";DDR_CTL_START"){
					$write_end_file=1;
				}
			}
			
			my $reg = lc($line[1]);
			my $dat = lc($line[2]);
			
			if (($line[1] =~/0x903/) ||($line[1] =~/0x98A/) ||($line[1] =~/0x9800/)){
				if ($line[2] =~/0x00000000/) {
						# if ($zro_addr_cnt==0){
						# 	if ($write_end_file > 0) {
						# 		print OUT_END "z $reg,";
						# 	}
						# 	else {
						# 		print OUT "z $reg,";
						# 	}
						# }
						# $zro_addr_cnt++;
					}else{
						# if ($zro_addr_cnt > 0){
						# 	my $hexval = sprintf "%08x", $zro_addr_cnt;
						# 	print OUT "0x$hexval\n";
						# 	$zro_addr_cnt = 0;
						# }
						if ($write_end_file > 0) {
							print OUT_END "w $reg,$dat\n";
						}
						else {
							print OUT "w $reg,$dat\n";
						}
					}
			}
		}
		elsif (/DELAY/){
		#	print OUT "n 0x00100000\n";
		}
		$cmdCount++;
   }

   if ($zro_addr_cnt > 0){
		my $hexval = sprintf "%08x", $zro_addr_cnt;
		print OUT "0x$hexval\n";
		$zro_addr_cnt = 0;
	}


	if ($end_delay =~ /^no$/) {print OUT "n 0x00001000\n";}
  
   close(IN);
   close(OUT);
   close(OUT_END);
#   print "Number of bytes converted = $cmdCount\n";
}


sub parse_to_quake_file
{ 
	
	 my $in = $_[0];
	 my $out = $_[1]; 

	 my $cmdCount;

#   ($in, $out) = @ARGV;
   die "Missing input file name.\n" unless $in;
#   die "Missing output file name.\n" unless $out;
   $cmdCount = 0;
   
   my $out_file = $out;
   
   #print "$out_file\n";
   
   open(IN, "< $in");
   my @script=<IN>;
 
   open(OUT, "> $out_file");
   
   my $start_write = "NO";
   foreach(@script){	
		if (/WM32/){
			my @line = split(' ', $_);
				
			$line[1] =~ s/^\s+//;
			$line[1] =~ s/\s+$//;
			
			$line[2] =~ s/^\s+//;
			$line[2] =~ s/\s+$//;
			
			my $reg = lc($line[1]);
			my $dat = lc($line[2]);
			
			if (($line[1] =~/0x903/) ||($line[1] =~/0x98000000/) ||($line[1] =~/0x98000004/)){
				if ($start_write =~ /^YES$/i){
					print OUT "w $reg,$dat\n";
				}
			}
		}
		elsif (/DELAY/){
			if ($start_write =~ /^YES$/i){
				print OUT "n 0x00001000\n";
			}
			$start_write = "YES";
		}
		if ($start_write =~ /^YES$/i){
			$cmdCount++;
		}
   }
	 print OUT "n 0x00001000\n";
  
   close(IN);
   close(OUT);
#   print "Number of bytes converted = $cmdCount\n";
}
