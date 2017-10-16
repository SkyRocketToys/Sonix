#!/usr/bin/perl -w
use strict;

# static value
my $hw_setting_size=0x1000;
my $flash_layout_size=0x1000;

#dynamic
my $nvram_size=0;
my $bootsel_size=0;
my $factory_size=0;
my $data_section_size=0;
my $storage_section_size=0;
my $rtos_size=0;
my $platform_name="no";
my $flash_total_size=0;
my $image_dir="";
my $imagetool_dir="";
my $target_file = "serial_flashlayout.conf";
my $rescue_size=0;
my $rescue_system = "y";
my $flashloyout_conf;

foreach (@ARGV){
	if (/^nvram_size=.*/){
		s/.*=//g;
		$nvram_size = $_;
		$nvram_size = $nvram_size * 1024;
	}
	elsif (/^platform_name=.*/){
		s/.*=//g;
		$platform_name = $_;
	}
	elsif (/^bootsel_size=.*/){
		s/.*=//g;
		$bootsel_size = $_;
		$bootsel_size = $bootsel_size * 1024;
	}
	elsif (/^factory_size=.*/){
		s/.*=//g;
		$factory_size = $_;
		$factory_size = $factory_size * 1024;
	}
	elsif (/^data_section_size=.*/){
		s/.*=//g;
		$data_section_size = $_;
		$data_section_size = $data_section_size * 1024;
	}
	elsif (/^storage_section_size=.*/){
		s/.*=//g;
		$storage_section_size = $_;
                if ($storage_section_size eq "") {
                        $storage_section_size = 0;
                }
		$storage_section_size = $storage_section_size * 1024;
	}
	elsif (/^rtos_size=.*/){
		s/.*=//g;
		$rtos_size = $_;
		$rtos_size = $rtos_size * 1024;
	}
	elsif (/^rescue_size=.*/){
		s/.*=//g;
		if ($rescue_system eq "y") {
			$rescue_size = $_;
			$rescue_size = $rescue_size * 1024;
			}else {
				$rescue_size = 0;
			}
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
	elsif (/^imagetool_dir=.*/){
		s/.*=//g;
		$imagetool_dir = $_;
	}
	elsif (/^config_file=.*/){
		s/.*=//g;
		$target_file = $_;
	}
	elsif(/^rescue_system=.*/){
		s/.*=//g;
		$rescue_system= $_;
	}
	elsif(/^flashloyout_conf=.*/){
		s/.*=//g;
		$flashloyout_conf= $_;
	}


	

}
my $total_part_size=0;
$total_part_size = $hw_setting_size + $bootsel_size + $flash_layout_size + $nvram_size + $rtos_size + $rescue_size + $factory_size + $data_section_size + $storage_section_size;

if ($total_part_size > $flash_total_size) {die "\n ERROR: please reduce partition size, used size($total_part_size), flash size:($flash_total_size)\n";}
elsif ($total_part_size < $flash_total_size) {print "\n WARRNING: some flash space is free, used size:($total_part_size), flash size:($flash_total_size) \n";}
else {print "\n total_part_size match flash_total_size = $flash_total_size\n";}


my $end_addr=0;
my $start_addr=0;
open oFILE, ">" . $target_file or die "Can't open '$target_file': $!";
open oHEADER, ">" . $flashloyout_conf or die "Can't open '$flashloyout_conf': $!";
#header
print oFILE "#flash-layout\n";
print oFILE "#Name\t\tStart address\t\tEnd address\t\tlocation\n";


#hw-setting
$end_addr=$hw_setting_size - 1;
my $start_addr_str = sprintf("0x%x", $start_addr);
my $end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "hw-setting\t$start_addr_str\t\t\t$end_addr_str\t\t\t$imagetool_dir/hw_setting.image.d\n";
print oHEADER "#define hw_setting_st\t$start_addr_str\n";
print oHEADER "#define hw_setting_nd\t$end_addr_str\n";


#boot sel
$start_addr+=$hw_setting_size;
$end_addr+=$bootsel_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "bootsel\t\t$start_addr_str\t\t\t$end_addr_str\t\t\t$image_dir/BOOTSL.bin\n";
print oHEADER "#define bootsel_st\t$start_addr_str\n";
print oHEADER "#define bootsel_nd\t$end_addr_str\n";


#flash layout
$start_addr+=$bootsel_size;
$end_addr+=$flash_layout_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "flash-layout	$start_addr_str\t\t\t$end_addr_str\t\t\t$imagetool_dir/flash_layout.bin.d\n";
print oHEADER "#define flash_layout_st\t$start_addr_str\n";
print oHEADER "#define flash_layout_nd\t$end_addr_str\n";


#rtos
$start_addr+=$flash_layout_size;
$end_addr+=$rtos_size;
$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "rtos\t\t$start_addr_str\t\t\t$end_addr_str\t\t$image_dir/KERNEL.bin\n";
print oHEADER "#define rtos_st\t$start_addr_str\n";
print oHEADER "#define rtos_nd\t$end_addr_str\n";


#factory
$start_addr = ($flash_total_size - $rescue_size - $storage_section_size - $data_section_size - $nvram_size - $factory_size);
$end_addr = ($flash_total_size - $rescue_size - $storage_section_size - $data_section_size - $nvram_size - 1);

$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "factory\t\t$start_addr_str\t\t\t$end_addr_str\t\t$image_dir/factory.bin\n";
print oHEADER "#define factory_st\t$start_addr_str\n";
print oHEADER "#define factory_nd\t$end_addr_str\n";


#nvram
$start_addr = ($flash_total_size - $rescue_size - $storage_section_size - $data_section_size - $nvram_size);
$end_addr = ($flash_total_size - $rescue_size - $storage_section_size - $data_section_size - 1);

if ($nvram_size > 0) {$start_addr_str = sprintf("0x%x", $start_addr);}
else {$start_addr_str = sprintf("0x%x", $end_addr);}
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "nvram\t\t$start_addr_str\t\t\t$end_addr_str\t\t\t$image_dir/nvram.bin\n";
print oHEADER "#define nvram_st\t$start_addr_str\n";
print oHEADER "#define nvram_nd\t$end_addr_str\n";


#data
$start_addr = ($flash_total_size - $rescue_size - $storage_section_size - $data_section_size);
$end_addr = ($flash_total_size - $rescue_size - $storage_section_size - 1);

$start_addr_str = sprintf("0x%x", $start_addr);
$end_addr_str = sprintf("0x%x", $end_addr);
print oFILE "data\t\t$start_addr_str\t\t\t$end_addr_str\t\t$image_dir/data.bin\n";
print oHEADER "#define data_st\t$start_addr_str\n";
print oHEADER "#define data_nd\t$end_addr_str\n";


#storage
if ($storage_section_size != 0) {
	$start_addr = ($flash_total_size - $rescue_size - $storage_section_size);
	$end_addr = ($flash_total_size - $rescue_size - 1);

	$start_addr_str = sprintf("0x%x", $start_addr);
	$end_addr_str = sprintf("0x%x", $end_addr);
	print oFILE "storage\t\t$start_addr_str\t\t$end_addr_str\t\tnull\n";
	print oHEADER "#define storage_st\t$start_addr_str\n";
	print oHEADER "#define storage_nd\t$end_addr_str\n";
}


#rescue
if ($rescue_system eq "y") {
	$start_addr = ($flash_total_size - $rescue_size);
	$end_addr = ($flash_total_size - 1);

	$start_addr_str = sprintf("0x%x", $start_addr);
	$end_addr_str = sprintf("0x%x", $end_addr);
	print oFILE "rescue\t\t$start_addr_str\t\t\t$end_addr_str\t\t$image_dir/RESCUE.bin\n";
	print oHEADER "#define rescue_st\t$start_addr_str\n";
	print oHEADER "#define rescue_nd\t$end_addr_str\n";
}


#end
print oFILE "add\t\t0x00000000\t\t0x00000000\t\tnull\n";
close oFILE;
close oHEADER;
