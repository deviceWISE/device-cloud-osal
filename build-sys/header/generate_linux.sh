#!/bin/bash

rm os.h

osname=$(lsb_release -si) # get os name
osname=$(echo $osname | awk '{print tolower($0)}') # to lower case

if [[ $osname == "android" ]]; then
	echo "*** Generating os.h for Android ***"
	cat build-sys/header/os_h_top.in os/os_android.h os/os_posix.h build-sys/header/os_h_bot.in > os.h

elif [[ $osname == "vxworks" ]]; then
	echo "*** Generating os.h for VxWorks ***"
	cat build-sys/header/os_h_top.in os/os_vxworks.h os/os_posix.h build-sys/header/os_h_bot.in > os.h

else
	echo "*** Generating os.h for generic Linux ***"
	cat build-sys/header/os_h_top.in os/os_posix.h build-sys/header/os_h_bot.in > os.h

fi
