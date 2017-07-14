@echo off

IF EXIST "os.h" (
	del "os.h"
)
copy "build-sys\header\os_h_top.in" + "os\os_win.h" + "os\os_win_macros.h" + "build-sys\header\os_h_bot.in" "os.h"