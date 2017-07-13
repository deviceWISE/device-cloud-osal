@echo off

del os.h
copy build-sys/header/os_h_top.in + os/os_win.h + build-sys/header/os_h_bot.in os.h