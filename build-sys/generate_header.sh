#!/bin/bash
#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#

# Project variables
PROJECT_VENDOR="Wind River Systems"
PROJECT_VERSION="0.9.0"
PROJECT_VERSION_MAJOR="0"
PROJECT_VERSION_MINOR="9"
PROJECT_VERSION_PATCH="0"
PROJECT_VERSION_TWEAK="0"
PROJECT_START_YEAR="2017"

INPUT_FILE=src/os.h.in
INPUT_SYMBOL=@OS_FUNCTION_DEF@

# Generate copyright
CURRENT_YEAR=`date +"%Y"`
if [ "$PROJECT_START_YEAR" == "$CURRENT_YEAR" ]; then
	COPYRIGHT_YEAR=$PROJECT_START_YEAR
else
	COPYRIGHT_YEAR="$PROJECT_START_YEAR-$CURRENT_YEAR"
fi
PROJECT_COPYRIGHT="Copyright (C) $COPYRIGHT_YEAR $PROJECT_VENDOR, All Rights Reserved."

# Replace symbols in input file
sed 's/"\\n"/"\\\\n"/' src/os_posix.h > os_posix_tmp.h
REPLACE=`unifdef os_posix_tmp.h $*`
awk -v value="$REPLACE" -v symbol="$INPUT_SYMBOL" '$0 ~ symbol{gsub($0,value)}1' $INPUT_FILE | \
	sed -e "s/@PROJECT_COPYRIGHT@/$PROJECT_COPYRIGHT/g" \
		-e "s/@PROJECT_VERSION@/$PROJECT_VERSION/g" \
		-e "s/@PROJECT_VERSION_MAJOR@/$PROJECT_VERSION_MAJOR/g" \
		-e "s/@PROJECT_VERSION_MINOR@/$PROJECT_VERSION_MINOR/g" \
		-e "s/@PROJECT_VERSION_PATCH@/$PROJECT_VERSION_PATCH/g" \
		-e "s/@PROJECT_VERSION_TWEAK@/$PROJECT_VERSION_TWEAK/g"
exit $?
