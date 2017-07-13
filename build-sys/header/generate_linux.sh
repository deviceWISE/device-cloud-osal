#!/bin/sh

rm os.h
cat build-sys/header/os_h_top.in os/os_posix.h build-sys/header/os_h_bot.in > os.h