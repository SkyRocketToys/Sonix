#!/usr/bin/perl -w
use strict;

my $outir = "";
my $phy_flashlayout = "";
my $src_dir = "";
my $physf_command ="./src/phy_sf";

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
	else{
		print $_ , "\n";
		print "the image parameter is wrong!\n";
	}	
}

print "execute make:\n";
system ("make -C ./src/");
print "execute tool:\n";
system("cd ./src/ && ./phy_sf -l $phy_flashlayout -o $outir -s $src_dir");