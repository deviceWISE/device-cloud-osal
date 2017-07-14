#!/bin/bash

# 
# Bash script that generates the library header, os.h, for Linux-based systems
# 
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
# 
# The right to copy, distribute or otherwise make use of this software
# may be licensed only pursuant to the terms of an applicable Wind River
# license agreement.  No license to Wind River intellectual property rights is
# granted herein.  All rights not licensed by Wind River are reserved by Wind
# River.
# 

echo "Removing old os.h (if present)"
rm -f os.h

osname=$(lsb_release -si) # get os name
osname=$(echo $osname | awk '{print tolower($0)}') # to lower case

if [[ $osname == "android" ]]; then
	echo "Generating os.h for Android"
	cat build-sys/header/os_h_top.in os/os_android.h os/os_posix.h os/os_posix_macros.h build-sys/header/os_h_bot.in > os.h

elif [[ $osname == "vxworks" ]]; then
	echo "Generating os.h for VxWorks"
	cat build-sys/header/os_h_top.in os/os_vxworks.h os/os_posix.h os/os_posix_macros.h build-sys/header/os_h_bot.in > os.h

else
	echo "Generating os.h for generic Linux"
	cat build-sys/header/os_h_top.in os/os_posix.h os/os_posix_macros.h build-sys/header/os_h_bot.in > os.h

fi

echo "os.h setup done!"