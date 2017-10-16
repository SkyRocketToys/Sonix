#!/bin/sh

src_file=$1
usbdev_mode=$2
usbdev_hpd_src=$3
usbdev_asic_io_num=$4
usbdev_plugin_trig_lev=$5

# usb mode
if [ -n "$usbdev_mode" ]
then
	sed -i "s/^-class_mode,int,4,.*/-class_mode,int,4,$usbdev_mode/g" $src_file
fi

#hotplug mode
if [ -n "$usbdev_hpd_src" ]
then
	sed -i "s/^-hotplug_mode,int,4,.*/-hotplug_mode,int,4,$usbdev_hpd_src/g" $src_file
fi

#hotplug gpio num
if [ -n "$usbdev_asic_io_num" ]
then
	sed -i "s/^-hotplug_gpio_num,int,4,.*/-hotplug_gpio_num,int,4,$usbdev_asic_io_num/g" $src_file
fi

#hotplug gpio trig lev
if [ -n "$usbdev_plugin_trig_lev" ]
then
	sed -i "s/^-hotplug_gpio_trig_lev,int,4,.*/-hotplug_gpio_trig_lev,int,4,$usbdev_plugin_trig_lev/g" $src_file
fi

# dos2unix $src_file
