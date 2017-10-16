#!/usr/bin/perl -w
use strict;

my $gpio_file = "./gpio.txt";
my $target_file = "./snx_gpio_conf.h";
my $flash_type = "";
my $platform = "";
my $name_pin = "";
my $name_pinmux = "";
my $name_mode = "";
my $name_outputvalue = "";
my $name_interrupt = "";
my $name_package = "";

foreach (@ARGV){
	if (/^platform=.*/){
		s/.*=//g;
		$platform = $_;
	}
	elsif (/^flash_type=.*/){
		s/.*=//g;
		$flash_type = $_;
	}
	elsif (/^gpio_conf=.*/){
		s/.*=//g;
		$target_file = $_;
	}	
}

if($flash_type =~ /^nand$/i){
	$flash_type = "NAND_Flash";
}elsif($flash_type =~ /^sf$/i){
	$flash_type = "Serial_Flash";
}


open(oFILE,$gpio_file) || die("Cannot Open $gpio_file: $!");
my @gpio_data = <oFILE>;
close(oFILE);

open oFILE, ">" . $target_file or die "Cannot Open $target_file: $!";
print oFILE "/*
* 1--- \"CONFIG_xxIO_xx_ENABLE\" has options:1/0(ENABLE/DISABLE)               
* 2--- \"CONFIG_xxIO_xx_MODE\" has options:1/0(OUTPUT/INPUT)  
* 3--- \"CONFIG_xxIO_xx_OUTPUTVALUE\" has options:1/0(for mode=0,skip outputvalue)               
* 4--- \"CONFIG_xxIO_xx_Interrupt\" has options:0/1/2/3(DISABLE/EDGE_SINGLE_RISE/EDGE_SINGLE_FALL/EDGE_BOTH)
*/\n";

my $start_record = "no";
my $start_name = "no";
foreach (@gpio_data){
	my $this_line = $_;
	if (($this_line =~ /^\/\*/)||($this_line =~ /^\*/)||($this_line =~ /^[\s]*$/)){
		next;
	}
	
	if($start_record =~ /^yes$/i){
		$this_line =~ s/[\s]*$//;
		my @line_array =	split /[\s]+/,$this_line;
		my $pin = $line_array[0];
		my $pinmux = $line_array[1];
		my $mode = $line_array[2];
		my $outputvalue = $line_array[3];
		my $interrupt = $line_array[4];
		my $pin_package = $line_array[5];
		
		my $value_pinmux = "";
		my $value_mode = "";
		my $value_interrupt = "";
		if($pinmux =~ /^enable$/i){
			$value_pinmux = 1;
		}elsif($pinmux =~ /^disable$/i){
			$value_pinmux = 0;
		}
		###
		if($mode =~ /^output$/i){
			$value_mode = 1;
		}elsif($mode =~ /^input$/i){
			$value_mode = 0;
		}
		###
		if($interrupt =~ /^disable$/i){
			$value_interrupt = 0;
		}elsif($interrupt =~ /^EDGE_SINGLE_RISE$/i){
			$value_interrupt = 1;
		}elsif($interrupt =~ /^EDGE_SINGLE_FALL$/i){
			$value_interrupt = 2;
		}elsif($interrupt =~ /^EDGE_BOTH$/i){
			$value_interrupt = 3;
		}
		
		my @packages = split /,/,$pin_package;
		my $this_line_sn = "no";
		my $this_line_fl = "no";
		my $flash_exist = "no";
		foreach(@packages){
			if($_ =~ /^$platform$/i){
				$this_line_sn = "yes";
			}
			if($_ =~ /^$flash_type$/i){
				$this_line_fl = "yes";
			}
			if($_ =~ /_flash$/i){
				$flash_exist = "yes";
			}
		}
		if($this_line_sn =~ /^yes$/i){
			if($flash_exist =~ /^no$/i){
				print oFILE "#define CONFIG_$pin"."_ENABLE $value_pinmux\n";
				print oFILE "#define CONFIG_$pin"."_$name_mode $value_mode\n";
				if($value_mode == 1){print oFILE "#define CONFIG_$pin"."_$name_outputvalue $outputvalue\n";}
				print oFILE "#define CONFIG_$pin"."_$name_interrupt $value_interrupt\n\n";
			}elsif($flash_exist =~ /^yes$/i){
				if($this_line_fl =~ /^yes$/i){
					print oFILE "#define CONFIG_$pin"."_ENABLE $value_pinmux\n";
					print oFILE "#define CONFIG_$pin"."_$name_mode $value_mode\n";
					if($value_mode == 1){print oFILE "#define CONFIG_$pin"."_$name_outputvalue $outputvalue\n";}
					print oFILE "#define CONFIG_$pin"."_$name_interrupt $value_interrupt\n\n";						
				}else{
					print oFILE "#define CONFIG_$pin"."_ENABLE 0\n\n";
				}
			}
		}else{
				print oFILE "#define CONFIG_$pin"."_ENABLE 0\n\n";
		}		
	}
	
	if ($start_name =~ /^yes$/i){
		$start_name = "no";
		my @line_array =	split /[\s]+/,$this_line;
		$name_pin = $line_array[0];
		$name_pinmux = $line_array[1];
		$name_mode = $line_array[2];
		$name_outputvalue = $line_array[3];
		$name_interrupt = $line_array[4];
		$name_package = $line_array[5];
	}
	
	if($this_line =~ /^===/){
		$start_record = "yes";
	}
	
	if($this_line =~ /^---/){
		$start_name = "yes";
	}
}
close oFILE;