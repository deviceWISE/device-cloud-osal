@echo off

::
:: Bash script that generates the library header, os.h, for Windows-based systems
:: 
:: Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
:: 
:: The right to copy, distribute or otherwise make use of this software
:: may be licensed only pursuant to the terms of an applicable Wind River
:: license agreement.  No license to Wind River intellectual property rights is
:: granted herein.  All rights not licensed by Wind River are reserved by Wind
:: River.
::

IF EXIST "os.h" (
	del "os.h"
)
copy "build-sys\header\os_h_top.in" + "os\os_win.h" + "os\os_win_macros.h" + "build-sys\header\os_h_bot.in" "os.h"